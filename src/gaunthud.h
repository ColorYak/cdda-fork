#pragma once
#ifndef CATA_SRC_GAUNTHUD_H
#define CATA_SRC_GAUNTHUD_H

#include <map>
#include <string>

class avatar;
class panel_layout;

namespace gaunthud
{

// Layout key in panel_manager's layouts map.
constexpr const char *layout_id = "gaunthud";

// Register the GauntHUD layout in the given map. Called from
// `initialize_default_panel_layouts()` after the JSON-defined
// sidebar widgets have been loaded.
void register_layout( std::map<std::string, panel_layout> &layouts );

// Paint every readout at its corner-derived origin. Called from
// `game::draw_panels` when the current layout is `layout_id`, in
// place of the sidebar column loop.
void paint_hud( const avatar &u );

} // namespace gaunthud

#endif // CATA_SRC_GAUNTHUD_H
