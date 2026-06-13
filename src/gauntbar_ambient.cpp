#include "gauntbar_ambient.h"

// One-row ambient-conditions strip at the top of the GauntBar sidebar.

#include <array>
#include <map>
#include <string>

#include "avatar.h"
#include "color.h"
#include "cursesdef.h"
#include "game.h"
#include "gauntbar_draw.h"
#include "gauntbar_layout.h"
#include "make_static.h"
#include "options.h"
#include "output.h"
#include "overmapbuffer.h"
#include "panels.h"
#include "string_formatter.h"
#include "translation.h"
#include "type_id.h"
#include "units.h"
#include "weather.h"
#include "weather_type.h"

namespace
{

namespace D = gauntbar::draw;

// === Light ===

inline constexpr const char *NF_MD_LIGHTBULB_ON = u8"\U000F06E8";
inline constexpr const char *NF_MD_LIGHTBULB_ON_80 = u8"\U000F1A55";
inline constexpr const char *NF_MD_LIGHTBULB_ON_50 = u8"\U000F1A52";
inline constexpr const char *NF_MD_LIGHTBULB_ON_20 = u8"\U000F1A4F";
inline constexpr const char *NF_MD_LIGHTBULB_OUTLINE = u8"\U000F0336";

struct lighting_glyph {
    const char *glyph;
    nc_color color;
};

lighting_glyph lighting_for_vision_mod( float vision_mod )
{
    if( vision_mod <= 1.0f ) {
        return {NF_MD_LIGHTBULB_ON, c_yellow};
    }
    if( vision_mod <= 2.0f ) {
        return {NF_MD_LIGHTBULB_ON_80, c_white};
    }
    if( vision_mod <= 3.0f ) {
        return {NF_MD_LIGHTBULB_ON_50, c_light_gray};
    }
    if( vision_mod <= 4.0f ) {
        return {NF_MD_LIGHTBULB_ON_20, c_light_cyan};
    }
    return {NF_MD_LIGHTBULB_OUTLINE, c_light_blue};
}

D::cell<1> light_cell( const avatar &u )
{
    const lighting_glyph lg = lighting_for_vision_mod( u.fine_detail_vision_mod() );
    return D::cell{ D::segment{ lg.glyph, lg.color } };
}

// === Temperature ===

inline constexpr const char *NF_MD_TEMPERATURE_CELSIUS = u8"\U000F0504";
inline constexpr const char *NF_MD_TEMPERATURE_FAHRENHEIT = u8"\U000F0505";
inline constexpr const char *NF_MD_TEMPERATURE_KELVIN = u8"\U000F0506";

const flag_id json_flag_THERMOMETER( "THERMOMETER" );

D::cell<1> temp_cell( const avatar &u, weather_manager &wx )
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

    double value;
    const char *glyph;
    if( scale == "celsius" ) {
        value = units::to_celsius( temp );
        glyph = NF_MD_TEMPERATURE_CELSIUS;
    } else if( scale == "kelvin" ) {
        value = units::to_kelvin( temp );
        glyph = NF_MD_TEMPERATURE_KELVIN;
    } else {
        value = units::to_fahrenheit( temp );
        glyph = NF_MD_TEMPERATURE_FAHRENHEIT;
    }

    return D::cell{
        D::segment{ string_format( "%.0f%s", value, glyph ), c_light_gray },
    };
}

// === Weather ===

inline constexpr const char *NF_WEATHER_DAY_SUNNY = u8"\uE30D";
inline constexpr const char *NF_WEATHER_CLOUD = u8"\uE33D";
inline constexpr const char *NF_WEATHER_FOG = u8"\uE313";
inline constexpr const char *NF_WEATHER_RAIN = u8"\uE318";
inline constexpr const char *NF_WEATHER_RAIN_WIND = u8"\uE317";
inline constexpr const char *NF_WEATHER_SNOW = u8"\uE31A";
inline constexpr const char *NF_WEATHER_SNOW_WIND = u8"\uE35E";
inline constexpr const char *NF_WEATHER_LIGHTNING = u8"\uE315";
inline constexpr const char *NF_WEATHER_SOLAR_ECLIPSE = u8"\uE368";
inline constexpr const char *NF_WEATHER_HURRICANE = u8"\uE36C";
inline constexpr const char *NF_WEATHER_SANDSTORM = u8"\uE37A";
inline constexpr const char *NF_WEATHER_MOON_ALT_FULL = u8"\uE3D5";

struct weather_glyph {
    const char *glyph;
    nc_color color;
    // Whether to render the weather_type's name alongside the glyph.
    bool show_name;
};

