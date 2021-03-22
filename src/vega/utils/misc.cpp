#include "misc.hpp"

#include <assert.h>
#include <cctype>
#include <stdexcept>

namespace detail {

void throw_runtime_error_impl(const char* message)
{
    throw std::runtime_error(message);
}

} // namespace detail

void utils::to_lower(char* text, size_t size) noexcept
{
    assert(text);

    while (*text && size) {
        *text = static_cast<char>(std::tolower(*text));
        ++text;
        --size;
    }
}
