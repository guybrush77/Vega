#pragma once

#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/queue.hpp"
#include "etna/renderpass.hpp"
#include "etna/surface.hpp"
#include "etna/swapchain.hpp"
#include "etna/synchronization.hpp"

struct FramebufferInfo {
    etna::Framebuffer draw;
    etna::Framebuffer gui;
    etna::Extent2D    extent;
};

class SwapchainManager {
  public:
    SwapchainManager(
        etna::Device           device,
        etna::RenderPass       renderpass,
        etna::RenderPass       gui_renderpass,
        etna::SurfaceKHR       surface,
        uint32_t               min_image_count,
        etna::SurfaceFormatKHR surface_format,
        etna::Format           depth_format,
        etna::Extent2D         extent,
        etna::Queue            presentation_queue,
        etna::PresentModeKHR   present_mode);

    SwapchainManager(const SwapchainManager&) = delete;
    SwapchainManager& operator=(const SwapchainManager&) = delete;
    SwapchainManager(SwapchainManager&&) noexcept        = delete;
    SwapchainManager& operator=(SwapchainManager&&) noexcept = delete;

    auto AcquireNextImage(etna::Semaphore semaphore, etna::Fence fence = {}) -> etna::Return<uint32_t>;

    auto QueuePresent(uint32_t image_index, std::initializer_list<etna::Semaphore> wait_semaphores) -> etna::Result;

    auto ImageCount() const noexcept -> uint32_t { return etna::narrow_cast<uint32_t>(m_surface_views.size()); }

    auto MinImageCount() const noexcept -> uint32_t { return m_min_image_count; }

    auto GetFramebufferInfo(uint32_t image_index) const noexcept -> FramebufferInfo;

  private:
    etna::UniqueSwapchainKHR             m_swapchain{};
    std::vector<etna::UniqueImageView2D> m_surface_views{};
    std::vector<etna::UniqueImage2D>     m_depth_images{};
    std::vector<etna::UniqueImageView2D> m_depth_views{};
    std::vector<etna::UniqueFramebuffer> m_framebuffers{};
    std::vector<etna::UniqueFramebuffer> m_gui_framebuffers{};
    etna::Device                         m_device{};
    etna::Queue                          m_presentation_queue{};
    etna::Extent2D                       m_extent{};
    uint32_t                             m_min_image_count{};
};
