#pragma once
#include <type_traits>

namespace arglet::util {
template <class T>
T exchange(T& value, T&& moved) noexcept(
    std::is_nothrow_move_constructible_v<T>&&
        std::is_nothrow_move_assignable_v<T>) {
    T tmp = static_cast<T&&>(value);
    value = static_cast<T&&>(moved);
    return tmp;
}
} // namespace arglet::util
