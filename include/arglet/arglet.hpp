#pragma once
#include <array>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

namespace arglet {

template <size_t N>
using string_literal = char const (&)[N];

enum class flag_form { Short = 1, Long = 2, Both = 3 };

template <class Tag, flag_form form>
struct flag;

template <class Tag>
struct flag<Tag, flag_form::Short> {
    [[no_unique_address]] Tag tag;
    char short_form = '\0';
    bool value = false;
    constexpr char const** parse(char const** begin, char const** end) {
        auto this_arg = begin[0];
        if (this_arg[0] == '-' && this_arg[1] == short_form &&
            this_arg[2] == '\0') {
            value = true;
            return begin + 1;
        } else {
            return begin;
        }
    }
    bool& operator[](Tag) { return value; }
};
template <class Tag>
struct flag<Tag, flag_form::Long> {
    [[no_unique_address]] Tag tag;
    std::string_view long_form{};
    bool value = false;
    constexpr char const** parse(char const** begin, char const** end) {
        if (long_form == begin[0]) {
            value = true;
            return begin + 1;
        } else {
            return begin;
        }
    }
    bool& operator[](Tag) { return value; }
};
template <class Tag>
struct flag<Tag, flag_form::Both> {
    [[no_unique_address]] Tag tag;
    char short_form = '\0';
    std::string_view long_form{};
    bool value = false;
    constexpr char const** parse(char const** begin, char const** end) {
        auto this_arg = begin[0];
        if (this_arg[0] == '-' && this_arg[1] == short_form &&
            this_arg[2] == '\0') {
            value = true;
            return begin + 1;
        } else if (long_form == this_arg) {
            value = true;
            return begin + 1;
        } else {
            return begin;
        }
    }
    bool& operator[](Tag) { return value; }
};
template <class Tag>
flag(Tag tag, char) -> flag<Tag, flag_form::Short>;
template <class Tag, size_t N>
flag(Tag, string_literal<N>) -> flag<Tag, flag_form::Long>;
template <class Tag, size_t N>
flag(Tag, char, string_literal<N>) -> flag<Tag, flag_form::Both>;

template <class... Arg>
struct sequence : Arg... {
    using Arg::operator[]...;

    constexpr char const** parse(char const** begin, char const** end) {
        ((begin != end) && ... &&
         (begin = Arg::parse(begin, end), begin != end));
        return begin;
    }
};
template <class... Arg>
sequence(Arg...) -> sequence<Arg...>;

template <class... Arg>
struct section : Arg... {
    using Arg::operator[]...;

    constexpr char const** parse(char const** begin, char const** end) {
        char const** initial = nullptr;
        do {
            initial = begin;
            ((begin != end) && ... &&
             (begin = Arg::parse(begin, end), begin != end));
            ;
        } while (initial != begin);
        return begin;
    }
};
template <class... Arg>
section(Arg...) -> section<Arg...>;

template <class Tag>
struct string {
    [[no_unique_address]] Tag tag;
    std::string_view value;
    constexpr char const** parse(char const** begin, const char** end) {
        value = *begin;
        return begin + 1;
    }
    constexpr std::string_view& operator[](Tag) { return value; }
};
template <class T>
string(T) -> string<T>;
template <class T>
string(T, std::string_view) -> string<T>;

template <class... Flag>
struct flag_set : Flag... {
    using Flag::operator[]...;
    constexpr const char** parse(const char** begin, const char** end) {
        while (begin != end) {
            char const* this_arg = *begin;
            if (this_arg[0] == '-') {
                bool successful = true;
                int i = 1;
                bool current_state[]{Flag::value...};
                while (char c = this_arg[i++]) {
                    bool any_flag_matches =
                        ((Flag::short_form == c && (Flag::value = true)) ||
                         ...);
                    if (!any_flag_matches) {
                        successful = false;
                        break;
                    }
                }
                if (successful) {
                    begin++;
                    continue;
                } else {
                    // Reset previous state
                    i = 0;
                    ((Flag::value = current_state[i++]), ...);
                }
            }
            if (((Flag::long_form == this_arg && (Flag::value = true)) ||
                 ...)) {
                begin++;
                continue;
            }
            return begin;
        }
        return begin;
    }
};
template<class... Flag>
flag_set(Flag...) -> flag_set<Flag...>;

// To-do: re-implement in terms of updated flag
// template <class Tag, size_t N>
// struct option_flag {
//     [[no_unique_address]] Tag tag;
//     flag_list<N> flags{};
//     std::string_view value{};
//     constexpr char const** parse(char const** args) {
//         char const* next_arg = args[1];
//
//         if (flags.contains(args[0]) && next_arg != nullptr) {
//             value = next_arg;
//             return args + 2;
//         } else {
//             return args;
//         }
//     }
//     constexpr std::string_view& operator[](Tag) { return value; }
// };
// template <class T, size_t... I>
// option_flag(T, string_literal<I>...) -> option_flag<T, sizeof...(I)>;
// template <class T, size_t N>
// option_flag(T, flag_list<N>) -> option_flag<T, N>;
// template <class T, size_t N>
// option_flag(T, flag_list<N>, std::string_view) -> option_flag<T, N>;

struct ignore_arg_t {
    template <int>
    void operator[](int) {}
    constexpr char const** parse(char const** begin, const char** end) { return begin + (begin != end); }
};
constexpr ignore_arg_t ignore_arg = {};

// template <class Elem>
// struct list_of {
//     std::vector<Elem> values;
//     char const** parse(char const** args) {}
// };
} // namespace arglet
