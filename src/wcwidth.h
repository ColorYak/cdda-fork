#pragma once
#ifndef CATA_SRC_WCWIDTH_H
#define CATA_SRC_WCWIDTH_H

#include <cstdint>

/* Get character width in columns.  See wcwidth.cpp for details.
 */
int mk_wcwidth( uint32_t ucs );

/* When true, mk_wcwidth() reports codepoints in the Unicode Private Use
 * Area (BMP U+E000..U+F8FF, plus the SPUA-A and SPUA-B planes
 * U+F0000..U+FFFFD and U+100000..U+10FFFD) as two columns wide instead
 * of one. Set from the FONT_WIDE_PUA_GLYPHS option via
 * options_manager::update_options_cache().
 */
extern bool wide_pua_glyphs;

#endif // CATA_SRC_WCWIDTH_H
