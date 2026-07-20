// Real reimplementations of symbols whose upstream TUs the storybook
// deliberately doesn't link. See `CMakeLists.txt` (next to
// `GAUNTHUD_RENDER_SUPPORT_SOURCES`) for the not-linked list and
// rationale, and `dead_stubs.cpp` for the silent-abort stubs that
// exist purely to satisfy the linker.

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "catacharset.h"
#include "color.h"
#include "cursesdef.h"
#include "cursesport.h"
#include "game_ui.h"
#include "output.h"
#include "point.h"

// === SDL / UI no-ops (cursesport.cpp's refresh path) ===

namespace cata_cursesport
{
// Real impl in sdltiles.cpp blits the cell buffer to the SDL
// renderer. Storybook scrapes the cell buffer directly into its
// own SDL_Surface; the on-screen renderer is never created.
void curses_drawwindow( const catacurses::window & /*w*/ )
{
}
} // namespace cata_cursesport

// Real impl in sdltiles.cpp swaps the SDL renderer's back buffer.
void refresh_display()
{
}

namespace game_ui
{
// Real impl in game_ui.cpp resizes catacurses windows after a window
// resize / font change. Unreachable from the render path.
void init_ui()
{
}
} // namespace game_ui

namespace catacurses
{
// Real impl in `sdltiles.cpp` returns false ("trust SDL to do the
// right thing instead"); `color_manager::load_default()` branches
// on this to pick between two layouts for the dark-gray colour
// pairs. Mirror the SDL build so storybook PNGs match the in-game
// rendering byte-for-byte.
bool supports_256_colors()
{
    return false;
}
} // namespace catacurses

// === Engine globals (read but never meaningful) ===
//
// `cursesport::newwin` reads TERMX/TERMY only when ncols/nlines == 0
// (to derive a full-screen window). The storybook always passes
// explicit dimensions, so the reads are dead. The FULL_SCREEN_*
// globals are referenced from `color_manager::show_gui`, which is
// linked-but-unreachable.

int TERMX = 0;
int TERMY = 0;
int FULL_SCREEN_WIDTH = 0;
int FULL_SCREEN_HEIGHT = 0;

// === Output helpers (tiny copies from output.cpp) ===

void wprintz( const catacurses::window &w, const nc_color &FG, const std::string &text )
{
    wattron( w, FG );
    wprintw( w, text );
    wattroff( w, FG );
}

void mvwprintz( const catacurses::window &w, const point &p, const nc_color &FG,
                const std::string &text )
{
    wattron( w, FG );
    mvwprintw( w, p, text );
    wattroff( w, FG );
}

// `cursesport::mvwaddch` / `mvwhline` route single chars through
// this for box-drawing characters. The storybook only emits plain
// glyphs via `wprintz`, but the path is reachable through
// `wprintw`'s width-1 fast path. The trimmed version skips the
// LINE_XOXO_C remapping — the storybook doesn't draw borders, so
// the special `LINE_*` constants (0x100000 + n) should never reach
// here. Assert that to catch silent UTF-8 truncation if a story
// ever pulls in a bordered window.
std::string string_from_int( const catacurses::chtype ch )
{
    assert( ch < 0x80 );
    const char c = static_cast<char>( ch );
    return std::string( 1, c );
}

// Render path uses `string_format("%d°C", ...)` and friends, which
// land here on the format-once path. Trimmed copy of the
// non-Windows, non-Cygwin branch from `output.cpp::raw_string_format`.
// NOLINTNEXTLINE(cert-dcl50-cpp)
std::string cata::string_formatter::raw_string_format( const char *format, ... )
{
    va_list args;
    va_start( args, format );

    std::vector<char> buffer( 256, '\0' );
    for( ;; ) {
        va_list args_copy;
        va_copy( args_copy, args );
        const int result = vsnprintf( buffer.data(), buffer.size(), format, args_copy );
        va_end( args_copy );

        if( result < 0 ) {
            va_end( args );
            throw std::runtime_error(
                std::string( "Bad format string for printf: \"" ) + format + "\"" );
        }
        if( static_cast<std::size_t>( result ) < buffer.size() ) {
            break;
        }
        buffer.resize( static_cast<std::size_t>( result ) + 1 );
    }

    va_end( args );
    return std::string( buffer.data() );
}

