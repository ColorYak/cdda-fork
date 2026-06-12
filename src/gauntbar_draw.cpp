#include "gauntbar_draw.h"

#include "catacharset.h"
#include "output.h"
#include "point.h"

namespace gauntbar::draw
{

void draw_cell( const catacurses::window &w, int row, int anchor_x,
                anchor a, const std::string &text, const nc_color &base_color )
{
    const int text_w = utf8_width( text, /*ignore_tags=*/true );
    const int x = compute_left( a, anchor_x, text_w );
    // Scratch lvalue for the in/out colour reference; unused after.
    nc_color dummy = base_color;
    print_colored_text( w, point( x, row ), dummy, base_color, text );
}

void draw_cell_clamped( const catacurses::window &w, int row, int anchor_x,
                        anchor a, int max_cols,
                        const std::string &text, const nc_color &base_color )
{
    draw_cell( w, row, anchor_x, a, trim_by_length( text, max_cols ), base_color );
}

} // namespace gauntbar::draw
