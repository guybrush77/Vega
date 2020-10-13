#include <stdexcept>

namespace etna {

void throw_runtime_error(const char* description)
{
    throw std::runtime_error(description);
}

} // namespace etna
