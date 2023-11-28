#pragma once

#include "Configuration.h"
#include "cairo.h"
#include "pango/pango-layout.h"
#include "src/Buffer.h"
#include "src/Sources.h"

struct Size {
    int cx;
    int cy;
};

struct Rect {
    int x;
    int y;
    int cx;
    int cy;

    bool Contains(int x_, int y_) const {
        return x_ >= x && x_ <= x + cx && y_ >= y && y_ <= y + cy;
    }
};

struct DrawnWidget {
    Rect position;
};

struct DrawnPanel {
    std::shared_ptr<Buffer> buffer;
    Size size;
    std::vector<DrawnWidget> widgets;
};

struct Panel {
    static bool Draw(const Configuration::Panel& panelConfig, const std::string& outputName,
                     BufferPool& bufferPool, DrawnPanel& drawn);
};
