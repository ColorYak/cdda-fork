#pragma once
#ifndef GAUNTHUD_STORIES_H
#define GAUNTHUD_STORIES_H

#include "storybook.h"

// GauntHUD story module. Renders one PNG per hand-authored
// `world_data` permutation and writes an index.html alongside.

namespace gaunthud_story
{

// Render every gaunthud story into `cfg.out_dir`. Exits the process
// on failure (mirrors the rest of the storybook — no fallbacks).
void run( const storybook::config &cfg );

} // namespace gaunthud_story

#endif // GAUNTHUD_STORIES_H
