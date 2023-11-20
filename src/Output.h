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

    void Draw(int panelId, Anchor anchor, Buffer& buffer, const Size& size);
    void Hide();

    std::string name;

   private:
    Output(wl_output* wloutput, OnNamedCallback onNamed)
        : m_wloutput(wloutput), m_onNamed(onNamed) {}

    std::array<std::unique_ptr<ShellSurface>, 2> m_surfaces;
    wl_output* m_wloutput;
    // Temporary callback until named, registers amoung the other outputs when name received
    OnNamedCallback m_onNamed;
};

class Outputs {
   public:
    static std::unique_ptr<Outputs> Create();

    void Add(const std::shared_ptr<Roots> roots, wl_output* output);

    void ForEach(std::function<void(std::shared_ptr<Output>)> callback);
    std::shared_ptr<Output> Get(const std::string& name) const;

   private:
    Outputs() {}
    std::map<std::string, std::shared_ptr<Output>> m_map;
};
