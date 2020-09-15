#include <arglet/arglet.hpp>
#include <iostream>

namespace tags {
struct hello_t {};
struct goodbye_t {};
struct print_name_t {};
struct program_name_t {};
constexpr hello_t hello{};
constexpr goodbye_t goodbye{};
constexpr print_name_t print_name;
constexpr program_name_t program_name{};
} // namespace tags

auto get_parser() {
    using namespace arglet;
    using namespace tags;

    return sequence{
        string{program_name},
        section{
            flag{hello, "-h", "--hello"},
            flag{print_name, "-n", "--print-name"},
            flag{goodbye, "-g", "--goodbye"}}};
}
int main(int argc, char const* argv[]) {
    auto parser = get_parser();
    parser.parse(argv);

    std::cout << parser[tags::program_name] << std::endl;

    if (parser[tags::hello]) {
        std::cout << ">>  hello, world!" << std::endl;
    }
    if (parser[tags::goodbye]) {
        std::cout << ">>  goodbye, world!" << std::endl;
    }
}
