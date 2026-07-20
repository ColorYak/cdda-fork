#include "storybook_shims.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <cstdio>
#include <cstdlib>

#include "color.h"         // all_colors, init_default_color_pairs
#include "cursesport.h"    // cata_cursesport::base_color
#include "wcwidth.h"       // pua_widths

namespace storybook::shims
{

namespace
{

// Hardcoded palette. In-game `windowsPalette` is filled from
// data/raw/colors.json (and the user's overrides in
// config/base_colors.json) via `color_loader`. The 16 colour names
// and their ordering match `color_loader::main_color_names()`.
struct rgb {
    Uint8 r, g, b;
};

constexpr std::array<rgb, 16> PALETTE = { {
        /*  0 BLACK    */ {  13,  13,  14 },
        /*  1 RED      */ { 230,  60,  60 },
        /*  2 GREEN    */ {  25, 143,  52 },
        /*  3 BROWN    */ { 125,  75,  45 },
        /*  4 BLUE     */ {  25,  45, 135 },
        /*  5 MAGENTA  */ { 110,  75, 140 },
        /*  6 CYAN     */ {  11, 143, 169 },
        /*  7 GRAY     */ { 160, 160, 160 },
        /*  8 DGRAY    */ { 110, 110, 110 },
        /*  9 LRED     */ { 240, 129, 129 },
        /* 10 LGREEN   */ {  91, 214,  71 },
        /* 11 YELLOW   */ { 235, 235,  75 },
        /* 12 LBLUE    */ {  64,  88, 235 },
        /* 13 LMAGENTA */ { 150,  70, 200 },
        /* 14 LCYAN    */ {  67, 225, 230 },
        /* 15 WHITE    */ { 243, 243, 243 },
    }
};

} // namespace

SDL_Color sdl_color( cata_cursesport::base_color c )
{
    const std::size_t idx = static_cast<std::size_t>( c ) & 0xff;
    const rgb p = idx < PALETTE.size() ? PALETTE[idx] : PALETTE[15];
    return { p.r, p.g, p.b, 255 };
}

TTF_Font *open_font( const char *path )
{
    TTF_Font *font = TTF_OpenFont( path, FONT_SIZE );
    if( !font ) {
        std::fprintf( stderr, "TTF_OpenFont '%s': %s\n", path, TTF_GetError() );
        std::exit( 1 );
    }
    return font;
}

void apply_engine_options()
{
    pua_widths = pua_width_mode::nerdfonts;
}

void init_color_pairs()
{
    init_default_color_pairs();
    all_colors.load_default();
}

} // namespace storybook::shims
