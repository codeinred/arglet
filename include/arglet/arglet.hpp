#pragma once
#include <array>
#include <cstddef>
#include <cstdio>
#include <optional>
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
    using state_type = bool;
    [[no_unique_address]] Tag tag;
    char short_form = '\0';
    bool value = false;
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        auto this_arg = begin[0];
        if (this_arg[0] == '-' && this_arg[1] == short_form &&
            this_arg[2] == '\0') {
            value = true;
            return begin + 1;
        } else {
            return begin;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    bool& operator[](Tag) { return value; }
    constexpr bool parse_char(char c) noexcept {
        if (short_form == c) {
            value = true;
            return true;
        } else {
            return false;
        }
    }
    constexpr bool parse_long_form(const char*) const noexcept { return false; }
};
template <class Tag>
struct flag<Tag, flag_form::Long> {
    using state_type = bool;
    [[no_unique_address]] Tag tag;
    std::string_view long_form{};
    bool value = false;
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        if (long_form == begin[0]) {
            value = true;
            return begin + 1;
        } else {
            return begin;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    bool& operator[](Tag) { return value; }
    constexpr bool parse_char(char) const noexcept { return false; }
    constexpr bool parse_long_form(const char* arg) {
        if (long_form == arg) {
            value = true;
            return true;
        } else {
            return false;
        }
    }
};
template <class Tag>
struct flag<Tag, flag_form::Both> {
    using state_type = bool;

    [[no_unique_address]] Tag tag;
    char short_form = '\0';
    std::string_view long_form{};
    bool value = false;
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
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
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    bool& operator[](Tag) { return value; }
    constexpr bool parse_char(char c) noexcept {
        if (short_form == c) {
            value = true;
            return true;
        } else {
            return false;
        }
    }
    constexpr bool parse_long_form(const char* arg) {
        if (long_form == arg) {
            value = true;
            return true;
        } else {
            return false;
        }
    }
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
        // this is cast to void because we don't need the result of this
        // fold expression. Casting it to void prevents an unused value warning
        (void)((begin != end) && ... &&
               (begin = Arg::parse(begin, end), begin != end));
        return begin;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
};
template <class... Arg>
sequence(Arg...) -> sequence<Arg...>;

template <class... Arg>
struct section : Arg... {
    using Arg::operator[]...;

    constexpr char const** parse(char const** begin, char const** end) {
        bool has_args = begin != end;
        while (has_args) {
            auto current = begin;
            // We have args to parse as long as begin != end
            // And as long as at least one argument is successfully parsed.
            // If an argument is successfully parsed, then current < begin
            has_args = ((begin = Arg::parse(begin, end), begin != end) && ... &&
                        (current < begin));
        }
        return begin;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
};
template <class... Arg>
section(Arg...) -> section<Arg...>;

template <class... T>
auto save_state(T... values) {
    return [... saved_state = values](T&... values_to_restore) {
        ((void)(values_to_restore = saved_state), ...);
    };
}

template <class... Flag>
struct flag_set : Flag... {
    using Flag::operator[]...;
    constexpr const char** parse(const char** begin, const char** end) {
        while (begin != end) {
            char const* this_arg = *begin;
            if (this_arg[0] == '-') {
                bool successful = true;
                auto reset_state = save_state(Flag::value...);

                int i = 1;
                while (char c = this_arg[i++]) {
                    bool any_flag_matches = (Flag::parse_char(c) || ...);
                    if (!any_flag_matches) {
                        successful = false;
                        reset_state(Flag::value...);
                        break;
                    }
                }
                if (successful) {
                    begin++;
                    continue;
                }
            }
            if ((Flag::parse_long_form(this_arg) || ...)) {
                begin++;
                continue;
            }
            return begin;
        }
        return begin;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
};
template <class... Flag>
flag_set(Flag...) -> flag_set<Flag...>;

template <class T>
struct null_value_t {
    constexpr operator std::optional<T>() const { return std::nullopt; }
};
template <class T>
constexpr null_value_t<T> null_value{};

template <class Tag, class T, flag_form form>
struct value_flag;

template <class Tag, class T>
struct value_flag<Tag, T, flag_form::Short> {
    [[no_unique_address]] Tag tag;
    std::optional<T> value;
    char short_form = '\0';

