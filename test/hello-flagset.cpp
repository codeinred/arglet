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
        section {
            flag_set(
                fs_item{hello, 'h', "--hello"},
                fs_item{print_name, 'n', "--print-name"},
                fs_item{goodbye, 'g', "--goodbye"})}};
}
int main(int argc, char const* argv[]) {
    auto parser = get_parser();
    parser.parse(argv);

    if (parser[tags::print_name]) {
        std::cout << "Running " << parser[tags::program_name] << std::endl;
    }
    if (parser[tags::hello]) {
        std::cout << ">>  hello, world!" << std::endl;
    }
    if (parser[tags::goodbye]) {
        std::cout << ">>  goodbye, world!" << std::endl;
    }
}
