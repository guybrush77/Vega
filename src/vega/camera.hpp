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
    float    distance;
};

struct CameraLimits {
    struct {
        Degrees min;
        Degrees max;
    } elevation;
    struct {
        Degrees min;
        Degrees max;
    } azimuth;
    struct {
        float min;
        float max;
    } distance;
};

class Camera {
  public:
    static Camera Create(
        Orientation orientation,
        ForwardAxis forward_axis,
        UpAxis      up_axis,
        ObjectView  object_view,
        AABB        object,
        Degrees     fovy,
        float       aspect,
        float       near = 0.1f,
        float       far  = FLT_MAX);

    auto GetViewMatrix() const noexcept -> glm::mat4;
    auto GetPerspectiveMatrix() const noexcept { return m_perspective.matrix; }
    auto GetSphericalCoordinates() const noexcept -> SphericalCoordinates;
    auto GetLimits() const noexcept -> const CameraLimits&;

    void Orbit(Degrees elevation_delta, Degrees azimuth_delta) noexcept;
    void Zoom(float delta) noexcept;

    void UpdateAspect(float aspect) noexcept;
    void UpdateSphericalCoordinates(const SphericalCoordinates& coordinates) noexcept;

  private:
    Camera(
        Radians      elevation,
        Radians      azimuth,
        float        distance,
        glm::vec3    forward,
        glm::vec3    up,
        glm::vec3    right,
        Radians      fovy,
        glm::mat4    perspective,
        float        aspect,
        float        near,
        float        far,
        AABB         object,
        CameraLimits limits)
        : m_coords{ elevation, azimuth, distance }, m_basis{ forward, up, right },
          m_perspective{ fovy, aspect, near, far, perspective }, m_object{ object }, m_limits{ limits }
    {}

    struct Coords {
        Radians elevation;
        Radians azimuth;
        float   distance;
    };

    struct Basis {
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
    };

    struct Perspective {
        Radians   fovy;
        float     aspect;
        float     near;
        float     far;
        glm::mat4 matrix;
    };

    Coords       m_coords;
    Basis        m_basis;
    Perspective  m_perspective;
    AABB         m_object;
    CameraLimits m_limits;
};
