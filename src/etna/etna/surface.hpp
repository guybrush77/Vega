#pragma once

#include "core.hpp"

namespace etna {

class SurfaceKHR {
  public:
    SurfaceKHR() noexcept = default;

    SurfaceKHR(std::nullptr_t) noexcept {}
    SurfaceKHR(VkInstance instance, VkSurfaceKHR surface) noexcept : m_instance(instance), m_surface(surface) {}

    operator VkSurfaceKHR() const noexcept { return m_surface; }

    explicit operator bool() const noexcept { return m_surface != nullptr; }

    bool operator==(const SurfaceKHR& rhs) const noexcept
    {
        return m_instance == rhs.m_instance && m_surface == rhs.m_surface;
    }
    bool operator!=(const SurfaceKHR& rhs) const noexcept { return !operator==(rhs); }

  private:
    template <typename>
    friend class UniqueHandle;

    void Destroy() noexcept;

    VkInstance   m_instance{};
    VkSurfaceKHR m_surface{};
};

} // namespace etna
