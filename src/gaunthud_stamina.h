#pragma once
#ifndef CATA_SRC_GAUNTHUD_STAMINA_H
#define CATA_SRC_GAUNTHUD_STAMINA_H

#include "gaunthud_stamina_render.h" // IWYU pragma: export
#include "gaunthud_hud.h"

class avatar;

namespace gaunthud::stamina
{

stamina_data extract( const avatar &u );
frame prepare( const avatar &u );

} // namespace gaunthud::stamina

#endif // CATA_SRC_GAUNTHUD_STAMINA_H
