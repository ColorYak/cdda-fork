#include "gaunthud_world.h"

#include <string>
#include <utility>

#include "avatar.h"
#include "calendar.h"
#include "cursesdef.h"
#include "display.h"
#include "game.h"
#include "gaunthud_cells.h"
#include "make_static.h"
#include "options.h"
#include "overmapbuffer.h"
#include "type_id.h"
#include "units.h"
#include "weather.h"
#include "weather_type.h"

namespace
{

namespace W = gaunthud::world;

W::time_data extract_time( const avatar &u )
{
    // Mirrors the gating in `display::time_string`: exact clock when
    // the character has a watch, vague descriptor when outside, and
    // nothing at all otherwise. The vanilla helper returns "???" in
    // the last case; we omit the cell instead.
    if( u.has_watch() ) {
        return { to_string_time_of_day( calendar::turn ) };
    }
    if( is_creature_outside( u ) ) {
        return { display::time_approx() };
    }
    return {};
}

W::date_data extract_date( const avatar & )
{
    W::season_kind kind = W::season_kind::unknown;
    switch( season_of_year( calendar::turn ) ) {
        case SPRING:
            kind = W::season_kind::spring;
            break;
        case SUMMER:
            kind = W::season_kind::summer;
            break;
        case AUTUMN:
            kind = W::season_kind::autumn;
            break;
        case WINTER:
            kind = W::season_kind::winter;
            break;
        case NUM_SEASONS:
            break;
    }
    const int day = day_of_season<int>( calendar::turn ) + 1;
    return { kind, day };
}

W::light_data extract_light( const avatar &u )
{
    return { u.fine_detail_vision_mod() };
}

const flag_id json_flag_THERMOMETER( "THERMOMETER" );

W::temp_data extract_temp( const avatar &u, weather_manager &wx )
{
    const bool has_thermometer =
        u.cache_has_item_with( json_flag_THERMOMETER ) ||
        u.has_flag( STATIC( json_character_flag( "THERMOMETER" ) ) );
    if( !has_thermometer ) {
        return {};
    }

    // Matches the value the engine's stock thermometer displays via
    // `display::get_temp`.
    const units::temperature temp = wx.get_temperature( u.pos_bub() );
    const std::string &scale = get_option<std::string>( "USE_CELSIUS" );

    if( scale == "celsius" ) {
        return { true, units::to_celsius( temp ), W::temp_unit::celsius };
    }
    if( scale == "kelvin" ) {
        return { true, units::to_kelvin( temp ), W::temp_unit::kelvin };
    }
    return { true, units::to_fahrenheit( temp ), W::temp_unit::fahrenheit };
}

W::weather_data extract_weather( const avatar &u, const weather_manager &wx )
{
    // No weather underground or indoors.
    if( u.posz() < 0 || g->is_sheltered( u.pos_bub() ) ) {
        return {};
    }
    return { wx.weather_id.str(), wx.weather_id->name.translated() };
}

W::wind_data extract_wind( const avatar &u, const weather_manager &wx )
{
    // No wind underground or indoors.
    const bool sheltered = g->is_sheltered( u.pos_bub() );
    if( u.posz() < 0 || sheltered ) {
        return {};
    }
    // Match the engine's stock wind readout (`display::wind_text_color`).
    const oter_id &cur_om_ter = overmap_buffer.ter( u.pos_abs_omt() );
    const double windpower = get_local_windpower(
                                 wx.windspeed, cur_om_ter, u.pos_abs(),
                                 wx.winddirection, sheltered );
    return { true, windpower, wx.winddirection };
}

} // namespace

namespace gaunthud::world
{

world_data extract( const avatar &u, weather_manager &wx )
{
    return {
        extract_time( u ),
        extract_date( u ),
        extract_light( u ),
        extract_temp( u, wx ),
        extract_weather( u, wx ),
        extract_wind( u, wx ),
    };
}

frame prepare( const avatar &u )
{
    cells::render_result r = render( extract( u, get_weather() ) );
    return {
        r.size,
        [buf = std::move( r.buffer )]( const catacurses::window &w ) {
            cells::paint( w, buf );
        },
    };
}

} // namespace gaunthud::world
