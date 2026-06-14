#include "gauntbar_stories.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "cursesdef.h"
#include "gauntbar.h"
#include "gauntbar_ambient_render.h"
#include "point.h"
#include "storybook.h"

namespace
{

namespace A = gauntbar::ambient;
namespace fs = std::filesystem;

struct ambient_story {
    std::string slug;
    std::string label;
    A::ambient_data data;
};

// Small builders so the story table reads as data, not
// aggregate-init soup.
A::light_data light( float vm )
{
    return { vm };
}
A::temp_data temp_c( double c )
{
    return { true, c, A::temp_unit::celsius };
}
A::temp_data temp_f( double f )
{
    return { true, f, A::temp_unit::fahrenheit };
}
A::temp_data no_temp()
{
    return { false, 0.0, A::temp_unit::celsius };
}
A::weather_data weather( std::string id, std::string name = "" )
{
    if( name.empty() ) {
        name = id;
    }
    return { std::move( id ), std::move( name ) };
}
A::weather_data no_weather()
{
    return { "", "" };
}
A::wind_data wind( double windpower, int direction )
{
    return { true, windpower, direction };
}
A::wind_data no_wind()
{
    return { false, 0.0, 0 };
}

std::vector<ambient_story> ambient_stories()
{
    return {
        {   "clear-midday", "Clear midday outdoors",
            { light( 1.0f ), temp_c( 22.0 ), weather( "sunny", "Sunny" ), wind( 7.0, 270 ) }
        },
        {   "cloudy-cool", "Cloudy, cool afternoon",
            { light( 2.0f ), temp_c( 14.0 ), weather( "cloudy" ), wind( 12.0, 180 ) }
        },
        {   "thunderstorm", "Thunderstorm",
            { light( 3.0f ), temp_c( 18.0 ), weather( "thunder", "Thunder Storm" ), wind( 35.0, 90 ) }
        },
        {   "blizzard", "Blizzard",
            { light( 3.5f ), temp_c( -12.0 ), weather( "snowstorm", "Snowstorm" ), wind( 55.0, 45 ) }
        },
        {   "fog-night", "Foggy night, no thermometer",
            { light( 4.5f ), no_temp(), weather( "fog", "Fog" ), wind( 2.0, 0 ) }
        },
        {   "portal-storm", "Portal storm onset",
            { light( 2.5f ), temp_c( 8.0 ), weather( "close_portal_storm", "Portal Storm" ), wind( 28.0, 315 ) }
        },
        {   "indoors", "Sheltered indoors (no wind, no weather)",
            { light( 2.0f ), temp_c( 21.0 ), no_weather(), no_wind() }
        },
        {   "fahrenheit", "Fahrenheit unit",
            { light( 1.0f ), temp_f( 72.0 ), weather( "sunny", "Sunny" ), wind( 4.0, 90 ) }
        },
    };
}

} // namespace

namespace gauntbar_story
{

void run( const storybook::config &cfg )
{
    fs::create_directories( cfg.out_dir );

    const std::vector<ambient_story> amb = ambient_stories();
    storybook::section ambient_section{ "Ambient zone variants", {} };

    for( const ambient_story &s : amb ) {
        catacurses::window w = catacurses::newwin( 1, gauntbar::width, point::zero );
        A::render( w, s.data );

        const std::string png_name = "ambient-" + s.slug + ".png";
        const fs::path png = cfg.out_dir / png_name;
        if( !storybook::render_window_to_png( w, png ) ) {
            std::exit( 1 );
        }
        std::printf( "  %s\n", png.string().c_str() );

        ambient_section.entries.push_back( { s.slug, s.label, png_name } );
    }

    // Placeholder section for full-sidebar stories — currently no
    // entries, since only the ambient zone is implemented.
    storybook::section full_section{ "Full sidebar states", {} };

    storybook::write_index_html( cfg.out_dir,
                                 "GauntBar storybook",
                                 { ambient_section, full_section } );
    std::printf( "Wrote %s\n",
                 ( cfg.out_dir / "index.html" ).string().c_str() );
}

} // namespace gauntbar_story
