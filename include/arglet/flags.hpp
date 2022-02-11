#pragma once
#include <string_view>

namespace arglet::flags {
using std::string_view;
struct NoFlag {};
constexpr NoFlag no_flag = NoFlag {};

enum class ParamSyntax {
    ShortFlag,
    LongFlag,
    EitherFlag,
};
enum class FlagKind {
    BoolFlag,
    OneArg,
    OptionalArg,
    ArgList,
};

template <class Type>
struct TypeSpec {
    using type = Type;
};

template <ParamSyntax>
struct ShortFlagT : TypeSpec<NoFlag> {};
template <>
struct ShortFlagT<ParamSyntax::ShortFlag> : TypeSpec<char> {};
template <>
struct ShortFlagT<ParamSyntax::EitherFlag> : TypeSpec<char> {};
template <ParamSyntax syntax>
using short_flag_t = typename ShortFlagT<syntax>::type;

template <ParamSyntax>
struct LongFlagT : TypeSpec<NoFlag> {};
template <>
struct LongFlagT<ParamSyntax::LongFlag> : TypeSpec<string_view> {};
template <>
struct LongFlagT<ParamSyntax::EitherFlag> : TypeSpec<string_view> {};
template <ParamSyntax syntax>
using long_flag_t = typename LongFlagT<syntax>::type;

template <ParamSyntax syntax, FlagKind kind>
struct flag_t {
    short_flag_t<syntax> short_flag;
    long_flag_t<syntax> long_flag;
};

} // namespace arglet::flags
