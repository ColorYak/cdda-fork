#pragma once
#ifndef CATA_SRC_GAUNTHUD_LAYOUT_H
#define CATA_SRC_GAUNTHUD_LAYOUT_H

#include <algorithm>
#include <cstddef>
#include <utility>

#include "catacharset.h"
#include "gaunthud_cells.h"
#include "point.h"

// Layout strategies. Each strategy turns a sequence of `cells::cell<N>`
// values into a `cell_buffer` + the `extent` it occupies. One strategy
// per usage pattern; a readout's `render` chooses which to use.

namespace gaunthud::layout
{

// Packs cells left-to-right, one column between non-empty cells,
// height 1. Width falls out of the sum of cell widths and gaps.
//
// Future strategies (e.g. aligned-row, where multiple readouts share
// lane positions across rows) land in this namespace.
class packer
{
    public:
        template<std::size_t N>
        void push( const cells::cell<N> &c ) {
            int cell_width = 0;
            for( const cells::segment &s : c.segments ) {
                cell_width += utf8_width( s.text );
            }
            if( cell_width == 0 ) {
                return;
            }
            if( x > 0 ) {
                x += 1;
            }
            for( const cells::segment &s : c.segments ) {
                if( s.text.empty() ) {
                    continue;
                }
                const int w = utf8_width( s.text );
                buffer.push_back( { point( x, 0 ), s.color, s.text } );
                if( !s.peripheral ) {
                    if( anchor_left < 0 ) {
                        anchor_left = x;
                    }
                    anchor_right = x + w;
                }
                x += w;
            }
        }

        cells::render_result build() && {
            if( x == 0 ) {
                return { cells::extent{ 0, 0 }, {}, 0, 0 };
            }
            const int ml = anchor_left >= 0 ? anchor_left : 0;
            const int mr = anchor_right > 0 ? x - anchor_right : 0;
            return { cells::extent{ x, 1 }, std::move( buffer ), ml, mr };
        }

    private:
        int x = 0;
        int anchor_left = -1;
        int anchor_right = 0;
        cells::cell_buffer buffer;
};

// Wraps a `packer` and horizontally centres its output within a
// fixed width.
class centerer
{
    public:
        explicit centerer( int fixed_width ) : fixed_width( fixed_width ) {}

        template<std::size_t N>
        void push( const cells::cell<N> &c ) {
            inner.push( c );
        }

        cells::render_result build() && {
            cells::render_result r = std::move( inner ).build();
            if( r.size.width == 0 ) {
                return { cells::extent{ 0, 0 }, {}, 0, 0 };
            }
            const int offset = std::max( 0, ( fixed_width - r.size.width ) / 2 );
            for( cells::positioned_segment &ps : r.buffer ) {
                ps.pos.x += offset;
            }
            return { cells::extent{ fixed_width, 1 }, std::move( r.buffer ),
                     r.margin_left + offset, r.margin_right + offset };
        }

    private:
        int fixed_width;
        packer inner;
};

} // namespace gaunthud::layout

#endif // CATA_SRC_GAUNTHUD_LAYOUT_H
