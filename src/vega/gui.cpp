#include "gui.hpp"
#include "camera.hpp"
#include "platform.hpp"
#include "scene.hpp"
#include "utils/cast.hpp"
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

END_DISABLE_WARNINGS

#include <charconv>
#include <string>

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
    }

    // Create Descriptor Pool
    {
        DescriptorPoolSize pool_sizes[] = {

            { DescriptorType::Sampler, 1 },
            { DescriptorType::CombinedImageSampler, 1 },
            { DescriptorType::SampledImage, 1 },
            { DescriptorType::StorageImage, 1 },
            { DescriptorType::UniformTexelBuffer, 1 },
            { DescriptorType::StorageTexelBuffer, 1 },
            { DescriptorType::UniformBuffer, 1 },
            { DescriptorType::StorageBuffer, 1 },
            { DescriptorType::UniformBufferDynamic, 1 },
            { DescriptorType::StorageBufferDynamic, 1 },
            { DescriptorType::InputAttachment, 1 }
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

void Gui::OnScroll(double xoffset, double yoffset)
{
    m_mouse_state.scroll.x = etna::narrow_cast<float>(xoffset);
    m_mouse_state.scroll.y = etna::narrow_cast<float>(yoffset);
}

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
    m_mouse_state.scroll.x       = 0;
    m_mouse_state.scroll.y       = 0;
}

bool Gui::IsAnyWindowHovered() const noexcept
{
    return ImGui::IsAnyWindowHovered();
}

void AddLabel(const char* label, const char* tooltip)
{
    ImGui::SameLine();
    ImGui::TextUnformatted(label);

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }
}

