#pragma once
#ifndef CATA_SRC_GAUNTBAR_H
#define CATA_SRC_GAUNTBAR_H

#include <map>
#include <string>

class panel_layout;

namespace gauntbar
{

// Layout key in panel_manager's layouts map.
constexpr const char *layout_id = "gauntbar";

// Sidebar width in columns. Content is authored against this width;
// changing it will reflow every cell.
constexpr int width = 44;

// Add the GauntBar layout to the given map. Call after the JSON-defined
// sidebar widgets have been loaded into it.
void register_layout( std::map<std::string, panel_layout> &layouts );

} // namespace gauntbar

#endif // CATA_SRC_GAUNTBAR_H
