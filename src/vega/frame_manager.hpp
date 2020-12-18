#pragma once

#include "etna/command.hpp"
#include "etna/device.hpp"
#include "etna/synchronization.hpp"

struct FrameInfo {
    uint32_t index;
    struct {
        etna::CommandBuffer draw;
        etna::CommandBuffer gui;
    } cmd_buffers;
    struct {
        etna::Semaphore image_acquired;
        etna::Semaphore draw_completed;
        etna::Semaphore gui_completed;
    } semaphores;
    struct {
        etna::Fence image_ready;
    } fence;
};

class FrameManager {
  public:
    FrameManager(etna::Device device, uint32_t queue_family_index, uint32_t frame_count);

    auto NextFrame() -> const FrameInfo;

  private:
    etna::Device                           m_device;
    etna::UniqueCommandPool                m_command_pool;
    std::vector<etna::UniqueCommandBuffer> m_draw_command_buffers;
    std::vector<etna::UniqueCommandBuffer> m_gui_command_buffers;
    std::vector<etna::UniqueSemaphore>     m_image_acquired_sempahores;
    std::vector<etna::UniqueSemaphore>     m_draw_completed_sempahores;
    std::vector<etna::UniqueSemaphore>     m_gui_completed_sempahores;
    std::vector<etna::UniqueFence>         m_frame_available_fences;
    std::vector<FrameInfo>                 m_frame_info;
    uint32_t                               m_frame_count;
    uint32_t                               m_next_frame;
};
