#pragma once
#ifndef CATA_SRC_WCWIDTH_H
#define CATA_SRC_WCWIDTH_H

#include <cstdint>

/* Get character width in columns.  See wcwidth.cpp for details.
 */
int mk_wcwidth( uint32_t ucs );

/* Controls how mk_wcwidth() measures Private Use Area codepoints.
 * Set from the FONT_PUA_WIDTHS option via
 * options_manager::update_options_cache().
 */
enum class pua_width_mode : int {
    standard,   // all PUA = 1 column (default)
    nerdfonts,  // NerdFonts double-width icon layout
};

extern pua_width_mode pua_widths;

#endif // CATA_SRC_WCWIDTH_H
