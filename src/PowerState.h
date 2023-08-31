#pragma once

#include <stdint.h>

#include "Source.h"
#include "src/ScriptContext.h"

struct PowerState {
    bool IsCharging;
    uint8_t Capacity;

    bool operator==(const PowerState& other) {
        return IsCharging == other.IsCharging && Capacity == other.Capacity;
    }
    void Publish(ScriptContext& scriptContext) {
        auto table = scriptContext.Source("power");
        table["isCharging"] = IsCharging;
        table["capacity"] = (int)Capacity;
    }
};
