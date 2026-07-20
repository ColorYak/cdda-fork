#include "gaunthud_stories.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "cursesdef.h"
#include "gaunthud_cells.h"
#include "move_mode.h"
#include "point.h"
#include "storybook.h"

namespace
{

namespace fs = std::filesystem;

} // namespace

namespace gaunthud_story
{

void run( const storybook::config &cfg )
{
    fs::create_directories( cfg.out_dir );

    // Placeholder section for full-sidebar stories — currently no
    // entries, since only the world zone is implemented.
    storybook::section full_section{ "Full sidebar states", {} };

    storybook::write_index_html( cfg.out_dir,
                                 "GauntHUD storybook",
                                 { world_section, stamina_section, safemode_section, full_section } );
    std::printf( "Wrote %s\n",
                 ( cfg.out_dir / "index.html" ).string().c_str() );
}

} // namespace gaunthud_story
