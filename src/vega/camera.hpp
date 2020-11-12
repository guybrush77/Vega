#pragma once

#include "etna/core.hpp"

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
    explicit Degrees(long double value) noexcept : value(static_cast<float>(value)) {}
    float value;
};

inline Degrees operator"" _deg(long double value) noexcept
{
    return Degrees(value);
}

inline Degrees operator"" _deg(unsigned long long value) noexcept
{
    return Degrees(static_cast<long double>(value));
}

class Camera {
  public:
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
    auto Center() const noexcept { return m_center; }
    auto Position() const noexcept -> glm::vec3;

    void Orbit(Degrees horizontal, Degrees vertical) noexcept;

    void UpdateExtent(etna::Extent2D extent) noexcept;

  private:
    Camera(float fovy, float distance, glm::vec3 center, glm::mat4 view, glm::mat4 perspective)
        : m_fovy(fovy), m_distance(distance), m_center(center), m_view(view), m_perspective(perspective)
    {}

    float     m_fovy;
    float     m_distance;
    glm::vec3 m_center;
    glm::mat4 m_view;
    glm::mat4 m_perspective;
};
