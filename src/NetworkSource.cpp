#include "NetworkSource.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

#include "spdlog/spdlog.h"

// Inspired by github.com/ajrisi/lsif
//
static char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
    switch (sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}
std::shared_ptr<NetworkSource> NetworkSource::Create(MainLoop &mainLoop,
                                                     std::shared_ptr<ScriptContext> scriptContext) {
    auto sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return nullptr;
    }
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    itimerspec timer = {.it_interval = {.tv_sec = 30}, .it_value = {.tv_sec = 1}};
    auto ret = timerfd_settime(fd, 0, &timer, nullptr);
    if (ret == -1) {
        spdlog::error("Failed to set timer: {}", strerror(errno));
        return nullptr;
    }
    auto source = std::shared_ptr<NetworkSource>(new NetworkSource(sock, fd, scriptContext));
    mainLoop.Register(fd, "NetworkSource", source);
    return source;
}

void NetworkSource::Initialize() { ReadState(); }

void NetworkSource::ReadState() {
    char buf[8192] = {0};
    struct ifconf ifc = {0};
    char ip[INET6_ADDRSTRLEN] = {0};
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(m_socket, SIOCGIFCONF, &ifc) < 0) {
        return;
    }
    auto ifr = ifc.ifc_req;
    auto nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    Networks networks;
    for (unsigned long i = 0; i < nInterfaces; i++) {
        auto item = &ifr[i];
        auto addr = &item->ifr_addr;
        // Get interface flags
        if (ioctl(m_socket, SIOCGIFFLAGS, item) < 0) {
            continue;
        }
        //  Ignore loopback
        if (item->ifr_flags & IFF_LOOPBACK) {
            continue;
        }
        // extended flags SIOCGIFPFLAGS
        // SIOCGIWSTATS
        NetworkState network = {};
        network.isUp = (item->ifr_flags & IFF_UP) != 0;
        // Get interface address
        if (ioctl(m_socket, SIOCGIFADDR, item) < 0) {
            continue;
        }
        network.interface = item->ifr_name;
        network.address = get_ip_str(addr, ip, sizeof(ip));
        networks.push_back(std::move(network));
    }
    m_sourceDirtyFlag = true;  // m_networks != networks;
    if (m_sourceDirtyFlag) {
        m_networks = networks;
        m_scriptContext->Publish(m_networks);
    }
}

void NetworkSource::OnRead() {
    spdlog::info("Check power");
    uint64_t ignore;
    read(m_timerfd, &ignore, sizeof(ignore));
    ReadState();
}
