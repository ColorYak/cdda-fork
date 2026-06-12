#pragma once
#ifndef CATA_SRC_GAUNTBAR_DRAW_H
#define CATA_SRC_GAUNTBAR_DRAW_H

#include <string>

#include "color.h"
#include "cursesdef.h"

// Cell placement and drawing for GauntBar rows. Owns the `anchor` enum
// (which edge of the cell sits at a given x), the math that maps an
// anchor + width to a left-edge x, and thin wrappers over `output.h` /
// `catacharset.h` that render a cell. No UTF-8 or color-tag handling of
// our own.

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

// `(text, color)` matches the return shape of `display::*_color`
// helpers, so values from those can flow through unchanged.
struct cell {
    std::string text;
    nc_color color = c_white;
};

// Place `text` at `(anchor_x, row)` with the given anchor edge. Honours
// `<color_*>` tags; `base_color` covers untagged runs.
void draw_cell( const catacurses::window &w, int row, int anchor_x,
                anchor a, const std::string &text, const nc_color &base_color );

void draw_cell_clamped( const catacurses::window &w, int row, int anchor_x,
                        anchor a, int max_cols,
                        const std::string &text, const nc_color &base_color );

inline void draw_cell( const catacurses::window &w, int row, int anchor_x,
                       anchor a, const cell &c )
{
    draw_cell( w, row, anchor_x, a, c.text, c.color );
}

inline void draw_cell_clamped( const catacurses::window &w, int row, int anchor_x,
                               anchor a, int max_cols, const cell &c )
{
    draw_cell_clamped( w, row, anchor_x, a, max_cols, c.text, c.color );
}

} // namespace gauntbar::draw

#endif // CATA_SRC_GAUNTBAR_DRAW_H
