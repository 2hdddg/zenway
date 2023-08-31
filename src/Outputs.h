#pragma once

#include <wayland-client-protocol.h>

#include <iterator>
#include <map>
#include <memory>

#include "Output.h"
#include "Roots.h"

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
