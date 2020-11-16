#pragma once

#include "platform.hpp"

#include "utils/math.hpp"

BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

enum class CameraUp { Normal, Inverted };

enum class Orientation { RightHanded, LeftHanded };

enum class ForwardAxis { PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ };

enum class UpAxis { PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ };

enum class ObjectView { Front, Back, Left, Right, Top, Bottom };

struct SphericalCoordinates {
    Radians  elevation;
    Radians  azimuth;
    CameraUp camera_up;
};

class Camera {
  public:
    static Camera Create(
        Orientation orientation,
        ForwardAxis forward,
        UpAxis      up,
        ObjectView  camera_view,
        AABB        aabb,
        Degrees     fovy,
        float       aspect);

    auto GetViewMatrix() const noexcept -> glm::mat4;
    auto GetPerspectiveMatrix() const noexcept { return m_perspective; }
    auto GetSphericalCoordinates() const noexcept -> SphericalCoordinates;

    void Orbit(Degrees elevation_delta, Degrees azimuth_delta) noexcept;
    void UpdateAspect(float aspect) noexcept;
    void UpdateView(Radians elevation, Radians azimuth, CameraUp camera_up) noexcept;

  private:
    Camera(
        Radians   elevation,
        Radians   azimuth,
        float     distance,
        Radians   fovy,
        float     aspect,
        glm::vec3 forward,
        glm::vec3 up,
        glm::vec3 right,
        glm::vec3 center,
        glm::mat4 perspective)
        : m_elevation(elevation), m_azimuth(azimuth), m_distance(distance), m_fovy(fovy), m_aspect(aspect),
          m_forward(forward), m_up(up), m_right(right), m_center(center), m_perspective(perspective)
    {}

    Radians   m_elevation;
    Radians   m_azimuth;
    float     m_distance;
    Radians   m_fovy;
    float     m_aspect;
    glm::vec3 m_forward;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_center;
    glm::mat4 m_perspective;
};
