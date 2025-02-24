#pragma once
#include <exception>
#include <optional>
#include <format>
#include <iostream>
#include <stdexcept>

inline constexpr std::nullopt_t nil = std::nullopt;

class nullexception : public std::exception {
    public:
        nullexception() = default;

        [[nodiscard]] inline auto what() const noexcept -> const char* override {
            return "nil";
        }
};


template <class E = nullexception>
    requires std::is_base_of_v<std::exception, E>
class error {
    public:
        error() = default;

        constexpr error(E&& err) : err(std::forward<E>(err)) {}

        constexpr error(std::string&& str) : err(E(std::forward<std::string>(str))) {}

        template <class... Args>
        constexpr error(std::format_string<Args...> fmt, Args... args)
            : err(E(std::format(fmt, std::forward<Args>(args)...))) {}

        [[nodiscard]] inline constexpr auto to_string() const -> std::string {
            return std::string(err.has_value() ? err->what() : "nil");
        }

        [[nodiscard]] inline constexpr auto what() const -> std::string{
            return this->to_string();
        }

        friend std::ostream& operator<<(std::ostream& os, const error& err) {
            os << err.what();
            return os;
        }

        constexpr inline operator std::optional<E>() const {
            return err;
        }

        constexpr inline bool operator==(std::nullopt_t nullopt) const {
            return err == nullopt;
        }

    private:
        std::optional<E> err = nil; // Store the error (if any)
};



template <class E>
struct std::formatter<error<E>> : std::formatter<std::string> {
        constexpr auto parse(std::format_parse_context& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const error<E>& obj, FormatContext& ctx) const {
            return std::formatter<std::string>::format(obj.to_string(), ctx);
        }
};


//[[gnu::constructor(0)]]
//int entry() {
//    error<std::runtime_error> err("error: {}", 42);
//    error err2;
//    if (err != nil) {
//        std::print("{}\n", err);
//    }
//
//    if(err2 != nil) {
//        std::print("{}\n", err2);
//    } else {
//        std::print("nothing\n");
//    }
//
//    return 0;
//}