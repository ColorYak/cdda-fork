#pragma once
#ifndef CATA_SRC_GAUNTHUD_SAFEMODE_RENDER_H
#define CATA_SRC_GAUNTHUD_SAFEMODE_RENDER_H

#include <string>

#include "gaunthud_cells.h"

namespace gaunthud::safemode
{

enum class mode {
    on,
    off,
    stop,
};

struct safemode_data {
    mode state = mode::on;

    // Auto-reactivation (only meaningful when state == off):
    bool autosafemode = false;
    int turns_elapsed = 0;  // turns since safe mode turned off
    int turns_threshold = 0; // AUTOSAFEMODETURNS option value

    // Spotted label (only meaningful when state == stop):
    std::string spotted;  // e.g. "Zombie spotted", "Enemies spotted"
};

cells::render_result render( const safemode_data &d );

} // namespace gaunthud::safemode

#endif // CATA_SRC_GAUNTHUD_SAFEMODE_RENDER_H
