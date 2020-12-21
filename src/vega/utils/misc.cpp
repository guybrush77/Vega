#include "misc.hpp"

void throw_runtime_error_impl(const std::string& s)
{
    throw std::runtime_error(s);
}
