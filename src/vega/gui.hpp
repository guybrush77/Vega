#pragma once

#include "etna/descriptor.hpp"
#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/instance.hpp"
#include "etna/queue.hpp"
#include "etna/renderpass.hpp"

#include <memory>

struct GLFWwindow;
struct ImFont;

class Camera;
class Scene;

struct Fonts {
    ImFont* regular   = nullptr;
    ImFont* monospace = nullptr;
};

class Window {
  public:
    virtual void Draw() = 0;
    virtual ~Window()   = default;

  protected:
    friend class Gui;
    Fonts m_fonts;
};

class CameraWindow : public Window {
  public:
    CameraWindow(Camera* camera) : m_camera(camera) {}

    void Draw() override;

  private:
    int     m_base_id = 0x7ffff100;
    Camera* m_camera = nullptr;
};

class SceneWindow : public Window {
  public:
    SceneWindow(Scene* scene) : m_scene(scene) {}

    void Draw() override;

  private:
    Scene* m_scene = nullptr;
};

class Gui {
  public:
    using UniqueWindow = std::unique_ptr<Window>;

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

    void OnKey(int key, int scancode, int action, int mods);
    void OnCursorPosition(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void OnScroll(double xoffset, double yoffset);
    void OnFramebufferSize(int width, int height);
    void OnContentScale(float xscale, float yscale);

    template <typename T, typename... Args>
    void AddWindow(Args... args)
    {
        m_windows.push_back(UniqueWindow(new T(std::forward<Args...>(args...))));
        m_windows.back()->m_fonts = m_fonts;
    }

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
    Fonts                      m_fonts;
    MouseState                 m_mouse_state;
    etna::UniqueDescriptorPool m_descriptor_pool;
    etna::Queue                m_graphics_queue;
    etna::Extent2D             m_extent;
    std::vector<UniqueWindow>  m_windows;
};
