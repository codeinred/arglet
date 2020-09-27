#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
struct hello_t {} hello;
struct goodbye_t {} goodbye;
struct print_name_t {} print_name;
struct program_name_t {} program_name;
} // namespace tags

auto get_parser() {
    using namespace arglet;

    return sequence{
        string{tags::program_name},
        flag_group{
            flag{tags::hello, 'h', "--hello"},
            flag{tags::print_name, 'n', "--print-name"},
            flag{tags::goodbye, 'g', "--goodbye"}}};
}

int main(int argc, char const* argv[]) {
    auto parser = get_parser();
    int num_args_parsed = parser.parse(argc, argv);

    if(std::optional name = parser[tags::program_name]) {
        std::cout << ">> Running " << name.value() << std::endl;
    }
    if (parser[tags::hello]) {
        std::cout << ">>  hello, world!" << std::endl;
    }
    if (parser[tags::goodbye]) {
        std::cout << ">>  goodbye, world!" << std::endl;
    }

    if(num_args_parsed < argc) {
        for(int i = num_args_parsed; i < argc; i++) {
            std::cout << "Couldn't parse: " << argv[i] << '\n';
        }
        return 1;
    } else {
        return 0;
    }
}
