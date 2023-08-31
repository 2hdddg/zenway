#include "SwayJson.h"

#define CATCH_CONFIG_MAIN 1
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parse simplified tree", "[parse]") {
    std::string json = R"J(
    {
      "type": "root",
      "nodes": [
        {
          "type": "output",
          "name": "eDP-1",
          "nodes": [
            {
              "type": "workspace",
              "name": "1",
              "nodes": [
                {
                  "type": "con",
                  "name": "vim .",
                  "app_id": "Alactritty",
                  "focused": true
                }
              ]
            },
            {
              "type": "workspace",
              "name": "2",
              "nodes": [
                {
                  "type": "con",
                  "id": 456,
                  "name": "Whatever",
                  "app_id": "Firefox",
                  "focused": false
                },
                {
                  "type": "con",
                  "id": 456,
                  "name": "Whatever",
                  "app_id": "Chromium",
                  "focused": false
                }
              ]
            }
          ]
        }
      ]
    }
    )J";
    auto outputs = SwayJson::GetTree(json);
    // Can get output by id
    auto output = outputs.Get("eDP-1");
    REQUIRE(output);
    CHECK(output->workspaces.size() == 2);
    // Querying for unknown output should return null
    auto unknownOutput = outputs.Get("bad");
    CHECK(unknownOutput == nullptr);
    // Can get workspace by id
    auto workspace = output->Get("1");
    REQUIRE(workspace);
    CHECK(workspace->focused);
    CHECK(workspace->name == "1");
    // Check the application in the workspace
    REQUIRE(workspace->applications.size() == 1);
    auto application = *workspace->applications.begin();
    CHECK(application.focused);
    CHECK(application.name == "vim .");
    CHECK(application.app_id == "Alactritty");
    // Check the other workspace
    workspace = output->Get("2");
    REQUIRE(workspace);
    CHECK(!workspace->focused);
    CHECK(workspace->name == "2");
    // Check the application in the other workspace
    REQUIRE(workspace->applications.size() == 2);
    // Requesting non existent workspace should return null
    workspace = output->Get("bad");
    CHECK(!workspace);
}

