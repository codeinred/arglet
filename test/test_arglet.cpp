#include <arglet/flags.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("Check that we can create flag maps") {
    using namespace arglet::flags;
    using tuplet::tuple;
    using std::string_view_literals::operator""sv;

    auto tup = tuple {
        flag_arg<1, 1> {'d', "--dragon_fruit"},
        flag_arg<1, 1> {'v', "--version"},
        flag_arg<1, 1> {'e', "--eggplant"},
        flag_arg<1, 1> {'b', "--banana"},
        flag_arg<1, 1> {'c', "--cantelope"},
        flag_arg<1, 1> {'h', "--help"},
        flag_arg<0, 1> {{}, "--fondue"},
        flag_arg<1, 1> {'a', "--apple"},
        flag_arg<0, 1> {{}, "--glacier"},
        flag_arg<1, 0> {'g', {}},
    };

    auto short_args = make_short_flag_map(tup);
    auto long_args = make_long_flag_map(tup);

    REQUIRE(
        short_args.get_keys()
        == std::array {'a', 'b', 'c', 'd', 'e', 'g', 'h', 'v'});

    REQUIRE(
        long_args.get_keys()
        == std::array {
            "--apple"sv,
            "--banana"sv,
            "--cantelope"sv,
            "--dragon_fruit"sv,
            "--eggplant"sv,
            "--fondue"sv,
            "--glacier"sv,
            "--help"sv,
            "--version"sv});

    SECTION("Check that the search function works") {
        for (char ch = 'a'; ch <= 'z'; ch++) {
            auto idx = short_args.search(ch);

            if (idx > 0) {
                REQUIRE(ch >= short_args[idx].key);
            } else {
                REQUIRE(ch < short_args[1].key);
            }
        }
    }
}
