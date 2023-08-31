#pragma once

#include "Theme.h"
#include "pango/pango-layout.h"

class WidgetBase {
   public:
    WidgetBase() : m_isDirty(true) {}
    virtual ~WidgetBase(){};

    virtual void Draw(PangoLayout& layout, const Theme& theme) = 0;
    virtual bool IsDirty() { return m_isDirty; }

   protected:
    bool m_isDirty;

    // virtual void Pause();
    // virtual void Resume();
};