/*
TEST_CASE("Parse tree", "[parse]") {
    std::string json = R"J(
  {
        "id": 1,
        "name": "root",
        "rect": {
                "x": 0,
                "y": 0,
                "width": 1920,
                "height": 1080
        },
        "focused": false,
        "focus": [
                3
        ],
        "border": "none",
        "current_border_width": 0,
        "layout": "splith",
        "orientation": "horizontal",
        "percent": null,
        "window_rect": {
                "x": 0,
                "y": 0,
                "width": 0,
                "height": 0
        },
        "deco_rect": {
                "x": 0,
                "y": 0,
                "width": 0,
                "height": 0
        },
        "geometry": {
                "x": 0,
                "y": 0,
                "width": 0,
                "height": 0
        },
        "window": null,
        "urgent": false,
        "floating_nodes": [
        ],
        "sticky": false,
        "type": "root",
        "nodes": [
                {
                        "id": 2147483647,
                        "name": "__i3",
                        "rect": {
                                "x": 0,
                                "y": 0,
                                "width": 1920,
                                "height": 1080
                        },
                        "focused": false,
                        "focus": [
                                2147483646
                        ],
                        "border": "none",
                        "current_border_width": 0,
                        "layout": "output",
                        "orientation": "horizontal",
                        "percent": null,
                        "window_rect": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "deco_rect": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "geometry": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "window": null,
                        "urgent": false,
                        "floating_nodes": [
                        ],
                        "sticky": false,
                        "type": "output",
                        "nodes": [
                                {
                                        "id": 2147483646,
                                        "name": "__i3_scratch",
                                        "rect": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 1920,
                                                "height": 1080
                                        },
                                        "focused": false,
                                        "focus": [
                                        ],
                                        "border": "none",
                                        "current_border_width": 0,
                                        "layout": "splith",
                                        "orientation": "horizontal",
                                        "percent": null,
                                        "window_rect": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "deco_rect": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "geometry": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "window": null,
                                        "urgent": false,
                                        "floating_nodes": [
                                        ],
                                        "sticky": false,
                                        "type": "workspace"
                                }
                        ]
                },
                {
                        "id": 3,
                        "name": "eDP-1",
                        "rect": {
                                "x": 0,
                                "y": 0,
                                "width": 1920,
                                "height": 1080
                        },
                        "focused": false,
                        "focus": [
                                4
                        ],
                        "border": "none",
                        "current_border_width": 0,
                        "layout": "output",
                        "orientation": "none",
                        "percent": 1.0,
                        "window_rect": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "deco_rect": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "geometry": {
                                "x": 0,
                                "y": 0,
                                "width": 0,
                                "height": 0
                        },
                        "window": null,
                        "urgent": false,
                        "floating_nodes": [
                        ],
                        "sticky": false,
                        "type": "output",
                        "active": true,
                        "primary": false,
                        "make": "Unknown",
                        "model": "0x38ED",
                        "serial": "0x00000000",
                        "scale": 1.0,
                        "transform": "normal",
                        "current_workspace": "1",
                        "modes": [
                                {
                                        "width": 1920,
                                        "height": 1080,
                                        "refresh": 60052
                                }
                        ],
                        "current_mode": {
                                "width": 1920,
                                "height": 1080,
                                "refresh": 60052
                        },
                        "nodes": [
                                {
                                        "id": 4,
                                        "name": "1",
                                        "rect": {
                                                "x": 0,
                                                "y": 23,
                                                "width": 1920,
                                                "height": 1057
                                        },
                                        "focused": false,
                                        "focus": [
                                                6,
                                                5
                                        ],
                                        "border": "none",
                                        "current_border_width": 0,
                                        "layout": "splith",
                                        "orientation": "horizontal",
                                        "percent": null,
                                        "window_rect": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "deco_rect": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "geometry": {
                                                "x": 0,
                                                "y": 0,
                                                "width": 0,
                                                "height": 0
                                        },
                                        "window": null,
                                        "urgent": false,
                                        "floating_nodes": [
                                        ],
                                        "sticky": false,
                                        "num": 1,
                                        "output": "eDP-1",
                                        "type": "workspace",
                                        "representation": "H[URxvt termite]",
                                        "nodes": [
                                                {
                                                        "id": 5,
                                                        "name": "urxvt",
                                                        "rect": {
                                                                "x": 0,
                                                                "y": 23,
                                                                "width": 960,
                                                                "height": 1057
                                                        },
                                                        "focused": false,
                                                        "focus": [
                                                        ],
                                                        "border": "normal",
                                                        "current_border_width":
2, "layout": "none", "orientation": "none", "percent": 0.5, "window_rect": {
                                                                "x": 2,
                                                                "y": 0,
                                                                "width": 956,
                                                                "height": 1030
                                                        },
                                                        "deco_rect": {
                                                                "x": 0,
                                                                "y": 0,
                                                                "width": 960,
                                                                "height": 25
                                                        },
                                                        "geometry": {
                                                                "x": 0,
                                                                "y": 0,
                                                                "width": 1124,
                                                                "height": 422
                                                        },
                                                        "window": 4194313,
                                                        "urgent": false,
                                                        "floating_nodes": [
                                                        ],
                                                        "sticky": false,
                                                        "type": "con",
                                                        "fullscreen_mode": 0,
                                                        "pid": 23959,
                                                        "app_id": null,
                                                        "visible": true,
                                                        "shell": "xwayland",
                                                        "inhibit_idle": true,
                                                        "idle_inhibitors": {
                                                                "application":
"none", "user": "visible"
                                                        },
                                                        "window_properties": {
                                                                "class":
"URxvt", "instance": "urxvt", "title": "urxvt", "transient_for": null
                                                        },
                                                        "nodes": [
                                                        ]
                                                },
                                                {
                                                        "id": 6,
                                                        "name": "",
                                                        "rect": {
                                                                "x": 960,
                                                                "y": 23,
                                                                "width": 960,
                                                                "height": 1057
                                                        },
                                                        "focused": true,
                                                        "focus": [
                                                        ],
                                                        "border": "normal",
                                                        "current_border_width":
2, "layout": "none", "orientation": "none", "percent": 0.5, "window_rect": {
                                                                "x": 2,
                                                                "y": 0,
                                                                "width": 956,
                                                                "height": 1030
                                                        },
                                                        "deco_rect": {
                                                                "x": 0,
                                                                "y": 0,
                                                                "width": 960,
                                                                "height": 25
                                                        },
                                                        "geometry": {
                                                                "x": 0,
                                                                "y": 0,
                                                                "width": 817,
                                                                "height": 458
                                                        },
                                                        "window": null,
                                                        "urgent": false,
                                                        "floating_nodes": [
                                                        ],
                                                        "sticky": false,
                                                        "type": "con",
                                                        "fullscreen_mode": 0,
                                                        "pid": 25370,
                                                        "app_id": "termite",
                                                        "visible": true,
                                                        "shell": "xdg_shell",
                                                        "inhibit_idle": false,
                                                        "idle_inhibitors": {
                                                                "application":
"none", "user": "fullscreen"
                                                        },
                                                        "nodes": [
                                                        ]
                                                }
                                        ]
                                }
                        ]
                }
        ]
}
  )J";
    Outputs outputs;
    auto tree = SwayJson::GetTree(json);
    REQUIRE(1 == 0);
}
*/

int main(int argc, char *argv[]) { return Catch::Session().run(argc, argv); }
