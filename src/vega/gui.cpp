#include "gui.hpp"
#include "camera.hpp"
#include "platform.hpp"
#include "utils/resource.hpp"

#include "etna/command.hpp"
#include "etna/synchronization.hpp"

BEGIN_DISABLE_WARNINGS

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_access.hpp>

#include <spdlog/spdlog.h>

END_DISABLE_WARNINGS

namespace {

static Gui& Self(GLFWwindow* window)
{
    return *static_cast<Gui*>(glfwGetWindowUserPointer(window));
}

} // namespace

Gui::Gui(
    etna::Instance       instance,
    etna::PhysicalDevice gpu,
    etna::Device         device,
    uint32_t             queue_family_index,
    etna::Queue          graphics_queue,
    etna::RenderPass     renderpass,
    GLFWwindow*          window,
    etna::Extent2D       extent,
    uint32_t             min_image_count,
    uint32_t             image_count)
    : m_graphics_queue(graphics_queue), m_extent(extent)
{
    using namespace etna;

    // Setup GLFW callbacks
    {
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            Self(window).OnKey(key, scancode, action, mods);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            Self(window).OnCursorPosition(xpos, ypos);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            Self(window).OnMouseButton(button, action, mods);
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            Self(window).OnScroll(xoffset, yoffset);
        });

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            Self(window).OnFramebufferSize(width, height);
        });

        glfwSetWindowContentScaleCallback(window, [](GLFWwindow* window, float xscale, float yscale) {
            Self(window).OnContentScale(xscale, yscale);
        });

        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }

    // ImGui Init
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        spdlog::info("ImGui {}", ImGui::GetVersion());
    }

    // Create Descriptor Pool
    {
        DescriptorPoolSize pool_sizes[] = {

            { DescriptorType::Sampler, 1000 },
            { DescriptorType::CombinedImageSampler, 1000 },
            { DescriptorType::SampledImage, 1000 },
            { DescriptorType::StorageImage, 1000 },
            { DescriptorType::UniformTexelBuffer, 1000 },
            { DescriptorType::StorageTexelBuffer, 1000 },
            { DescriptorType::UniformBuffer, 1000 },
            { DescriptorType::StorageBuffer, 1000 },
            { DescriptorType::UniformBufferDynamic, 1000 },
            { DescriptorType::StorageBufferDynamic, 1000 },
            { DescriptorType::InputAttachment, 1000 }
        };
        m_descriptor_pool = device.CreateDescriptorPool(pool_sizes);
    }

    // Init Vulkan
    {
        auto init_info = ImGui_ImplVulkan_InitInfo{

            .Instance        = instance,
            .PhysicalDevice  = gpu,
            .Device          = device,
            .QueueFamily     = queue_family_index,
            .Queue           = graphics_queue,
            .PipelineCache   = nullptr,
            .DescriptorPool  = *m_descriptor_pool,
            .MinImageCount   = min_image_count,
            .ImageCount      = image_count,
            .MSAASamples     = VK_SAMPLE_COUNT_1_BIT,
            .Allocator       = nullptr,
            .CheckVkResultFn = nullptr
        };
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_Init(&init_info, renderpass);
    }

    // Set Style
    ImGui::StyleColorsDark();

    // Add fonts
    {
        auto font_config = ImFontConfig();

        font_config.FontDataOwnedByAtlas = false;

        auto regular = GetResource("fonts/Roboto-Regular.ttf");
        auto data    = const_cast<unsigned char*>(regular.data);
        auto size    = narrow_cast<int>(regular.size);

        m_fonts.regular = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, size, 24.0f, &font_config);

        auto monospace = GetResource("fonts/RobotoMono-Regular.ttf");
        data           = const_cast<unsigned char*>(monospace.data);
        size           = narrow_cast<int>(monospace.size);

        font_config.FontDataOwnedByAtlas = false;

        m_fonts.monospace = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, size, 24.0f, &font_config);
    }

    // Upload Fonts
    {
        auto command_pool   = device.CreateCommandPool(queue_family_index, CommandPoolCreate::Transient);
        auto command_buffer = command_pool->AllocateCommandBuffer();

        command_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        ImGui_ImplVulkan_CreateFontsTexture(*command_buffer);
        command_buffer->End();

        graphics_queue.Submit(*command_buffer);

        device.WaitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

Gui::~Gui() noexcept
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::OnKey(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{}

void Gui::OnCursorPosition(double xpos, double ypos)
{
    auto xposf = etna::narrow_cast<float>(xpos);
    auto yposf = etna::narrow_cast<float>(ypos);

    if (m_mouse_state.cursor.position.x >= 0.0f) {
        m_mouse_state.cursor.delta.x = xposf - m_mouse_state.cursor.position.x;
        m_mouse_state.cursor.delta.y = yposf - m_mouse_state.cursor.position.y;
    }
    m_mouse_state.cursor.position.x = xposf;
    m_mouse_state.cursor.position.y = yposf;
}

void Gui::OnMouseButton(int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_mouse_state.buttons.left.is_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_mouse_state.buttons.right.is_pressed = action == GLFW_PRESS;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        m_mouse_state.buttons.middle.is_pressed = action == GLFW_PRESS;
    }
}

void Gui::OnScroll(double /*xoffset*/, double /*yoffset*/)
{}

void Gui::OnFramebufferSize(int /*width*/, int /*height*/)
{}

void Gui::OnContentScale(float /*xscale*/, float /*yscale*/)
{}

void Gui::UpdateViewport(etna::Extent2D extent, uint32_t min_image_count)
{
    ImGui_ImplVulkan_SetMinImageCount(min_image_count);
    m_extent = extent;
}

void Gui::Draw(
    etna::CommandBuffer cmd_buffer,
    etna::Framebuffer   framebuffer,
    etna::Semaphore     wait_semaphore,
    etna::Semaphore     signal_semaphore,
    etna::Fence         finished_fence)
{
    using namespace etna;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window : m_windows) {
        window->Draw();
    }

    ImGui::Render();

    auto  render_area = Rect2D{ Offset2D{ 0, 0 }, m_extent };
    auto  clear_color = ClearColor::Transparent;
    auto* draw_data   = ImGui::GetDrawData();

    cmd_buffer.ResetCommandBuffer();
    cmd_buffer.Begin();
    cmd_buffer.BeginRenderPass(framebuffer, render_area, { clear_color });
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmd_buffer);
    cmd_buffer.EndRenderPass();
    cmd_buffer.End();

    m_graphics_queue.Submit(
        cmd_buffer,
        { wait_semaphore },
        { PipelineStage::ColorAttachmentOutput },
        { signal_semaphore },
        finished_fence);

    m_mouse_state.cursor.delta.x = 0;
    m_mouse_state.cursor.delta.y = 0;
}

