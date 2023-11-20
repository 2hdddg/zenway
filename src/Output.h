#pragma once

#include <wayland-client-protocol.h>

#include <functional>
#include <map>
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

    virtual ~Output() {
        wl_output_destroy(m_wloutput);
        m_wloutput = nullptr;
    }

    std::string name;
    std::array<std::unique_ptr<ShellSurface>, 2> surfaces;

   private:
    Output(wl_output* wloutput, OnNamedCallback onNamed)
        : m_wloutput(wloutput), m_onNamed(onNamed) {}

    wl_output* m_wloutput;
    // Temporary callback until named, registers amoung the other outputs when name received
    OnNamedCallback m_onNamed;
};

class Outputs {
   public:
    static std::unique_ptr<Outputs> Create(const std::shared_ptr<Roots> roots);

    void Add(wl_output* output);

    void ForEach(std::function<void(std::shared_ptr<Output>)> callback);
    std::shared_ptr<Output> Get(const std::string& name) const;

   private:
    Outputs(const std::shared_ptr<Roots> roots) : m_roots(roots) {}
    const std::shared_ptr<Roots> m_roots;
    std::map<std::string, std::shared_ptr<Output>> m_map;
};
