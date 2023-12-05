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
struct pa_source_info;

class PulseAudioSource : public Source {
   public:
    static std::unique_ptr<PulseAudioSource> Create(std::shared_ptr<MainLoop> zenMainloop,
                                                    std::shared_ptr<ScriptContext> scriptContext);
    virtual ~PulseAudioSource();

    void OnStateChange();
    void OnServerChange(const pa_server_info*);
    void OnSinkChange(const pa_sink_info*);

   private:
    PulseAudioSource(std::shared_ptr<MainLoop> zenMainloop, pa_threaded_mainloop* mainLoop,
                     std::shared_ptr<ScriptContext> scriptContext, pa_mainloop_api* api,
                     pa_context* ctx)
        : m_zenMainloop(zenMainloop),
          m_mainLoop(mainLoop),
          m_scriptContext(scriptContext),
          m_api(api),
          m_ctx(ctx) {}
    std::shared_ptr<MainLoop> m_zenMainloop;
    pa_threaded_mainloop* m_mainLoop;
    std::shared_ptr<ScriptContext> m_scriptContext;
    pa_mainloop_api* m_api;
    pa_context* m_ctx;
    AudioState m_sourceState;
};
