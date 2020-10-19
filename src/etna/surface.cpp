#include "surface.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

void SurfaceKHR::Destroy() noexcept
{
    assert(m_surface);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    spdlog::info(COMPONENT "Destroyed VkSurfaceKHR {}", fmt::ptr(m_surface));

    m_instance = nullptr;
    m_surface  = nullptr;
}

} // namespace etna
