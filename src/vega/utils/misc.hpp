#pragma once

#include <fmt/format.h>

void throw_runtime_error_impl(const std::string& s);

template <typename... Args>
inline void throw_runtime_error(const char* format, Args... args)
{
    throw_runtime_error_impl(fmt::format(format, args...));
}

template <typename... Args>
inline void throw_runtime_error_if(bool condition, const char* format, Args... args)
{
    if (condition) {
        throw_runtime_error_impl(fmt::format(format, args...));
    }
}
