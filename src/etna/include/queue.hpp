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

    void QueuePresentKHR(SwapchainKHR swapchain, uint32_t image_index, Array<Semaphore> wait_semaphores);

    void Submit(CommandBuffer command_buffer);

    void Submit(
        CommandBuffer        command_buffer,
        Array<Semaphore>     wait_semaphores,
        Array<PipelineStage> wait_stages,
        Array<Semaphore>     signal_semaphores,
        Fence                fence);

  private:
    VkQueue m_queue{};
};

} // namespace etna
