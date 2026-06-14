# Storybook — standalone PNG renderer for CDDA UI surfaces.
#
# Toolchain assumption: GCC + GNU `ld`/`lld`. The build relies on
# `-ffunction-sections` / `-Wl,--gc-sections` for link-time pruning,
# the Itanium C++ ABI for the layout-independent mangling that
# `dead_stubs.cpp` exploits, and LTO being off (see
# `INTERPROCEDURAL_OPTIMIZATION OFF` below). Other toolchains haven't
# been tested.
#
# Configure example (workspace root):
#   cmake -S Cataclysm-DDA/stories -B build/stories -G Ninja \
#         -DFONT_PATH=/abs/path/to/font.{ttf,otf}
#
# See [build-cdda-stories.sh](../../build-cdda-stories.sh) for the
# canonical invocation.

cmake_minimum_required(VERSION 3.20)

project(CataclysmStorybook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    # Release: render-only code; debug info isn't useful and noticeably
    # slows linking. Iteration speed > debuggability here.
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Path to the font to bake into the binary as a preprocessor define.
if(NOT FONT_PATH OR FONT_PATH STREQUAL "")
    message(FATAL_ERROR
        "FONT_PATH is required. Pass -DFONT_PATH=/abs/path/to/font.{ttf,otf}.")
endif()
if(NOT EXISTS "${FONT_PATH}")
    message(FATAL_ERROR "FONT_PATH '${FONT_PATH}' does not exist.")
endif()

# === Dependencies ===

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_TTF   REQUIRED IMPORTED_TARGET SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)

find_program(CCACHE ccache)
if(CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
endif()

# === Source layout ===

set(CDDA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Storybook-owned sources: tooling + entry point + per-module story
# files. Touches no upstream files.
set(STORYBOOK_SOURCES
    main.cpp
    storybook.cpp
    storybook_shims.cpp
    stubs.cpp
    dead_stubs.cpp
    gauntbar_stories.cpp
)

# Engine TUs the render path drags in transitively. Built iteratively
# from linker errors — start minimal, add the TU defining each
# unresolved symbol, repeat until link succeeds. Keep this list small;
# if it balloons past ~30 entries, the gather/render split has leaked
# and needs tightening.
set(GAUNTBAR_RENDER_SUPPORT_SOURCES
    # Direct dependencies of storybook.cpp / gauntbar_ambient_render.cpp.
    ${CDDA_SRC}/gauntbar_ambient_render.cpp  # render entry point
    ${CDDA_SRC}/color.cpp                    # init_colors + colorpairs[]
    ${CDDA_SRC}/catacharset.cpp              # wide_pua_glyphs + utf8 helpers
    ${CDDA_SRC}/cursesport.cpp               # WINDOW struct + werase/newwin
    ${CDDA_SRC}/point.cpp                    # point::zero
    ${CDDA_SRC}/string_formatter.cpp         # string_format
    ${CDDA_SRC}/wcwidth.cpp                  # mk_wcwidth
    ${CDDA_SRC}/demangle.cpp                 # used by string_format template instantiations
    # TUs deliberately NOT linked (each drags in machinery the render
    # path doesn't need; see stubs.cpp / dead_stubs.cpp for the shims
    # that satisfy the linker in their place):
    #   output.cpp       — query_popup → cataimgui, debug chains
    #   debug.cpp        — debug UI / logging
    #   json.cpp         — JSON loader + flexbuffer parse
    #   popup.cpp        — cataimgui popup stack
    #   translations.cpp — gettext + locale setup
    #   sdltiles.cpp     — full SDL renderer + tile cache
    #   game_ui.cpp      — UI resize / font-change orchestration
)

add_executable(storybook
    ${STORYBOOK_SOURCES}
    ${GAUNTBAR_RENDER_SUPPORT_SOURCES}
)

target_include_directories(storybook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CDDA_SRC}
)

# Mirror the engine's `-isystem` treatment of `src/third-party/`.
target_include_directories(storybook SYSTEM PRIVATE
    ${CDDA_SRC}/third-party
)

target_link_libraries(storybook PRIVATE
    PkgConfig::SDL2
    PkgConfig::SDL2_TTF
    PkgConfig::SDL2_IMAGE
)

target_compile_definitions(storybook PRIVATE
    # Engine code expects this; we're rendering the TILES-only
    # `cata_cursesport::WINDOW` cell buffer.
    TILES
    STORYBOOK_FONT_PATH="${FONT_PATH}"
)

target_compile_options(storybook PRIVATE
    # GCC 16: silence a spurious SFINAE warning in upstream `vehicle.h`.
    -Wno-sfinae-incomplete
    # Emit one section per function / data item so `--gc-sections`
    # can drop the unreachable engine code that gets linked in only
    # to satisfy symbol references — e.g. `color_manager::show_gui`
    # (drags in the imgui popup stack) and the JSON serialisation
    # paths in `color.cpp`. The dead bodies that DO survive linking
    # (because they're referenced from the keep-set's vtables or
    # transitively from kept TUs) are silently no-op'd by
    # `dead_stubs.cpp`; see that file's header for the ABI trick.
    -ffunction-sections
    -fdata-sections
)

target_link_options(storybook PRIVATE
    -Wl,--gc-sections
)

set_target_properties(storybook PROPERTIES
    # LTO would see both `dead_stubs.cpp`'s look-alike types and the
    # engine's real types and either miscompile or refuse to link.
    # See `dead_stubs.cpp` header for the ABI trick this protects.
    INTERPROCEDURAL_OPTIMIZATION OFF
)
