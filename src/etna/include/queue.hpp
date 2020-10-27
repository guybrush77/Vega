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

    void QueuePresentKHR(SwapchainKHR swapchain, uint32_t image_index, ArrayView<Semaphore> wait_semaphores);

    void Submit(CommandBuffer command_buffer);

    void Submit(
        CommandBuffer        command_buffer,
        ArrayView<Semaphore>     wait_semaphores,
        ArrayView<PipelineStage> wait_stages,
        ArrayView<Semaphore>     signal_semaphores,
        Fence                fence);

  private:
    VkQueue m_queue{};
};

} // namespace etna
