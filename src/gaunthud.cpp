#include "gaunthud.h"

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "cuboid_rectangle.h"
#include "cursesdef.h"
#include "debug.h"
#include "game.h"
#include "gaunthud_world.h"
#include "gaunthud_hud.h"
#include "gaunthud_log.h"
#include "gaunthud_map.h"
#include "gaunthud_place.h"
#include "gaunthud_safemode.h"
#include "gaunthud_stamina.h"
#include "output.h"
#include "panels.h"
#include "point.h"
#include "translation.h"
#include "ui_manager.h"

#if defined(TILES)
#include "sdltiles.h"
#endif

namespace
{

using gaunthud::corner;
using gaunthud::corner_stack;
using gaunthud::readout;
using gaunthud::cells::extent;

// Convert a readout's cell-space placement into the coordinate space of
// `ui_adaptor::dimensions`, so it can be tested for overlap against overlay
// menu dimensions. Mirrors `ui_adaptor::position`.
rectangle<point> readout_rect( const point &origin, const extent &size )
{
#if defined(TILES)
    const window_dimensions dim =
        get_window_dimensions( origin, point( size.width, size.height ) );
    return rectangle<point>( dim.window_pos_pixel,
                             dim.window_pos_pixel + dim.window_size_pixel );
#else
    return rectangle<point>( origin, origin + point( size.width, size.height ) );
#endif
}

bool rect_obscured( const rectangle<point> &r,
                    const std::vector<rectangle<point>> &overlays )
{
    const half_open_rectangle<point> hr( r.p_min, r.p_max );
    for( const rectangle<point> &o : overlays ) {
        if( o.p_max.x <= o.p_min.x || o.p_max.y <= o.p_min.y ) {
            continue;
        }
        if( hr.overlaps( o ) ) {
            return true;
        }
    }
    return false;
}

// Single place that says which readouts go where, in what order.
// Empty stacks are fine; the corner contributes nothing.
const std::array<corner_stack, 6> &corners()
{
    static const std::array<corner_stack, 6> table = { {
            corner_stack{ corner::tl, { { "world", to_translation( "World" ), gaunthud::world::prepare } } },
            corner_stack{ corner::tr, {
#if defined(TILES)
                    { "map",     to_translation( "Map" ),     gaunthud::map::prepare },
#endif
                    { "place",   to_translation( "Place" ),   gaunthud::place::prepare },
                } },
            corner_stack{ corner::bl, {} },
            corner_stack{ corner::bc, { { "stamina", to_translation( "Stamina" ), gaunthud::stamina::prepare } } },
            corner_stack{ corner::tc, { { "safemode", to_translation( "Safe Mode" ), gaunthud::safemode::prepare } } },
            corner_stack{ corner::br, { { "log",     to_translation( "Log" ),     gaunthud::log::prepare } } },
        }
    };

    return table;
}

// Master proxy id. When this proxy's toggle is off, `paint_hud`
// paints nothing — the whole HUD is hidden, useful for clean
// terrain screenshots.
constexpr const char *hud_proxy_id = "hud";

// Registered with `panel_manager` purely to satisfy two structural
// assumptions in panels.cpp:
//
// * `panel_manager::init` and the `MOVE_PANEL` handler in `show_adm`
//   both dereference `get_current_layout().panels().begin()` when
//   computing the first-panel width offset. With an empty panels
//   vector this dereferences `end()` — UB. Registering at least one
//   proxy keeps the vector non-empty.
// * `update_offsets` is private; we can't call it from `gaunthud::`.
//   Every proxy reports `width = 0`, so the existing call sites
//   naturally call `update_offsets( 0 )` on init and on layout
//   switch, which is what we want — the terrain window then takes
//   up the full screen width via the resize callback in
//   `create_or_get_main_ui_adaptor`.
//
// `render()` must return `true`. The intuitive choice — return
// `false` so the engine's `draw_panels` column loop skips it — is
// already covered by the early-return branch in `game::draw_panels`.
// What `render = true` matters for is the `}` admin menu's
// `row_indices` map: with `render = false` the map is empty,
// `num_rows - 1` underflows to `SIZE_MAX`, and `row_indices.at()`
// throws `std::out_of_range` when the menu is opened.
window_panel build_proxy( const std::string &id, const translation &name )
{
    return window_panel(
        []( const draw_args & ) { /* unreachable: short-circuited by paint_hud */ },
        id, name,
        /*ht=*/1, /*wd=*/0, /*default_toggle=*/true,
        /*render_func=*/[]() {
            return true;
        },
        /*force_draw=*/false );
}

} // namespace

