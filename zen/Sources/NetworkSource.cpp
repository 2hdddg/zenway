#include "zen/Sources/NetworkSource.h"

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
std::shared_ptr<NetworkSource> NetworkSource::Create(std::shared_ptr<MainLoop> mainloop) {
    auto sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return nullptr;
    }
    auto fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (fd == -1) {
        spdlog::error("Failed to create timer: {}", strerror(errno));
        return nullptr;
    }
    itimerspec timer = {.it_interval = {.tv_sec = 30, .tv_nsec = 0},
                        .it_value = {.tv_sec = 1, .tv_nsec = 0}};
    auto ret = timerfd_settime(fd, 0, &timer, nullptr);
    if (ret == -1) {
        spdlog::error("Failed to set timer: {}", strerror(errno));
        return nullptr;
    }
    auto source = std::shared_ptr<NetworkSource>(new NetworkSource(mainloop, sock, fd));
    mainloop->RegisterIoHandler(fd, "NetworkSource", source);
    return source;
}

void NetworkSource::Initialize() {
    ReadState();
    m_drawn = m_published = false;
}

void NetworkSource::ReadState() {
    char buf[8192] = {0};
    struct ifconf ifc = {0, {0}};
    char ip[INET6_ADDRSTRLEN] = {0};
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(m_socket, SIOCGIFCONF, &ifc) < 0) {
        return;
    }
    auto ifr = ifc.ifc_req;
    auto nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    bool alerted = false;
    bool changed = false;
    // Note that only interfaces that are up is iterated here
    // so to detect interfaces that were up but now is down, clear
    // all interfaces first
    for (auto &keyValue : m_networks) {
        auto &network = keyValue.second;
        network.isUp = false;
    }
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
        NetworkState network = {.isAlerted = false, .isUp = false, .address = ""};
        network.isUp = (item->ifr_flags & IFF_UP) != 0;
        // Get interface address
        if (ioctl(m_socket, SIOCGIFADDR, item) < 0) {
            continue;
        }
        network.address = get_ip_str(addr, ip, sizeof(ip));
        if (m_networks.contains(item->ifr_name)) {
            // Existing network. Might have changed from down -> up
            // or changed address.
            auto &existing = m_networks[item->ifr_name];
            if (existing != network) {
                changed = true;
                m_networks[item->ifr_name] = std::move(network);
            } else {
                existing.isUp = true;
                existing.isAlerted = false;
            }
        } else {
            // New network
            m_networks[item->ifr_name] = std::move(network);
            changed = true;
            // Is this an alert? A positive one perhaps.
        }
    }
    // Check if there is any networks that has gone down
    for (auto &keyValue : m_networks) {
        auto &network = keyValue.second;
        if (!network.isUp && !network.isAlerted) {
            network.isAlerted = true;
            alerted = true;
            changed = true;
        }
    }
    m_drawn = m_published = !changed;
    if (alerted) {
        spdlog::info("Network source is triggering alert");
        m_mainloop->AlertAndWakeup();
    }
}

bool NetworkSource::OnRead() {
    spdlog::info("Check network");
    uint64_t ignore;
    auto n = read(m_timerfd, &ignore, sizeof(ignore));
    if (n <= 0) {
        // Either block or no events
        return false;
    }
    ReadState();
    return !m_published;
}

void NetworkSource::Publish(const std::string_view sourceName, ScriptContext &scriptContext) {
    if (m_published) return;
    scriptContext.Publish(sourceName, m_networks);
    m_published = true;
}
