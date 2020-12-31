#include "misc.hpp"

#include <stdexcept>

namespace detail {

void throw_runtime_error_impl(const char* message)
{
    throw std::runtime_error(message);
}

} // namespace detail