namespace gaunthud
{

bool readout_enabled( const std::string &id )
{
    const panel_layout &layout =
        panel_manager::get_manager().get_current_layout();
    for( const window_panel &p : layout.panels() ) {
        if( p.get_id() == id ) {
            return p.toggle;
        }
    }
    return true;
}

point compute_origin( corner where, cells::extent size, int offset,
                      int margin_left, int margin_right )
{
    const int anchor_width = size.width - margin_left - margin_right;
    switch( where ) {
        case corner::tl:
            return point( 0, offset );
        case corner::tr:
            return point( TERMX - size.width, offset );
        case corner::bl:
            return point( 0, TERMY - offset - size.height );
        case corner::br:
            return point( TERMX - size.width, TERMY - offset - size.height );
        case corner::bc:
            return point( ( ( TERMX - anchor_width ) / 2 ) - margin_left,
                          TERMY - offset - size.height );
        case corner::tc:
            return point( ( ( TERMX - anchor_width ) / 2 ) - margin_left, offset );
    }
    debugmsg( "GauntHUD compute_origin: unknown corner %d",
              static_cast<int>( where ) );
    return point::zero;
}

void register_layout( std::map<std::string, panel_layout> &layouts )
{
    // Master first so it heads the `}` admin menu.
    std::vector<window_panel> panels{
        build_proxy( hud_proxy_id, to_translation( "HUD" ) ),
        build_proxy( "log", to_translation( "Log" ) ),
#if defined(TILES)
        build_proxy( "map", to_translation( "Map" ) ),
#endif
        build_proxy( "place", to_translation( "Place" ) ),
    };
    layouts.emplace( layout_id,
                     panel_layout( to_translation( "GauntHUD" ), panels ) );
}

void paint_hud( const avatar &u )
{
    // Master toggle: hide the whole HUD when the `hud` proxy is off.
    if( !readout_enabled( hud_proxy_id ) ) {
        return;
    }

    // Dimensions of every overlay stacked above the main game UI. A readout
    // overlapping one renders underneath it and shows through the translucent
    // background as noise.
    std::vector<rectangle<point>> overlays;
    if( g ) {
        if( const auto main_ui = g->create_or_get_main_ui_adaptor() ) {
            overlays = ui_manager::dimensions_above( *main_ui );
        }
    }

    for( const corner_stack &stack : corners() ) {
        // Collect placements for the whole corner first; the obscured check
        // below needs all of them before anything is painted.
        struct placement {
            frame f;
            point origin;
        };
        std::vector<placement> placements;
        int offset = 0;
        for( const readout &r : stack.readouts ) {
            // Per-readout toggle: if a proxy with the readout's id is
            // registered, consult its `toggle` field. Readouts without a
            // matching proxy (e.g. world) always paint.
            if( !readout_enabled( r.id ) ) {
                continue;
            }
            frame f = r.prepare( u );
            if( f.size.width <= 0 || f.size.height <= 0 ) {
                continue;
            }
            const point origin = compute_origin( stack.where, f.size, offset,
                                                   f.margin_left, f.margin_right );
            offset += f.size.height;
            placements.push_back( { std::move( f ), origin } );
        }
        if( placements.empty() ) {
            continue;
        }

        // All-or-none per corner: if any readout in the corner is obscured by
        // an overlay menu, hide the entire corner rather than painting a
        // partial stack.
        bool obscured = false;
        for( const placement &p : placements ) {
            if( rect_obscured( readout_rect( p.origin, p.f.size ), overlays ) ) {
                obscured = true;
                break;
            }
        }
        if( obscured ) {
            continue;
        }

        for( const placement &p : placements ) {
            const frame &f = p.f;
            catacurses::window w = catacurses::newwin(
                                       f.size.height, f.size.width, p.origin );
            if( f.backdrop_override ) {
                catacurses::set_window_bg_alpha( w, *f.backdrop_override );
            }
            werase( w );
            f.paint( w );
            wnoutrefresh( w );
        }
    }
}

} // namespace gaunthud
