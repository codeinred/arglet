#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
constexpr arglet::tag<0> subcommand;
} // namespace tags

int hello(int, char const**);
int goodbye(int, char const**);
int print_name(int, char const**);

auto get_parser() {
    using namespace arglet;

    return sequence{
        ignore_arg,
        command_set{
            tags::subcommand,
            nullptr,
            option{'h', "hello", hello},
            option{'g', "goodbye", goodbye},
            option{'n', "name", print_name}}};
}

int main(int argc, char const* argv[]) {
    bool good = true;
    using namespace arglet::test;
    auto parser = get_parser();

    good = good && test(parser, "-h");
    good = good && check(parser, hello);
    parser = get_parser();

    good = good && test(parser, "-g");
    good = good && check(parser, goodbye);
    parser = get_parser();

    good = good && test(parser, "-n");
    good = good && check(parser, print_name);
    parser = get_parser();

    good = good && test(parser, "hello");
    good = good && check(parser, hello);
    parser = get_parser();

    good = good && test(parser, "goodbye");
    good = good && check(parser, goodbye);
    parser = get_parser();

    good = good && test(parser, "name");
    good = good && check(parser, print_name);
    parser = get_parser();

    good = good && test(parser);
    good = good && check(parser, nullptr);
    parser = get_parser();

    return !good;
}

int hello(int, char const**) {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
int goodbye(int, char const**) {
    std::cout << "Goodbye, world!" << std::endl;
    return 0;
}
int print_name(int, char const** argv) {
    std::cout << "Program name: " << argv[0] << std::endl;
    return 0;
}
