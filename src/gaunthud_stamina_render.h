#pragma once
#ifndef CATA_SRC_GAUNTHUD_STAMINA_RENDER_H
#define CATA_SRC_GAUNTHUD_STAMINA_RENDER_H

#include "gaunthud_cells.h"
#include "move_mode.h"

namespace gaunthud::stamina
{

struct stamina_data {
    int current = 0;
    int previous = 0;
    int max = 0;
    move_mode_type mode = move_mode_type::WALKING;
};

inline constexpr int bar_cells = 20;

cells::render_result render( const stamina_data &d );

} // namespace gaunthud::stamina

#endif // CATA_SRC_GAUNTHUD_STAMINA_RENDER_H
