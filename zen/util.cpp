#include "zen/util.h"

std::string Util::HtmlEscape(const std::string& s) {
    std::string e;
    e.reserve(s.length());
    for (auto c : s) {
        switch (c) {
            case '&':
                e.append("&amp;");
                break;
            case '\"':
                e.append("&quot;");
                break;
            case '\'':
                e.append("&apos;");
                break;
            case '<':
                e.append("&lt;");
                break;
            case '>':
                e.append("&gt;");
                break;
            default:
                e += c;
                break;
        }
    }
    return e;
}
