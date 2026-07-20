#pragma once
#ifndef CATA_SRC_GAUNTHUD_HUD_H
#define CATA_SRC_GAUNTHUD_HUD_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "cursesdef.h"
#include "gaunthud_cells.h"
#include "translation.h"

class avatar;
struct point;

namespace catacurses
{
class window;
} // namespace catacurses

// Orchestrator-internal types. Included by `gaunthud.cpp` and by
// every per-readout shim that builds a `frame`. Not part of the
// public surface in `gaunthud.h`.

namespace gaunthud
{

enum class corner {
    tl,
    tr,
    bl,
    br,
    bc,
    tc,
};

// Returned by a readout's `prepare`: the size the readout wants and
// a callable that paints into a window of that size. The size is
// needed before `catacurses::newwin`; the closure captures whatever
// intermediate the readout produced so it doesn't have to be
// re-rendered.
struct frame {
    cells::extent size;
    std::function<void( const catacurses::window & )> paint;
    std::optional<catacurses::window_backdrop> backdrop_override = std::nullopt;
    int margin_left = 0;
    int margin_right = 0;
};

struct readout {
    std::string id;
    translation name;
    std::function<frame( const avatar & )> prepare;
};

// Order within `readouts` is the stack order in `where`.
struct corner_stack {
    corner where;
    std::vector<readout> readouts;
};

// Translates a corner + readout size + accumulated corner offset
// into the top-left point at which to create the readout's curses
// window. `offset` is the accumulated vertical extent of readouts
// already placed in the same corner.
point compute_origin( corner where, cells::extent size, int offset,
                      int margin_left = 0, int margin_right = 0 );

// Whether the readout with the given id is currently enabled, i.e.
// its proxy panel's `toggle` field is on. Readouts without a matching
// proxy are always enabled.
bool readout_enabled( const std::string &id );

} // namespace gaunthud

#endif // CATA_SRC_GAUNTHUD_HUD_H
