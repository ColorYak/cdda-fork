#include "gaunthud_stories.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "cursesdef.h"
#include "gaunthud_world_render.h"
#include "gaunthud_cells.h"
#include "move_mode.h"
#include "point.h"
#include "storybook.h"

namespace
{

namespace W = gaunthud::world;
namespace fs = std::filesystem;

struct world_story {
    std::string slug;
    std::string label;
    W::world_data data;
};

// Small builders so the story table reads as data, not
// aggregate-init soup.
W::time_data time_exact( std::string s )
{
    return { std::move( s ) };
}
W::time_data time_approx( std::string s )
{
    return { std::move( s ) };
}
W::time_data no_time()
{
    return {};
}
W::date_data date( W::season_kind s, int day )
{
    return { s, day };
}
W::date_data no_date()
{
    return {};
}
W::light_data light( float vm )
{
    return { vm };
}
W::temp_data temp_c( double c )
{
    return { true, c, W::temp_unit::celsius };
}
W::temp_data temp_f( double f )
{
    return { true, f, W::temp_unit::fahrenheit };
}
W::temp_data no_temp()
{
    return { false, 0.0, W::temp_unit::celsius };
}
W::weather_data weather( std::string id, std::string name = "" )
{
    if( name.empty() ) {
        name = id;
    }
    return { std::move( id ), std::move( name ) };
}
W::weather_data no_weather()
{
    return { "", "" };
}
W::wind_data wind( double windpower, int direction )
{
    return { true, windpower, direction };
}
W::wind_data no_wind()
{
    return { false, 0.0, 0 };
}

std::vector<world_story> world_stories()
{
    return {
        {   "clear-midday", "Clear midday outdoors, watch + thermometer",
            {
                time_exact( "12:14:03" ), date( W::season_kind::summer, 17 ),
                light( 1.0f ), temp_c( 22.0 ), weather( "sunny", "Sunny" ), wind( 7.0, 270 )
            }
        },
        {   "cloudy-cool", "Cloudy spring afternoon",
            {
                time_exact( "16:42:09" ), date( W::season_kind::spring, 4 ),
                light( 2.0f ), temp_c( 14.0 ), weather( "cloudy" ), wind( 12.0, 180 )
            }
        },
        {   "thunderstorm", "Autumn thunderstorm",
            {
                time_exact( "19:08:51" ), date( W::season_kind::autumn, 23 ),
                light( 3.0f ), temp_c( 18.0 ), weather( "thunder", "Thunder Storm" ), wind( 35.0, 90 )
            }
        },
        {   "blizzard", "Winter blizzard",
            {
                time_exact( "08:25:19" ), date( W::season_kind::winter, 61 ),
                light( 3.5f ), temp_c( -12.0 ), weather( "snowstorm", "Snowstorm" ), wind( 55.0, 45 )
            }
        },
        {   "fog-night-no-watch", "Foggy night, no watch (approx time), no thermometer",
            {
                time_approx( "Night" ), date( W::season_kind::autumn, 12 ),
                light( 4.5f ), no_temp(), weather( "fog", "Fog" ), wind( 2.0, 0 )
            }
        },
        {   "portal-storm", "Portal storm onset",
            {
                time_exact( "14:33:00" ), date( W::season_kind::summer, 41 ),
                light( 2.5f ), temp_c( 8.0 ), weather( "close_portal_storm", "Portal Storm" ),
                wind( 28.0, 315 )
            }
        },
        {   "indoors", "Sheltered indoors (no time, no weather, no wind)",
            {
                no_time(), date( W::season_kind::spring, 8 ),
                light( 2.0f ), temp_c( 21.0 ), no_weather(), no_wind()
            }
        },
        {   "fahrenheit", "Fahrenheit unit",
            {
                time_exact( "9:15:00 AM" ), date( W::season_kind::summer, 30 ),
                light( 1.0f ), temp_f( 72.0 ), weather( "sunny", "Sunny" ), wind( 4.0, 90 )
            }
        },
    };
}

} // namespace

namespace gaunthud_story
{

void run( const storybook::config &cfg )
{
    fs::create_directories( cfg.out_dir );

    const std::vector<world_story> stories = world_stories();
    storybook::section world_section{ "World zone variants", {} };

    for( const world_story &s : stories ) {
        const gaunthud::cells::render_result r = W::render( s.data );
        catacurses::window w = catacurses::newwin(
                                   r.size.height, r.size.width, point::zero );
        gaunthud::cells::paint( w, r.buffer );

        const std::string png_name = "world-" + s.slug + ".png";
        const fs::path png = cfg.out_dir / png_name;
        if( !storybook::paint_window_to_png( w, png ) ) {
            std::exit( 1 );
        }
        std::printf( "  %s\n", png.string().c_str() );

        world_section.entries.push_back( { s.slug, s.label, png_name } );
    }

    // Placeholder section for full-sidebar stories — currently no
    // entries, since only the world zone is implemented.
    storybook::section full_section{ "Full sidebar states", {} };

    storybook::write_index_html( cfg.out_dir,
                                 "GauntHUD storybook",
                                 { world_section, stamina_section, safemode_section, full_section } );
    std::printf( "Wrote %s\n",
                 ( cfg.out_dir / "index.html" ).string().c_str() );
}

} // namespace gaunthud_story
