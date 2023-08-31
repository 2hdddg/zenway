#include "Theme.h"

static void replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) return;
    str.replace(start_pos, from.length(), to);
}
std::unique_ptr<Theme> Theme::Create() { return std::make_unique<Theme>(); }

WorkspaceTheme Theme::ForWorkspace(const bool focused, const std::string& workspaceName,
                                   const std::string applicationName) {
    const char* MARKUP_UNFOCUSED =
        "<span weight='heavy' size='30pt' color='#918175' "
        "></span>"
        "<span weight='heavy' size='30pt' color='#1c1b19' "
        "background='#918175'>$ws</span>"
        "<span weight='heavy' size='30pt' color='#918175' "
        "></span>"
        "<span size='30pt' color='#918175' "
        "rise='1pt'></span>"
        "<span size='24pt' color='#1c1b19' background='#918175' rise='4pt'>"
        "$app</span>"
        "<span size='30pt' color='#918175' "
        "rise='1pt'></span>";
    const char* MARKUP_FOCUSED =
        "<span weight='heavy' size='30pt' color='#519f50' "
        "></span>"
        "<span weight='heavy' size='30pt' color='#1c1b19' "
        "background='#519f50'>$ws</span>"
        "<span weight='heavy' size='30pt' color='#519f50' "
        "></span>"
        "<span size='30pt' color='#519f50' "
        "rise='1pt'></span>"
        "<span size='24pt' color='#1c1b19' background='#519f50' "
        "rise='4pt'>$app</span>"
        "<span size='30pt' color='#519f50' "
        "rise='1pt'></span>";
    std::string markup = focused ? MARKUP_FOCUSED : MARKUP_UNFOCUSED;
    replace(markup, "$ws", workspaceName);
    replace(markup, "$app", applicationName);
    return WorkspaceTheme{.Markup = markup, .OffsetX = 25, .SpacingY = 10};
}
