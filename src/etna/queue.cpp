#include "etna/queue.hpp"
#include "etna/command.hpp"
#include "etna/swapchain.hpp"
#include "etna/synchronization.hpp"

#include <algorithm>
#include <array>
#include <cassert>

namespace etna {

Result
Queue::QueuePresentKHR(SwapchainKHR swapchain, uint32_t image_index, std::initializer_list<Semaphore> wait_semaphores)
{
    assert(m_queue);

    constexpr size_t kMaxSemaphores = 16;

    std::array<VkSemaphore, kMaxSemaphores> vk_semaphores;

    if (wait_semaphores.size() > vk_semaphores.size()) {
        throw_etna_error(__FILE__, __LINE__, "Too many elements in std::initializer_list<Semaphore>");
    }

    std::transform(wait_semaphores.begin(), wait_semaphores.end(), vk_semaphores.begin(), [](Semaphore semaphore) {
        return VkSemaphore(semaphore);
    });

    auto vk_swapchain = static_cast<VkSwapchainKHR>(swapchain);
    auto vk_size      = narrow_cast<uint32_t>(wait_semaphores.size());

    auto present_info = VkPresentInfoKHR{

        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = vk_size,
        .pWaitSemaphores    = vk_semaphores.data(),
        .swapchainCount     = 1,
        .pSwapchains        = &vk_swapchain,
        .pImageIndices      = &image_index,
        .pResults           = nullptr
    };

    auto vk_result = vkQueuePresentKHR(m_queue, &present_info);

    return static_cast<Result>(vk_result);
}

void Queue::Submit(CommandBuffer command_buffer)
{
    Submit(command_buffer, {}, {}, {}, {});
}

void Queue::Submit(
    CommandBuffer                        command_buffer,
    std::initializer_list<Semaphore>     wait_semaphores,
    std::initializer_list<PipelineStage> wait_stages,
    std::initializer_list<Semaphore>     signal_semaphores,
    Fence                                fence)
{
    assert(m_queue);

    constexpr size_t kMaxWaitSemaphores   = 16;
    constexpr size_t kMaxSignalSemaphores = 16;

    std::array<VkSemaphore, kMaxWaitSemaphores>   vk_wait_semaphores;
    std::array<VkSemaphore, kMaxSignalSemaphores> vk_signal_semaphores;

    if (wait_semaphores.size() > vk_wait_semaphores.size() || signal_semaphores.size() > vk_signal_semaphores.size()) {
        throw_etna_error(__FILE__, __LINE__, "Too many elements in std::initializer_list<Semaphore>");
    }

    auto vk_command_buffer = static_cast<VkCommandBuffer>(command_buffer);
    auto vk_wait_stages    = reinterpret_cast<const VkPipelineStageFlags*>(wait_stages.begin());
    auto vk_semaphore      = [](Semaphore semaphore) { return VkSemaphore(semaphore); };

    std::transform(wait_semaphores.begin(), wait_semaphores.end(), vk_wait_semaphores.begin(), vk_semaphore);
    std::transform(signal_semaphores.begin(), signal_semaphores.end(), vk_signal_semaphores.begin(), vk_semaphore);

    auto submit_info = VkSubmitInfo{

        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = narrow_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores      = wait_semaphores.size() ? vk_wait_semaphores.data() : nullptr,
        .pWaitDstStageMask    = wait_stages.size() ? vk_wait_stages : nullptr,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &vk_command_buffer,
        .signalSemaphoreCount = narrow_cast<uint32_t>(signal_semaphores.size()),
        .pSignalSemaphores    = signal_semaphores.size() ? vk_signal_semaphores.data() : nullptr
    };

    vkQueueSubmit(m_queue, 1, &submit_info, fence);
}

} // namespace etna
