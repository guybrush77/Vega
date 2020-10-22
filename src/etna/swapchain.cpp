#include "swapchain.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {
UniqueSwapchainKHR SwapchainKHR::Create(VkDevice vk_device, const VkSwapchainCreateInfoKHR& create_info)
{
    VkSwapchainKHR vk_swapchain{};

    if (auto result = vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_swapchain); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateSwapchainKHR error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkSwapchainKHR {}", fmt::ptr(vk_swapchain));

    return UniqueSwapchainKHR(SwapchainKHR(vk_swapchain, vk_device, create_info.imageFormat));
}

void SwapchainKHR::Destroy() noexcept
{
    assert(m_swapchain);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    spdlog::info(COMPONENT "Destroyed VkSwapchainKHR {}", fmt::ptr(m_swapchain));

    m_swapchain = nullptr;
    m_device    = nullptr;
    m_format    = {};
}

} // namespace etna
