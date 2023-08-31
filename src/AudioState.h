#pragma once

#include <stdint.h>

#include "Source.h"
#include "src/ScriptContext.h"

struct AudioState {
    bool Muted;
    float Volume;

    bool operator==(const AudioState& other) {
        return Muted == other.Muted && Volume == other.Volume;
    }
    void Publish(ScriptContext& scriptContext) {
        auto table = scriptContext.Source("audio");
        table["muted"] = Muted;
        table["volume"] = Volume;
    }
};