    constexpr char const** parse(char const** begin, char const** end) {
        auto this_arg = begin[0];
        if ((end - begin) >= 2 && this_arg[0] == '-' &&
            this_arg[1] == short_form && this_arg[2] == '\0') {
            value.emplace(begin[1]);
            return begin + 2;
        } else {
            return begin;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag, class T>
struct value_flag<Tag, T, flag_form::Long> {
    [[no_unique_address]] Tag tag;
    std::optional<T> value;
    std::string_view long_form{};

    constexpr char const** parse(char const** begin, char const** end) {
        if ((end - begin) >= 2 && long_form == begin[0]) {
            value.emplace(begin[1]);
            return begin + 2;
        } else {
            return begin;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag, class T>
struct value_flag<Tag, T, flag_form::Both> {
    [[no_unique_address]] Tag tag;
    std::optional<T> value;
    char short_form = '\0';
    std::string_view long_form{};

    constexpr char const** parse(char const** begin, char const** end) {
        if ((end - begin) >= 2) {
            auto arg0 = begin[0];
            if ((arg0[0] == '-' && arg0[1] == short_form && arg0[2] == '\0') ||
                (long_form == arg0)) {
                value.emplace(begin[1]);
                return begin + 2;
            } else {
                return begin;
            }
        } else {
            return begin;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag, class T>
value_flag(Tag, T, char) -> value_flag<Tag, T, flag_form::Short>;
template <class Tag, class T, size_t N>
value_flag(Tag, T, string_literal<N>) -> value_flag<Tag, T, flag_form::Long>;
template <class Tag, class T, size_t N>
value_flag(Tag, T, char, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Both>;
template <class Tag, class T>
value_flag(Tag, null_value_t<T>, char) -> value_flag<Tag, T, flag_form::Short>;
template <class Tag, class T, size_t N>
value_flag(Tag, null_value_t<T>, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Long>;
template <class Tag, class T, size_t N>
value_flag(Tag, null_value_t<T>, char, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Both>;
template <class Tag, class T>
value_flag(Tag, std::optional<T>, char) -> value_flag<Tag, T, flag_form::Short>;
template <class Tag, class T, size_t N>
value_flag(Tag, std::optional<T>, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Long>;
template <class Tag, class T, size_t N>
value_flag(Tag, std::optional<T>, char, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Both>;

template <class Tag, class T>
struct value {
    [[no_unique_address]] Tag tag;
    std::optional<T> value{};
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        value.emplace(begin[0]);
        return begin + 1;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag, class T>
value(Tag, T) -> value<Tag, T>;
template <class Tag, class T>
value(Tag, null_value_t<T>) -> value<Tag, T>;
template <class Tag, class T>
value(Tag, std::optional<T>) -> value<Tag, T>;
template <class Tag>
value(Tag) -> value<Tag, std::string_view>;

template <class Tag>
struct string {
    [[no_unique_address]] Tag tag;
    std::optional<std::string_view> value{};
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        value.emplace(begin[0]);
        return begin + 1;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag>
string(Tag, std::string_view) -> string<Tag>;
template <class Tag>
string(Tag, null_value_t<std::string_view>) -> string<Tag>;
template <class Tag>
string(Tag, std::optional<std::string_view>) -> string<Tag>;
template <class Tag>
string(Tag) -> string<Tag>;

struct ignore_arg_t {
    template <int>
    void operator[](int) {}
    constexpr char const** parse(char const** begin, const char** end) {
        return begin + (begin != end);
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
};
constexpr ignore_arg_t ignore_arg = {};

template <class Tag, class Elem>
struct list_of {
    [[no_unique_address]] Tag tag;
    std::vector<Elem> value;
    char const** parse(char const** begin, const char** end) {
        intptr_t length = end - begin;
        if (length > 0) {
            value.clear();
            value.reserve(length);
            for (intptr_t i = 0; i < length; i++) {
                value.emplace_back(begin[i]);
            }
        }
        return end;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};

namespace detail {
struct ignore_function_arg {
    ignore_function_arg() = default;
    ignore_function_arg(ignore_function_arg const&) = default;
    template <class T>
    constexpr ignore_function_arg(T&&) noexcept {}
};
} // namespace detail

template <class T, flag_form type>
struct option;

template <class T>
struct option<T, flag_form::Short> {
    char short_form;
    T option_value{};
    template <class U>
    bool match_assign(std::string_view arg, U& value) {
        if (arg.size() == 2 && arg[0] == '-' && arg[1] == short_form) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
    template <class U>
    constexpr bool match_assign_char(char c, U& value) {
        if (short_form == c) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
    constexpr bool match_assign_long_form(std::string_view,
                                          detail::ignore_function_arg) const {
        return false;
    }
};

template <class T>
struct option<T, flag_form::Long> {
    std::string_view long_form;
    T option_value{};
    template <class U>
    constexpr bool match_assign(std::string_view arg, U& value) {
        if (long_form == arg) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
    constexpr bool match_assign_char(char c,
                                     detail::ignore_function_arg) const {
        return false;
    }
    template <class U>
    constexpr bool match_assign_long_form(std::string_view arg, U& value) {
        if (long_form == arg) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
};
template <class T>
struct option<T, flag_form::Both> {
    char short_form;
    std::string_view long_form;
    T option_value{};
    template <class U>
    constexpr bool match_assign(std::string_view arg, U& value) {
        if ((arg.size() == 2 && arg[0] == '-' && arg[1] == short_form) ||
            long_form == arg) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
    template <class U>
    constexpr bool match_assign_char(char c, U& value) {
        if (short_form == c) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
    template <class U>
    constexpr bool match_assign_long_form(std::string_view arg, U& value) {
        if (long_form == arg) {
            value = option_value;
            return true;
        } else {
            return false;
        }
    }
};

template <class T>
option(char, T) -> option<T, flag_form::Short>;
template <size_t N, class T>
option(string_literal<N>, T) -> option<T, flag_form::Long>;
template <class T>
option(std::string_view, T) -> option<T, flag_form::Long>;
template <size_t N, class T>
option(char, string_literal<N>, T) -> option<T, flag_form::Both>;
template <class T>
option(char, std::string_view, T) -> option<T, flag_form::Both>;

template <class Tag, class T>
struct option_state {
    [[no_unique_address]] Tag tag;
    T value{};
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};

template <class Tag, class T, bool is_optional, flag_form... forms>
struct option_set : option_state<Tag, std::conditional_t<is_optional, std::optional<T>, T>>, option<T, forms>... {
    using state_impl = option_state<Tag, std::conditional_t<is_optional, std::optional<T>, T>>;
    using state_impl::value;
    using state_impl::operator[];
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        std::string_view arg = begin[0];
        begin += ((option<T, forms>::match_assign(arg, value) || ...));
        return begin;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    constexpr bool parse_char(char c) {
        return (option<T, forms>::match_assign_char(c, value) || ...);
    }
    constexpr bool parse_long_form(char c) {
        return (option<T, forms>::match_assign_long_form(c, value) || ...);
    }
};
template<class Tag, class T, flag_form... forms>
option_set(Tag, T, option<T, forms>...) -> option_set<Tag, T, false, forms...>;
template<class Tag, class T, flag_form... forms>
option_set(Tag, std::optional<T>, option<T, forms>...) -> option_set<Tag, T, true, forms...>;

using command_fn = int (*)(int, char const**);

int unimplemented_command(int, char const**) {
    printf("[No implementation was specified for this subcommand]");
    return 1;
}

template <class Tag, flag_form... forms>
struct command_set : option_state<Tag, command_fn>, option<command_fn, forms>... {
    using state_impl = option_state<Tag, command_fn>;
    using state_impl::value;
    using state_impl::operator[];
    std::string_view command_name;
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        std::string_view arg = begin[0];
        command_name = arg;
        return begin + (option<command_fn, forms>::match_assign(arg, value) || ...);
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }

    auto& operator[](Tag) { return *this; }
    auto const& operator[](Tag) const { return *this; }
    constexpr void set_default_command(command_fn func) noexcept {
        value = func;
    }
    constexpr command_fn get_default_command() const noexcept { return value; }
    constexpr operator bool() const { return value != nullptr; }
    int operator()(int argc, char const** argv) const {
        if (value) {
            return value(argc, argv);
        } else {
            if (!command_name.data()) {
                puts("Error: Missing subcommand. Try --help for usage.");
            } else {
                printf(
                    "Unrecognized subcommand '%.*s'. Try --help for usage.\n",
                    (int)command_name.size(), command_name.data());
            }
            return 1;
        }
    }
};
template<class Tag, flag_form... forms>
command_set(Tag, command_fn, option<command_fn, forms>...) -> command_set<Tag, forms...>;
template<class Tag, flag_form... forms>
command_set(Tag, std::nullptr_t, option<command_fn, forms>...) -> command_set<Tag, forms...>;

template <class T>
option<T, flag_form::Long> operator>>(std::string_view s, T value) {
    return {s, value};
}
} // namespace arglet
