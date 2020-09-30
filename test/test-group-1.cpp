#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
using arglet::tag;
constexpr tag<0> hello;
constexpr tag<1> print_name;
constexpr tag<2> goodbye;
} // namespace tags

auto get_parser() {
    using namespace arglet;

    return sequence {
        ignore_arg,
        group {
            flag {tags::hello, 'h', "--hello"},
            flag {tags::print_name, 'n', "--print-name"},
            flag {tags::goodbye, 'g', "--goodbye"}}};
}

int main(int argc, char const* argv[]) {
    using namespace arglet::test;
    bool good = true;
    auto parser = get_parser();

    // Check that no flags are set with no arguments
    test(parser);
    good = good && check(parser, false, false, false);
    parser = get_parser(); // Reset the parser

    // Check that each flag works individually
    test(parser, "-h");
    good = good && check(parser, true, false, false);
    parser = get_parser(); // Reset the parser

    test(parser, "-n");
    good = good && check(parser, false, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "-g");
    good = good && check(parser, false, false, true);
    parser = get_parser(); // Reset the parser

    // Check that pairs of flags work
    test(parser, "-h", "-n");
    good = good && check(parser, true, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "-n", "-g");
    good = good && check(parser, false, true, true);
    parser = get_parser(); // Reset the parser

    test(parser, "-h", "-g");
    good = good && check(parser, true, false, true);
    parser = get_parser(); // Reset the parser

    // Check that pairs of flags work in reverse order
    test(parser, "-n", "-h");
    good = good && check(parser, true, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "-g", "-n");
    good = good && check(parser, false, true, true);
    parser = get_parser(); // Reset the parser

    test(parser, "-g", "-h");
    good = good && check(parser, true, false, true);
    parser = get_parser(); // Reset the parser

    // Check that all 3 flags work
    test(parser, "-h", "-n", "-g");
    good = good && check(parser, true, true, true);
    parser = get_parser(); // Reset the parser

    // Check that each flag works individually
    test(parser, "--hello");
    good = good && check(parser, true, false, false);
    parser = get_parser(); // Reset the parser

    test(parser, "--print-name");
    good = good && check(parser, false, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "--goodbye");
    good = good && check(parser, false, false, true);
    parser = get_parser(); // Reset the parser

    // Check that pairs of flags work
    test(parser, "--hello", "--print-name");
    good = good && check(parser, true, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "--print-name", "--goodbye");
    good = good && check(parser, false, true, true);
    parser = get_parser(); // Reset the parser

    test(parser, "--hello", "--goodbye");
    good = good && check(parser, true, false, true);
    parser = get_parser(); // Reset the parser

    // Check that pairs of flags work in reverse order
    test(parser, "--print-name", "--hello");
    good = good && check(parser, true, true, false);
    parser = get_parser(); // Reset the parser

    test(parser, "--goodbye", "--print-name");
    good = good && check(parser, false, true, true);
    parser = get_parser(); // Reset the parser

    test(parser, "--goodbye", "--hello");
    good = good && check(parser, true, false, true);
    parser = get_parser(); // Reset the parser

    // Check that all 3 flags work
    test(parser, "--hello", "--print-name", "--goodbye");
    good = good && check(parser, true, true, true);
    parser = get_parser(); // Reset the parser

    return !good;
}
