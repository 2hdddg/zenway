#pragma once

#include <memory>
#include <set>

#include "sol/sol.hpp"
#include "src/ScriptContext.h"
#include "src/ShellSurface.h"

class Configuration {
   public:
    struct Widget {
        static void Parse(const sol::table& table, std::vector<Widget>& widgets);
        sol::function render;
        std::set<std::string> sources;
    };

    struct Panel {
        static Panel Parse(const sol::table panels, const char* name, int index);
        std::vector<Widget> widgets;
        int index;
        Anchor anchor;
        int screenBorderOffset;
    };

    static std::shared_ptr<Configuration> Read(ScriptContext& ScriptContext);

    Panel leftPanel;
    Panel rightPanel;
};
