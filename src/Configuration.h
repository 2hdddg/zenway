#pragma once

#include <memory>
#include <set>

#include "sol/sol.hpp"
#include "src/ScriptContext.h"
#include "src/ShellSurface.h"

int GetIntProperty(const sol::table& t, const char* name, int missing);

struct Padding {
    static Padding FromProperty(const sol::table& t, const char* name);
    static Padding FromTable(const sol::table& t);
    int left;
    int right;
    int top;
    int bottom;
};

class Configuration {
   public:
    struct Widget {
        static void Parse(const sol::table& table, std::vector<Widget>& widgets);
        sol::function render;
        std::set<std::string> sources;
        Padding padding;
    };

    struct Panel {
        static Panel Parse(const sol::table panels, const char* name, int index);
        std::vector<Widget> widgets;
        int index;
        Anchor anchor;
    };

    static std::shared_ptr<Configuration> Read(ScriptContext& ScriptContext, const char* file);

    Panel leftPanel;
    Panel rightPanel;
    int bufferWidth;
    int bufferHeight;
    int numBuffers;
};
