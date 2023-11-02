#include "SwayJson.h"

#include <spdlog/spdlog.h>

#include <iostream>
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
    std::string key = parsed;
    return ignore.find(key) == ignore.end();
};

void SwayJson::ParseEvent(const std::string &payload) {
    auto rootNode = json::parse(payload, filter, false /*ignore exceptions*/);
    std::string change = rootNode["change"];
    spdlog::trace("Change is: {}", change);
}

void SwayJson::ParseBarStateUpdateEvent(const std::string &payload, bool &visibleByModifier) {
    auto rootNode = json::parse(payload, filter, false /*ignore exceptions*/);
    // std::string barId = rootNode["id"];
    visibleByModifier = rootNode["visible_by_modifier"];
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

    auto application = Application{.name = applicationNode["name"]};
    int applicationId = applicationNode["id"];
    application.appId = applicationNode["app_id"];
    application.isFocused = applicationNode["focused"];
    application.isNextFocused = applicationId == nextFocusId;
    if (application.isFocused) {
        workspace.isFocused = true;
    }
    workspace.applications.push_back(std::move(application));
}

void SwayJson::ParseTree(const std::string &payload, std::shared_ptr<ScriptContext> scriptContext) {
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
        auto display = Display{.name = outputNode["name"]};  // workspaces->AddDisplay();
        /*
        auto output = outputs->Get(outputNode["name"]);
        if (!output) {
            // spdlog::info("Output not found: {}", outputNode["type"]);
            continue;
        }
        output->workspaces.clear();
        */
        auto workspaceNodes = outputNode["nodes"];
        for (auto workspaceNode : workspaceNodes) {
            if (workspaceNode["type"] != "workspace") {
                spdlog::error("Expected workspace");
                continue;
            }
            auto workspace = Workspace{.name = workspaceNode["name"]};
            //   Better way?
            auto focusNode = workspaceNode["focus"];
            int nextFocusId = -1;
            for (auto i : focusNode) {
                nextFocusId = i;
                break;
            }
            auto applicationNodes = workspaceNode["nodes"];
            for (auto applicationNode : applicationNodes) {
                ParseApplication(workspace, applicationNode, nextFocusId);
            }
            display.workspaces.push_back(std::move(workspace));
        }
        displays.push_back(std::move(display));
    }
    scriptContext->Publish(displays);
}
