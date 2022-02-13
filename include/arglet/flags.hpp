#pragma once
#include <arglet/util/array_map.hpp>
#include <array>
#include <span>
#include <string_view>
#include <tuplet/tuple.hpp>

namespace arglet::flags {
using std::string_view;

template <size_t NumShortFlags, size_t NumLongFlags>
struct flag_arg {
    constexpr static size_t NShortFlags = NumShortFlags;
    constexpr static size_t NLongFlags = NumLongFlags;
    std::array<char, NShortFlags> short_flags;
    std::array<string_view, NLongFlags> long_flags;
};
template <size_t NS, size_t NL>
flag_arg(std::array<char, NS>, std::array<string_view, NL>) -> flag_arg<NS, NL>;

// Generic type holding a reference to a concrete flag arg
struct any_flag_arg {
    struct vtable_t {
        std::span<const char> (*get_short_flags)(void const*);
        std::span<const string_view> (*get_long_flags)(void const*);
    };
    void const* pointer {};
    vtable_t const* vtable {};

    template <class FlagArg>
    struct VTables {
        constexpr static vtable_t table {
            // get_short_flags
            [](void const* pointer) -> std::span<const char> {
                FlagArg& arg = *(FlagArg*)pointer;

                return {arg.short_flags.begin(), arg.short_flags.size()};
            },

            // get_long_flags
            [](void const* pointer) -> std::span<const string_view> {
                FlagArg& arg = *(FlagArg*)pointer;

                return {arg.long_flags.begin(), arg.long_flags.size()};
            }};
    };

    // Initializes pointer and vtable to null
    any_flag_arg() = default;
    // this one is provided to avoid the templated overload being selected
    constexpr any_flag_arg(any_flag_arg& arg) noexcept
      : any_flag_arg(static_cast<any_flag_arg const&>(arg)) {}
    any_flag_arg(any_flag_arg const&) = default;
    any_flag_arg(any_flag_arg&&) = default;

    any_flag_arg& operator=(any_flag_arg const&) = default;
    any_flag_arg& operator=(any_flag_arg&&) = default;

    template <class FlagArg>
    constexpr any_flag_arg(FlagArg const& arg)
      : pointer(&arg)
      , vtable(&VTables<FlagArg>::table) {}

    constexpr auto get_short_flags() { vtable->get_short_flags(pointer); }
    constexpr auto get_long_flags() { vtable->get_long_flags(pointer); }
};

// Create an array_map of long flags for every flag arg in a tuple containing
// flag args
template <class... T>
constexpr auto make_long_flag_map(tuplet::tuple<T...> const& flags)
    -> util::array_map<string_view, any_flag_arg, (T::NLongFlags + ...)> {
    using map_t =
        util::array_map<string_view, any_flag_arg, (T::NLongFlags + ...)>;
    using entry_t = typename map_t::entry_type;
    map_t map {};
    flags.for_each([&map, i = 0](auto& flag_arg) mutable {
        for (string_view key : flag_arg.long_flags) {
            map[i++] = {key, any_flag_arg(flag_arg)};
        }
    });

    map.sort();

    return map;
}

// Create an array_map of short flags for every flag in a tuple containing flag
// args
template <class... T>
constexpr auto make_short_flag_map(tuplet::tuple<T...> const& flags)
    -> util::array_map<char, any_flag_arg, (T::NShortFlags + ...)> {
    using map_t = util::array_map<char, any_flag_arg, (T::NShortFlags + ...)>;
    using entry_t = typename map_t::entry_type;
    map_t map {};
    flags.for_each([&map, i = 0](auto& flag_arg) mutable {
        for (char key : flag_arg.short_flags) {
            map[i++] = {key, any_flag_arg(flag_arg)};
        }
    });

    map.sort();

    return map;
}

} // namespace arglet::flags
