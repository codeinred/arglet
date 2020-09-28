#pragma once
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

// arglet::index implementation
// arglet::string_literal implementation
// arglet::flag_form implementation
namespace arglet {
template <size_t I>
using index = std::integral_constant<size_t, I>;

template <size_t N>
using string_literal = char const (&)[N];

enum class flag_form { Short = 1, Long = 2, Both = 3 };
} // namespace arglet

// Implementation details
namespace arglet::detail {
template <size_t I, class T>
struct tuple_elem {
    static T decl_elem(index<I>);

    [[no_unique_address]] T elem;

    constexpr decltype(auto) operator[](index<I>) & { return (elem); }
    constexpr decltype(auto) operator[](index<I>) const& { return (elem); }
    constexpr decltype(auto) operator[](index<I>) && {
        return (std::move(*this).elem);
    }
};

template <size_t I, class... T>
struct partial_tuple {}; // Represents an empty tuple

template <size_t I, class T>
struct partial_tuple<I, T> : tuple_elem<I, T> {
    using tuple_elem<I, T>::decl_elem;
    using tuple_elem<I, T>::operator[];
};

template <size_t I, class T, class... Rest>
struct partial_tuple<I, T, Rest...> : tuple_elem<I, T>,
                                      partial_tuple<I + 1, Rest...> {
    using tuple_elem<I, T>::decl_elem;
    using tuple_elem<I, T>::operator[];
    using partial_tuple<I + 1, Rest...>::decl_elem;
    using partial_tuple<I + 1, Rest...>::operator[];
};
} // namespace arglet::detail

// arglet::util::ignore_function_arg implementation
// arglet::util::save_state implementation
// arglet::util::type_array implementation
// arglet::is_optional implementation
// arglet::unwrap_optional implementation
namespace arglet::util {
struct ignore_function_arg {
    ignore_function_arg() = default;
    ignore_function_arg(ignore_function_arg const&) = default;
    template <class T>
    constexpr ignore_function_arg(T&&) noexcept {}
};

template <class... T>
auto save_state(T... values) {
    return [... saved_state = values](T&... values_to_restore) {
        ((void)(values_to_restore = saved_state), ...);
    };
}

template <class... T>
struct type_array : detail::partial_tuple<0, T...> {
    using detail::partial_tuple<0, T...>::operator[];
};

template <class T>
struct is_optional {
    using type = T;
    constexpr static bool value = false;
    using optional_type = std::optional<T>;
};

template <class T>
struct is_optional<std::optional<T>> {
    using type = T;
    using optional_type = std::optional<T>;
    constexpr static bool value = true;
};

template <class T>
constexpr bool is_optional_v = is_optional<T>::value;

template <class T>
using unwrap_optional = typename is_optional<T>::type;

template <class T>
using wrap_optional = typename is_optional<T>::optional_type;

template <class F>
using parse_result_t = std::invoke_result_t<F, std::string_view>;

template <class T>
struct construct_from_sv_t {
    T operator()(std::string_view arg) const { return T{arg}; }
};
template <class T>
constexpr construct_from_sv_t<T> construct_from_sv{};
} // namespace arglet::util

// arglet::flag_matcher
namespace arglet {
template <flag_form form>
struct flag_matcher;

template <>
struct flag_matcher<flag_form::Short> {
    char short_form;
    constexpr bool matches(const char* arg) {
        return arg[0] == '-' && arg[1] == short_form && arg[2] == '\0';
    }

    template <class Value, class NewValue = Value>
    constexpr bool parse_char(char c, Value& value, NewValue&& new_value) {
        if (short_form == c) {
            value = std::forward<NewValue>(new_value);
            return true;
        } else {
            return false;
        }
    }
    constexpr bool parse_long_form(util::ignore_function_arg,
                                   util::ignore_function_arg,
                                   util::ignore_function_arg) {
        return false;
    }
};
template <>
struct flag_matcher<flag_form::Long> {
    std::string_view long_form;

    constexpr bool matches(const char* arg) { return long_form == arg; }

    constexpr bool parse_char(util::ignore_function_arg,
                              util::ignore_function_arg,
                              util::ignore_function_arg) {
        return false;
    }
    template <class Value, class NewValue = Value>
    constexpr bool parse_long_form(std::string_view arg, Value& value,
                                   NewValue&& new_value) {
        if (long_form == arg) {
            value = std::forward<NewValue>(new_value);
            return true;
        } else {
            return false;
        }
    }
};
template <>
struct flag_matcher<flag_form::Both> {
    char short_form;
    std::string_view long_form;

