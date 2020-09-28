#include <arglet/arglet.hpp>
#include <cmath>
#include <filesystem>
#include <iostream>

namespace tags {
using namespace arglet;
constexpr tag<0> scope;
constexpr tag<1> use_color;
constexpr tag<2> list_directories;
constexpr tag<3> long_format;
constexpr tag<4> size_display_format;
constexpr tag<5> reverse_order;
constexpr tag<6> show_block_size;
constexpr tag<7> sort_method;
constexpr tag<8> list_recurse;
constexpr tag<9> block_size;
constexpr tag<10> files;
} // namespace tags
enum class show_mode { regular, almost_all, all };
enum class size_display_mode { bytes, kibi, kilo };
enum class color_mode { always, never, automatic };
enum class sort_mode { none, file_size, time, extension };
constexpr auto parse_block_size =
    [](std::string_view arg) -> unsigned long long {
    unsigned long long value = 0;
    auto [scan, errc] = std::from_chars(arg.begin(), arg.end(), value);
    if (scan == arg.end()) {
        return value;
    } else if (scan + 1 == arg.end()) {
        switch (*scan) {
            case 'K': return value * std::pow(1024ull, 1);
            case 'M': return value * std::pow(1024ull, 2);
            case 'G': return value * std::pow(1024ull, 3);
            case 'T': return value * std::pow(1024ull, 4);
            case 'P': return value * std::pow(1024ull, 5);
            case 'E': return value * std::pow(1024ull, 6);
            case 'Z': return value * std::pow(1024ull, 7);
            case 'Y': return value * std::pow(1024ull, 8);
        }
        throw std::logic_error(
            std::string("Unrecognized suffix in --block_size=") + arg.data());
    } else if (scan + 2 == arg.end() && scan[1] == 'B') {
        switch (*scan) {
            case 'K': return value * std::pow(1000ull, 1);
            case 'M': return value * std::pow(1000ull, 2);
            case 'G': return value * std::pow(1000ull, 3);
            case 'T': return value * std::pow(1000ull, 4);
            case 'P': return value * std::pow(1000ull, 5);
            case 'E': return value * std::pow(1000ull, 6);
            case 'Z': return value * std::pow(1000ull, 7);
            case 'Y': return value * std::pow(1000ull, 8);
        }
        throw std::logic_error(
            std::string("Unrecognized suffix in --block_size=") + arg.data());
    }
    throw std::logic_error(
        std::string(
            "Block size specification must be a number in --block_size=")
        + arg.data());
};
auto get_parser() {
    using namespace tags;
    using namespace arglet;
    return sequence {
        ignore_arg,
        group {
            flag_group {
                option_set {
                    scope,
                    show_mode::regular,
                    option {'a', show_mode::all},
                    option {'A', show_mode::almost_all}},
                option_set {
                    use_color,
                    color_mode::never,
                    option {"--color=always", color_mode::always},
                    option {"--color=never", color_mode::never},
                    option {"--color=auto", color_mode::automatic}},
                flag {list_directories, 'd'},
                flag {long_format, 'l'},
                option_set {
                    size_display_format,
                    size_display_mode::bytes,
                    option {'h', "--human_readable", size_display_mode::kibi},
                    option {"--si", size_display_mode::kilo}},
                flag {reverse_order, 'r'},
                flag {list_recurse, 'R'},
                flag {show_block_size, 's'},
                option_set {
                    sort_method,
                    sort_mode::none,
                    option {'S', sort_mode::file_size},
                    option {'t', sort_mode::time},
                    option {'X', sort_mode::extension}}},
            prefixed_value {
                block_size, "--block-size=", 1024ull, parse_block_size},
            item {files, std::vector<std::filesystem::path>()}}};
}

int main(int argc, char const** argv) {
    auto parser = get_parser();
    parser.parse(argc, argv);
    for (auto& file : parser[tags::files]) {
        std::cout << file << '\n';
    }
}
