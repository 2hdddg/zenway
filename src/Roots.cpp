#include "Roots.h"

void Roots::FlushAndDispatchCommands() const { wl_display_roundtrip(display); }
