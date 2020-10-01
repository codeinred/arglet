#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
using arglet::tag;
constexpr tag<0> hello;
constexpr tag<1> goodbye;
constexpr tag<2> strings;
} // namespace tags

auto get_parser() {
    using namespace arglet;

    return sequence {
        ignore_arg,
        group {
            flag {tags::hello, "--hello"},
            flag {tags::goodbye, "--goodbye"},
            item {tags::strings, std::vector<std::string_view>()}}};
}

int main() {
    using namespace std::literals;
    using namespace arglet::test;
    bool good = true;
    auto parser = get_parser();

    good = good && test(parser, "x", "y", "--hello");
    good = good && check(parser, true, false, std::vector{"x"sv, "y"sv});
    parser = get_parser();

    good = good && test(parser, "x", "y", "--goodbye");
    good = good && check(parser, false, true, std::vector{"x"sv, "y"sv});
    parser = get_parser();

    good = good && test(parser, "x", "y", "--goodbye", "--hello");
    good = good && check(parser, true, true, std::vector{"x"sv, "y"sv});
    parser = get_parser();

    return !good;
}
