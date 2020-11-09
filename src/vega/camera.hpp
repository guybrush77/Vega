#pragma once

#include "core.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

struct AABB final {
    glm::vec3 min;
    glm::vec3 max;

    auto Center() const noexcept { return 0.5f * (min + max); }
    auto ExtentX() const noexcept { return max.x - min.x; }
    auto ExtentY() const noexcept { return max.y - min.y; }
    auto ExtentZ() const noexcept { return max.z - min.z; }
};

enum class Orientation { RightHanded, LeftHanded };

enum class CameraForward {
    PositiveX,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ,
};

enum class CameraUp {
    PositiveX,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ,
};

enum class ObjectView { Front, Back, Left, Right, Top, Bottom };

struct Degrees final {
    explicit Degrees(float value) noexcept : value(value) {}
    float value;
};

class Camera {
  public:
    Camera(glm::mat4 view, glm::mat4 perspective) : m_view(view), m_perspective(perspective) {}

    static Camera Create(
        Orientation    orientation,
        CameraForward  forward,
        CameraUp       up,
        ObjectView     camera_view,
        etna::Extent2D extent,
        AABB           aabb,
        Degrees        fovy);

    auto View() const noexcept { return m_view; }
    auto Perspective() const noexcept { return m_perspective; }

  private:
    glm::mat4 m_view;
    glm::mat4 m_perspective;
};
