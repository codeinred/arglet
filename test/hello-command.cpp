#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
struct program_name_t {} program_name;
struct subcommand_t {} subcommand;
} // namespace tags

int hello(int, char const**);
int goodbye(int, char const**);
int print_name(int, char const**);

auto get_parser() {
    using namespace arglet;

    return sequence{
        string{tags::program_name},
        command_set{
            tags::subcommand,
            option{"hello", hello},
            option{"goodbye", goodbye},
            option{"help", arglet::unimplemented_command},
            option{"name", print_name}}};
}

int main(int argc, char const* argv[]) {
    auto parser = get_parser();
    parser.parse(argc, argv);
    return parser[tags::subcommand](argc, argv);
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
