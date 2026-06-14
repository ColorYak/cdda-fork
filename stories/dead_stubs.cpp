// Silent dead-code stubs for engine APIs reachable through
// `color_manager`'s JSON / UI methods (`serialize`, `deserialize`,
// `show_gui`, `load_custom`, `save_custom`). Those methods are
// linked but unreachable from `main()`: the storybook only ever
// calls `load_default()`. The linker keeps them anyway because
// `-Wl,--gc-sections` plus `-ffunction-sections` is not aggressive
// enough to prune their transitive closure out of `color.cpp.o`.
//
// This file deliberately includes no engine headers. Every type
// below is a one-off forward declaration whose only job is to
// produce the right mangled symbol name for the linker. The classes
// declared here are not the engine's `Json` / `JsonValue` /
// `JsonObject` / `JsonOut` / `tiny_bitset` / `ui_adaptor` — they
// share names with them at the source level, but they're entirely
// separate types compiled in a TU that doesn't see the originals.
//
// Each stub aborts on call so that any future accidental reach into
// these symbols at runtime fails loudly.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iosfwd>
#include <string>
#include <string_view>

namespace
{
[[noreturn]] void dead( const char *name )
{
    std::fprintf( stderr,
                  "[storybook] dead-code stub `%s` was called. The storybook "
                  "reached an engine code path it doesn't link.\n",
                  name );
    std::abort();
}
} // namespace

// --- string_view_cmp (header-inline JSON path) ---
//
// `JsonObject::find_map_key_idx` (an inline in `flexbuffer_json.h`)
// is reachable via `color_manager::deserialize`. It calls
// `string_view_cmp`, which lives in `cata_utility.cpp`. We don't
// link `cata_utility.cpp`, so reproduce the real impl here — the
// path is dead at runtime but the function might end up inlined
// elsewhere.

int string_view_cmp( std::string_view a, std::string_view b )
{
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    const int r = n ? std::memcmp( a.data(), b.data(), n ) : 0;
    if( r != 0 ) {
        return r;
    }
    if( a.size() < b.size() ) {
        return -1;
    }
    if( a.size() > b.size() ) {
        return 1;
    }
    return 0;
}

// --- JSON I/O surface ---

namespace flexbuffers
{
enum class Type : int;
} // namespace flexbuffers

class JsonPath;

// `Json` is a class with non-static `throw_error` and static
// `flexbuffer_type_to_string`. Mimic the shape so the mangling
// matches.
class Json
{
    public:
        void throw_error( const JsonPath &, int, const std::string & ) const;
        static const std::string &flexbuffer_type_to_string( flexbuffers::Type );
};

void Json::throw_error( const JsonPath &, int, const std::string & ) const
{
    dead( "Json::throw_error" );
}

const std::string &Json::flexbuffer_type_to_string( flexbuffers::Type )
{
    dead( "Json::flexbuffer_type_to_string" );
}

class JsonValue
{
    public:
        void throw_error( const std::string & ) const;
};

void JsonValue::throw_error( const std::string & ) const
{
    dead( "JsonValue::throw_error" );
}

class JsonObject
{
    public:
        void report_unvisited() const;
        void error_no_member( std::string_view ) const;
};

void JsonObject::report_unvisited() const
{
    dead( "JsonObject::report_unvisited" );
}

void JsonObject::error_no_member( std::string_view ) const
{
    dead( "JsonObject::error_no_member" );
}

class JsonOut
{
    public:
        JsonOut( std::ostream &, bool, int );
        void start_array( bool = false );
        void end_array();
        void start_object( bool = false );
        void end_object();
        void member( std::string_view );
        void write( std::string_view );
};

JsonOut::JsonOut( std::ostream &, bool, int )
{
    dead( "JsonOut::JsonOut" );
}
void JsonOut::start_array( bool )
{
    dead( "JsonOut::start_array" );
}
void JsonOut::end_array()
{
    dead( "JsonOut::end_array" );
}
void JsonOut::start_object( bool )
{
    dead( "JsonOut::start_object" );
}
void JsonOut::end_object()
{
    dead( "JsonOut::end_object" );
}
void JsonOut::member( std::string_view )
{
    dead( "JsonOut::member" );
}
void JsonOut::write( std::string_view )
{
    dead( "JsonOut::write" );
}

// --- Tiny bitset (used by JsonObject's visited-fields tracking) ---

class tiny_bitset
{
    public:
        void resize_heap( std::size_t ) noexcept;
};

void tiny_bitset::resize_heap( std::size_t ) noexcept
{
    dead( "tiny_bitset::resize_heap" );
}

// --- ui_adaptor (only the one method color.cpp's show_gui references) ---

namespace catacurses
{
class window;
} // namespace catacurses

class ui_adaptor
{
    public:
        void position_from_window( const catacurses::window & );
};

void ui_adaptor::position_from_window( const catacurses::window & )
{
    dead( "ui_adaptor::position_from_window" );
}
