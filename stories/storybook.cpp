#include "storybook.h"

#include "storybook_shims.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "catacharset.h"   // utf8_width, UTF8_getch
#include "cursesdef.h"     // catacurses::window
#include "cursesport.h"    // cata_cursesport::WINDOW, cursecell, base_color

namespace
{

using storybook::shims::FONT_HEIGHT;
using storybook::shims::FONT_WIDTH;
using storybook::shims::sdl_color;

// === Tooling state ===

TTF_Font *g_font = nullptr;

// === Cell buffer -> SDL surface ===

SDL_Surface *blank_surface( int w_px, int h_px )
{
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
                            0, w_px, h_px, 32, SDL_PIXELFORMAT_RGBA32 );
    if( !surf ) {
        std::fprintf( stderr, "SDL_CreateRGBSurfaceWithFormat: %s\n", SDL_GetError() );
        std::exit( 1 );
    }
    // Match the engine's black background (sdltiles.cpp skips BG fills
    // for base_color::black; we paint it once up front instead).
    const SDL_Color bg = sdl_color( cata_cursesport::base_color::black );
    SDL_FillRect( surf, nullptr,
                  SDL_MapRGBA( surf->format, bg.r, bg.g, bg.b, bg.a ) );
    return surf;
}

void render_cell( SDL_Surface *surf, TTF_Font *font, int row, int col,
                  const cata_cursesport::cursecell &cell )
{
    // Wide character continuation: the first cell's TTF render already
    // covers both column widths. Skip.
    if( cell.ch.empty() ) {
        return;
    }

    int cw = utf8_width( cell.ch );
    if( cw < 1 ) {
        cw = 1;
    }

    SDL_Rect dest{
        col * FONT_WIDTH,
        row * FONT_HEIGHT,
        cw * FONT_WIDTH,
        FONT_HEIGHT
    };

    // Non-black background: paint the cell rect explicitly (black is
    // already covered by the surface fill in `blank_surface`).
    if( cell.BG != cata_cursesport::base_color::black ) {
        const SDL_Color bg = sdl_color( cell.BG );
        SDL_FillRect( surf, &dest,
                      SDL_MapRGBA( surf->format, bg.r, bg.g, bg.b, bg.a ) );
    }

    // The engine default-constructs `cursecell` with `ch = " "`, so
    // every untouched cell lands here. Skip the TTF round-trip; the
    // BG paint above (or `blank_surface`) is the whole render.
    if( cell.ch == " " ) {
        return;
    }

    const SDL_Color fg = sdl_color( cell.FG );
    SDL_Surface *glyph = TTF_RenderUTF8_Blended( font, cell.ch.c_str(), fg );
    if( !glyph ) {
        // Tofu / missing glyph: log and skip.
        std::fprintf( stderr, "TTF_RenderUTF8_Blended failed for U+%04X (%s): %s\n",
                      UTF8_getch( cell.ch ), cell.ch.c_str(), TTF_GetError() );
        return;
    }
    // Centre the glyph in `dest`, cropping symmetrically when it overflows.
    // Mirrors `CachedTTFFont::create_glyph` in `sdl_font.cpp`; see the
    // comment there for why odd-pixel overflow drops the leading edge.
    SDL_Rect src{ 0, 0, glyph->w, glyph->h };
    SDL_Rect dst = dest;
    if( src.w < dst.w ) {
        dst.x += ( dst.w - src.w ) / 2;
        dst.w = src.w;
    } else if( src.w > dst.w ) {
        src.x = ( src.w - dst.w + 1 ) / 2;
        src.w = dst.w;
    }
    if( src.h < dst.h ) {
        dst.y += ( dst.h - src.h ) / 2;
        dst.h = src.h;
    } else if( src.h > dst.h ) {
        src.y = ( src.h - dst.h + 1 ) / 2;
        src.h = dst.h;
    }
    SDL_BlitSurface( glyph, &src, surf, &dst );
    SDL_FreeSurface( glyph );
}

