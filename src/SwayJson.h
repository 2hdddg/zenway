#include <memory>

#include "Outputs.h"
#include "Workspaces.h"
#include "src/ScriptContext.h"

struct SwayJson {
    static void ParseTree(const std::string& payload, std::shared_ptr<ScriptContext> scriptContext);
    static void ParseEvent(const std::string& payload);
    static void ParseBarStateUpdateEvent(const std::string& payload, bool& visibleByModifier);
};
