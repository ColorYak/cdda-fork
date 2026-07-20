#pragma once
#ifndef CATA_SRC_GAUNTHUD_MAP_H
#define CATA_SRC_GAUNTHUD_MAP_H

#include "gaunthud_hud.h"

class avatar;

namespace gaunthud::map
{

// Shared with the place readout, which sits directly under the
// map in the TR stack and centres its content within the same
// column width.
constexpr int map_width = 30;

// TILES-only readout: in non-TILES builds the corner table omits
// `map::prepare`, so the function body is also gated. Declaration
// stays visible so `gaunthud.cpp` can include unconditionally.
frame prepare( const avatar &u );

} // namespace gaunthud::map

#endif // CATA_SRC_GAUNTHUD_MAP_H
