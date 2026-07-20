#include "gaunthud_place_render.h"

#include <string>
#include <utility>

#include "catacharset.h"
#include "color.h"
#include "gaunthud_cells.h"
#include "gaunthud_layout.h"
#include "gaunthud_map.h"
#include "output.h"

namespace
{

namespace C = gaunthud::cells;

C::cell<1> render_name_cell( const std::string &name, int budget )
{
    if( name.empty() || budget <= 0 ) {
        return {};
    }
    return C::cell{ C::segment{ trim_by_length( name, budget ), c_light_gray } };
}

C::cell<1> render_coords_cell( const std::string &coords )
{
    if( coords.empty() ) {
        return {};
    }
    return C::cell{ C::segment{ coords, c_dark_gray } };
}

} // namespace

namespace gaunthud::place
{

cells::render_result render( const place_data &d )
{
    const int coords_width = utf8_width( d.coords );
    const int name_budget = gaunthud::map::map_width - coords_width - 3; // margins and gap

    layout::centerer c( gaunthud::map::map_width );
    c.push( render_name_cell( d.name, name_budget ) );
    c.push( render_coords_cell( d.coords ) );
    return std::move( c ).build();
}

} // namespace gaunthud::place