void CameraWindow::Draw()
{
    static const auto width = ImGui::CalcTextSize("Camera Up ").x;

    const auto& limits = m_camera->GetLimits();

    auto id = m_base_id;

    ImGui::Begin("Camera");

    ImGui::PushItemWidth(-width);

    {
        ImGui::Text("View");

        auto coordinates     = m_camera->GetSphericalCoordinates();
        auto offset          = m_camera->GetOffset();
        auto camera_up_label = ToString(coordinates.camera_up);
        auto camera_up_index = static_cast<int>(coordinates.camera_up);

        bool camera_elevation;
        {
            ImGui::PushID(id++);

            camera_elevation = ImGui::SliderAngle(
                "",
                &coordinates.elevation.value,
                limits.elevation.min.value,
                limits.elevation.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            AddLabel("Elevation", "Camera Elevation");
        }

        bool camera_azimuth;
        {
            ImGui::PushID(id++);

            camera_azimuth = ImGui::SliderAngle(
                "",
                &coordinates.azimuth.value,
                limits.azimuth.min.value,
                limits.azimuth.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            AddLabel("Azimuth", "Camera Azimuth");
        }

        bool camera_up;
        {
            ImGui::PushID(id++);

            camera_up = ImGui::SliderInt(
                "",
                &camera_up_index,
                0,
                1,
                camera_up_label,
                ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

            ImGui::PopID();

            AddLabel("Camera Up", "Is Camera Inverted");
        }

        bool camera_distance;
        {
            ImGui::PushID(id++);

            camera_distance = ImGui::SliderFloat(
                "",
                &coordinates.distance,
                limits.distance.min,
                limits.distance.max,
                nullptr,
                ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

            ImGui::PopID();

            AddLabel("Distance", "Camera Distance");
        }

        bool offset_horizontal;
        {
            ImGui::PushID(id++);

            offset_horizontal = ImGui::SliderFloat("", &offset.horizontal, limits.offset_x.min, limits.offset_x.max);

            ImGui::PopID();

            AddLabel("Track H", "Camera Track Horizontal");
        }

        bool offset_vertical;
        {
            ImGui::PushID(id++);

            offset_vertical = ImGui::SliderFloat("", &offset.vertical, limits.offset_y.min, limits.offset_y.max);

            ImGui::PopID();

            AddLabel("Track V", "Camera Track Vertical");
        }

        if (camera_elevation || camera_azimuth || camera_up || camera_distance) {
            coordinates.camera_up = static_cast<CameraUp>(camera_up_index);
            m_camera->UpdateSphericalCoordinates(coordinates);
        }

        if (offset_horizontal || offset_vertical) {
            m_camera->UpdateOffset(offset);
        }
    }

    ImGui::Dummy({ 0, 20 });

    {
        ImGui::Text("Perspective");

        auto perspective = m_camera->GetPerspective();

        bool fovy;
        {
            ImGui::PushID(id++);

            fovy = ImGui::SliderAngle(
                "",
                &perspective.fovy.value,
                limits.fov_y.min.value,
                limits.fov_y.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            AddLabel("Fov V", "Field of View Vertical");
        }

        bool clip_planes;
        {
            ImGui::PushID(id++);

            clip_planes = ImGui::DragFloatRange2(
                "",
                &perspective.near,
                &perspective.far,
                perspective.far_max / 10000.0f,
                perspective.near_min,
                perspective.far_max,
                "%.1f",
                "%.1f",
                ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_Logarithmic);

            ImGui::PopID();

            AddLabel("Near/Far", "Near/Far Clipping Planes");
        }

        if (fovy || clip_planes) {
            m_camera->UpdatePerspective(perspective);
        }
    }

    ImGui::Dummy({ 0, 20 });

    {
        ImGui::Text("Coordinate System");

        auto basis         = m_camera->GetBasis();
        auto labels        = ToStringArray<Axis>();
        auto labels_count  = static_cast<int>(labels.size());
        auto forward_index = basis.forward.ToInt();
        auto up_index      = basis.up.ToInt();

        bool forward_changed;
        {
            ImGui::PushID(id++);

            forward_changed = ImGui::Combo("", &forward_index, labels.data(), labels_count);

            ImGui::PopID();

            AddLabel("Forward", "Camera Forward Direction");
        }

        bool up_changed;
        {
            ImGui::PushID(id++);

            up_changed = ImGui::Combo("", &up_index, labels.data(), labels_count);

            ImGui::PopID();

            AddLabel("Up", "Camera Up Direction");
        }

        auto forward = Forward::FromInt(forward_index);
        auto up      = Up::FromInt(up_index);

        bool forward_x = forward == Axis::PositiveX || forward == Axis::NegativeX;
        bool forward_y = forward == Axis::PositiveY || forward == Axis::NegativeY;
        bool forward_z = forward == Axis::PositiveZ || forward == Axis::NegativeZ;
        bool up_x      = up == Axis::PositiveX || up == Axis::NegativeX;
        bool up_y      = up == Axis::PositiveY || up == Axis::NegativeY;
        bool up_z      = up == Axis::PositiveZ || up == Axis::NegativeZ;

        if (forward_changed) {
            if (forward_x && up_x) {
                up = Up(Axis::PositiveY);
            } else if (forward_y && up_y) {
                up = Up(Axis::PositiveZ);
            } else if (forward_z && up_z) {
                up = Up(Axis::PositiveY);
            }
        }

        if (up_changed) {
            if (forward_x && up_x) {
                forward = Forward(Axis::PositiveY);
            } else if (forward_y && up_y) {
                forward = Forward(Axis::NegativeZ);
            } else if (forward_z && up_z) {
                forward = Forward(Axis::PositiveY);
            }
        }

        if (up != basis.up || forward != basis.forward) {
            const auto& object      = m_camera->GetObject();
            const auto& perspective = m_camera->GetPerspective();

            *m_camera = Camera::Create(
                Orientation::RightHanded,
                forward,
                up,
                ObjectView::Front,
                object,
                ToDegrees(perspective.fovy),
                perspective.aspect);
        }
    }

    ImGui::PopItemWidth();

    ImGui::End();
}

struct DrawProperty final {
    void operator()(int i)
    {
        int value = i;
        ImGui::InputInt(label, &value, 0, 0, ImGuiInputTextFlags_ReadOnly);
    }
    void operator()(float f)
    {
        float value = f;
        ImGui::InputFloat(label, &value, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
    }
    void operator()(const std::string& s)
    {
        char buffer[64];

        size_t size  = std::min(s.size(), sizeof(buffer) - 1);
        buffer[size] = 0;

        std::memcpy(buffer, s.c_str(), size);

        ImGui::InputText(label, buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
    }

    const char* label = nullptr;
};

void DrawObjectProperties(ObjectPtr object, int* ptr_id)
{
    auto& id = *ptr_id;

    for (const auto& [key, value] : object->Properties()) {
        ImGui::PushID(id++);
        std::visit(DrawProperty{ key.c_str() }, value);
        ImGui::PopID();
    }
}

void DrawObjectFields(ObjectPtr object, int* ptr_id)
{
    static constexpr auto editable = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
    static constexpr auto readonly = ImGuiInputTextFlags_ReadOnly;

    auto& metadata = object->GetMetadata();
    auto& id       = *ptr_id;

    if (object->HasProperties()) {
        DrawObjectProperties(object, ptr_id);
    }

    for (const auto& field : metadata.fields) {
        auto flags           = editable;
        auto float_step      = 0.01f;
        auto float_step_fast = 1.0f;
        auto int_step        = 1;
        auto int_step_fast   = 100;

        if (false == field.is_editable) {
            flags           = readonly;
            float_step      = 0;
            float_step_fast = 0;
            int_step        = 0;
            int_step_fast   = 0;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        }

        ImGui::PushID(id++);

        switch (field.value_type) {
        case ValueType::Null:
            break;
        case ValueType::Float: {
            float& value = object->GetField(field.name);
            ImGui::InputFloat(field.label, &value, float_step, float_step_fast, "%.3f", flags);
            break;
        }
        case ValueType::Int: {
            int& value = object->GetField(field.name);
            ImGui::InputInt(field.label, &value, int_step, int_step_fast, flags);
            break;
        }
        case ValueType::Reference: {
            Object& value = object->GetField(field.name);
            DrawObjectFields(&value, ptr_id);
            break;
        }
        case ValueType::String: {
            std::string& value = object->GetField(field.name);

            char buffer[64];

            size_t size  = std::min(value.size(), sizeof(buffer) - 1);
            buffer[size] = 0;

            std::memcpy(buffer, value.c_str(), size);

            if (ImGui::InputText(field.label, buffer, sizeof(value), flags)) {
                value = buffer;
            }
            break;
        }
        case ValueType::Vec3: {
            glm::vec3& value = object->GetField(field.name);
            ImGui::InputFloat3(field.label, &value[0], "%.3f", flags);
            break;
        }
        }

        ImGui::PopID();

        if (false == field.is_editable) {
            ImGui::PopStyleVar();
        }
    }
}

void DrawNode(NodePtr node)
{
    using namespace ImGui;

    const auto& metadata = node->GetMetadata();

    auto id = node->GetID().value;

    assert(id <= 0x007ffff0); // TODO

    id = id << 8;

    PushID(id++);

    if (TreeNode(metadata.object_label)) {
        DrawObjectFields(node, &id);
        std::ranges::for_each(node->GetChildren(), [](NodePtr node) { DrawNode(node); });
        TreePop();
    }

    PopID();
}

void SceneWindow::Draw()
{
    using namespace ImGui;

    Begin("Scene");
    DrawNode(m_scene->GetRootNodePtr());
    End();
}
