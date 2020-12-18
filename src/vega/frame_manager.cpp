#include "frame_manager.hpp"

FrameManager::FrameManager(etna::Device device, uint32_t queue_family_index, uint32_t frame_count)
    : m_device(device), m_frame_count(frame_count), m_next_frame(0)
{
    m_command_pool = device.CreateCommandPool(queue_family_index, etna::CommandPoolCreate::ResetCommandBuffer);

    for (uint32_t frame_index = 0; frame_index < frame_count; ++frame_index) {
        m_draw_command_buffers.push_back(m_command_pool->AllocateCommandBuffer());
        m_gui_command_buffers.push_back(m_command_pool->AllocateCommandBuffer());
        m_image_acquired_sempahores.push_back(device.CreateSemaphore());
        m_draw_completed_sempahores.push_back(device.CreateSemaphore());
        m_gui_completed_sempahores.push_back(device.CreateSemaphore());
        m_frame_available_fences.push_back(device.CreateFence(etna::FenceCreate::Signaled));
        m_frame_info.push_back(FrameInfo{
            frame_index,
            { *m_draw_command_buffers.back(), *m_gui_command_buffers.back() },
            { *m_image_acquired_sempahores.back(),
              *m_draw_completed_sempahores.back(),
              *m_gui_completed_sempahores.back() },
            { *m_frame_available_fences.back() },
        });
    }
}

const FrameInfo FrameManager::NextFrame()
{
    auto frame_index       = m_next_frame;
    auto image_ready_fence = m_frame_info[frame_index].fence.image_ready;

    m_next_frame = (m_next_frame + 1) % m_frame_count;

    m_device.WaitForFence(image_ready_fence);
    m_device.ResetFence(image_ready_fence);

    return m_frame_info[frame_index];
}
