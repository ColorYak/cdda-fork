#include "gauntbar.h"

#include <string>
#include <vector>

#include "cursesdef.h"
#include "game.h"
#include "gauntbar_ambient.h"
#include "messages.h"
#include "output.h"
#include "panels.h"
#include "translation.h"

namespace
{

// Duplicated from panels.cpp's draw_messages, which has internal linkage.
// Kept byte-identical so behaviour tracks the engine.
void draw_gauntbar_messages( const draw_args &args )
{
    const catacurses::window &w = args._win;

    werase( w );
    const int line = getmaxy( w ) - 1;
    const int maxlength = getmaxx( w );
    Messages::display_messages( w, 1, 0 /*topline*/, maxlength - 1, line );
    wnoutrefresh( w );
}

#if defined(TILES)
void draw_gauntbar_minimap( const draw_args &args )
{
    const catacurses::window &w = args._win;

    werase( w );
    g->draw_pixel_minimap( w );
    wnoutrefresh( w );
}
#endif

std::vector<window_panel> build_panels()
{
    std::vector<window_panel> panels;

    panels.emplace_back( gauntbar::ambient::draw, "GauntBar",
                         to_translation( "GauntBar" ), 1, gauntbar::width, true );

    panels.emplace_back( draw_gauntbar_messages, "Log", to_translation( "Log" ), -2,
                         gauntbar::width, true );
#if defined(TILES)
    panels.emplace_back( draw_gauntbar_minimap, "Map", to_translation( "Map" ), -1,
                         gauntbar::width, true, default_render, true );
#endif

    return panels;
}

} // namespace

namespace gauntbar
{

void register_layout( std::map<std::string, panel_layout> &layouts )
{
    layouts.emplace( layout_id,
                     panel_layout( to_translation( "GauntBar" ), build_panels() ) );
}

} // namespace gauntbar
