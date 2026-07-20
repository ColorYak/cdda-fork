#include "live_view.h"

#include <algorithm> // min & max
#include <memory>

#include "cached_options.h"
#include "color.h"
#include "coordinates.h"
#include "cursesdef.h"
#include "game.h"
#include "map.h"
#include "output.h"
#include "panels.h"
#include "translations.h"
#include "ui_manager.h"

namespace
{

constexpr int START_LINE = 1;
constexpr int MIN_BOX_HEIGHT = 3;

} //namespace

live_view::live_view() = default;
live_view::~live_view() = default;

void live_view::init()
{
    hide();
}

void live_view::hide()
{
    ui = nullptr;
}

void live_view::show( const tripoint &p )
{
    map &here = get_map();

    mouse_position = p;
    if( !ui ) {
        ui = std::make_unique<ui_adaptor>();
        ui->on_screen_resize( [this, &here]( ui_adaptor & ui ) {
            const auto [width, origin_x] =
                panel_manager::get_manager().sidebar_anchored_overlay( 30, 60 );

            const int max_height = pixel_minimap_option ? TERMY / 2 : TERMY;
            const int line_limit = max_height - 2;
            const visibility_variables &cache = here.get_visibility_variables_cache();
            int line_out = START_LINE;
            // HACK: using dummy window to get the window height without refreshing.
            win = catacurses::newwin( 1, width, point::zero );
            g->pre_print_all_tile_info( tripoint_bub_ms( mouse_position ), win, line_out, line_limit, cache );
            const int live_view_box_height = std::min( max_height, std::max( line_out + 2, MIN_BOX_HEIGHT ) );

            win = catacurses::newwin( live_view_box_height, width,
                                      point( origin_x, 0 ) );
            ui.position_from_window( win );
        } );
        ui->on_redraw( [this, &here]( const ui_adaptor & ) {
            werase( win );
            const visibility_variables &cache = here.get_visibility_variables_cache();
            int line_out = START_LINE;
            g->pre_print_all_tile_info( tripoint_bub_ms( mouse_position ), win, line_out, getmaxy( win ) - 2,
                                        cache );
            draw_border( win );
            center_print( win, 0, c_white, _( "< <color_green>Mouse view</color> >" ) );
            wnoutrefresh( win );
        } );
    }
    // Always mark ui for resize as the required box height may have changed.
    ui->mark_resize();
}

bool live_view::is_enabled()
{
    return ui != nullptr;
}
