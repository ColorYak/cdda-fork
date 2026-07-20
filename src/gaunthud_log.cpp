#include "gaunthud_log.h"

#include <algorithm>

#include "cursesdef.h"
#include "messages.h"
#include "output.h"

namespace gaunthud::log
{

frame prepare( const avatar & )
{
    // The painter closure leaves a 1-col margin on each side, so usable text
    // width is window width minus 2.
    constexpr int height_cap = 8;
    const int width_cap = std::max( 40, TERMX / 3 );
    const auto ext = Messages::measure_messages_extent( width_cap - 2, height_cap );
    if( ext.height == 0 ) {
        // No surviving messages (empty log, or all expired under TTL). Let
        // the orchestrator skip the readout.
        return { cells::extent{ 0, 0 }, []( const catacurses::window & ) {} };
    }
    return {
        cells::extent{ std::min( width_cap, ext.width + 2 ), ext.height },
        []( const catacurses::window & w ) {
            Messages::display_messages( w, 1, 0,
                                        getmaxx( w ) - 1, getmaxy( w ) - 1 );
        },
    };
}

} // namespace gaunthud::log
