#pragma once
#ifndef GAUNTBAR_STORIES_H
#define GAUNTBAR_STORIES_H

#include "storybook.h"

// GauntBar story module. Renders one PNG per hand-authored
// `ambient_data` permutation and writes an index.html alongside.

namespace gauntbar_story
{

// Render every gauntbar story into `cfg.out_dir`. Exits the process
// on failure (mirrors the rest of the storybook — no fallbacks).
void run( const storybook::config &cfg );

} // namespace gauntbar_story

#endif // GAUNTBAR_STORIES_H
