#include "etna/surface.hpp"

#include <cassert>

namespace etna {

void SurfaceKHR::Destroy() noexcept
{
    assert(m_surface);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    m_instance = nullptr;
    m_surface  = nullptr;
}

} // namespace etna
