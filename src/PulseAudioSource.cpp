#include "PulseAudioSource.h"

#include <pulse/def.h>
#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include <spdlog/spdlog.h>

#include "src/ScriptContext.h"

static void on_state(pa_context* ctx, void* data) {
    static_cast<PulseAudioSource*>(data)->OnStateChange();
}

static void on_server(pa_context* ctx, const pa_server_info* info, void* data) {
    static_cast<PulseAudioSource*>(data)->OnServerChange(info);
}

static void on_sink(pa_context* c, const pa_sink_info* info, int eol, void* data) {
    if (!info) return;
    static_cast<PulseAudioSource*>(data)->OnSinkChange(info);
}

static void on_subscribe(pa_context* ctx, pa_subscription_event_type_t event_and_facility,
                         uint32_t idx, void* data) {
    auto event = PA_SUBSCRIPTION_EVENT_TYPE_MASK & event_and_facility;
    if (event != PA_SUBSCRIPTION_EVENT_CHANGE) {
        // Handle add/remove?
        return;
    }
    auto facility = PA_SUBSCRIPTION_EVENT_FACILITY_MASK & event_and_facility;
    switch (facility) {
        case PA_SUBSCRIPTION_EVENT_SERVER:
            pa_context_get_server_info(ctx, on_server, data);
            break;
        case PA_SUBSCRIPTION_EVENT_SINK:
            pa_context_get_sink_info_by_index(ctx, idx, on_sink, data);
            break;
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            pa_context_get_sink_info_list(ctx, on_sink, data);
            break;
    }
}

std::unique_ptr<PulseAudioSource> PulseAudioSource::Create(
    std::string_view name, std::shared_ptr<MainLoop> zenMainloop,
    std::shared_ptr<ScriptContext> scriptContext) {
    auto mainloop = pa_threaded_mainloop_new();
    if (!mainloop) return nullptr;
    pa_threaded_mainloop_lock(mainloop);
    auto api = pa_threaded_mainloop_get_api(mainloop);
    if (!api) {
        pa_threaded_mainloop_free(mainloop);
        return nullptr;
    }
    auto context = pa_context_new(api, "zen");
    if (!context) {
        pa_threaded_mainloop_free(mainloop);
        api->quit(api, 0);
        return nullptr;
    }
    auto backend = std::unique_ptr<PulseAudioSource>(
        new PulseAudioSource(name, zenMainloop, mainloop, scriptContext, api, context));
    // From now on the backend will free on error

    if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        return nullptr;
    }
    pa_context_set_state_callback(context, on_state, backend.get());
    if (pa_threaded_mainloop_start(mainloop) < 0) {
        return nullptr;
    }
    pa_threaded_mainloop_unlock(mainloop);
    return backend;
}

PulseAudioSource::~PulseAudioSource() {
    pa_threaded_mainloop_stop(m_mainLoop);
    pa_context_disconnect(m_ctx);
    m_api->quit(m_api, 0);
    pa_threaded_mainloop_free(m_mainLoop);
}

void PulseAudioSource::OnStateChange() {
    switch (pa_context_get_state(m_ctx)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY: {
            // Connected
            // Request basic server information
            pa_context_get_server_info(m_ctx, on_server, this);
            // Subscribe on changes
            pa_context_set_subscribe_callback(m_ctx, on_subscribe, this);
            const auto events = (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK |
                                                         PA_SUBSCRIPTION_MASK_SINK_INPUT |
                                                         PA_SUBSCRIPTION_MASK_SERVER);
            pa_context_subscribe(m_ctx, events, nullptr, nullptr);
            break;
        }
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            // Not connected
            break;
    }
}

void PulseAudioSource::OnServerChange(const pa_server_info* info) {
    spdlog::info("PulseAudio server, default sink:{}, default source: {}", info->default_sink_name,
                 info->default_source_name);
    // Make sure we get a default value
    pa_context_get_sink_info_by_name(m_ctx, info->default_sink_name, on_sink, this);
}

void PulseAudioSource::OnSinkChange(const pa_sink_info* info) {
    // Calculate average volume over all channels
    auto volume = float(pa_cvolume_avg(&info->volume));
    constexpr auto NORM = PA_VOLUME_NORM;
    AudioState newState = {.Muted = info->mute == 1,
                           .Volume = std::round((volume / NORM) * 100.0F),
                           .PortType = "unknown"};
    spdlog::info("PulseAudio sink: {0} ({1}), mute:{2}, volume:{3}", info->name, info->description,
                 newState.Muted, newState.Volume);
    if (info->active_port) {
        spdlog::info("PulseAudio active port: {0} ({1})", info->active_port->name,
                     info->active_port->description);
        switch (info->active_port->type) {
            case PA_DEVICE_PORT_TYPE_SPEAKER:
                newState.PortType = "speaker";
                break;
            case PA_DEVICE_PORT_TYPE_HEADPHONES:
            case PA_DEVICE_PORT_TYPE_HEADSET:
            case PA_DEVICE_PORT_TYPE_LINE:
            case PA_DEVICE_PORT_TYPE_USB:
                newState.PortType = "headphones";
                break;
            case PA_DEVICE_PORT_TYPE_HDMI:
            case PA_DEVICE_PORT_TYPE_TV:
                newState.PortType = "tv";
                break;
            default:
                spdlog::info("PulseAudio unknown port type: {}", info->active_port->type);
                break;
        }
    }
    if (newState == m_sourceState) return;
    spdlog::debug("Audio source is dirty");
    m_sourceDirtyFlag = true;
    m_sourceState = newState;
    m_scriptContext->Publish(m_name, m_sourceState);
    m_zenMainloop->Wakeup();
}
