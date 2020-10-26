#include "synchronization.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

UniqueSemaphore Semaphore::Create(VkDevice vk_device, const VkSemaphoreCreateInfo& create_info)
{
    VkSemaphore vk_semaphore{};

    if (auto result = vkCreateSemaphore(vk_device, &create_info, nullptr, &vk_semaphore); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateSemaphore error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkSemaphore {}", fmt::ptr(vk_semaphore));

    return UniqueSemaphore(Semaphore(vk_semaphore, vk_device));
}

void Semaphore::Destroy() noexcept
{
    assert(m_semaphore);

    vkDestroySemaphore(m_device, m_semaphore, nullptr);

    spdlog::info(COMPONENT "Destroyed VkSemaphore {}", fmt::ptr(m_semaphore));

    m_semaphore = nullptr;
    m_device    = nullptr;
}

auto Fence::Create(VkDevice vk_device, const VkFenceCreateInfo& create_info) -> UniqueFence
{
    VkFence vk_fence{};

    if (auto result = vkCreateFence(vk_device, &create_info, nullptr, &vk_fence); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateFence error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkFence {}", fmt::ptr(vk_fence));

    return UniqueFence(Fence(vk_fence, vk_device));
}

void Fence::Destroy() noexcept
{
    assert(m_fence);

    vkDestroyFence(m_device, m_fence, nullptr);

    spdlog::info(COMPONENT "Destroyed VkFence {}", fmt::ptr(m_fence));

    m_fence  = nullptr;
    m_device = nullptr;
}

} // namespace etna
