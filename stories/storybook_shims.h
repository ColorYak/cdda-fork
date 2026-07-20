#pragma once
#ifndef GAUNTHUD_STORYBOOK_SHIMS_H
#define GAUNTHUD_STORYBOOK_SHIMS_H

// Engine surrogates for the storybook — data and one-shot init that
// fakes the role of the engine's options / colour-pair / font setup.
//
// In the live game these values come from config files
// (`fonts.json`, `options.json`, `base_colors.json`) via
// `font_loader`, `options_manager`, and `color_loader`. The
// storybook bakes them in to avoid carrying those layers.
//
// Internal to the storybook target — not part of the public
// `storybook::` API.

#include <SDL.h>
#include <SDL_ttf.h>

#include "cursesport.h"

namespace storybook::shims
{

// Hardcoded font metrics. In-game these come from FONT_WIDTH /
// FONT_HEIGHT / FONT_SIZE options in options.json.
constexpr int FONT_WIDTH  = 22;
constexpr int FONT_HEIGHT = 43;
constexpr int FONT_SIZE   = 36;

// Translate a `cata_cursesport::base_color` (0..15) to an SDL_Color
// from the hardcoded palette. Falls back to white on out-of-range.
SDL_Color sdl_color( cata_cursesport::base_color c );

// Open the font at `FONT_SIZE`. Exits the process on failure.
// Returned font is owned by the caller.
TTF_Font *open_font( const char *path );

// Apply hardcoded equivalents of the engine options the render path
// reads at runtime. Currently just sets
// `pua_widths = pua_width_mode::nerdfonts` (in-game wired through
// the `FONT_PUA_WIDTHS` option) so Nerd Font icon glyphs occupy
// two cells.
void apply_engine_options();

// Fill `cata_cursesport::colorpairs[]` from the engine's default
// pair table, then call `all_colors.load_default()` to populate the
// `c_*` lookup macros. Together these are the minimum cold-start
// the render path needs from `color.cpp`.
//
// We can't just call `init_colors()` directly: it also seeds
// `color_by_string_map` (via `to_translation`, pulling the
// translations layer) and calls `all_colors.load_custom({})` (via
// the JSON loader). Neither is linked into the storybook.
void init_color_pairs();

} // namespace storybook::shims

#endif // GAUNTHUD_STORYBOOK_SHIMS_H
