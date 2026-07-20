#include "gaunthud_stamina_render.h"

#include <algorithm>
#include <string>
#include <utility>

#include "color.h"
#include "gaunthud_cells.h"
#include "gaunthud_layout.h"
#include "move_mode.h"

namespace
{

inline constexpr const char *NF_EXTRA_PROGRESS_FULL_LEFT   = u8"\uEE03";
inline constexpr const char *NF_EXTRA_PROGRESS_FULL_MID    = u8"\uEE04";
inline constexpr const char *NF_EXTRA_PROGRESS_FULL_RIGHT  = u8"\uEE05";
inline constexpr const char *NF_EXTRA_PROGRESS_EMPTY_LEFT  = u8"\uEE00";
inline constexpr const char *NF_EXTRA_PROGRESS_EMPTY_MID   = u8"\uEE01";
inline constexpr const char *NF_EXTRA_PROGRESS_EMPTY_RIGHT = u8"\uEE02";

// Movement mode icons (Material Design Icons via Nerd Fonts)
inline constexpr const char *NF_FA_CROSSHAIRS  = u8"\uF05B";     // prone
inline constexpr const char *NF_MD_DOMINO_MASK = u8"\U000F1023"; // crouch
inline constexpr const char *NF_MD_WALK        = u8"\U000F0583"; // walk
inline constexpr const char *NF_MD_RUN_FAST    = u8"\U000F046E"; // run

struct mode_style {
    const char *glyph;
    nc_color color;
};

mode_style style_for_mode( move_mode_type mode )
{
    switch( mode ) {
        case move_mode_type::PRONE:
            return { NF_FA_CROSSHAIRS, c_magenta };
        case move_mode_type::CROUCHING:
            return { NF_MD_DOMINO_MASK, c_cyan };
        case move_mode_type::WALKING:
            return { NF_MD_WALK, c_light_gray };
        case move_mode_type::RUNNING:
            return { NF_MD_RUN_FAST, c_red };
    }
    return { NF_MD_WALK, c_white };
}

// Quantise a stamina value to filled cells. Returns [0, bar_cells].
int to_filled( int value, int max )
{
    if( value <= 0 || max <= 0 ) {
        return 0;
    }
    const int cells = gaunthud::stamina::bar_cells;
    const int f = ( ( static_cast<long long>( value ) * cells ) + ( max / 2 ) ) / max;
    return std::clamp( f, 0, cells );
}

nc_color bar_color( const gaunthud::stamina::stamina_data &d )
{
    const bool low = d.current * 4 <= d.max; // ≤ 25%
    const bool losing = d.current < d.previous;
    if( low ) {
        return losing ? c_light_red : c_red;
    }
    return losing ? c_light_blue : c_blue;
}

} // namespace

namespace gaunthud::stamina
{

cells::render_result render( const stamina_data &d )
{
    if( d.max <= 0 ) {
        return { cells::extent{ 0, 0 }, {} };
    }

    const int filled = to_filled( d.current, d.max );
    const mode_style ms = style_for_mode( d.mode );

    std::string bar;
    for( int c = 0; c < bar_cells; ++c ) {
        const bool is_filled = c < filled;
        if( c == 0 ) {
            bar += is_filled ? NF_EXTRA_PROGRESS_FULL_LEFT : NF_EXTRA_PROGRESS_EMPTY_LEFT;
        } else if( c == bar_cells - 1 ) {
            bar += is_filled ? NF_EXTRA_PROGRESS_FULL_RIGHT : NF_EXTRA_PROGRESS_EMPTY_RIGHT;
        } else {
            bar += is_filled ? NF_EXTRA_PROGRESS_FULL_MID : NF_EXTRA_PROGRESS_EMPTY_MID;
        }
    }

    const int pct = d.max > 0 ? ( ( d.current * 100 ) + ( d.max / 2 ) ) / d.max : 0;

    layout::packer p;
    p.push( cells::cell{
        cells::segment{ ms.glyph, ms.color, /*peripheral=*/true },
        cells::segment{ std::move( bar ), bar_color( d ) },
    } );
    if( filled < bar_cells ) {
        p.push( cells::cell{
            cells::segment{ std::to_string( pct ) + "%", c_dark_gray, /*peripheral=*/true },
        } );
    }
    return std::move( p ).build();
}

} // namespace gaunthud::stamina
