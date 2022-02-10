#include <arglet/arglet.hpp>

// If this compiles, we gucci
int main() {
    using std::literals::operator""sv;
    using namespace arglet;
    constexpr auto identity = [](auto x) { return x; };
    using identity_t = std::decay_t<decltype(identity)>;

    static_assert(std::is_same_v<
                  decltype(item {tag_v<0>, identity}),
                  item<
                      tag<0>,
                      value_parser<
                          std::vector<std::string_view>,
                          identity_t,
                          true>>>);
    static_assert(std::is_same_v<
                  decltype(list {tag_v<0>, identity}),
                  list<
                      tag<0>,
                      value_parser<
                          std::vector<std::string_view>,
                          identity_t,
                          true>>>);
    static_assert(std::is_same_v<
                  decltype(value {tag_v<0>, identity}),
                  value<
                      tag<0>,
                      value_parser<
                          std::optional<std::string_view>,
                          identity_t,
                          true>>>);

    static_assert(std::is_same_v<
                  decltype(prefixed_value {tag_v<0>, "--my_value=", identity}),
                  prefixed_value<
                      tag<0>,
                      flag_form::Long,
                      value_parser<
                          std::optional<std::string_view>,
                          identity_t,
                          true>>>);

    static_assert(std::is_same_v<
                  decltype(value_flag {tag_v<0>, "--my_value", identity}),
                  value_flag<
                      tag<0>,
                      flag_form::Long,
                      value_parser<
                          std::optional<std::string_view>,
                          identity_t,
                          true>>>);

    return 0;
}
