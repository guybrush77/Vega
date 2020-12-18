#include "swapchain_manager.hpp"

SwapchainManager::SwapchainManager(
    etna::Device           device,
    etna::RenderPass       renderpass,
    etna::RenderPass       gui_renderpass,
    etna::SurfaceKHR       surface,
    uint32_t               min_image_count,
    etna::SurfaceFormatKHR surface_format,
    etna::Format           depth_format,
    etna::Extent2D         extent,
    etna::Queue            presentation_queue,
    etna::PresentModeKHR   present_mode)
    : m_device(device), m_presentation_queue(presentation_queue), m_extent(extent), m_min_image_count(min_image_count)
{
    using namespace etna;

    auto usage = ImageUsage::ColorAttachment;

    m_swapchain = device.CreateSwapchainKHR(surface, min_image_count, surface_format, extent, usage, present_mode);

    auto surface_images = device.GetSwapchainImagesKHR(*m_swapchain);

    for (const auto& color_image : surface_images) {
        auto depth_image = device.CreateImage(
            depth_format,
            extent,
            ImageUsage::DepthStencilAttachment,
            MemoryUsage::GpuOnly,
            ImageTiling::Optimal);

        auto color_view      = device.CreateImageView(color_image, ImageAspect::Color);
        auto depth_view      = device.CreateImageView(*depth_image, ImageAspect::Depth);
        auto framebuffer     = device.CreateFramebuffer(renderpass, { *color_view, *depth_view }, extent);
        auto gui_framebuffer = device.CreateFramebuffer(gui_renderpass, { *color_view }, extent);

        m_surface_views.push_back(std::move(color_view));
        m_depth_images.push_back(std::move(depth_image));
        m_depth_views.push_back(std::move(depth_view));
        m_framebuffers.push_back(std::move(framebuffer));
        m_gui_framebuffers.push_back(std::move(gui_framebuffer));
    }
}

etna::Return<uint32_t> SwapchainManager::AcquireNextImage(etna::Semaphore semaphore, etna::Fence fence)
{
    return m_device.AcquireNextImageKHR(*m_swapchain, semaphore, fence);
}

etna::Result SwapchainManager::QueuePresent(
    uint32_t                               image_index,
    std::initializer_list<etna::Semaphore> wait_semaphores)
{
    return m_presentation_queue.QueuePresentKHR(*m_swapchain, image_index, wait_semaphores);
}

FramebufferInfo SwapchainManager::GetFramebufferInfo(uint32_t image_index) const noexcept
{
    return FramebufferInfo{ *m_framebuffers[image_index], *m_gui_framebuffers[image_index], m_extent };
}
