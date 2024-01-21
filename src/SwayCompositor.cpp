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

static bool Filter(int /*depth*/, json::parse_event_t event, json &parsed) {
    static const std::set<std::string> ignore = {"rect",
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
    if (event != json::parse_event_t::key) {
        return true;
    }
    if (!parsed.is_string()) {
        return true;
    }
    auto key = parsed.get<std::string>();
    return ignore.find(key) == ignore.end();
};

static bool IsNodeType(basic_json<> node, const char *expectedType) {
    auto type = node["type"];
    if (type.is_null()) return false;
    if (!type.is_string()) return false;
    return type.get<std::string>() == expectedType;
}

static bool IsArray(basic_json<> node) { return !node.is_null() && node.is_array(); }

static std::string GetString(basic_json<> node, const char *attr) {
    auto v = node[attr];
    if (v.is_null()) return "";
    // TODO: Warn
    if (!v.is_string()) return "";
    return v.get<std::string>();
}

static std::string GetName(basic_json<> node) { return GetString(node, "name"); }

static bool GetFocused(basic_json<> node) { return node["focused"].get<bool>(); }

static void ParseBarStateUpdateEvent(const std::string &payload, bool &visibleByModifier) {
    auto rootNode = json::parse(payload, Filter, false /*ignore exceptions*/);
    if (rootNode.is_discarded()) {
        spdlog::error("Failed to parse Sway status update event");
        return;
    }
    auto visible = rootNode["visible_by_modifier"];
    if (visible.is_null() || !visible.is_boolean()) {
        spdlog::error("Failed to get visible modifier from Sway status update event");
        return;
    }
    visibleByModifier = visible.get<bool>();
}

static void ParseApplication(Workspace &workspace, nlohmann::basic_json<> applicationNode,
                             int nextFocusId) {
    if (!IsNodeType(applicationNode, "con") && !IsNodeType(applicationNode, "floating_con")) {
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
    auto application = Application{.name = GetName(applicationNode),
                                   .appId = GetString(applicationNode, "app_id"),
                                   .isFocused = false};
    int applicationId = applicationNode["id"].get<int>();
    // In sway the only focused app is the one in the focused workspace. When that app is focused
    // the workspace is not. To simplify usage this considers the focused app in a non focused
    // workspace to be the next in line.
    application.isFocused = GetFocused(applicationNode);
    if (application.isFocused) {
        workspace.isFocused = true;
    } else {
        application.isFocused = applicationId == nextFocusId;
    }
    workspace.applications.push_back(std::move(application));
}

static std::optional<Displays> ParseTree(const std::string &payload) {
    auto rootNode = json::parse(payload, Filter, false /*ignoring exceptions*/);
    if (rootNode.is_discarded()) {
        spdlog::error("Failed to parse Sway tree");
        return {};
    }
    if (!IsNodeType(rootNode, "root")) {
        spdlog::error("Sway tree root node should be of value root");
        return {};
    }
    auto outputNodes = rootNode["nodes"];
    if (!IsArray(outputNodes)) {
        spdlog::error("Sway tree has invalid nodes");
        return {};
    }
    // Iterate over displays/outputs
    Displays displays;
    for (auto outputNode : outputNodes) {
        if (!outputNode.is_object()) {
            spdlog::error("Expected Sway node to be object");
            continue;
        }
        // Ensure that node is output
        if (!IsNodeType(outputNode, "output")) {
            continue;
        }
        // Retrieve name of output/display
        auto display = Display(GetName(outputNode));
        // Iterate over workspaces
        auto workspaceNodes = outputNode["nodes"];
        if (!IsArray(workspaceNodes)) {
            continue;
        }
        for (auto workspaceNode : workspaceNodes) {
            if (!IsNodeType(workspaceNode, "workspace")) {
                spdlog::error("Expected workspace");
                continue;
            }
            auto workspace = Workspace(GetName(workspaceNode));
            //   Better way?
            auto focusNode = workspaceNode["focus"];
            int nextFocusId = -1;
            for (auto n : focusNode) {
                nextFocusId = n.get<int>();
                break;
            }
            workspace.isFocused = GetFocused(workspaceNode);
            auto applicationNodes = workspaceNode["nodes"];
            if (!IsArray(applicationNodes)) {
                continue;
            }
            for (auto applicationNode : applicationNodes) {
                ParseApplication(workspace, applicationNode, nextFocusId);
            }
            display.isFocused = display.isFocused || workspace.isFocused;
            display.workspaces.push_back(std::move(workspace));
        }
        displays.push_back(std::move(display));
    }
    return displays;
}

std::shared_ptr<SwayCompositor> SwayCompositor::Connect(MainLoop &mainLoop, Visibility visibility) {
    auto path = getenv("SWAYSOCK");
    if (path == nullptr) {
      spdlog::error("SWAYSOCK not set");
      return nullptr;
    }
    spdlog::debug("Connecting to sway at {}", path);
    auto fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return nullptr;
    }
    struct sockaddr_un remote {
        .sun_family = AF_UNIX, .sun_path = "",
    };
    strcpy(remote.sun_path, path);
    auto data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(fd, (struct sockaddr *)&remote, data_len) == -1) {
        close(fd);
        return nullptr;
    }
    auto t = std::shared_ptr<SwayCompositor>(new SwayCompositor(fd, visibility));
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
        return false;
    }
    uint32_t len = *((uint32_t *)(hdr + MAGIC_LENGTH));
    m_payload = "";
    m_payload.resize(len + 1);
    auto msg = Message(*((uint32_t *)(hdr + MAGIC_LENGTH + 4)));
    read(m_fd, m_payload.data(), len);
    switch (msg) {
        case Message::GET_TREE: {
            spdlog::trace("Received sway tree");
            auto maybeDisplays = ParseTree(m_payload);
            if (maybeDisplays) {
                m_displays = std::move(*maybeDisplays);
                m_drawn = m_published = false;
            }  // else, error!
            break;
        }
        case Message::SUBSCRIBE:
            spdlog::debug("Sway subscriptions confirmed");
            break;
        case Message::EVENT_WORKSPACE:
            spdlog::trace("Sway workspace event");
            // TODO: Parse workspace change instead of requesting tree
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_WINDOW:
            spdlog::trace("Sway window event");
            // TODO: Parse window change instead of requesting tree
            Send(m_fd, Message::GET_TREE, "");
            break;
        case Message::EVENT_BAR_STATE_UPDATE:
            bool visible;
            ParseBarStateUpdateEvent(m_payload, visible);
            spdlog::debug("Sway bar state event, visible: {}", visible);
            m_visibility(visible);
            m_drawn = m_published = false;
            break;
        case Message::EVENT_SHUTDOWN:
            spdlog::trace("Sway shutdown event");
            break;
        default:
            spdlog::error("Received unhandled sway message: {}", (uint32_t)msg);
            break;
    }
    // True if state changed
    return !m_published;
}

void SwayCompositor::Publish(const std::string_view sourceName, ScriptContext &scriptContext) {
    scriptContext.Publish(sourceName, m_displays);
    m_published = true;
}
