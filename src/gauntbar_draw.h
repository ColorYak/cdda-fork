#pragma once
#ifndef CATA_SRC_GAUNTBAR_DRAW_H
#define CATA_SRC_GAUNTBAR_DRAW_H

#include <array>
#include <cstddef>
#include <string>

#include "catacharset.h"
#include "color.h"
#include "cursesdef.h"
#include "output.h"
#include "point.h"

// Cell placement and drawing for GauntBar rows. Renders one or more
// `segment`s per cell directly via `wprintz`, bypassing the engine's
// `<color_*>` tag pipeline. All widths and x positions are terminal
// columns (matching `utf8_width`), not bytes.

namespace gauntbar::draw
{

// Which edge of the text sits at the anchor x.
enum class anchor {
    left,
    centre,
    right,
};

// Left-edge x for text of the given column width placed at the anchor.
constexpr int compute_left( anchor a, int anchor_x, int text_width )
{
    switch( a ) {
        case anchor::left:
            return anchor_x;
        case anchor::centre:
            return anchor_x - ( text_width / 2 );
        case anchor::right:
            return anchor_x - text_width + 1;
    }
    return anchor_x;
}

// A pre-coloured run. `(text, color)` matches the shape returned by
// `display::*_color` helpers, so values from those can flow into a
// segment unchanged. A default-constructed `segment{}` is treated as
// empty by `draw_cell`.
struct segment {
    std::string text;
    nc_color color = c_white;
};

// An ordered list of `N` segments rendered left-to-right.
// Cells whose segment count varies at runtime always return
// their maximum-N variant with unused slots default-constructed.
template<std::size_t N>
struct cell {
    std::array<segment, N> segments{};
};

// `cell{ segment{...}, segment{...} }` becomes `cell<2>`
// without spelling N.
template<typename... S>
cell( S... ) -> cell<sizeof...( S )>;

// Place a cell's segments at `(anchor_x, row)` with the given anchor.
template<std::size_t N>
void draw_cell( const catacurses::window &w, int row, int anchor_x,
                anchor a, const cell<N> &c )
{
    if constexpr( N == 0 ) {
        return;
    } else {
        int total = 0;
        for( const segment &s : c.segments ) {
            total += utf8_width( s.text );
        }
        if( total == 0 ) {
            return;
        }
        wmove( w, point( compute_left( a, anchor_x, total ), row ) );
        for( const segment &s : c.segments ) {
            if( !s.text.empty() ) {
                wprintz( w, s.color, s.text );
            }
        }
    }
}

} // namespace gauntbar::draw

#endif // CATA_SRC_GAUNTBAR_DRAW_H
