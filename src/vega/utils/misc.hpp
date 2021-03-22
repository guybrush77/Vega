#pragma once

namespace detail {

void throw_runtime_error_impl(const char* message);

} // namespace detail

namespace utils {

inline void throw_runtime_error(const char* message)
{
    detail::throw_runtime_error_impl(message);
}

inline void throw_runtime_error_if(bool condition, const char* message)
{
    if (condition) {
        detail::throw_runtime_error_impl(message);
    }
}

void to_lower(char* text, size_t size = ~0U) noexcept;

} // namespace utils
