#include "render_context.hpp"

#include "buffer_manager.hpp"
#include "camera.hpp"
#include "descriptor_manager.hpp"
#include "gui.hpp"
#include "lights.hpp"
#include "scene.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

RenderContext::RenderContext(
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
    TextureLoader*       texture_loader,
    Scene*               scene)
    : m_device(device), m_graphics_queue(graphics_queue), m_pipeline(pipeline), m_pipeline_layout(pipeline_layout),
      m_window(window), m_swapchain_manager(swapchain_manager), m_frame_manager(frame_manager),
      m_descriptor_manager(descriptor_manager), m_gui(gui), m_camera(camera), m_lights(lights),
      m_buffer_manager(buffer_manager), m_texture_loader(texture_loader), m_scene(scene)
{}

void RenderContext::ProcessUserInput()
{
    auto mouse_state  = m_gui->GetMouseState();
    bool is_scrolling = mouse_state.scroll.y != 0;

    if (mouse_state.buttons.IsNonePressed() && !is_scrolling) {
        if (m_mouse_look != MouseLook::None) {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        m_is_any_window_hovered = false;
        m_mouse_look            = MouseLook::None;
        return;
    }

    if (!m_scene->GetRootNode()->HasChildren()) {
        return;
    }

    if (m_mouse_look == MouseLook::None) {
        if (m_is_any_window_hovered || m_gui->IsAnyWindowHovered()) {
            m_is_any_window_hovered = true;
            return;
        }
        if (mouse_state.buttons.left.is_pressed) {
            m_mouse_look = MouseLook::Orbit;
        } else if (mouse_state.buttons.right.is_pressed) {
            m_mouse_look = MouseLook::Track;
        } else if (mouse_state.buttons.middle.is_pressed) {
            m_mouse_look = MouseLook::Zoom;
        } else {
            constexpr auto scroll_sensitivity = 6;
            m_camera->Zoom(scroll_sensitivity * mouse_state.scroll.y);
            return;
        }
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (m_mouse_look == MouseLook::Orbit) {
        auto rot_x = Degrees(mouse_state.cursor.delta.x);
        auto rot_y = Degrees(mouse_state.cursor.delta.y);
        m_camera->Orbit(rot_y, rot_x);
    } else if (m_mouse_look == MouseLook::Track) {
        m_camera->Track(mouse_state.cursor.delta.x, mouse_state.cursor.delta.y);
    } else if (m_mouse_look == MouseLook::Zoom) {
        m_camera->Zoom(mouse_state.cursor.delta.y);
    }
}

RenderContext::Status RenderContext::StartRenderLoop()
{
    using namespace etna;

    auto status  = Status::GuiEvent;
    m_is_running = true;

    auto image_ready_fences = std::vector<Fence>(m_swapchain_manager->ImageCount());

    while (m_is_running) {
        if (glfwWindowShouldClose(m_window)) {
            m_is_running = false;
            status       = Status::WindowClosed;
            break;
        }

        glfwPollEvents();

        auto frame       = m_frame_manager->NextFrame();
        auto image_index = uint32_t{};

        if (auto next_image = m_swapchain_manager->AcquireNextImage(frame.semaphores.image_acquired); next_image) {
            image_index = next_image.value();
            if (image_ready_fences[image_index] != Fence::Null &&
                image_ready_fences[image_index] != frame.fence.image_ready) {
                m_device.WaitForFence(image_ready_fences[image_index]);
            }
            image_ready_fences[image_index] = frame.fence.image_ready;
        } else if (next_image.result() == Result::ErrorOutOfDateKHR) {
            m_is_running = false;
            status       = Status::SwapchainOutOfDate;
            continue;
        } else {
            throw std::runtime_error("AcquireNextImage failed!");
        }

        auto framebuffers = m_swapchain_manager->GetFramebufferInfo(image_index);

        ProcessUserInput();

        auto draw_list = m_scene->ComputeDrawList();
        auto extent    = framebuffers.extent;

        auto view        = m_camera->ComputeViewMatrix();
        auto perspective = m_camera->ComputePerspectiveMatrix();

        auto camera = CameraUniform{ view, perspective };

        m_descriptor_manager->Set(frame.index, camera);

        auto key_color = m_lights->KeyRef().ComputePremultipliedColor();
        auto key_dir   = m_lights->KeyRef().ComputeDir();

        auto fill_color = m_lights->FillRef().ComputePremultipliedColor();
        auto fill_dir   = m_lights->FillRef().ComputeDir();

        auto lights = LightsUniform{};
        {
            lights.key.color = glm::vec4(key_color.r, key_color.g, key_color.b, 0);
            lights.key.dir   = glm::vec4(key_dir.x, key_dir.y, key_dir.z, 0) * view;

            lights.fill.color = glm::vec4(fill_color.r, fill_color.g, fill_color.b, 0);
            lights.fill.dir   = glm::vec4(fill_dir.x, fill_dir.y, fill_dir.z, 0) * view;
        }

        m_descriptor_manager->Set(frame.index, lights);

        auto image_views = std::vector<ImageView2D>{};

        for (const auto& [index, mesh, material, transform] : draw_list) {
            const auto& value   = material->GetProperty("diffuse.texture");
            const auto  texture = std::get_if<std::string>(&value);
            auto image_view     = texture ? m_texture_loader->GetImage(*texture) : m_texture_loader->GetDefaultImage();
            m_descriptor_manager->Set(image_view);
            image_views.push_back(image_view);
        }

        auto clear_color    = ClearColor::Transparent;
        auto clear_depth    = ClearDepthStencil::Default;
        auto render_area    = Rect2D{ Offset2D{ 0, 0 }, extent };
        auto framebuffer    = framebuffers.draw;
        auto transforms_set = m_descriptor_manager->GetTransformsSet(frame.index);
        auto width          = narrow_cast<float>(extent.width);
        auto height         = narrow_cast<float>(extent.height);
        auto viewport       = Viewport{ 0, height, width, -height, 0, 1 };
        auto scissor        = Rect2D{ Offset2D{ 0, 0 }, extent };

        frame.cmd_buffers.draw.ResetCommandBuffer(CommandBufferReset::ReleaseResources);
        frame.cmd_buffers.draw.Begin(CommandBufferUsage::OneTimeSubmit);
        frame.cmd_buffers.draw.BeginRenderPass(framebuffer, render_area, { clear_color, clear_depth });
        frame.cmd_buffers.draw.BindPipeline(PipelineBindPoint::Graphics, m_pipeline);
        frame.cmd_buffers.draw.SetViewport(viewport);
        frame.cmd_buffers.draw.SetScissor(scissor);

        for (const auto& [index, mesh, material, transform] : draw_list) {
            auto graphics        = PipelineBindPoint::Graphics;
            auto material_set    = m_descriptor_manager->GetTextureSet(image_views[index]);
            auto descriptor_sets = { transforms_set, material_set };
            auto model_transform = ModelUniform{ transform };
            auto offsets         = { m_descriptor_manager->Set(frame.index, index, model_transform) };
            auto vertex_buffer   = m_buffer_manager->GetBuffer(mesh->GetVertexBuffer());
            auto index_buffer    = m_buffer_manager->GetBuffer(mesh->GetIndexBuffer());

            frame.cmd_buffers.draw.BindVertexBuffers(vertex_buffer);
            frame.cmd_buffers.draw.BindIndexBuffer(index_buffer, IndexType::Uint32);
            frame.cmd_buffers.draw.BindDescriptorSets(graphics, m_pipeline_layout, 0, descriptor_sets, offsets);
            frame.cmd_buffers.draw.DrawIndexed(mesh->GetIndexCount(), 1, mesh->GetFirstIndex());
        }

        frame.cmd_buffers.draw.EndRenderPass();
        frame.cmd_buffers.draw.End();

        m_descriptor_manager->Flush(frame.index);

        m_graphics_queue.Submit(
            frame.cmd_buffers.draw,
            { frame.semaphores.image_acquired },
            { PipelineStage::ColorAttachmentOutput },
            { frame.semaphores.draw_completed },
            {});

        m_gui->Draw(
            frame.cmd_buffers.gui,
            framebuffers.gui,
            frame.semaphores.draw_completed,
            frame.semaphores.gui_completed,
            frame.fence.image_ready);

        m_swapchain_manager->QueuePresent(image_index, { frame.semaphores.gui_completed });
    }

    return status;
}

void RenderContext::StopRenderLoop()
{
    m_is_running = false;
}
