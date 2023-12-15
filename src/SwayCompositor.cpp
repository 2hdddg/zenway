#include "src/SwayCompositor.h"

#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <set>

using namespace nlohmann;

const std::set<std::string> ignore = {"rect",
                                      "window_rect",
                                      "deco_rect",
                                      "geometry",
                                      "border",
                                      "layout",
                                      "current_border_width",
                                      "orientation",
                                      "percent",
                                      "sticky",
                                      "window_properties",
                                      "idle_inhibitors",
                                      "representation",
                                      "transform",
                                      "scale",
                                      "serial",
                                      "modes",
                                      "current_mode"};

const auto filter = [](int depth, json::parse_event_t event, json &parsed) {
    if (event != json::parse_event_t::key) return true;
    auto key = parsed.get<std::string>();
    return ignore.find(key) == ignore.end();
};

static void ParseEvent(const std::string &payload) {
    auto rootNode = json::parse(payload, filter, false /*ignore exceptions*/);
    auto change = rootNode["change"].get<std::string>();
    spdlog::trace("Change is: {}", change);
}

static void ParseBarStateUpdateEvent(const std::string &payload, bool &visibleByModifier) {
    auto rootNode = json::parse(payload, filter, false /*ignore exceptions*/);
    // std::string barId = rootNode["id"];
    visibleByModifier = rootNode["visible_by_modifier"].get<bool>();
}

static void ParseApplication(Workspace &workspace, nlohmann::basic_json<> applicationNode,
                             int nextFocusId) {
    if (applicationNode["type"] != "con" && applicationNode != "floating_con") {
        spdlog::error("Expected app");
        return;
    }
    auto applicationname = applicationNode["name"];
    // When name is null and there is an orientation different
    // from none there are more nodes below! Maybe name is not the correct
    // check...
    if (applicationname.is_null()) {
        auto applicationNodes = applicationNode["nodes"];
        for (auto &innerApplicationNode : applicationNodes) {
            ParseApplication(workspace, innerApplicationNode, nextFocusId);
        }
        return;
    }

    auto application = Application{.name = applicationNode["name"].get<std::string>()};
    int applicationId = applicationNode["id"].get<int>();
    // Can be null
    auto appId = applicationNode["app_id"];
    if (!appId.is_null()) {
        application.appId = appId.get<std::string>();
    }
    // In sway the only focused app is the one in the focused workspace. When that app is focused
    // the workspace is not. To simplify usage this considers the focused app in a non focused
    // workspace to be the next in line.
    application.isFocused = applicationNode["focused"].get<bool>();
    if (application.isFocused) {
        workspace.isFocused = true;
    } else {
        application.isFocused = applicationId == nextFocusId;
    }
    workspace.applications.push_back(std::move(application));
}

static void ParseTree(const std::string &payload, Manager &manager) {
    auto rootNode = json::parse(payload, filter, false /*ignore exceptions*/);
    if (rootNode["type"] != "root") {
        spdlog::error("Expected root");
        return;
    }
    Displays displays;
    auto outputNodes = rootNode["nodes"];
    for (auto outputNode : outputNodes) {
        if (outputNode["type"] != "output") {
            // spdlog::error("Expected output but was {}", outputNode["type"]);
            continue;
        }
        auto display =
            Display{.name = outputNode["name"].get<std::string>()};  // workspaces->AddDisplay();
        auto workspaceNodes = outputNode["nodes"];
        for (auto workspaceNode : workspaceNodes) {
            if (workspaceNode["type"] != "workspace") {
                spdlog::error("Expected workspace");
                continue;
            }
            auto workspace = Workspace{.name = workspaceNode["name"].get<std::string>()};
            //   Better way?
            auto focusNode = workspaceNode["focus"];
            int nextFocusId = -1;
            for (auto n : focusNode) {
                nextFocusId = n.get<int>();
                break;
            }
            workspace.isFocused = workspaceNode["focused"].get<bool>();
            auto applicationNodes = workspaceNode["nodes"];
            for (auto applicationNode : applicationNodes) {
                ParseApplication(workspace, applicationNode, nextFocusId);
            }
            display.isFocused = display.isFocused || workspace.isFocused;
            display.workspaces.push_back(std::move(workspace));
        }
        displays.push_back(std::move(display));
    }
    manager.Publish(displays);
}
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

std::shared_ptr<SwayCompositor> SwayCompositor::Connect(MainLoop &mainLoop,
                                                        std::shared_ptr<Manager> manager) {
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
    auto t = std::shared_ptr<SwayCompositor>(new SwayCompositor(fd, manager));
    mainLoop.Register(fd, "Sway", t);
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

bool SwayCompositor::OnRead() {
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
            ParseTree(m_payload, *m_manager);
            break;
        case Message::SUBSCRIBE:
            spdlog::debug("Sway subscriptions confirmed");
            break;
        case Message::EVENT_WORKSPACE:
            spdlog::trace("Sway workspace event");
            ParseEvent(m_payload);
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_WINDOW:
            spdlog::trace("Sway window event");
            ParseEvent(m_payload);
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_BAR_STATE_UPDATE:
            bool visible;
            ParseBarStateUpdateEvent(m_payload, visible);
            spdlog::debug("Sway bar state event, visible: {}", visible);
            if (visible) {
                m_manager->Show();
            } else {
                m_manager->Hide();
            }
            break;
        case Message::EVENT_SHUTDOWN:
            spdlog::trace("Sway shutdown event");
            ParseEvent(m_payload);
            break;
        default:
            spdlog::error("Received unhandled sway message: {}", (uint32_t)msg);
            break;
    }
    return true;
}
