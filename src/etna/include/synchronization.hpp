#pragma once

#include "core.hpp"

namespace etna {

class Semaphore {
  public:
    Semaphore() noexcept = default;
    Semaphore(std::nullptr_t) noexcept {}

    operator VkSemaphore() const noexcept { return m_semaphore; }

    bool operator==(const Semaphore&) const = default;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Semaphore(VkSemaphore semaphore, VkDevice device) noexcept : m_semaphore(semaphore), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkSemaphoreCreateInfo& create_info) -> UniqueSemaphore;

    void Destroy() noexcept;

    VkSemaphore m_semaphore{};
    VkDevice    m_device{};
};

class Fence {
  public:
    Fence() noexcept = default;
    Fence(std::nullptr_t) noexcept {}

    operator VkFence() const noexcept { return m_fence; }

    bool operator==(const Fence&) const = default;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Fence(VkFence fence, VkDevice device) noexcept : m_fence(fence), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkFenceCreateInfo& create_info) -> UniqueFence;

    void Destroy() noexcept;

    VkFence  m_fence{};
    VkDevice m_device{};
};
} // namespace etna
