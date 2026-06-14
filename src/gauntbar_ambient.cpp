#include "gauntbar_ambient.h"

#include <string>

#include "avatar.h"
#include "cursesdef.h"
#include "game.h"
#include "make_static.h"
#include "options.h"
#include "overmapbuffer.h"
#include "panels.h"
#include "type_id.h"
#include "units.h"
#include "weather.h"
#include "weather_type.h"

namespace
{

namespace A = gauntbar::ambient;

A::light_data extract_light( const avatar &u )
{
    return { u.fine_detail_vision_mod() };
}

const flag_id json_flag_THERMOMETER( "THERMOMETER" );

A::temp_data extract_temp( const avatar &u, weather_manager &wx )
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
        return { true, units::to_celsius( temp ), A::temp_unit::celsius };
    }
    if( scale == "kelvin" ) {
        return { true, units::to_kelvin( temp ), A::temp_unit::kelvin };
    }
    return { true, units::to_fahrenheit( temp ), A::temp_unit::fahrenheit };
}

A::weather_data extract_weather( const avatar &u, const weather_manager &wx )
{
    // No weather underground or indoors.
    if( u.posz() < 0 || g->is_sheltered( u.pos_bub() ) ) {
        return {};
    }
    return { wx.weather_id.str(), wx.weather_id->name.translated() };
}

A::wind_data extract_wind( const avatar &u, const weather_manager &wx )
{
    // No wind underground or indoors.
    if( u.posz() < 0 || g->is_sheltered( u.pos_bub() ) ) {
        return {};
    }
    // Match the engine's stock wind readout (`display::wind_text_color`).
    const oter_id &cur_om_ter = overmap_buffer.ter( u.pos_abs_omt() );
    const double windpower = get_local_windpower(
                                 wx.windspeed, cur_om_ter, u.pos_abs(),
                                 wx.winddirection, g->is_sheltered( u.pos_bub() ) );
    return { true, windpower, wx.winddirection };
}

} // namespace

namespace gauntbar::ambient
{

void draw( const draw_args &args )
{
    const avatar &u = args._ava;
    weather_manager &wx = get_weather();
    werase( args._win );
    render( args._win, {
        extract_light( u ),
        extract_temp( u, wx ),
        extract_weather( u, wx ),
        extract_wind( u, wx ),
    } );
    wnoutrefresh( args._win );
}

} // namespace gauntbar::ambient
