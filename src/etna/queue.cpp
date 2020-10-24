#include "queue.hpp"
#include "command.hpp"
#include "swapchain.hpp"
#include "synchronization.hpp"

#include <cassert>
#include <vector>

namespace etna {

void Queue::QueuePresentKHR(SwapchainKHR swapchain, uint32_t image_index, Array<Semaphore> wait_semaphores)
{
    assert(m_queue);

    VkSwapchainKHR vk_swapchain = swapchain;

    std::vector<VkSemaphore> vk_wait_semaphores(wait_semaphores.begin(), wait_semaphores.end());

    auto present_info = VkPresentInfoKHR{

        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext              = nullptr,
        .waitSemaphoreCount = narrow_cast<uint32_t>(vk_wait_semaphores.size()),
        .pWaitSemaphores    = vk_wait_semaphores.data(),
        .swapchainCount     = 1,
        .pSwapchains        = &vk_swapchain,
        .pImageIndices      = &image_index,
        .pResults           = nullptr
    };

    vkQueuePresentKHR(m_queue, &present_info);
}

void Queue::Submit(CommandBuffer command_buffer)
{
    Submit(command_buffer, nullptr, nullptr, nullptr);
}

void Queue::Submit(
    CommandBuffer        command_buffer,
    Array<Semaphore>     wait_semaphores,
    Array<PipelineStage> wait_stages,
    Array<Semaphore>     signal_semaphores)
{
    assert(m_queue);

    VkCommandBuffer                   vk_command_buffer = command_buffer;
    std::vector<VkSemaphore>          vk_wait_semaphores(wait_semaphores.begin(), wait_semaphores.end());
    std::vector<VkPipelineStageFlags> vk_wait_stages(wait_semaphores.size());
    std::vector<VkSemaphore>          vk_signal_semaphores(signal_semaphores.begin(), signal_semaphores.end());

    for (size_t i = 0; i < vk_wait_stages.size(); ++i) {
        vk_wait_stages[i] = GetVk(wait_stages[i]);
    }

    VkSubmitInfo info = {

        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = wait_semaphores.size(),
        .pWaitSemaphores      = wait_semaphores.empty() ? nullptr : vk_wait_semaphores.data(),
        .pWaitDstStageMask    = wait_semaphores.empty() ? nullptr : vk_wait_stages.data(),
        .commandBufferCount   = 1,
        .pCommandBuffers      = &vk_command_buffer,
        .signalSemaphoreCount = signal_semaphores.size(),
        .pSignalSemaphores    = signal_semaphores.empty() ? nullptr : vk_signal_semaphores.data()
    };

    vkQueueSubmit(m_queue, 1, &info, {});
}

} // namespace etna
