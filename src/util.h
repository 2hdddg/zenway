#pragma once

#include <string>

// Utility functions exposed to Lua
struct Util {
    static std::string HtmlEscape(const std::string& s);
};
