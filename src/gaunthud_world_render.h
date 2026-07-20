#pragma once
#ifndef CATA_SRC_GAUNTHUD_WORLD_RENDER_H
#define CATA_SRC_GAUNTHUD_WORLD_RENDER_H

#include <string>

#include "gaunthud_cells.h"

namespace gaunthud::world
{

// Per-field convention: an empty / zero / `unknown` sentinel
// collapses the cell, and the surrounding row closes around it.
// Per-struct comments only call out the sentinel where it isn't
// the default-constructed value, or where collapse has a
// non-obvious knock-on (wind below).

// Pre-formatted at extract time because the 12h/24h choice depends
// on the `24_HOUR` option that the render path can't read.
struct time_data {
    std::string text;
};

// Local enum mirrored from the engine's `season_type` so the
// render TU stays storybook-linkable (no `calendar.h` dep).
enum class season_kind {
    unknown,
    spring,
    summer,
    autumn,
    winter,
};

// `day` is 1-indexed; the engine's `day_of_season` is 0-indexed
// and extract adds the +1 so render can render the value
// unmodified.
struct date_data {
    season_kind season = season_kind::unknown;
    int day = 0;
};

// Continuous vision modifier from
// `Character::fine_detail_vision_mod()`: ~1.0 bright, climbing
// through ~5 = effectively blind.
struct light_data {
    float vision_mod = 1.0f;
};

enum class temp_unit {
    fahrenheit,
    celsius,
    kelvin,
};

// `value` is pre-converted to `unit` at extract time because the
// unit choice depends on the `USE_CELSIUS` option that render
// can't read.
struct temp_data {
    bool has_thermometer = false;
    double value = 0.0;
    temp_unit unit = temp_unit::celsius;
};

// Both `id` and `name` are carried so render can fall back on the
// pre-translated `name` when `id` has no entry in its glyph table
// — keeps unknown weather (typically mods) renderable.
struct weather_data {
    std::string id;
    std::string name;
};

// Two-stage gate: `present = false` collapses regardless of
// windpower (e.g. sheltered). When present, render applies its
// own low-windpower cutoff as a visual choice.
struct wind_data {
    bool present = false;
    double local_windpower = 0.0;
    int direction = 0;
};

struct world_data {
    time_data time;
    date_data date;
    light_data light;
    temp_data temp;
    weather_data weather;
    wind_data wind;
};

cells::render_result render( const world_data &d );

} // namespace gaunthud::world

#endif // CATA_SRC_GAUNTHUD_WORLD_RENDER_H
