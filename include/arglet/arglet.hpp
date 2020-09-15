#pragma once
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

namespace arglet {

template <size_t N>
using string_literal = char const (&)[N];

template <class Parser, size_t... Size>
constexpr decltype(auto) set_defaults(Parser&& p,
                                      string_literal<Size>... default_args) {
    using arg_t = const char*;
    std::array<arg_t, sizeof...(Size) + 1> args{default_args..., nullptr};
    p.parse(args.data());
    return std::forward<Parser>(p);
}

template <size_t N>
struct flag_list {
    std::string_view flags[N];
    constexpr bool contains(char const* c) const noexcept {
        for (auto f : flags) {
            if (f == c) {
                return true;
            }
        }
        return false;
    }
    constexpr size_t index(const char* c) const noexcept {
        for (size_t i = 0; i < N; i++) {
            if (flags[i] == c) {
                return i;
            }
        }
        return N;
    }
};
template <size_t... I>
flag_list(string_literal<I>...) -> flag_list<sizeof...(I)>;

template <class... Arg>
struct sequence : Arg... {
    using Arg::operator[]...;

    constexpr char const** parse(char const** args) {
        (*args && ... && *(args = Arg::parse(args)));
        return args;
    }
};
template <class... Arg>
sequence(Arg...) -> sequence<Arg...>;

template <class... Arg>
struct section : Arg... {
    using Arg::operator[]...;

    constexpr char const** parse(char const** args) {
        char const** initial = nullptr;
        do {
            initial = args;
            (*args && ... && *(args = Arg::parse(args)));
        } while (initial != args);
        return args;
    }
};
template <class... Arg>
section(Arg...) -> section<Arg...>;

template <class Tag>
struct string {
    [[no_unique_address]] Tag tag;
    std::string_view value;
    constexpr char const** parse(char const** args) {
        value = *args;
        return args + 1;
    }
    constexpr std::string_view& operator[](Tag) { return value; }
};
template <class T>
string(T) -> string<T>;
template <class T>
string(T, std::string_view) -> string<T>;

template <class Tag, size_t N>
struct flag {
    [[no_unique_address]] Tag tag;
    flag_list<N> flags;
    bool value = false;
    constexpr char const** parse(char const** args) {
        if (flags.contains(*args)) {
            value = true;
            return args + 1;
        } else {
            return args;
        }
    }
    constexpr bool operator[](Tag) { return value; }
};

namespace flagset_details {
template <class Derived, class Tag, size_t I>
struct tag_index {
    constexpr bool& operator[](Tag) {
        return static_cast<Derived*>(this)->flags[I];
    }
    constexpr bool operator[](Tag) const {
        return static_cast<Derived const*>(this)->flags[I];
    }
};
template <class Derived, size_t I, class Tag, class... Rest>
struct flag_set_bases : tag_index<Derived, Tag, I>,
                        flag_set_bases<Derived, I + 1, Rest...> {
    using tag_index<Derived, Tag, I>::operator[];
    using flag_set_bases<Derived, I + 1, Rest...>::operator[];
};
template <class Derived, size_t I, class Tag>
struct flag_set_bases<Derived, I, Tag> : tag_index<Derived, Tag, I> {
    using tag_index<Derived, Tag, I>::operator[];
};
} // namespace flagset_details

template <class Tag>
struct fs_item {
    [[no_unique_address]] Tag tag;
    char short_form;
    std::string_view long_form;
};
template <class Tag, size_t Size>
fs_item(Tag, char, string_literal<Size>) -> fs_item<Tag>;

template <class... Tags>
struct flag_set
  : flagset_details::flag_set_bases<flag_set<Tags...>, 0, Tags...> {

    constexpr static size_t N = sizeof...(Tags);
    flag_list<N> long_form;
    std::array<char, N> short_form;
    std::array<bool, N> flags{};

    using flagset_details::flag_set_bases<flag_set<Tags...>, 0,
                                          Tags...>::operator[];
    constexpr flag_set(fs_item<Tags>... members) noexcept
      : long_form{members.long_form...},
        short_form{members.short_form...}, flags{} {}

    char const** parse(char const** args) {
        const std::string_view this_arg = args[0];
        if (this_arg.size() > 1 && this_arg[0] == '-') {
            auto new_state = flags;
            bool all_recognized = true;
            for (char c : this_arg.substr(1)) {
                bool recognized = false;
                for (int i = 0; i < N; i++) {
                    if (short_form[i] == c) {
                        new_state[i] = true;
                        recognized = true;
                    }
                }
                if (!recognized) {
                    all_recognized = false;
                    break;
                }
            }
            if (all_recognized) {
                flags = new_state;
                return args + 1;
            }
        }
        auto index = long_form.index(args[0]);
        if (index < N) {
            flags[index] = true;
            return args + 1;
        }
        return args;
    }
};
template <class... Tag>
flag_set(fs_item<Tag>...) -> flag_set<Tag...>;

template <class T, size_t... I>
flag(T, string_literal<I>...) -> flag<T, sizeof...(I)>;
template <class T, size_t... I>
flag(T, string_literal<I>..., bool) -> flag<T, sizeof...(I)>;
template <class T, size_t N>
flag(T, flag_list<N>) -> flag<T, N>;
template <class T, size_t N>
flag(T, flag_list<N>, bool) -> flag<T, N>;

template <class Tag, size_t N>
struct option_flag {
    [[no_unique_address]] Tag tag;
    flag_list<N> flags{};
    std::string_view value{};
    constexpr char const** parse(char const** args) {
        char const* next_arg = args[1];

        if (flags.contains(args[0]) && next_arg != nullptr) {
            value = next_arg;
            return args + 2;
        } else {
            return args;
        }
    }
    constexpr std::string_view& operator[](Tag) { return value; }
};
template <class T, size_t... I>
option_flag(T, string_literal<I>...) -> option_flag<T, sizeof...(I)>;
template <class T, size_t N>
option_flag(T, flag_list<N>) -> option_flag<T, N>;
template <class T, size_t N>
option_flag(T, flag_list<N>, std::string_view) -> option_flag<T, N>;

struct ignore_arg_t {
    template <int>
    void operator[](int) {}
    constexpr char const** parse(char const** args) { return args + 1; }
};
constexpr ignore_arg_t ignore_arg = {};

// template <class Elem>
// struct list_of {
//     std::vector<Elem> values;
//     char const** parse(char const** args) {}
// };
} // namespace arglet
