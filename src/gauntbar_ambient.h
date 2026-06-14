#pragma once
#ifndef CATA_SRC_GAUNTBAR_AMBIENT_H
#define CATA_SRC_GAUNTBAR_AMBIENT_H

#include "gauntbar_ambient_render.h" // IWYU pragma: export

struct draw_args;

namespace gauntbar::ambient
{

void draw( const draw_args &args );

} // namespace gauntbar::ambient

#endif // CATA_SRC_GAUNTBAR_AMBIENT_H
