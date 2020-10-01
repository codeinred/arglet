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
        flag_group {
            flag {tags::hello, 'h', "--hello"},
            option_set {
                tags::print_name,
                false,
                option {'n', "--print-name", true},
                option {'x', "--dont-print-name", false}},
            flag {tags::goodbye, 'g', "--goodbye"}}};
}

int main(int argc, char const* argv[]) {
    using namespace arglet::test;
    bool good = true;

    // Check that no flags are set with no arguments
    {
        auto result = test(get_parser());
        good = good && check(result, false, false, false);
    }

    // Check that each flag works individually
    {
        auto result = test(get_parser(), "-h");
        good = good && check(result, true, false, false);
    }

    {
        auto result = test(get_parser(), "-n");
        good = good && check(result, false, true, false);
    }

    {
        auto result = test(get_parser(), "-g");
        good = good && check(result, false, false, true);
    }

    // Check that pairs of flags work
    {
        auto result = test(get_parser(), "-h", "-n");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "-n", "-g");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "-h", "-g");
        good = good && check(result, true, false, true);
    }

    // Check that pairs of flags work in reverse order
    {
        auto result = test(get_parser(), "-n", "-h");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "-g", "-n");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "-g", "-h");
        good = good && check(result, true, false, true);
    }

    // Check that all 3 flags work
    {
        auto result = test(get_parser(), "-h", "-n", "-g");
        good = good && check(result, true, true, true);
    }

    // Check that flags work when grouped
    {
        auto result = test(get_parser(), "-hn");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "-ng");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "-hg");
        good = good && check(result, true, false, true);
    }

    // Check flags work in reverse order when grouped
    {
        auto result = test(get_parser(), "-nh");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "-gn");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "-gh");
        good = good && check(result, true, false, true);
    }

    // Check that all 3 flags work when grouped
    {
        auto result = test(get_parser(), "-hng");
        good = good && check(result, true, true, true);
    }

    // Check that each flag works individually
    {
        auto result = test(get_parser(), "--hello");
        good = good && check(result, true, false, false);
    }

    {
        auto result = test(get_parser(), "--print-name");
        good = good && check(result, false, true, false);
    }

    {
        auto result = test(get_parser(), "--goodbye");
        good = good && check(result, false, false, true);
    }

    // Check that pairs of flags work
    {
        auto result = test(get_parser(), "--hello", "--print-name");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "--print-name", "--goodbye");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "--hello", "--goodbye");
        good = good && check(result, true, false, true);
    }

    // Check that pairs of flags work in reverse order
    {
        auto result = test(get_parser(), "--print-name", "--hello");
        good = good && check(result, true, true, false);
    }

    {
        auto result = test(get_parser(), "--goodbye", "--print-name");
        good = good && check(result, false, true, true);
    }

    {
        auto result = test(get_parser(), "--goodbye", "--hello");
        good = good && check(result, true, false, true);
    }

    // Check that all 3 flags work
    {
        auto result =
            test(get_parser(), "--hello", "--print-name", "--goodbye");
        good = good && check(result, true, true, true);
    }

    // Test that dont-print-name overwrites print-name
    {
        auto result = test(get_parser(), "-n", "-x");
        good = good && check(result, false, false, false);
    }

    {
        auto result = test(get_parser(), "-nx");
        good = good && check(result, false, false, false);
    }

    {
        auto result = test(get_parser(), "--print-name", "--dont-print-name");
        good = good && check(result, false, false, false);
    }

    // Test that dont-print-name overwrites print-name
    {
        auto result = test(get_parser(), "-h", "-n", "-x");
        good = good && check(result, true, false, false);
    }

    {
        auto result = test(get_parser(), "-hnx");
        good = good && check(result, true, false, false);
    }

    {
        auto result =
            test(get_parser(), "--hello", "--print-name", "--dont-print-name");
        good = good && check(result, true, false, false);
    }

    // Test that dont-print-name overwrites print-name
    {
        auto result = test(get_parser(), "-n", "-x");
        good = good && check(result, false, false, false);
    }

    {
        auto result = test(get_parser(), "-nx");
        good = good && check(result, false, false, false);
    }

    {
        auto result = test(get_parser(), "--print-name", "--dont-print-name");
        good = good && check(result, false, false, false);
    }

    return !good;
}
