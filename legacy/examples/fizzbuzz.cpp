#include <arglet/arglet.hpp>

std::string_view fizzbuzz(std::string_view arg) {
    using namespace std::literals;
    long val = 0;
    std::from_chars(arg.data(), arg.data() + arg.size(), val);
    switch ((val % 3 == 0) + 2 * (val % 5 == 0)) {
        case 0: return arg;
        case 1: return "Fizz";
        case 2: return "Buzz";
        case 3: return "FizzBuzz";
    }
    return "[Bad Argument]";
}

auto get_parser() {
    using namespace arglet;

    return sequence {ignore_arg, list {tag_v<0>, fizzbuzz}};
}

int main(int argc, char const** argv) {
    auto p = get_parser();
    p.parse(argc, argv);
    for (std::string_view& v : p[arglet::tag_v<0>]) {
        printf("%s\n", v.data());
    }
    return 0;
}