const std::map<std::string, weather_glyph> &weather_glyphs()
{
    static const std::map<std::string, weather_glyph> table = {
        // data/core/weather.json
        {"clear", {NF_WEATHER_DAY_SUNNY, c_dark_gray, false}},

        // data/json/weather_type.json
        {"sunny", {NF_WEATHER_DAY_SUNNY, c_white, true}},
        {"cloudy", {NF_WEATHER_CLOUD, c_light_gray, false}},

        {"light_drizzle", {NF_WEATHER_RAIN, c_light_gray, false}},
        {"drizzle", {NF_WEATHER_RAIN, c_light_gray, true}},
        {"rain", {NF_WEATHER_RAIN, c_light_gray, true}},
        {"rainstorm", {NF_WEATHER_RAIN_WIND, c_light_blue, true}},

        {"thunder", {NF_WEATHER_LIGHTNING, c_yellow, true}},
        {"lightning", {NF_WEATHER_LIGHTNING, c_yellow, true}},

        {"flurries", {NF_WEATHER_SNOW, c_light_gray, false}},
        {"snowing", {NF_WEATHER_SNOW, c_light_gray, true}},
        {"snowstorm", {NF_WEATHER_SNOW_WIND, c_light_cyan, true}},

        {"mist", {NF_WEATHER_FOG, c_light_gray, false}},
        {"fog", {NF_WEATHER_FOG, c_white, true}},

        {"early_portal_storm", {NF_WEATHER_HURRICANE, c_light_red, true}},
        {"distant_portal_storm", {NF_WEATHER_HURRICANE, c_red, true}},
        {"close_portal_storm", {NF_WEATHER_HURRICANE, c_red, true}},
        {"portal_storm", {NF_WEATHER_HURRICANE, c_red, true}},

        // data/json/effects_on_condition/scenario_specific_eocs.json
        {"moonlit_scenario_weather", {NF_WEATHER_MOON_ALT_FULL, c_light_gray, false}},

        // data/mods/Xedra_Evolved/weather_type.json
        {"blotted_sun", {NF_WEATHER_SOLAR_ECLIPSE, c_red, true}},
        {"magic_weather_thick_fog", {NF_WEATHER_FOG, c_light_gray, true}},
        {"magic_weather_light_drizzle", {NF_WEATHER_RAIN, c_light_gray, false}},
        {"magic_weather_drizzle", {NF_WEATHER_RAIN, c_light_gray, true}},
        {"magic_weather_rain", {NF_WEATHER_RAIN, c_light_gray, true}},
        {"magic_weather_rainstorm", {NF_WEATHER_RAIN_WIND, c_light_blue, true}},
        {"magic_weather_thunder", {NF_WEATHER_LIGHTNING, c_yellow, true}},
        {"magic_weather_lightning", {NF_WEATHER_LIGHTNING, c_yellow, true}},

        // data/mods/aftershock_exoplanet/weather_type.json
        {"afs_flurries", {NF_WEATHER_SNOW, c_light_gray, false}},
        {"afs_snowing", {NF_WEATHER_SNOW, c_light_gray, true}},
        {"afs_whiteout", {NF_WEATHER_SNOW_WIND, c_red, true}},
        {"afs_thunder", {NF_WEATHER_LIGHTNING, c_yellow, true}},
        {"afs_lightning", {NF_WEATHER_LIGHTNING, c_yellow, true}},

        // data/mods/desert_region/weather/desert_weather_type.json
        {"early_dust_storm", {NF_WEATHER_SANDSTORM, c_yellow, true}},
        {"dust_storm", {NF_WEATHER_SANDSTORM, c_red, true}},
    };
    return table;
}

D::cell<2> weather_cell( const avatar &u, const weather_manager &wx )
{
    // No weather underground or indoors.
    if( u.posz() < 0 || g->is_sheltered( u.pos_bub() ) ) {
        return {};
    }
    const std::string wid = wx.weather_id.str();
    const auto it = weather_glyphs().find( wid );
    if( it == weather_glyphs().end() ) {
        return D::cell{
            D::segment{},
            D::segment{ wx.weather_id->name.translated(), c_light_gray },
        };
    }
    const weather_glyph &wg = it->second;
    return D::cell{
        D::segment{ wg.glyph, wg.color },
        D::segment{
            wg.show_name ? wx.weather_id->name.translated() : std::string{},
            c_light_gray,
        },
    };
}

// === Wind ===

// Beaufort scale glyphs, indexed by Beaufort number 0–12.
inline constexpr std::array<const char *, 13> NF_WEATHER_WIND_BEAUFORT = {
    u8"\uE3AF", u8"\uE3B0", u8"\uE3B1", u8"\uE3B2", u8"\uE3B3",
    u8"\uE3B4", u8"\uE3B5", u8"\uE3B6", u8"\uE3B7", u8"\uE3B8",
    u8"\uE3B9", u8"\uE3BA", u8"\uE3BB",
};

