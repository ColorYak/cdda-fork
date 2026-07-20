// Storybook entry point. Runs every registered story module's
// `run`. Framework code lives in `storybook.{h,cpp}` and is
// consumer-agnostic; per-module stories live next to the engine
// code they exercise (e.g. `gaunthud_stories.{h,cpp}`).
//
// argv[1] is the output root. Each module writes its PNGs +
// index.html into a subdirectory under it.

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "gaunthud_stories.h"
#include "storybook.h"

int main( int argc, char **argv )
{
    if( argc < 2 ) {
        std::fprintf( stderr, "usage: %s <out_dir>\n", argv[0] );
        return 1;
    }
    const std::filesystem::path out_root = argv[1];

    storybook::init( STORYBOOK_FONT_PATH );

    // === Story modules ===
    // Add new modules in this block.
    gaunthud_story::run( { out_root / "gaunthud" } );

    storybook::shutdown();
    return 0;
}
