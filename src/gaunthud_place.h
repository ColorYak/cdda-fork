#pragma once
#ifndef CATA_SRC_GAUNTHUD_PLACE_H
#define CATA_SRC_GAUNTHUD_PLACE_H

#include "gaunthud_place_render.h" // IWYU pragma: export
#include "gaunthud_hud.h"

class avatar;

namespace gaunthud::place
{

place_data extract( const avatar &u );
frame prepare( const avatar &u );

} // namespace gaunthud::place

#endif // CATA_SRC_GAUNTHUD_PLACE_H
