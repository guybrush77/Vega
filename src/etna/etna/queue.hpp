#pragma once

#include "core.hpp"

namespace etna {

class Queue {
  public:
    Queue() noexcept {}
    Queue(std::nullptr_t) noexcept {}
    Queue(VkQueue queue, uint32_t family_index) noexcept : m_queue(queue), m_family_index(family_index) {}

    operator VkQueue() const noexcept { return m_queue; }

    bool operator==(const Queue&) const = default;

    auto FamilyIndex() const noexcept { return m_family_index; }

    auto QueuePresentKHR(SwapchainKHR swapchain, uint32_t image_index, std::initializer_list<Semaphore> wait_semaphores)
        -> Result;

    void Submit(CommandBuffer command_buffer);

    void Submit(
        CommandBuffer                        command_buffer,
        std::initializer_list<Semaphore>     wait_semaphores,
        std::initializer_list<PipelineStage> wait_stages,
        std::initializer_list<Semaphore>     signal_semaphores,
        Fence                                fence);

  private:
    VkQueue  m_queue{};
    uint32_t m_family_index{};
};

} // namespace etna
