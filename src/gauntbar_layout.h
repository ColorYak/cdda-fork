#pragma once
#ifndef CATA_SRC_GAUNTBAR_LAYOUT_H
#define CATA_SRC_GAUNTBAR_LAYOUT_H

#include "gauntbar.h"

// Named x positions derived from `gauntbar::width`. A palette, not a
// grid: rows pick lanes from here, but cross-row alignment is by
// convention rather than enforcement.

namespace gauntbar::layout
{

// 1-column inset on each side. No engine requirement; purely visual
// breathing room against the sidebar's vertical neighbours.
constexpr int left_col = 1;
constexpr int right_col = gauntbar::width - 2;
constexpr int centre_col = gauntbar::width / 2;
constexpr int quarter_col = gauntbar::width / 4;
constexpr int three_quarter_col = ( gauntbar::width * 3 ) / 4;

} // namespace gauntbar::layout

#endif // CATA_SRC_GAUNTBAR_LAYOUT_H
