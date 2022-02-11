#pragma once

#include <string_view>
namespace arglet {
using std::string_view;
struct token : string_view {
    using string_view::data;
    using string_view::string_view;

    token() = default;
    token(token const&) = default;

    // Returns true if data() == nullptr
    constexpr bool null() const { return string_view::data() == nullptr; }
    // Returns true if data() != nullptr
    constexpr bool has() const { return string_view::data() != nullptr; }
    // Returns true if the token is a valid token (data() != nullptr)
    constexpr operator bool() const { return string_view::data() != nullptr; }

    constexpr bool operator==(token const&) const = default;
    constexpr auto operator<=>(token const&) const = default;
};
} // namespace arglet
