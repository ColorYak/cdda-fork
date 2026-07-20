#pragma once
#ifndef CATA_SRC_GAUNTHUD_WORLD_H
#define CATA_SRC_GAUNTHUD_WORLD_H

#include "gaunthud_world_render.h" // IWYU pragma: export
#include "gaunthud_hud.h"

class avatar;
class weather_manager;

namespace gaunthud::world
{

world_data extract( const avatar &u, weather_manager &wx );
frame prepare( const avatar &u );

} // namespace gaunthud::world

#endif // CATA_SRC_GAUNTHUD_WORLD_H
