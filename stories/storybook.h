#pragma once
#ifndef CATA_STORIES_STORYBOOK_H
#define CATA_STORIES_STORYBOOK_H

// Shared tooling for the storybook renderer.
//
// Owns SDL/TTF/IMG init, the cell-buffer → PNG raster, and the
// index.html writer. Engine cold-start (palette, colour pairs,
// font load, render-time options) is delegated to
// `storybook_shims.h`. Story modules call into here; they don't
// touch SDL directly.

#include <filesystem>
#include <string>
#include <vector>

namespace catacurses
{
class window;
} // namespace catacurses

namespace storybook
{

// Per-module render context. `out_dir` is where the story module
// writes its PNGs and index.html. Created and populated by main.cpp,
// not by the story module — story modules stay opinion-free about
// where their output lands.
struct config {
    std::filesystem::path out_dir;
};

// One-time tooling setup. Initialises SDL_image, SDL_ttf, opens the
// font at `font_path`, and cold-starts just enough of the engine
// (`init_colors`, `wide_pua_glyphs = true`) to make `wprintz`
// populate the curses cell buffer correctly. Exits the process with
// a diagnostic on failure — the storybook has no fallback chain by
// design.
void init( const char *font_path );

// Counterpart to `init`. Closes the font, quits the SDL subsystems.
void shutdown();

// Scrape the cell buffer of `w`, raster each cell with the loaded
// font and palette, and write the result as a PNG to `png`. Returns
// false (with a diagnostic to stderr) on failure; story modules
// typically exit on false.
bool render_window_to_png( const catacurses::window &w,
                           const std::filesystem::path &png );

// One entry in an index.html section. `png_name` is the basename
// the browser sees in `<img src=...>` (relative to the html file).
struct entry {
    std::string slug;
    std::string label;
    std::string png_name;
};

struct section {
    std::string heading;
    std::vector<entry> entries;
};

// Write `out_dir/index.html` listing the given sections under `title`.
//
// The page includes a small JS shim that rescales each `<img>` to
// `naturalWidth / devicePixelRatio` so HiDPI displays render the
// PNGs at one device pixel per source pixel — matching the in-game
// sidebar rather than the OS's blown-up 2× / 1.5× CSS pixels.
void write_index_html( const std::filesystem::path &out_dir,
                       const std::string &title,
                       const std::vector<section> &sections );

} // namespace storybook

#endif // CATA_STORIES_STORYBOOK_H