// `catacharset.cpp::utf8_width` strips color tags before measuring;
// the storybook doesn't emit `<color_*>` markup so passing through
// unchanged is exact.
std::string remove_color_tags( std::string_view s )
{
    return std::string( s );
}

// === UTF helpers ===
//
// `catacharset.cpp` defines the `u32string_view` overload of
// `utf32_to_utf8` but delegates the single-codepoint case to the
// overload defined in `json.cpp`. We don't link `json.cpp`, so
// inline the small definition here.

std::string utf32_to_utf8( std::uint32_t ch )
{
    char out[5];
    char *buf = out;
    static const unsigned char utf8FirstByte[7] = {
        0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
    };
    int utf8Bytes;
    if( ch < 0x80 ) {
        utf8Bytes = 1;
    } else if( ch < 0x800 ) {
        utf8Bytes = 2;
    } else if( ch < 0x10000 ) {
        utf8Bytes = 3;
    } else if( ch <= 0x10FFFF ) {
        utf8Bytes = 4;
    } else {
        // Invalid codepoint — return U+FFFD.
        return "\xEF\xBF\xBD";
    }

    buf += utf8Bytes;
    switch( utf8Bytes ) {
        case 4:
            *--buf = ( ch | 0x80 ) & 0xBF;
            ch >>= 6;
            [[fallthrough]];
        case 3:
            *--buf = ( ch | 0x80 ) & 0xBF;
            ch >>= 6;
            [[fallthrough]];
        case 2:
            *--buf = ( ch | 0x80 ) & 0xBF;
            ch >>= 6;
            [[fallthrough]];
        case 1:
            *--buf = ch | utf8FirstByte[utf8Bytes];
    }
    return std::string( buf, utf8Bytes );
}

// === Debug ===
//
// The render path shouldn't hit `realDebugmsg` — when it does, that's
// a bug worth surfacing immediately. Print to stderr and abort; no
// modal popup (which requires the cataimgui stack).

void realDebugmsg( const char *filename, const char *line, const char *funcname,
                   const std::string &text )
{
    std::fprintf( stderr,
                  "[storybook] debugmsg from %s:%s in %s: %s\n",
                  filename, line, funcname, text.c_str() );
    std::abort();
}

// === Output / UI helpers used only by dead show_gui ===
//
// These take engine types (catacurses::window, nc_color, point) so
// they live alongside the real engine includes.

void draw_border( const catacurses::window &, nc_color, const std::string &, nc_color )
{
    std::fprintf( stderr, "[storybook] dead-code stub `draw_border` was called.\n" );
    std::abort();
}

std::size_t shortcut_print( const catacurses::window &, const point &, nc_color, nc_color,
                            const std::string & )
{
    std::fprintf( stderr, "[storybook] dead-code stub `shortcut_print` was called.\n" );
    std::abort();
}

void mvwrectf( const catacurses::window &, const point &, const nc_color &, int, int, int )
{
    std::fprintf( stderr, "[storybook] dead-code stub `mvwrectf` was called.\n" );
    std::abort();
}

void mvwvline( const catacurses::window &, const point &, const nc_color &, int, int )
{
    std::fprintf( stderr, "[storybook] dead-code stub `mvwvline` was called.\n" );
    std::abort();
}

void calcStartPos( int &, int, int, int )
{
    std::fprintf( stderr, "[storybook] dead-code stub `calcStartPos` was called.\n" );
    std::abort();
}

void draw_scrollbar( const catacurses::window &, int, int, int, const point &, nc_color, bool )
{
    std::fprintf( stderr, "[storybook] dead-code stub `draw_scrollbar` was called.\n" );
    std::abort();
}
