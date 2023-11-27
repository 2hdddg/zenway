#pragma once

#include <memory>
#include <set>

#include "sol/sol.hpp"
#include "src/ScriptContext.h"

int GetIntProperty(const sol::table& t, const char* name, int missing);

struct Padding {
    static Padding FromProperty(const sol::table& t, const char* name);
    static Padding FromTable(const sol::table& t);
    int left;
    int right;
    int top;
    int bottom;
};

enum class Anchor { Left, Right, Top, Bottom };

class Configuration {
   public:
    struct Widget {
        static void Parse(const sol::table& table, std::vector<Widget>& widgets);
        sol::function render;
        std::set<std::string> sources;
        Padding padding;
    };

    struct Panel {
        static Panel Parse(const sol::table panelTable, int index);
        std::vector<Widget> widgets;
        int index;
        Anchor anchor;
        bool isColumn;
        sol::optional<sol::function> check_display;

        bool CheckOutput(const std::string outputName) const;
    };

    static std::shared_ptr<Configuration> Read(ScriptContext& ScriptContext, const char* file);

    std::vector<Panel> panels;
    int bufferWidth;
    int bufferHeight;
    int numBuffers;
};
