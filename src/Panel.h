#pragma once

#include "Configuration.h"
#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/Buffer.h"
#include "src/Output.h"
#include "src/Sources.h"

struct DrawnWidget {
    Rect position;
};

struct DrawnPanel {
    // nullptr if not drawn
    std::shared_ptr<Buffer> buffer;
    Size size;
    std::vector<DrawnWidget> widgets;
};

struct Panel {
    static DrawnPanel Draw(const Configuration::Panel& panelConfig, const std::string& outputName,
                           BufferPool& bufferPool);
};
