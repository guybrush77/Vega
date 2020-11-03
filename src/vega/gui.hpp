#pragma once

#include "descriptor.hpp"
#include "device.hpp"
#include "image.hpp"
#include "instance.hpp"
#include "queue.hpp"
#include "renderpass.hpp"

struct GLFWwindow;

class Gui {
  public:
    Gui(etna::Instance       instance,
        etna::PhysicalDevice gpu,
        etna::Device         device,
        uint32_t             queue_family_index,
        etna::Queue          graphics_queue,
        etna::RenderPass     renderpass,
        GLFWwindow*          window,
        etna::Extent2D       extent,
        uint32_t             min_image_count,
        uint32_t             image_count);

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;
    Gui(Gui&&)                 = default;
    Gui& operator=(Gui&&) = default;

    ~Gui() noexcept;

    void Update(etna::Extent2D extent, uint32_t min_image_count);

    void Draw(
        etna::CommandBuffer cmd_buffer,
        etna::Framebuffer   framebuffer,
        etna::Semaphore     wait_semaphore,
        etna::Semaphore     signal_semaphore,
        etna::Fence         finished_fence);

  private:
    etna::UniqueDescriptorPool m_descriptor_pool;
    etna::Queue                m_graphics_queue;
    etna::Extent2D             m_extent;
};
