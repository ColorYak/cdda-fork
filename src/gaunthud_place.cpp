#include "gaunthud_place.h"

#include <string>
#include <tuple>
#include <utility>

#include "avatar.h"
#include "coordinates.h"
#include "cursesdef.h"
#include "display.h"
#include "gaunthud_cells.h"
#include "string_formatter.h"

namespace
{

constexpr const char *NF_MD_LAYERS = u8"\U000F0328";

std::string format_coords( const tripoint_abs_omt &loc )
{
    point_abs_omt abs_omt = loc.xy();
    point_abs_om om;
    point_om_omt omt;
    std::tie( om, omt ) = project_remain<coords::om>( abs_omt );
    return string_format( "%s%d %d'%d %d'%d",
                          NF_MD_LAYERS, loc.z(),
                          om.x(), omt.x(), om.y(), omt.y() );
}

} // namespace

namespace gaunthud::place
{

place_data extract( const avatar &u )
{
    const tripoint_abs_omt loc = u.pos_abs_omt();
    return {
        display::current_position_text( loc ),
        format_coords( loc ),
    };
}

frame prepare( const avatar &u )
{
    cells::render_result r = render( extract( u ) );
    if( r.size.width <= 0 || r.size.height <= 0 ) {
        return { cells::extent{ 0, 0 }, []( const catacurses::window & ) {} };
    }
    return {
        r.size,
        [buf = std::move( r.buffer )]( const catacurses::window &w ) {
            cells::paint( w, buf );
        },
    };
}

} // namespace gaunthud::place