    constexpr bool matches(const char* arg) {
        return (arg[0] == '-' && arg[1] == short_form && arg[2] == '\0') ||
               (long_form == arg);
    }
    template <class Value, class NewValue = Value>
    constexpr bool parse_char(char c, Value& value, NewValue&& new_value) {
        if (short_form == c) {
            value = std::forward<NewValue>(new_value);
            return true;
        } else {
            return false;
        }
    }
    template <class Value, class NewValue = Value>
    constexpr bool parse_long_form(const char* arg, Value& value,
                                   NewValue&& new_value) {
        if (long_form == arg) {
            value = std::forward<NewValue>(new_value);
            return true;
        } else {
            return false;
        }
    }
};
} // namespace arglet

// arglet::flag implementation
namespace arglet {
template <class Tag, flag_form form>
struct flag {
    using state_type = bool;
    [[no_unique_address]] Tag tag;
    flag_matcher<form> matcher;
    bool value = false;
    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        if (matcher.matches(begin[0])) {
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
        return matcher.parse_char(c, value, true);
    }
    constexpr bool parse_long_form(const char* arg) noexcept {
        return matcher.parse_long_form(arg, value, true);
    }
};
template <class Tag>
flag(Tag tag, char) -> flag<Tag, flag_form::Short>;
template <class Tag, size_t N>
flag(Tag, string_literal<N>) -> flag<Tag, flag_form::Long>;
template <class Tag, size_t N>
flag(Tag, char, string_literal<N>) -> flag<Tag, flag_form::Both>;
} // namespace arglet

// arglet:::value_flag implementation
namespace arglet {
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
                std::string_view arg = begin[1];
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
value_flag(Tag, std::optional<T>, char) -> value_flag<Tag, T, flag_form::Short>;
template <class Tag, class T, size_t N>
value_flag(Tag, std::optional<T>, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Long>;
template <class Tag, class T, size_t N>
value_flag(Tag, std::optional<T>, char, string_literal<N>)
    -> value_flag<Tag, T, flag_form::Both>;
} // namespace arglet

// arglet::value definition
namespace arglet {
template <class Tag, class T, class Func = util::construct_from_sv_t<T>>
struct value {
    [[no_unique_address]] Tag tag;
    T value;
    [[no_unique_address]] Func func;

