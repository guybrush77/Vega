#pragma once

#include "core.hpp"

namespace etna {

class SwapchainKHR {
  public:
    SwapchainKHR() noexcept = default;
    SwapchainKHR(std::nullptr_t) noexcept {}

    operator VkSwapchainKHR() const noexcept { return m_swapchain; }

    bool operator==(const SwapchainKHR&) const = default;

    auto Format() const noexcept { return static_cast<etna::Format>(m_format); }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    SwapchainKHR(VkSwapchainKHR swapchain, VkDevice device, VkFormat format) noexcept
        : m_swapchain(swapchain), m_device(device), m_format(format)
    {}

    static auto Create(VkDevice vk_device, const VkSwapchainCreateInfoKHR& create_info) -> UniqueSwapchainKHR;

    void Destroy() noexcept;

    VkSwapchainKHR m_swapchain{};
    VkDevice       m_device{};
    VkFormat       m_format{};
};

} // namespace etna
