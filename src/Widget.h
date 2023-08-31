#pragma once

#include "BufferPool.h"
#include "Output.h"
#include "WidgetBase.h"

class Widget {
   public:
    void Draw(Output& output, BufferPool& pool);
};
