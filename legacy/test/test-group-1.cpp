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

    return !good;
}
