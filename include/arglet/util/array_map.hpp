#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace arglet::util {
template <class Key, class Value>
struct map_entry {
    Key key {};
    Value value {};

    constexpr bool operator==(map_entry const& other) {
        return key == other.key;
    }
    constexpr auto operator<=>(map_entry const& other) {
        return key <=> other.key;
    }
};
template <class Key, class Value, size_t N>
class array_map {
   public:
    static_assert(N >= 1, "array_map expected a size of N >= 1");
    using entry_type = map_entry<Key, Value>;
    using key_type = Key;
    using value_type = Value;

    array_map() = default;
    array_map(array_map const&) = default;
    array_map(array_map&&) = default;

    constexpr void sort() { std::sort<entry_type*>(entries, entries + N); }

    constexpr entry_type& operator[](size_t i) noexcept { return entries[i]; }
    constexpr entry_type const& operator[](size_t i) const noexcept {
        return entries[i];
    }

    // Find the index of the item lexiconographically nearest to the arg using
    // binary search
    constexpr size_t search(Key arg) const {
        size_t min = 0, max = N, i = N / 2;
        while (max - min > 1) {
            auto cmp = arg <=> entries[i].key;
            if (cmp == 0) {
                return i;
            }
            if (cmp < 0) {
                max = i;
            } else {
                min = i;
            }
            i = (max + min) / 2;
        }
        return i;
    }

    constexpr entry_type* begin() noexcept { return entries; }
    constexpr entry_type* end() noexcept { return entries; }
    constexpr entry_type const* begin() const noexcept { return entries; }
    constexpr entry_type const* end() const noexcept { return entries; }

    // Performs a linear search to find the best element
    template <class Func>
    constexpr size_t minimize(Func&& fitness) const {
        using fit = std::decay_t<decltype(fitness(std::declval<Key>()))>;

        size_t result = 0;
        auto best_fit = fitness(entries[0]);
        for (size_t i = 1; i < N; i++) {
            auto current = fitness(entries[1]);
            if (current < best_fit) {
                best_fit = current;
                result = i;
            }
        }

        return result;
    }

    constexpr auto get_keys() const {
        std::array<Key, N> keys;
        auto it = keys.begin();
        for(auto& elem : entries) {
            *it++ = elem.key;
        }
        return keys;
    }

    constexpr static size_t size() noexcept {
        return N;
    }
   private:
    entry_type entries[N] {};
};
} // namespace arglet::util
