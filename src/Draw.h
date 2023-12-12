#pragma once

#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/Buffer.h"
#include "src/Configuration.h"

// This file and corresponding .cpp handles drawing of all configurable panels and
// their widgets.

struct DrawnWidget {
    Rect position;
};

struct DrawnPanel {
    DrawnPanel() : buffer(nullptr), size{} {}
    std::shared_ptr<Buffer> buffer;
    Size size;
    std::vector<DrawnWidget> widgets;
};

struct Draw {
    static bool Panel(const PanelConfig& panelConfig, const std::string& outputName,
                      BufferPool& bufferPool, DrawnPanel& drawn);
};
