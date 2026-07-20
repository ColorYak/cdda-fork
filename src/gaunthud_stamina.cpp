#include "gaunthud_stamina.h"

#include <utility>

#include "avatar.h"
#include "character.h"
#include "cursesdef.h"
#include "gaunthud_cells.h"
#include "move_mode.h"

namespace gaunthud::stamina
{

stamina_data extract( const avatar &u )
{
    const int live = u.get_stamina();
    const int max = u.get_stamina_max();
    const int deadband = u.last_move_stamina_burn();

    static int displayed = live;
    static int prev_displayed = live;

    // Force truth when player can act. Mirrors the logic in do_turn.cpp.
    const bool at_decision_point =
        u.get_moves() > 0 && !u.activity && !u.has_destination();

    const bool gain_exceeds_deadband = live - displayed > deadband;
    const bool loss_exceeds_deadband = displayed - live > deadband;
    const bool no_deadband = deadband <= 0; // no movement yet

    // A move can take multiple turns. Stamina is burned immediately, but is regained per turn.
    // This causes flickering, especially when when FORCE_UPDATE is set.
    // This approach works in practice for single steps as well as for auto-move.
    const bool update = live != displayed
        && ( at_decision_point || gain_exceeds_deadband || loss_exceeds_deadband || no_deadband );

    if( update ) {
        prev_displayed = displayed;
        displayed = live;
    }

    const move_mode_type mode = u.current_movement_mode()->type();
    return { displayed, prev_displayed, max, mode };
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

} // namespace gaunthud::stamina
