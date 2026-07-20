#include "gaunthud_safemode_render.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "color.h"
#include "gaunthud_cells.h"
#include "gaunthud_layout.h"

namespace gaunthud::safemode
{

namespace
{

// Powerline Extra — shaped separators
inline constexpr const char *NF_PLE_ICE_WAVEFORM_MIRRORED = u8"\uE0CA";
inline constexpr const char *NF_PLE_ICE_WAVEFORM          = u8"\uE0C8";

// Material Design Icons via Nerd Fonts
inline constexpr const char *NF_MD_SHIELD_OFF   = u8"\U000F099E";
inline constexpr const char *NF_MD_SHIELD_ALERT = u8"\U000F0ECC";

// 8-step pie countdown (Material Design circle_slice_1 … circle_slice_8)
inline constexpr std::array<const char *, 8> NF_MD_PIE = { {
        u8"\U000F0A9E", // circle_slice_1
        u8"\U000F0A9F", // circle_slice_2
        u8"\U000F0AA0", // circle_slice_3
        u8"\U000F0AA1", // circle_slice_4
        u8"\U000F0AA2", // circle_slice_5
        u8"\U000F0AA3", // circle_slice_6
        u8"\U000F0AA4", // circle_slice_7
        u8"\U000F0AA5", // circle_slice_8
    }
};

int pie_index( int elapsed, int threshold )
{
    if( threshold <= 0 || elapsed >= threshold ) {
        return -1;
    }
    const int remaining = threshold - elapsed;
    const int idx = ( ( remaining - 1 ) * 8 ) / threshold;
    return std::clamp( idx, 0, 7 );
}

cells::render_result render_off( const safemode_data &d )
{
    std::string content = std::string( NF_MD_SHIELD_OFF ) + " Safe Mode off";

    layout::packer p;
    p.push( cells::cell{
        cells::segment{ NF_PLE_ICE_WAVEFORM_MIRRORED, c_red },
        cells::segment{ std::move( content ), i_red },
        cells::segment{ NF_PLE_ICE_WAVEFORM_MIRRORED, i_red },
    } );
    if( d.autosafemode && d.turns_threshold > 0 ) {
        const int pi = pie_index( d.turns_elapsed, d.turns_threshold );
        if( pi >= 0 ) {
            p.push( cells::cell{
                cells::segment{ NF_MD_PIE[pi], c_dark_gray, /*peripheral=*/true },
            } );
        }
    }
    return std::move( p ).build();
}

cells::render_result render_stop( const safemode_data &d )
{
    std::string content = std::string( NF_MD_SHIELD_ALERT ) + " " + d.spotted;

    layout::packer p;
    p.push( cells::cell{
        cells::segment{ NF_PLE_ICE_WAVEFORM_MIRRORED, c_yellow },
        cells::segment{ std::move( content ), i_yellow },
        cells::segment{ NF_PLE_ICE_WAVEFORM_MIRRORED, i_yellow },
    } );
    return std::move( p ).build();
}

} // namespace

cells::render_result render( const safemode_data &d )
{
    switch( d.state ) {
        case mode::off:
            return render_off( d );
        case mode::stop:
            return render_stop( d );
        case mode::on:
            return { cells::extent{ 0, 0 }, {} };
    }
    return { cells::extent{ 0, 0 }, {} };
}

} // namespace gaunthud::safemode
