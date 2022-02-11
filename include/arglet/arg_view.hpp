#include <algorithm>
#include <string_view>

#include <arglet/token.hpp>
#include <arglet/util.hpp>

namespace arglet {
using std::string_view;

class arg_view {
    char const** start_ {nullptr};
    char const** end_ {nullptr};
    token current {};

    arg_view(char const** Start, char const** End) noexcept
      : start_(Start)
      , end_(End) {}

   public:
    /**
     * @brief Initializes arg_view. If either argc <= 0 or argv == nullptr, the
     * arg view will be empty
     *
     */
    constexpr arg_view(int argc, char const** argv) noexcept {
        if (argc < 0 || argv == nullptr) {
            // arg_view should be empty if either of these are true
            return;
        }
        // Now we know argc >= 0 and argv is not null
        start_ = argv;
        end_ = start_ + argc;

        // We only have a current argument if argc > 0
        if (argc > 0) {
            current = token(*argv);
        }
    }
    arg_view() = default;
    arg_view(arg_view const&) = default;

    // Pop the token at the front of the list of arguments. If the list of
    // arguments is empty, return an empty token (will evaluate to false whech
    // tested)
    constexpr token pop() noexcept {
        // This will break if fullptr is passed in
        if (start_ + 1 < end_) {
            // Increment the start
            start_ += 1;
            // Replace the current token with the next token, and return the old
            // value
            return util::exchange(current, token(*start_));
        } else {
            start_ = end_;
            // Replace the current token with an empty token
            return util::exchange(current, token());
        }
    }

    // Return the token at the front of the list of arguments. Returns an empty
    // token if the list is empty.
    constexpr token peek() const noexcept { return current; }

    // Checks if the current argument starts with a given character
    constexpr bool starts_with(char ch) const noexcept {
        return current.starts_with(ch);
    }

    // Checks if the current argument starts with the given string view
    constexpr bool starts_with(string_view sv) const noexcept {
        return current.starts_with(sv);
    }

    // Get the number of arguments in the view
    constexpr size_t size() const noexcept {
        return end_ - start_;
    }

    // Checks if the arg_view is empty (has 0 elements)
    constexpr bool empty() const noexcept { return start_ == end_; }

    // Checks if the arg_view has 1 or more elements
    constexpr bool has() const noexcept { return start_ < end_; }

    // Returns true if the arg_view has 1 or more elements
    constexpr operator bool() const noexcept { return start_ < end_; }
};
} // namespace arglet
