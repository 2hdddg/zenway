#include "zen/Configuration.h"

static bool FromChar(const char c, uint8_t& n) {
    if (c >= '0' && c <= '9') {
        n = c - '0';
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        n = c - 'a' + 10;
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        n = c - 'a' + 10;
        return true;
    }
    return false;
}

static bool FromChars(const char c1, const char c2, double& c) {
    uint8_t n1 = 0, n2 = 0;
    if (!FromChar(c1, n1) || !FromChar(c2, n2)) {
        return false;
    }
    c = ((n1 << 4) | n2) / 255.0;
    return true;
}

RGBA RGBA::FromString(const std::string& s) {
    // Should be on format #rrggbb or #rrggbbaa
    if (s.length() != 7 && s.length() != 9) {
        return RGBA{};
    }
    if (s[0] != '#') {
        return RGBA{};
    }
    RGBA c{};
    bool ok =
        FromChars(s[1], s[2], c.r) && FromChars(s[3], s[4], c.g) && FromChars(s[5], s[6], c.b);
    if (!ok) {
        return RGBA{};
    }
    if (s.length() == 7) {
        c.a = 1;
        return c;
    }
    if (!FromChars(s[7], s[8], c.a)) {
        return RGBA{};
    }
    return c;
}
