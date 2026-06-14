#pragma once
#ifndef CATA_SRC_GAUNTBAR_AMBIENT_RENDER_H
#define CATA_SRC_GAUNTBAR_AMBIENT_RENDER_H

#include <string>

namespace catacurses
{
class window;
} // namespace catacurses

namespace gauntbar::ambient
{

// Light level, as `Character::fine_detail_vision_mod()` returns it:
// 1.0 = bright, up through ~5 = effectively blind. Render buckets
// the continuous value into glyph tiers internally.
struct light_data {
    float vision_mod = 1.0f;
};

enum class temp_unit {
    fahrenheit,
    celsius,
    kelvin,
};

// `value` is already converted to `unit`; the unit choice happens
// at extract time, since it depends on the `USE_CELSIUS` option.
// `has_thermometer = false` collapses the cell entirely (matching
// the engine rule that you can't read a temperature you can't
// measure).
struct temp_data {
    bool has_thermometer = false;
    double value = 0.0;
    temp_unit unit = temp_unit::celsius;
};

// Empty `id` collapses the cell (e.g. underground or sheltered).
// `name` is the pre-translated `weather_type` display name; render
// falls back to it when the id has no entry in the glyph table.
struct weather_data {
    std::string id;
    std::string name;
};

// `present = false` collapses the cell regardless of windpower
// (e.g. sheltered). When present, render still hides the cell when
// `local_windpower` is too low to be worth surfacing — that
// threshold is a visual choice, kept in render.
struct wind_data {
    bool present = false;
    double local_windpower = 0.0;
    int direction = 0;
};

struct ambient_data {
    light_data light;
    temp_data temp;
    weather_data weather;
    wind_data wind;
};

void render( const catacurses::window &w, const ambient_data &d );

} // namespace gauntbar::ambient

#endif // CATA_SRC_GAUNTBAR_AMBIENT_RENDER_H