SDL_Surface *surface_from_window( const catacurses::window &w, TTF_Font *font )
{
    cata_cursesport::WINDOW *win = static_cast<cata_cursesport::WINDOW *>( w.get() );

    SDL_Surface *surf = blank_surface( win->width * FONT_WIDTH,
                                       win->height * FONT_HEIGHT );

    for( int y = 0; y < win->height && static_cast<size_t>( y ) < win->line.size(); ++y ) {
        const auto &line = win->line[y].chars;
        for( int x = 0; x < win->width && static_cast<size_t>( x ) < line.size(); ++x ) {
            render_cell( surf, font, y, x, line[x] );
        }
    }
    return surf;
}

} // namespace

namespace storybook
{

void init( const char *font_path )
{
    if( TTF_Init() != 0 ) {
        std::fprintf( stderr, "TTF_Init: %s\n", TTF_GetError() );
        std::exit( 1 );
    }
    if( ( IMG_Init( IMG_INIT_PNG ) & IMG_INIT_PNG ) == 0 ) {
        std::fprintf( stderr, "IMG_Init: %s\n", IMG_GetError() );
        std::exit( 1 );
    }

    g_font = shims::open_font( font_path );
    shims::apply_engine_options();
    shims::init_color_pairs();
}

void shutdown()
{
    if( g_font ) {
        TTF_CloseFont( g_font );
        g_font = nullptr;
    }
    IMG_Quit();
    TTF_Quit();
}

bool paint_window_to_png( const catacurses::window &w,
                          const std::filesystem::path &png )
{
    SDL_Surface *surf = surface_from_window( w, g_font );
    const int rc = IMG_SavePNG( surf, png.string().c_str() );
    SDL_FreeSurface( surf );
    if( rc != 0 ) {
        std::fprintf( stderr, "IMG_SavePNG '%s': %s\n",
                      png.string().c_str(), IMG_GetError() );
        return false;
    }
    return true;
}

void write_index_html( const std::filesystem::path &out_dir,
                       const std::string &title,
                       const std::vector<section> &sections )
{
    std::ofstream out( out_dir / "index.html" );
    out << "<!doctype html>\n<html><head><meta charset=\"utf-8\">\n";
    out << "<title>" << title << "</title>\n";
    out << "<style>\n"
        "  body { background: #0d0d0e; color: #f3f3f3; "
        "         font-family: sans-serif; margin: 0; padding: 1.5rem; }\n"
        "  h1 { font-size: 1.1rem; font-weight: 400; "
        "       border-bottom: 1px solid #444; padding-bottom: 0.3rem; }\n"
        "  h2 { font-size: 0.85rem; font-weight: 400; color: #888; "
        "       margin: 1.2rem 0 0.3rem 0; }\n"
        "  img { display: block; image-rendering: pixelated; "
        "        margin-bottom: 0.4rem; }\n"
        "</style>\n";
    // Match the in-game render size on HiDPI displays. Without this,
    // the browser uses each PNG pixel as one CSS pixel, which the
    // device pixel ratio then blows up by 2× (or 1.5×, etc.), so a
    // sidebar that's 22px wide per cell shows up at 44px+ on screen.
    // Setting an explicit CSS width of `naturalWidth / dpr` collapses
    // the PNG back to one device pixel per source pixel.
    out << "<script>\n"
        "  function fitToDevicePixels() {\n"
        "    const dpr = window.devicePixelRatio || 1;\n"
        "    for (const img of document.images) {\n"
        "      const apply = () => {\n"
        "        img.style.width  = (img.naturalWidth  / dpr) + 'px';\n"
        "        img.style.height = (img.naturalHeight / dpr) + 'px';\n"
        "      };\n"
        "      if (img.complete) apply(); else img.addEventListener('load', apply);\n"
        "    }\n"
        "  }\n"
        "  window.addEventListener('DOMContentLoaded', fitToDevicePixels);\n"
        "  window.matchMedia('(resolution: 1dppx)')\n"
        "        .addEventListener('change', fitToDevicePixels);\n"
        "</script>\n";
    out << "</head><body>\n";

    for( const section &sec : sections ) {
        out << "<h1>" << sec.heading << "</h1>\n";
        for( const entry &e : sec.entries ) {
            out << "<h2>" << e.label << "</h2>\n";
            out << "<img src=\"" << e.png_name << "\" alt=\"" << e.label << "\">\n";
        }
    }

    out << "</body></html>\n";
}

} // namespace storybook
