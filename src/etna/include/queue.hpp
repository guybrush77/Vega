#pragma once

#include "core.hpp"

namespace etna {

class Queue {
  public:
    Queue() noexcept {}
    Queue(std::nullptr_t) noexcept {}
    Queue(VkQueue queue) noexcept : m_queue(queue) {}

    operator VkQueue() const noexcept { return m_queue; }

    bool operator==(const Queue&) const = default;

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
    VkQueue m_queue{};
};

} // namespace etna
