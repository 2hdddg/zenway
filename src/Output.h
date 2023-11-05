#pragma once

#include <wayland-client-protocol.h>

#include <functional>
#include <memory>
#include <string>

#include "Buffer.h"
#include "Roots.h"
#include "ShellSurface.h"

class Output;
using OnNamedCallback = std::function<void(Output* output)>;

class Output {
   public:
    Output(const Output&) = delete;
    static void Create(const std::shared_ptr<Roots> roots, wl_output* wloutput,
                       OnNamedCallback onNamed);
    void OnName(const char* name);
    void OnDescription(const char* description);

    virtual ~Output() { wl_output_destroy(m_wloutput); }

    std::string name;
    std::array<std::unique_ptr<ShellSurface>, 2> surfaces;

   private:
    Output(wl_output* wloutput, OnNamedCallback onNamed)
        : m_wloutput(wloutput), m_onNamed(onNamed) {}

    wl_output* m_wloutput;
    // Temporary callback until named, registers amoung the other outputs when name received
    OnNamedCallback m_onNamed;
};
