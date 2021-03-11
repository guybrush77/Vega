#pragma once

#include "etna/device.hpp"
#include "etna/pipeline.hpp"
#include "etna/queue.hpp"

#include "descriptor_manager.hpp"
#include "frame_manager.hpp"
#include "swapchain_manager.hpp"

struct GLFWwindow;

class Gui;
class Camera;
class Lights;
class BufferManager;
class Scene;

class RenderContext {
  public:
    enum class Status { WindowClosed, SwapchainOutOfDate, GuiEvent };
    enum class MouseLook { None, Orbit, Zoom, Track };

    RenderContext() noexcept = default;

    RenderContext(
        etna::Device         device,
        etna::Queue          graphics_queue,
        etna::Pipeline       pipeline,
        etna::PipelineLayout pipeline_layout,
        GLFWwindow*          window,
        SwapchainManager*    swapchain_manager,
        FrameManager*        frame_manager,
        DescriptorManager*   descriptor_manager,
        Gui*                 gui,
        Camera*              camera,
        Lights*              lights,
        BufferManager*       buffer_manager,
        Scene*               scene);

    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    RenderContext(RenderContext&&) = default;
    RenderContext& operator=(RenderContext&&) = default;

    void ProcessUserInput();

    auto StartRenderLoop() -> Status;
    void StopRenderLoop();

  private:
    etna::Device         m_device;
    etna::Queue          m_graphics_queue;
    etna::Pipeline       m_pipeline;
    etna::PipelineLayout m_pipeline_layout;
    GLFWwindow*          m_window                = nullptr;
    SwapchainManager*    m_swapchain_manager     = nullptr;
    FrameManager*        m_frame_manager         = nullptr;
    DescriptorManager*   m_descriptor_manager    = nullptr;
    Gui*                 m_gui                   = nullptr;
    Camera*              m_camera                = nullptr;
    Lights*              m_lights                = nullptr;
    BufferManager*       m_buffer_manager        = nullptr;
    Scene*               m_scene                 = nullptr;
    MouseLook            m_mouse_look            = MouseLook::None;
    bool                 m_is_any_window_hovered = false;
    bool                 m_is_running            = false;
};
