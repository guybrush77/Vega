#include "gui.hpp"
#include "camera.hpp"
#include "lights.hpp"
#include "platform.hpp"
#include "scene.hpp"
#include "utils/cast.hpp"
#include "utils/resource.hpp"

#include "etna/command.hpp"
#include "etna/synchronization.hpp"

BEGIN_DISABLE_WARNINGS

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"
#include "imgui-filebrowser/imfilebrowser.h"
#include "imgui.h"
#include "imgui_internal.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_access.hpp>

END_DISABLE_WARNINGS

#include <charconv>
#include <string>

struct Window {
    bool PreBegin();
    void SetDefaultSize(float width_multiplier = 1.0f, float height_multiplier = 1.0f);
    void PostEnd();

    bool   visible{};
    ImVec2 default_size{};
};

class CameraWindow : public Window {
  public:
    CameraWindow(Camera* camera, Lights* lights) noexcept
        : Window{ VisibilityDefault }, m_camera(camera), m_lights(lights)
    {}

    void Draw();

    static constexpr bool VisibilityDefault = true;

  private:
    Camera* m_camera = nullptr;
    Lights* m_lights = nullptr;
};

class SceneWindow : public Window {
  public:
    SceneWindow(Scene* scene) noexcept : Window{ VisibilityDefault }, m_scene(scene) {}

    void Draw();

    static constexpr bool VisibilityDefault = true;

  private:
    bool DrawTreeNode(NodePtr node);
    auto DrawContextMenu(NodePtr node) -> NodePtr;
    void DrawNode(NodePtr node);
    void DrawObjectFields(ObjectPtr object, int* ptr_id);

    char        m_buffer[64]    = {};
    const void* m_selected_node = nullptr;
    const void* m_rename_node   = 0;
    Scene*      m_scene         = nullptr;
};

class FileBrowserWindow {
  public:
    FileBrowserWindow() noexcept
    {
        m_file_browser.SetTitle("Import");
        m_file_browser.SetTypeFilters({ ".obj" });
        m_file_browser.SetWindowSize(1000, 800);
    }

    void Open() { m_file_browser.Open(); }

    void Draw() { m_file_browser.Display(); }

    bool HasSelectedPath() { return m_file_browser.HasSelected(); }

    auto GetSelectedPath()
    {
        auto path = m_file_browser.GetSelected();
        m_file_browser.ClearSelected();
        return path;
    }

  private:
    ImGui::FileBrowser m_file_browser = ImGui::FileBrowser(ImGuiFileBrowserFlags_CloseOnEsc);
};

static Gui& Self(GLFWwindow* window)
{
    return *static_cast<Gui*>(glfwGetWindowUserPointer(window));
}

template <size_t N>
static void CopyToBuffer(char (&buffer)[N], const std::string& s)
{
    size_t size  = std::min(s.size(), sizeof(buffer) - 1);
    buffer[size] = 0;

    std::memcpy(buffer, s.c_str(), size);
}

static ImFont* LoadFont(const char* font_name, float font_size)
{
    auto font        = GetResource(font_name);
    auto data        = const_cast<unsigned char*>(font.data);
    auto size        = utils::narrow_cast<int>(font.size);
    auto font_config = ImFontConfig();

    font_config.FontDataOwnedByAtlas = false;

    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, size, font_size, &font_config);
}

