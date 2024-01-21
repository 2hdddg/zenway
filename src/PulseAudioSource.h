#pragma once

#include <memory>

#include "MainLoop.h"
#include "Sources.h"
#include "src/ScriptContext.h"

struct pa_context;
struct pa_threaded_mainloop;
struct pa_mainloop_api;
struct pa_server_info;
struct pa_sink_info;

class PulseAudioSource : public Source {
   public:
    static std::unique_ptr<PulseAudioSource> Create(std::shared_ptr<MainLoop> zenMainloop);
    virtual ~PulseAudioSource();

    void OnStateChange();
    void OnServerChange(const pa_server_info*);
    void OnSinkChange(const pa_sink_info*);
    void Publish(const std::string_view sourceName, ScriptContext& scriptContext) override;

   private:
    PulseAudioSource(std::shared_ptr<MainLoop> mainloop, pa_threaded_mainloop* mainLoop,
                     pa_mainloop_api* api, pa_context* ctx)
        : m_zenMainloop(mainloop), m_mainLoop(mainLoop), m_api(api), m_ctx(ctx) {}
    std::shared_ptr<MainLoop> m_zenMainloop;
    pa_threaded_mainloop* m_mainLoop;
    pa_mainloop_api* m_api;
    pa_context* m_ctx;
    AudioState m_sourceState;
    std::mutex m_mutex;
};
