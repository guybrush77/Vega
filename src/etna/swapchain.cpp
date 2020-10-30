#include "swapchain.hpp"

#include <cassert>

namespace etna {
UniqueSwapchainKHR SwapchainKHR::Create(VkDevice vk_device, const VkSwapchainCreateInfoKHR& create_info)
{
    VkSwapchainKHR vk_swapchain{};

    if (auto result = vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_swapchain); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueSwapchainKHR(SwapchainKHR(vk_swapchain, vk_device, create_info.imageFormat));
}

void SwapchainKHR::Destroy() noexcept
{
    assert(m_swapchain);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    m_swapchain = nullptr;
    m_device    = nullptr;
    m_format    = {};
}

} // namespace etna