static void UploadFonts(etna::Device device, etna::Queue graphics_queue)
{
    auto command_pool   = device.CreateCommandPool(graphics_queue.FamilyIndex(), etna::CommandPoolCreate::Transient);
    auto command_buffer = command_pool->AllocateCommandBuffer();

    // ImGui_ImplVulkan_DestroyFontsTexture(); TODO

    command_buffer->Begin(etna::CommandBufferUsage::OneTimeSubmit);
    ImGui_ImplVulkan_CreateFontsTexture(*command_buffer);
    command_buffer->End();

    graphics_queue.Submit(*command_buffer);

    device.WaitIdle();
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

struct GuiAccessor final {
    static void* OnOpen(ImGuiContext* /*context*/, ImGuiSettingsHandler* /*handler*/, const char* name)
    {
        return const_cast<char*>(name);
    }

    static void OnReadLine(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, void* entry, const char* line)
    {
        using namespace std::literals;

        static constexpr auto set_visibility = [](const char visible, bool visibility_default, bool* out) noexcept {
            if (visible == '0') {
                *out = false;
            } else if (visible == '1') {
                *out = true;
            } else {
                *out = visibility_default;
            }
        };

        if (static_cast<char*>(entry) == "Visibility"sv) {
            auto* gui    = static_cast<Gui*>(handler->UserData);
            auto  window = std::string_view(line);
            if (window.starts_with("Camera=")) {
                set_visibility(window.back(), CameraWindow::VisibilityDefault, &gui->m_windows.camera->visible);
            } else if (window.starts_with("Scene=")) {
                set_visibility(window.back(), SceneWindow::VisibilityDefault, &gui->m_windows.scene->visible);
            }
        }
    }

    static void OnWrite(ImGuiContext* /*context*/, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out)
    {
        auto* gui = static_cast<Gui*>(handler->UserData);
        out->append("[Windows][Visibility]\n");
        out->appendf("Camera=%d\n", gui->m_windows.camera->visible);
        out->appendf("Scene=%d\n", gui->m_windows.scene->visible);
    }
};

Gui::Gui(
    Parameters  parameters,
    Callbacks   callbacks,
    GLFWwindow* window,
    uint32_t    min_image_count,
    uint32_t    image_count,
    Camera*     camera,
    Scene*      scene,
    Lights*     lights)
    : m_callbacks(std::move(callbacks)), m_device(parameters.device), m_graphics_queue(parameters.graphics_queue),
      m_extent(parameters.extent)
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
        m_descriptor_pool = parameters.device.CreateDescriptorPool(pool_sizes);
    }

    // Init Vulkan
    {
        auto init_info = ImGui_ImplVulkan_InitInfo{

            .Instance        = parameters.instance,
            .PhysicalDevice  = parameters.gpu,
            .Device          = parameters.device,
            .QueueFamily     = parameters.graphics_queue.FamilyIndex(),
            .Queue           = parameters.graphics_queue,
            .PipelineCache   = nullptr,
            .DescriptorPool  = *m_descriptor_pool,
            .MinImageCount   = min_image_count,
            .ImageCount      = image_count,
            .MSAASamples     = VK_SAMPLE_COUNT_1_BIT,
            .Allocator       = nullptr,
            .CheckVkResultFn = nullptr
        };
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_Init(&init_info, parameters.renderpass);
    }

    // Set Style
    ImGui::StyleColorsDark();

    // Add fonts
    {
        glfwGetWindowContentScale(window, &m_content_scale.x, &m_content_scale.y);

        m_content_scale_changed = false;

        auto font_size = Fonts::FontSize * m_content_scale.y;

        m_fonts.regular   = LoadFont("fonts/Roboto-Regular.ttf", font_size);
        m_fonts.monospace = LoadFont("fonts/RobotoMono-Regular.ttf", font_size);

        UploadFonts(parameters.device, parameters.graphics_queue);
    }

    m_windows.camera      = std::make_unique<CameraWindow>(camera, lights);
    m_windows.scene       = std::make_unique<SceneWindow>(scene);
    m_windows.filebrowser = std::make_unique<FileBrowserWindow>();

    auto settings_handler = ImGuiSettingsHandler{};
    {
        settings_handler.TypeName   = "Windows";
        settings_handler.TypeHash   = ImHashStr("Windows");
        settings_handler.ReadOpenFn = GuiAccessor::OnOpen;
        settings_handler.ReadLineFn = GuiAccessor::OnReadLine;
        settings_handler.WriteAllFn = GuiAccessor::OnWrite;
        settings_handler.UserData   = this;
    }
    ImGui::GetCurrentContext()->SettingsHandlers.push_back(settings_handler);

    ImGui::GetStyle().ScaleAllSizes(m_content_scale.y);
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

