#pragma once

#include <memory>
#include <string>

struct WorkspaceTheme {
    std::string Markup;
    int OffsetX;
    int SpacingY;
};

class Theme {
   public:
    static std::unique_ptr<Theme> Create();
    WorkspaceTheme ForWorkspace(const bool focused, const std::string& workspaceName,
                                const std::string applicationName);
};
