#include "queue.hpp"
#include "command.hpp"

#include <cassert>

namespace etna {

void Queue::Submit(CommandBuffer command_buffer)
{
    assert(m_queue);

    VkCommandBuffer vk_command_buffer = command_buffer;

    VkSubmitInfo info = {

        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext                = nullptr,
        .waitSemaphoreCount   = 0,
        .pWaitSemaphores      = nullptr,
        .pWaitDstStageMask    = nullptr,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &vk_command_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores    = nullptr
    };

    vkQueueSubmit(m_queue, 1, &info, {});
}

} // namespace etna