void Gui::OnContentScale(float xscale, float yscale)
{
    m_content_scale         = { xscale, yscale };
    m_content_scale_changed = true;
}

void Gui::UpdateViewport(etna::Extent2D extent, uint32_t min_image_count)
{
    ImGui_ImplVulkan_SetMinImageCount(min_image_count);
    m_extent = extent;
}

void Gui::UpdateContentScale()
{
    ImGui::GetStyle() = ImGuiStyle();
    ImGui::GetStyle().ScaleAllSizes(m_content_scale.y);
    ImGui::GetIO().Fonts->Clear();

    auto font_size = Fonts::FontSize * m_content_scale.y;

    m_fonts.regular   = LoadFont("fonts/Roboto-Regular.ttf", font_size);
    m_fonts.monospace = LoadFont("fonts/RobotoMono-Regular.ttf", font_size);

    UploadFonts(m_device, m_graphics_queue);

    m_content_scale_changed = false;
}

void Gui::ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Import")) {
                m_windows.filebrowser->Open();
            }
            if (ImGui::MenuItem("Exit")) {
                m_callbacks.OnWindowClose();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Camera", nullptr, &m_windows.camera->visible);
            ImGui::MenuItem("Scene", nullptr, &m_windows.scene->visible);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Gui::Draw(
    etna::CommandBuffer cmd_buffer,
    etna::Framebuffer   framebuffer,
    etna::Semaphore     wait_semaphore,
    etna::Semaphore     signal_semaphore,
    etna::Fence         finished_fence)
{
    using namespace etna;

    if (m_content_scale_changed) {
        UpdateContentScale();
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ShowMenuBar();

    m_windows.camera->Draw();
    m_windows.scene->Draw();
    m_windows.filebrowser->Draw();

    if (m_windows.filebrowser->HasSelectedPath()) {
        m_callbacks.OnFileOpen(m_windows.filebrowser->GetSelectedPath().string());
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

void AddLabel(const char* label, const char* tooltip, float position)
{
    ImGui::SameLine(position);
    ImGui::TextUnformatted(label);

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(tooltip);
        ImGui::EndTooltip();
    }
}

bool Window::PreBegin()
{
    auto frame = ImGui::GetFrameCount();

    if (frame <= 3) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
        if (frame == 3) {
            ImGui::SetNextWindowSize(default_size, ImGuiCond_FirstUseEver);
        }
        return true;
    }
    return visible;
}

void Window::SetDefaultSize(float width_multiplier, float height_multiplier)
{
    if (ImGui::GetFrameCount() == 2) {
        auto [width, height] = ImGui::GetWindowSize();
        default_size         = { width_multiplier * width, height_multiplier * height };
    }
}

void Window::PostEnd()
{
    if (ImGui::GetFrameCount() <= 3) {
        ImGui::PopStyleVar();
    }
}

void CameraWindow::Draw()
{
    if (PreBegin() == false) {
        return;
    }

    const auto& limits = m_camera->GetLimits();

    ImGui::Begin("Camera", &visible);

    SetDefaultSize(2.0f, 1.5f);

    float label_width    = ImGui::CalcTextSize("Elevation").x + ImGui::GetStyle().ItemSpacing.x;
    float label_position = 0.0f;

    ImGui::PushItemWidth(-label_width);

    auto& style = ImGui::GetStyle();

    ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_MenuBarBg]);

    if (ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto coordinates     = m_camera->ComputeSphericalCoordinates();
        auto offset          = m_camera->GetOffset();
        auto camera_up_label = ToString(coordinates.camera_up);
        auto camera_up_index = static_cast<int>(coordinates.camera_up);

        bool camera_elevation;
        {
            ImGui::PushID("camera/elevation");

            camera_elevation = ImGui::SliderAngle(
                "",
                &coordinates.elevation.value,
                limits.elevation.min.value,
                limits.elevation.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            label_position = ImGui::GetCursorPosX() + ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x;

            AddLabel("Elevation", "Camera Elevation Angle", label_position);
        }

        bool camera_azimuth;
        {
            ImGui::PushID("camera/azimuth");

            camera_azimuth = ImGui::SliderAngle(
                "",
                &coordinates.azimuth.value,
                limits.azimuth.min.value,
                limits.azimuth.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            AddLabel("Azimuth", "Camera Azimuth Angle", label_position);
        }

        bool camera_up;
        {
            ImGui::PushID("camera/camera_up");

            camera_up = ImGui::SliderInt(
                "",
                &camera_up_index,
                0,
                1,
                camera_up_label,
                ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

            ImGui::PopID();

            AddLabel("Camera", "Is Camera Inverted", label_position);
        }

        bool camera_distance;
        {
            ImGui::PushID("camera/distance");

            camera_distance = ImGui::SliderFloat(
                "",
                &coordinates.distance,
                limits.distance.min,
                limits.distance.max,
                nullptr,
                ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

            ImGui::PopID();

            AddLabel("Distance", "Camera Distance", label_position);
        }

        bool offset_horizontal;
        {
            ImGui::PushID("camera/track_h");

            offset_horizontal = ImGui::SliderFloat("", &offset.horizontal, limits.offset_x.min, limits.offset_x.max);

            ImGui::PopID();

            AddLabel("Track H", "Camera Track Horizontal Distance", label_position);
        }

        bool offset_vertical;
        {
            ImGui::PushID("camera/track_v");

            offset_vertical = ImGui::SliderFloat("", &offset.vertical, limits.offset_y.min, limits.offset_y.max);

            ImGui::PopID();

            AddLabel("Track V", "Camera Track Vertical Distance", label_position);
        }

        if (camera_elevation || camera_azimuth || camera_up || camera_distance) {
            coordinates.camera_up = static_cast<CameraUp>(camera_up_index);
            m_camera->UpdateSphericalCoordinates(coordinates);
        }

        if (offset_horizontal || offset_vertical) {
            m_camera->UpdateOffset(offset);
        }
    }

    if (ImGui::CollapsingHeader("Perspective")) {
        auto perspective = m_camera->GetPerspective();

        bool fovy;
        {
            ImGui::PushID("camera/fov_v");

            fovy = ImGui::SliderAngle(
                "",
                &perspective.fovy.value,
                limits.fov_y.min.value,
                limits.fov_y.max.value,
                "%.1f deg",
                ImGuiSliderFlags_AlwaysClamp);

            ImGui::PopID();

            AddLabel("Fov V", "Field of View Vertical Angle", label_position);
        }

        bool clip_planes;
        {
            ImGui::PushID("camera/near_far");

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

            AddLabel("Near/Far", "Near/Far Clipping Planes", label_position);
        }

        if (fovy || clip_planes) {
            m_camera->UpdatePerspective(perspective);
        }
    }

    if (ImGui::CollapsingHeader("Coordinate System")) {
        auto basis         = m_camera->GetBasis();
        auto labels        = ToStringArray<Axis>();
        auto labels_count  = static_cast<int>(labels.size());
        auto forward_index = basis.forward.ToInt();
        auto up_index      = basis.up.ToInt();

        bool forward_changed;
        {
            ImGui::PushID("camera/forward");

            forward_changed = ImGui::Combo("", &forward_index, labels.data(), labels_count);

            ImGui::PopID();

            AddLabel("Forward", "Camera Forward Direction", label_position);
        }

        bool up_changed;
        {
            ImGui::PushID("camera/up");

            up_changed = ImGui::Combo("", &up_index, labels.data(), labels_count);

            ImGui::PopID();

            AddLabel("Up", "Camera Up Direction", label_position);
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

    if (ImGui::CollapsingHeader("Lights")) {
        ImGui::TextUnformatted("Key Light");
        {
            auto* multiplier = &m_lights->KeyRef().MultiplierRef();

            ImGui::PushID("camera/key_color");
            ImGui::ColorEdit3("", &m_lights->KeyRef().ColorRef());
            ImGui::PopID();
            AddLabel("Color", "Key Light Color", label_position);

            ImGui::PushID("camera/key_mul");
            ImGui::SliderFloat("", multiplier, 0, 2, "%.2f", ImGuiSliderFlags_Logarithmic);
            ImGui::PopID();
            AddLabel("Multiplier", "Key Light Multiplier", label_position);

            ImGui::PushID("camera/key_elevation");
            ImGui::SliderAngle("", &m_lights->KeyRef().ElevationRef(), -90, 90);
            ImGui::PopID();
            AddLabel("Elevation", "Key Light Elevation Angle", label_position);

            ImGui::PushID("camera/key_azimuth");
            ImGui::SliderAngle("", &m_lights->KeyRef().AzimuthRef(), -90, 90);
            ImGui::PopID();
            AddLabel("Azimuth", "Key Light Azimuth Angle", label_position);
        }

        ImGui::Dummy({ 0, 10 });

        ImGui::TextUnformatted("Fill Light");
        {
            auto* multiplier = &m_lights->FillRef().MultiplierRef();

            ImGui::PushID("camera/fill_color");
            ImGui::ColorEdit3("", &m_lights->FillRef().ColorRef());
            ImGui::PopID();
            AddLabel("Color", "Fill Light Color", label_position);

            ImGui::PushID("camera/fill_mul");
            ImGui::SliderFloat("", multiplier, 0, 2, "%.2f", ImGuiSliderFlags_Logarithmic);
            ImGui::PopID();
            AddLabel("Multiplier", "Fill Light Multiplier", label_position);

            ImGui::PushID("camera/fill_elevation");
            ImGui::SliderAngle("", &m_lights->FillRef().ElevationRef(), -90, 90);
            ImGui::PopID();
            AddLabel("Elevation", "Fill Light Elevation Angle", label_position);

            ImGui::PushID("camera/fill_azimuth");
            ImGui::SliderAngle("", &m_lights->FillRef().AzimuthRef(), -90, 90);
            ImGui::PopID();
            AddLabel("Azimuth", "Fill Light Azimuth Angle", label_position);
        }
    }

    ImGui::PopItemWidth();

    ImGui::PopStyleColor();

    ImGui::End();

    PostEnd();
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
        CopyToBuffer(buffer, s);
        ImGui::InputText(label, buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
    }

    const char* label = nullptr;
};

void DrawObjectProperties(ObjectPtr object, int* ptr_id)
{
    auto& id = *ptr_id;

    for (const auto& [key, value] : object->GetProperties()) {
        if (!IsReservedProperty(key)) {
            ImGui::PushID(id++);
            std::visit(DrawProperty{ key.c_str() }, value);
            ImGui::PopID();
        }
    }
}

void SceneWindow::DrawObjectFields(ObjectPtr object, int* ptr_id)
{
    static constexpr auto editable = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
    static constexpr auto readonly = ImGuiInputTextFlags_ReadOnly;

    auto& metadata = object->GetMetadata();
    auto& id       = *ptr_id;

    if (metadata.object_description) {
        auto text = const_cast<char*>(metadata.object_description);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::PushID(id++);
        ImGui::InputText("Type", text, strlen(text), readonly);
        ImGui::PopID();
        ImGui::PopStyleVar();
    }

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
        case ValueType::Null: break;
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
            char         buffer[64];
            std::string& value = object->GetField(field.name);

            CopyToBuffer(buffer, value);

            if (ImGui::InputText(field.label, buffer, sizeof(value), flags)) {
                value = buffer;
            }
            break;
        }
        case ValueType::Float3: {
            Float3& value = object->GetField(field.name);
            ImGui::InputFloat3(field.label, &value.x, "%.3f", flags);
            break;
        }
        }

        ImGui::PopID();

        if (false == field.is_editable) {
            ImGui::PopStyleVar();
        }
    }
}

bool SceneWindow::DrawTreeNode(NodePtr node)
{
    ImGui::AlignTextToFramePadding();

    int flags = ImGuiTreeNodeFlags_OpenOnArrow;
    if (m_selected_node == node) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = false;

    if (node == m_rename_node) {
        CopyToBuffer(m_buffer, node->GetName());

        opened = ImGui::TreeNodeEx(node, flags, "%s", "");

        auto spacing       = ImGui::GetStyle().ItemSpacing;
        auto inner_spacing = ImGui::GetStyle().ItemInnerSpacing;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { spacing.x - inner_spacing.x, spacing.y });
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0, inner_spacing.y });
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, { 1.0f, 1.0f, 1.0f, 0.3f });

        ImGui::SameLine();
        ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText(
                "##rename",
                m_buffer,
                sizeof(m_buffer),
                ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            node->SetName(m_buffer);
            m_rename_node = 0;
        }

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);

    } else {
        opened = ImGui::TreeNodeEx(node, flags, "%s", node->GetName().c_str());
    }

    if (false == node->IsRoot()) {
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("MOVE", &node, sizeof(node));
            ImGui::TextUnformatted(node->GetName().c_str());
            ImGui::EndDragDropSource();
        }
    }

    if (false == node->IsLeaf()) {
        if (auto payload = ImGui::GetDragDropPayload(); payload && payload->IsDataType("MOVE")) {
            Node* src_node = nullptr;
            memcpy(&src_node, payload->Data, utils::narrow_cast<size_t>(payload->DataSize));
            if (src_node && !src_node->IsAncestor(node)) {
                if (ImGui::BeginDragDropTarget()) {
                    if (ImGui::AcceptDragDropPayload("MOVE")) {
                        static_cast<InternalNode*>(node)->AttachNode(std::move(src_node->DetachNode()));
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }
    }

    return opened;
}

NodePtr SceneWindow::DrawContextMenu(NodePtr node)
{
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Rename Node")) {
            m_rename_node = node;
        }
        if (node->IsInternal()) {
            auto internal = static_cast<InternalNode*>(node);
            if (ImGui::BeginMenu("Add Node")) {
                if (ImGui::MenuItem("Translate")) {
                    internal->AddTranslateNode(0, 0, 0);
                }
                if (ImGui::MenuItem("Rotate")) {
                    internal->AddRotateNode(1, 0, 0, 0_rad);
                }
                if (ImGui::MenuItem("Scale")) {
                    internal->AddScaleNode(1.0f);
                }
                if (ImGui::MenuItem("Group")) {
                    internal->AddGroupNode();
                }
                ImGui::EndMenu();
            }
        }
        if (!node->IsRoot()) {
            if (ImGui::MenuItem("Delete Node")) {
                node->DetachNode();
                node = nullptr;
            }
        }

        ImGui::EndPopup();
    }
    return node;
}

