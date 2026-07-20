#pragma once
#ifndef CATA_SRC_GAUNTHUD_PLACE_RENDER_H
#define CATA_SRC_GAUNTHUD_PLACE_RENDER_H

#include <string>

#include "gaunthud_cells.h"

namespace gaunthud::place
{

struct place_data {
    std::string name;
    std::string coords;
};

cells::render_result render( const place_data &d );

} // namespace gaunthud::place

#endif // CATA_SRC_GAUNTHUD_PLACE_RENDER_H
