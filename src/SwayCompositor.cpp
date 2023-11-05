#include "src/SwayCompositor.h"

#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <set>

#include "SwayJson.h"

static constexpr auto MAGIC = "i3-ipc";
static constexpr auto MAGIC_LENGTH = 6;
static constexpr auto HEADER_SIZE = MAGIC_LENGTH + 4 + 4;

enum class Message : uint32_t {
    RUN_COMMAND = 0,
    GET_WORKSPACES,
    SUBSCRIBE,
    GET_OUTPUTS,
    GET_TREE,

    EVENT_WORKSPACE = 0x80000000,
    EVENT_WINDOW = 0x80000003,
    EVENT_SHUTDOWN = 0x80000006,
    EVENT_BAR_STATE_UPDATE = 0x80000014,
    EVENT_INPUT = 0x80000015,

};

std::shared_ptr<SwayCompositor> SwayCompositor::Connect(
    MainLoop &mainLoop, std::shared_ptr<Manager> manager,
    std::shared_ptr<ScriptContext> scriptContext) {
    auto path = getenv("SWAYSOCK");
    spdlog::debug("Connecting to sway at {}", path);
    auto fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return nullptr;
    }
    struct sockaddr_un remote {
        .sun_family = AF_UNIX
    };
    strcpy(remote.sun_path, path);
    auto data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(fd, (struct sockaddr *)&remote, data_len) == -1) {
        close(fd);
        return nullptr;
    }
    auto t = std::shared_ptr<SwayCompositor>(new SwayCompositor(fd, manager, scriptContext));
    mainLoop.Register(fd, "sway", t);
    // mainLoop.RegisterBatchHandler(t);
    t->Initialize();
    return t;
}

SwayCompositor::~SwayCompositor() { close(m_fd); }

static void Send(int fd, Message msg, const char *payload) {
    write(fd, MAGIC, MAGIC_LENGTH);
    uint32_t len = strlen(payload);
    write(fd, &len, 4);
    write(fd, &msg, 4);
    write(fd, payload, len);
    fsync(fd);
}

void SwayCompositor::Initialize() {
    // Get initial tree
    Send(m_fd, Message::GET_TREE, "");
    OnRead();
    // Initialize sway subscription
    auto payload = "['workspace','shutdown','window', 'bar_state_update']";
    Send(m_fd, Message::SUBSCRIBE, payload);
    OnRead();
}

void SwayCompositor::OnRead() {
    char hdr[HEADER_SIZE];
    read(m_fd, hdr, HEADER_SIZE);
    if (memcmp(hdr, MAGIC, MAGIC_LENGTH) != 0) {
        spdlog::error("Bad read");
    }
    uint32_t len = *((uint32_t *)(hdr + MAGIC_LENGTH));
    m_payload = "";
    m_payload.resize(len + 1);
    /* TODO: Fix this
    if (len > m_payload.capacity()) {
        m_payload.clear();
        m_payload.reserve(len + 1);
        m_payload.resize(len + 1);
    }
    */
    auto msg = Message(*((uint32_t *)(hdr + MAGIC_LENGTH + 4)));

    read(m_fd, m_payload.data(), len);
    //   Dispatch
    switch (msg) {
        case Message::GET_TREE:
            spdlog::trace("Received sway tree");
            SwayJson::ParseTree(m_payload, m_scriptContext);
            m_manager->DirtyWorkspace();
            break;
        case Message::SUBSCRIBE:
            spdlog::debug("Subscription confirmed");
            break;
        case Message::EVENT_WORKSPACE:
            spdlog::trace("Workspace event");
            SwayJson::ParseEvent(m_payload);
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_WINDOW:
            spdlog::trace("Window event");
            SwayJson::ParseEvent(m_payload);
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_BAR_STATE_UPDATE:
            bool visible;
            SwayJson::ParseBarStateUpdateEvent(m_payload, visible);
            spdlog::debug("Bar update, visible: {}", visible);
            if (visible) {
                m_manager->Show();
            } else {
                m_manager->Hide();
            }
            break;
        case Message::EVENT_SHUTDOWN:
            spdlog::trace("Shutdown event");
            SwayJson::ParseEvent(m_payload);
            break;
        default:
            spdlog::error("Received unhandled sway message: {}", (uint32_t)msg);
            break;
    }
}

/*
void SwayCompositor::OnBatchProcessed() {
    if (m_isVisible && m_panel->IsDirty()) {
        spdlog::info("Redraw statuses");
        m_outputs->ForEach([this](std::shared_ptr<Output> output) { m_panel->Draw(*output); });
    }
}
*/