void SceneWindow::DrawNode(NodePtr node)
{
    bool is_dangling_node = node->IsInternal() && !node->HasChildren();
    if (is_dangling_node) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    }

    bool opened = DrawTreeNode(node);

    if (is_dangling_node) {
        ImGui::PopStyleColor();
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            m_selected_node = node;
        } else {
            m_rename_node = nullptr;
        }
    }

    node = DrawContextMenu(node);

    if (opened) {
        if (node) {
            auto id = node->GetID().value << 8;
            DrawObjectFields(node, &id);
            std::ranges::for_each(node->GetChildren(), [this](NodePtr node) { DrawNode(node); });
        }
        ImGui::TreePop();
    }
}

void SceneWindow::Draw()
{
    if (PreBegin() == false) {
        return;
    }

    if (ImGui::GetFrameCount() == 3) {
        auto position = ImVec2{ 0.98f * ImGui::GetIO().DisplaySize.x - default_size.x, ImGui::GetCursorPosY() };
        ImGui::SetNextWindowPos(position, ImGuiCond_FirstUseEver);
    }

    ImGui::Begin("Scene", &visible);

    SetDefaultSize(4.0f, 5.0f);

    DrawNode(m_scene->GetRootNodePtr());

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
        m_selected_node = 0;
        m_rename_node   = 0;
    }

    ImGui::End();

    PostEnd();
}