// Thresholds mirror `get_wind_desc` in weather.cpp.
int wind_beaufort_number( double windpower )
{
    if( windpower < 1 ) {
        return 0;
    }
    if( windpower <= 3 ) {
        return 1;
    }
    if( windpower <= 7 ) {
        return 2;
    }
    if( windpower <= 12 ) {
        return 3;
    }
    if( windpower <= 18 ) {
        return 4;
    }
    if( windpower <= 24 ) {
        return 5;
    }
    if( windpower <= 31 ) {
        return 6;
    }
    if( windpower <= 38 ) {
        return 7;
    }
    if( windpower <= 46 ) {
        return 8;
    }
    if( windpower <= 54 ) {
        return 9;
    }
    if( windpower <= 63 ) {
        return 10;
    }
    if( windpower <= 72 ) {
        return 11;
    }
    return 12;
}

inline constexpr const char *NF_OCT_ARROW_UP = u8"\uF431";
inline constexpr const char *NF_OCT_ARROW_UP_RIGHT = u8"\uF46C";
inline constexpr const char *NF_OCT_ARROW_RIGHT = u8"\uF432";
inline constexpr const char *NF_OCT_ARROW_DOWN_RIGHT = u8"\uF43E";
inline constexpr const char *NF_OCT_ARROW_DOWN = u8"\uF433";
inline constexpr const char *NF_OCT_ARROW_DOWN_LEFT = u8"\uF424";
inline constexpr const char *NF_OCT_ARROW_LEFT = u8"\uF434";
inline constexpr const char *NF_OCT_ARROW_UP_LEFT = u8"\uF45C";

// Transforms, as `dirangle` is the direction the wind comes FROM,
// while arrow glyphs point the way the wind blows TO.
// Bucket boundaries mirror `get_wind_arrow` in weather.cpp.
const char *wind_direction_glyph( int dirangle )
{
    if( dirangle < 0 || dirangle >= 360 ) {
        return "";
    }
    if( dirangle <= 23 || dirangle > 338 ) {
        return NF_OCT_ARROW_DOWN;
    }
    if( dirangle <= 68 ) {
        return NF_OCT_ARROW_DOWN_LEFT;
    }
    if( dirangle <= 113 ) {
        return NF_OCT_ARROW_LEFT;
    }
    if( dirangle <= 158 ) {
        return NF_OCT_ARROW_UP_LEFT;
    }
    if( dirangle <= 203 ) {
        return NF_OCT_ARROW_UP;
    }
    if( dirangle <= 248 ) {
        return NF_OCT_ARROW_UP_RIGHT;
    }
    if( dirangle <= 293 ) {
        return NF_OCT_ARROW_RIGHT;
    }
    return NF_OCT_ARROW_DOWN_RIGHT;
}

// Buckets mirror `get_wind_color` in weather.cpp on break density, but
// use a warm escalation (gray → white → yellow) instead of the engine's
// cool palette so the cell reads as a sidebar register rather than a
// readout. Yellow caps at storm tier; nothing escalates beyond it.
nc_color wind_beaufort_color( int beaufort )
{
    if( beaufort <= 5 ) {
        // Beaufort 3–5: gentle, moderate, fresh breeze.
        return c_dark_gray;
    }
    if( beaufort <= 7 ) {
        // Beaufort 6–7: strong breeze, moderate gale.
        return c_light_gray;
    }
    if( beaufort <= 9 ) {
        // Beaufort 8–9: gale, strong gale.
        return c_white;
    }
    // Beaufort 10+: whole gale ("storm"), violent storm, hurricane.
    return c_yellow;
}

D::cell<1> wind_cell( const avatar &u, const weather_manager &wx )
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

    // Hide entirely when the wind isn't strong enough to blow out a
    // candle. Matches the `WIND_EXTINGUISH` rule in `item.cpp`.
    if( windpower <= 5 ) {
        return {};
    }

    const int beaufort = wind_beaufort_number( windpower );
    return D::cell{
        D::segment{
            string_format( "%s%s", NF_WEATHER_WIND_BEAUFORT[beaufort],
                           wind_direction_glyph( wx.winddirection ) ),
            wind_beaufort_color( beaufort ),
        },
    };
}

} // namespace

namespace gauntbar::ambient
{

void draw( const draw_args &args )
{
    namespace L = gauntbar::layout;
    namespace D = gauntbar::draw;

    const catacurses::window &w = args._win;
    const avatar &u = args._ava;
    weather_manager &wx = get_weather();

    werase( w );

    D::draw_cell( w, 0, L::left_col,          D::anchor::left,   light_cell( u ) );
    D::draw_cell( w, 0, L::quarter_col,       D::anchor::centre, temp_cell( u, wx ) );
    D::draw_cell( w, 0, L::three_quarter_col, D::anchor::centre, weather_cell( u, wx ) );
    D::draw_cell( w, 0, L::right_col,         D::anchor::right,  wind_cell( u, wx ) );

    wnoutrefresh( w );
}

} // namespace gauntbar::ambient
