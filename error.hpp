#pragma once
#include <exception>
#include <optional>
#include <format>
#include <iostream>
#include <stdexcept>

inline constexpr std::nullopt_t nil = std::nullopt;

template <class E>
    requires std::is_base_of_v<std::exception, E>
class error {
    public:
        error() = default;

        // Constructor to initialize with an exception object
        constexpr error(E&& err) : err(std::forward<E>(err)) {}

        // Constructor to initialize with a string
        constexpr error(std::string&& str) : err(E(std::forward<std::string>(str))) {}

        // Constructor to initialize with a format string and arguments
        template <class... Args>
        constexpr error(std::format_string<Args...> fmt, Args... args)
            : err(E(std::format(fmt, std::forward<Args>(args)...))) {}

        // Convert the error to a string representation
        [[nodiscard]] constexpr std::string to_string() const {
            return std::string(err.has_value() ? err->what() : "nil");
        }

        friend std::ostream& operator<<(std::ostream& os, const error& err) {
            os << err.to_string();
            return os;
        }

        operator std::optional<E>() const {
            return err;
        }

    private:
        std::optional<E> err = nil; // Store the error (if any)
};

//TODO: get formmater working: std::print, std::format, std::println
template <class E>
struct std::formatter<error<E>> : std::formatter<std::string> {
        constexpr auto parse(std::format_parse_context& ctx) {
            return ctx.begin();
        }

        auto format(const error<E>& obj, std::format_context& ctx) const {
            return std::formatter<std::string>::format(obj.to_string(), ctx);
        }
};

[[gnu::constructor(0)]]
int entry() {
    error<std::runtime_error> err("error: {}", 42);
//    std::string formatted = std::format("{}", err); // Use std::format explicitly
    std::cout << err << "\n"; // Print the formatted string

//    std::format("{}", err);
//    std::print("{}\n", err);
    return 0;
}