#include "gaunthud_map.h"

#if defined(TILES)
#include "cached_options.h"
#include "cursesdef.h"
#include "game.h"
#endif

namespace gaunthud::map
{

frame prepare( const avatar & )
{
#if defined(TILES)
    if( !pixel_minimap_option ) {
        return { cells::extent{ 0, 0 }, []( const catacurses::window & ) {} };
    }
    // Aspect ratio sized for a square readout at the ~2:1
    // monospace cell aspect we draw against.
    frame f{
        cells::extent{ map_width, map_width / 2 },
        []( const catacurses::window & w ) {
            g->draw_pixel_minimap( w );
        },
    };
    // The minimap looks better without the border around, when the
    // place readout is rendered below it looks calmer with both
    // being in the same box (the have the same width).
    if( !readout_enabled( "place" ) ) {
        f.backdrop_override = catacurses::window_backdrop::none;
    }
    return f;
#else
    // Non-TILES builds don't render the pixel minimap; the corner
    // table omits this readout, so this branch is unreachable.
    return { cells::extent{ 0, 0 }, []( const catacurses::window & ) {} };
#endif
}

} // namespace gaunthud::map