    constexpr char const** parse(char const** begin, const char**) {
        auto arg = std::string_view(begin[0]);
        if constexpr (util::is_optional_v<util::parse_result_t<Func>>) {
            auto result = func(arg);
            if (result) {
                if constexpr (util::is_optional_v<T>) {
                    value.emplace(*std::move(result));
                } else {
                    value = *result;
                }
                return begin + 1;
            } else {
                return begin;
            }
        } else {
            if constexpr (util::is_optional_v<T>) {
                value.emplace(func(arg));
            } else {
                value = func(arg);
            }
            return begin + 1;
        }
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};
template <class Tag, class T>
value(Tag, T) -> value<Tag, T>;
template <class Tag, class Func, class T>
value(Tag, T, Func) -> value<Tag, T, Func>;
} // namespace arglet

// arglet::string implementation
namespace arglet {
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
string(Tag, std::optional<std::string_view>) -> string<Tag>;
template <class Tag>
string(Tag) -> string<Tag>;
} // namespace arglet

// arglet::ignore_arg_t implementation
// arglet::ignore_arg implementation
namespace arglet {
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
} // namespace arglet

// arglet::list_item implementation
namespace arglet {
template <class Tag, class Elem, class Func = void>
struct list_item {
    [[no_unique_address]] Tag tag;
    [[no_unique_address]] Func func;
    std::vector<Elem> value;
    using result_type = util::parse_result_t<Func>;
    char const** parse(char const** begin, const char** end) {
        auto arg = std::string_view(begin[0]);
        if constexpr (util::is_optional_v<result_type>) {
            auto result = func(arg);
            if (result) {
                value.emplace_back(*std::move(result));
                return begin + 1;
            } else {
                return begin;
            }
        } else {
            value.emplace_back(func(arg));
        }
        return begin + 1;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};

template <class Tag, class Elem>
struct list_item<Tag, Elem, void> {
    [[no_unique_address]] Tag tag;
    std::vector<Elem> value;
    char const** parse(char const** begin, const char** end) {
        value.emplace_back(std::string_view(begin[0]));
        return begin + 1;
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }
};

template <class Tag, class Elem>
list_item(Tag, std::vector<Elem>) -> list_item<Tag, Elem, void>;
template <class Tag, class Func>
list_item(Tag, Func)
    -> list_item<Tag, util::unwrap_optional<util::parse_result_t<Func>>, Func>;
template <class Tag, class Func, class Elem>
list_item(Tag, Func, std::vector<Elem>) -> list_item<Tag, Elem, Func>;
} // namespace arglet

// arglet::list_remaining implementation
namespace arglet {
template <class Tag, class Elem, class Func = void>
struct list_remaining {
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
template <class Tag, class Elem>
struct list_remaining<Tag, Elem, void> {
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
} // namespace arglet

// arglet::option implementation
namespace arglet {
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
                                          util::ignore_function_arg) const {
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
    constexpr bool match_assign_char(char c, util::ignore_function_arg) const {
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
} // namespace arglet

// arglet::sequence implementation
namespace arglet {
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
} // namespace arglet

// arglet::group implementation
namespace arglet {
template <class... Arg>
struct group : Arg... {
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
group(Arg...) -> group<Arg...>;
} // namespace arglet

// arglet::flag_group implementation
namespace arglet {
template <class... Flag>
struct flag_group : Flag... {
    using Flag::operator[]...;
    constexpr const char** parse(const char** begin, const char** end) {
        while (begin != end) {
            char const* this_arg = *begin;
            if (this_arg[0] == '-') {
                bool successful = true;
                auto reset_state = util::save_state(Flag::value...);

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
flag_group(Flag...) -> flag_group<Flag...>;
} // namespace arglet

// arglet::option_set implementation
namespace arglet {
template <class Tag, class T, bool is_optional, flag_form... forms>
struct option_set {
   private:
    constexpr static auto indicies =
        std::make_index_sequence<sizeof...(forms)>();
    template <size_t... I>
    constexpr char const** parse_(const char** begin, const char**,
                                  std::index_sequence<I...>) {
        std::string_view arg = begin[0];
        return begin + (options[index<I>()].match_assign(arg, value) || ...);
    }
    template <size_t... I>
    constexpr bool parse_char_(char c, std::index_sequence<I...>) {
        return (options[index<I>()].match_assign_char(c, value) || ...);
    }
    template <size_t... I>
    constexpr bool parse_long_form_(char const* arg,
                                    std::index_sequence<I...>) {
        return (options[index<I>()].match_assign_long_form(arg, value) || ...);
    }

   public:
    using state_t = std::conditional_t<is_optional, std::optional<T>, T>;
    [[no_unique_address]] Tag tag;
    state_t value{};
    util::type_array<option<T, forms>...> options;

    auto& operator[](Tag) { return value; }
    auto const& operator[](Tag) const { return value; }

    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        return parse_(begin, end, indicies);
    }
    constexpr intptr_t parse(int argc, char const** argv) {
        return parse(argv, argv + argc) - argv;
    }
    constexpr bool parse_char(char c) { return parse_char_(c, indicies); }
    constexpr bool parse_long_form(const char* arg) {
        return parse_long_form_(arg, indicies);
    }
};
template <class Tag, class T, flag_form... forms>
option_set(Tag, T, option<T, forms>...) -> option_set<Tag, T, false, forms...>;
template <class Tag, class T, flag_form... forms>
option_set(Tag, std::optional<T>, option<T, forms>...)
    -> option_set<Tag, T, true, forms...>;
} // namespace arglet

// arglet::command_set implementation
namespace arglet {
using command_fn = int (*)(int, char const**);

int unimplemented_command(int, char const**) {
    printf("[No implementation was specified for this subcommand]\n");
    return 1;
}

template <class Tag, flag_form... forms>
struct command_set {
   private:
    constexpr static auto indicies =
        std::make_index_sequence<sizeof...(forms)>();
    template <size_t... I>
    constexpr char const** parse_(const char** begin, const char**,
                                  std::index_sequence<I...>) {
        std::string_view arg = begin[0];
        command_name = arg;
        return begin + (options[index<I>()].match_assign(arg, value) || ...);
    }

   public:
    [[no_unique_address]] Tag tag;
    command_fn value{};
    util::type_array<option<command_fn, forms>...> options;
    std::string_view command_name;

    constexpr char const** parse(char const** begin,
                                 [[maybe_unused]] char const** end) {
        return parse_(begin, end, indicies);
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
template <class Tag, flag_form... forms>
command_set(Tag, command_fn, option<command_fn, forms>...)
    -> command_set<Tag, forms...>;
template <class Tag, flag_form... forms>
command_set(Tag, std::nullptr_t, option<command_fn, forms>...)
    -> command_set<Tag, forms...>;
} // namespace arglet
