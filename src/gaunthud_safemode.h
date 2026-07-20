#pragma once
#ifndef CATA_SRC_GAUNTHUD_SAFEMODE_H
#define CATA_SRC_GAUNTHUD_SAFEMODE_H

#include "gaunthud_safemode_render.h" // IWYU pragma: export
#include "gaunthud_hud.h"

class avatar;

namespace gaunthud::safemode
{

safemode_data extract( const avatar &u );
frame prepare( const avatar &u );

} // namespace gaunthud::safemode

#endif // CATA_SRC_GAUNTHUD_SAFEMODE_H
