#include "synchronization.hpp"

#include <cassert>

namespace etna {

UniqueSemaphore Semaphore::Create(VkDevice vk_device, const VkSemaphoreCreateInfo& create_info)
{
    VkSemaphore vk_semaphore{};

    if (auto result = vkCreateSemaphore(vk_device, &create_info, nullptr, &vk_semaphore); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueSemaphore(Semaphore(vk_semaphore, vk_device));
}

void Semaphore::Destroy() noexcept
{
    assert(m_semaphore);

    vkDestroySemaphore(m_device, m_semaphore, nullptr);

    m_semaphore = nullptr;
    m_device    = nullptr;
}

auto Fence::Create(VkDevice vk_device, const VkFenceCreateInfo& create_info) -> UniqueFence
{
    VkFence vk_fence{};

    if (auto result = vkCreateFence(vk_device, &create_info, nullptr, &vk_fence); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueFence(Fence(vk_fence, vk_device));
}

void Fence::Destroy() noexcept
{
    assert(m_fence);

    vkDestroyFence(m_device, m_fence, nullptr);

    m_fence  = nullptr;
    m_device = nullptr;
}

} // namespace etna
