#include "gaunthud_safemode.h"

#include <utility>

#include "avatar.h"
#include "calendar.h"
#include "cursesdef.h"
#include "game.h"
#include "gaunthud_cells.h"
#include "options.h"
#include "output.h"

namespace gaunthud::safemode
{

safemode_data extract( const avatar &u )
{
    if( !g ) {
        return {};
    }
    safemode_data d;
    switch( g->safe_mode ) {
        case SAFE_MODE_OFF:
            d.state = mode::off;
            d.autosafemode = get_option<bool>( "AUTOSAFEMODE" );
            d.turns_elapsed = to_turns<int>( g->turnssincelastmon );
            d.turns_threshold = get_option<int>( "AUTOSAFEMODETURNS" );
            break;
        case SAFE_MODE_STOP: {
            d.state = mode::stop;
            // Mirrors the spotted-creature messages in game::mon_info_update()
            // (game.cpp, search for "spotted!").  We branch on new_seen_mon.size()
            // rather than newseen-mostseen, so mixed NPC+monster cases can
            // produce a different label than the log message.
            const monster_visible_info &mvi = u.get_mon_visible();
            if( mvi.new_seen_mon.empty() ) {
                d.spotted = "Hostile survivor spotted";
            } else if( mvi.new_seen_mon.size() == 1 ) {
                d.spotted = uppercase_first_letter( mvi.new_seen_mon.back()->name() ) + " spotted";
            } else {
                d.spotted = "Monsters spotted";
            }
            break;
        }
        case SAFE_MODE_ON:
            d.state = mode::on;
            break;
    }
    return d;
}

frame prepare( const avatar &u )
{
    cells::render_result r = render( extract( u ) );
    if( r.size.width <= 0 || r.size.height <= 0 ) {
        return { cells::extent{ 0, 0 }, []( const catacurses::window & ) {} };
    }
    return {
        r.size,
        [buf = std::move( r.buffer )]( const catacurses::window &w ) {
            cells::paint( w, buf );
        },
        catacurses::window_backdrop::translucent,
        r.margin_left,
        r.margin_right,
    };
}

} // namespace gaunthud::safemode