bool Gui::IsAnyWindowHovered() const noexcept
{
    return ImGui::IsAnyWindowHovered();
}

void CameraWindow::Draw()
{
    using namespace ImGui;

    Begin("Camera");

    {
        const char* up_labels[] = { "Normal", "Inverted" };

        auto coordinates = m_camera->GetSphericalCoordinates();
        auto elevation   = coordinates.elevation;
        auto azimuth     = coordinates.azimuth;
        auto camera_up   = coordinates.camera_up == CameraUp::Normal ? 0 : 1;

        bool elevation_changed = SliderAngle("Elevation", &elevation.value, -90, 90);
        bool azimuth_changed   = SliderAngle("Azimuth", &azimuth.value, -180, 180);
        bool camera_up_changed = SliderInt("Camera Up", &camera_up, 0, 1, up_labels[camera_up]);

        if (elevation_changed || azimuth_changed || camera_up_changed) {
            m_camera->UpdateView(elevation, azimuth, camera_up == 0 ? CameraUp::Normal : CameraUp::Inverted);
        }
    }

    {
        const auto view = m_camera->GetViewMatrix();
        const auto row0 = glm::row(view, 0);
        const auto row1 = glm::row(view, 1);
        const auto row2 = glm::row(view, 2);
        const auto row3 = glm::row(view, 3);

        Text("View");
        PushFont(m_fonts.monospace);
        Text("  % f % f % f % f", row0.x, row0.y, row0.z, row0.w);
        Text("  % f % f % f % f", row1.x, row1.y, row1.z, row1.w);
        Text("  % f % f % f % f", row2.x, row2.y, row2.z, row2.w);
        Text("  % f % f % f % f", row3.x, row3.y, row3.z, row3.w);
        PopFont();
        Separator();
    }

    {
        const auto view = m_camera->GetPerspectiveMatrix();
        const auto row0 = glm::row(view, 0);
        const auto row1 = glm::row(view, 1);
        const auto row2 = glm::row(view, 2);
        const auto row3 = glm::row(view, 3);

        Text("Perspective");
        PushFont(m_fonts.monospace);
        Text("  % f % f % f % f", row0.x, row0.y, row0.z, row0.w);
        Text("  % f % f % f % f", row1.x, row1.y, row1.z, row1.w);
        Text("  % f % f % f % f", row2.x, row2.y, row2.z, row2.w);
        Text("  % f % f % f % f", row3.x, row3.y, row3.z, row3.w);
        PopFont();
        Separator();
    }

    End();
}
