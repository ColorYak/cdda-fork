#pragma once
#ifndef CATA_SRC_GAUNTHUD_CELLS_H
#define CATA_SRC_GAUNTHUD_CELLS_H

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "color.h"
#include "cursesdef.h"
#include "output.h"
#include "point.h"

// Cell content and the one curses output call shared by every text
// readout. Storybook-linkable: no engine globals, no avatar.
//
// Pipeline split: layout strategies in `gaunthud::layout` consume
// `cell<N>` and produce a `cell_buffer` of positioned segments; this
// header owns the types they pass around, plus the `paint` routine
// that turns the buffer into glyphs.

namespace gaunthud::cells
{

struct extent {
    int width = 0;
    int height = 0;
};

// A pre-coloured run. `(text, color)` matches the shape returned by
// `display::*_color` helpers, so values from those can flow into a
// segment unchanged. A default-constructed `segment{}` is treated as
// empty by the layout strategies.
struct segment {
    std::string text;
    nc_color color = c_white;
    bool peripheral = false;
};

// An ordered list of `N` segments rendered left-to-right within a
// cell. Cells whose segment count varies at runtime always return
// their maximum-N variant with unused slots default-constructed.
template<std::size_t N>
struct cell {
    std::array<segment, N> segments{};
};

// `cell{ segment{...}, segment{...} }` becomes `cell<2>`
// without spelling N.
template<typename... S>
cell( S... ) -> cell<sizeof...( S )>;

// One coloured run placed at an absolute window-local position.
// Produced by the layout strategies, consumed by `paint`.
struct positioned_segment {
    point pos;
    nc_color color = c_white;
    std::string text;
};

using cell_buffer = std::vector<positioned_segment>;

// One type for both layout strategies and per-readout `render`
// functions, so strategy output flows into a render result
// without repacking.
struct render_result {
    extent size;
    cell_buffer buffer;
    // Peripheral columns excluded when bc/tc corners centre the readout.
    int margin_left = 0;
    int margin_right = 0;
};

// The only function in the cells/layout layer that touches curses
// output. Storybook also calls this.
inline void paint( const catacurses::window &w, const cell_buffer &buf )
{
    for( const positioned_segment &ps : buf ) {
        if( !ps.text.empty() ) {
            mvwprintz( w, ps.pos, ps.color, ps.text );
        }
    }
}

} // namespace gaunthud::cells

#endif // CATA_SRC_GAUNTHUD_CELLS_H
