#pragma once

#include "etna/descriptor.hpp"
#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/instance.hpp"
#include "etna/queue.hpp"
#include "etna/renderpass.hpp"

#include <functional>
#include <memory>

struct GLFWwindow;
struct ImFont;

class Camera;
class Lights;
class Scene;

class CameraWindow;
class FileBrowserWindow;
class LightsWindow;
class SceneWindow;

using UniqueCameraWindow      = std::unique_ptr<CameraWindow>;
using UniqueFileBrowserWindow = std::unique_ptr<FileBrowserWindow>;
using UniqueLightsWindow      = std::unique_ptr<LightsWindow>;
using UniqueSceneWindow       = std::unique_ptr<SceneWindow>;

class Gui {
  public:
    struct Parameters final {
        etna::Instance       instance;
        etna::PhysicalDevice gpu;
        etna::Device         device;
        etna::Queue          graphics_queue;
        etna::RenderPass     renderpass;
        etna::Extent2D       extent;
    };

    struct Callbacks final {
        std::function<void()>                     OnWindowClose;
        std::function<void(std::string filepath)> OnFileOpen;
    };

    Gui() noexcept = default;

    Gui(Parameters  parameters,
        Callbacks   callbacks,
        GLFWwindow* window,
        uint32_t    min_image_count,
        uint32_t    image_count,
        Camera*     camera,
        Scene*      scene,
        Lights*     lights);

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;
    Gui(Gui&&)                 = default;
    Gui& operator=(Gui&&) = default;

    ~Gui() noexcept;

    void OnKey(int key, int scancode, int action, int mods);
    void OnCursorPosition(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void OnScroll(double xoffset, double yoffset);
    void OnFramebufferSize(int width, int height);
    void OnContentScale(float xscale, float yscale);

    void UpdateViewport(etna::Extent2D extent, uint32_t min_image_count);

    void Draw(
        etna::CommandBuffer cmd_buffer,
        etna::Framebuffer   framebuffer,
        etna::Semaphore     wait_semaphore,
        etna::Semaphore     signal_semaphore,
        etna::Fence         finished_fence);

    auto GetMouseState() const noexcept { return m_mouse_state; }
    bool IsAnyWindowHovered() const noexcept;

    struct MouseState final {
        struct Cursor final {
            struct Position final {
                float x = 0;
                float y = 0;
            } position;
            struct Delta final {
                float x = 0;
                float y = 0;
            } delta;
        } cursor;
        struct Buttons final {
            struct Left final {
                bool is_pressed = false;
            } left;

            struct Right final {
                bool is_pressed = false;
            } right;

            struct Middle final {
                bool is_pressed = false;
            } middle;

            bool IsAnyPressed() const noexcept { return left.is_pressed || right.is_pressed || middle.is_pressed; }
            bool IsNonePressed() const noexcept { return !left.is_pressed && !right.is_pressed && !middle.is_pressed; }
        } buttons;
        struct Scroll final {
            float x = 0;
            float y = 0;
        } scroll;
    };

  private:
    friend struct GuiAccessor;

    void UpdateContentScale();
    void ShowMenuBar();

    struct Fonts final {
        static constexpr float FontSize = 15.0f;

        ImFont* regular   = nullptr;
        ImFont* monospace = nullptr;
    };

    struct ContentScale final {
        float x;
        float y;
    };

    struct Windows final {
        UniqueSceneWindow       scene;
        UniqueCameraWindow      camera;
        UniqueLightsWindow      lights;
        UniqueFileBrowserWindow filebrowser;
    };

    Callbacks                  m_callbacks;
    Fonts                      m_fonts;
    ContentScale               m_content_scale;
    bool                       m_content_scale_changed;
    MouseState                 m_mouse_state;
    etna::Device               m_device;
    etna::UniqueDescriptorPool m_descriptor_pool;
    etna::Queue                m_graphics_queue;
    etna::Extent2D             m_extent;
    Windows                    m_windows;
};
