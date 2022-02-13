#pragma once
#include <algorithm>
#include <cstddef>

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

    // Find the index of the item lexiconographically nearest to the arg
    constexpr size_t search(Key arg) const {
        size_t min = 0, max = N, i = N / 2;
        while (min < max) {
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

   private:
    entry_type entries[N] {}
};
} // namespace arglet::util
