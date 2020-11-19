#include "camera.hpp"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>
#include <stdexcept>

#include <algorithm>

namespace {

struct Dimensions final {
    float width;
    float height;
    float depth;
};

static Dimensions ComputeDimensions(ForwardAxis forward, UpAxis up, AABB aabb)
{
    auto extent_x = aabb.ExtentX();
    auto extent_y = aabb.ExtentY();
    auto extent_z = aabb.ExtentZ();

    if (up == UpAxis::PositiveX || up == UpAxis::NegativeX) {
        if (forward == ForwardAxis::PositiveY || forward == ForwardAxis::NegativeY) {
            return Dimensions{ .width = extent_z, .height = extent_x, .depth = extent_y };
        } else if (forward == ForwardAxis::PositiveZ || forward == ForwardAxis::NegativeZ) {
            return Dimensions{ .width = extent_y, .height = extent_x, .depth = extent_z };
        }
    }
    if (up == UpAxis::PositiveY || up == UpAxis::NegativeY) {
        if (forward == ForwardAxis::PositiveX || forward == ForwardAxis::NegativeX) {
            return Dimensions{ .width = extent_z, .height = extent_y, .depth = extent_x };
        } else if (forward == ForwardAxis::PositiveZ || forward == ForwardAxis::NegativeZ) {
            return Dimensions{ .width = extent_x, .height = extent_y, .depth = extent_z };
        }
    }
    if (up == UpAxis::PositiveZ || up == UpAxis::NegativeZ) {
        if (forward == ForwardAxis::PositiveX || forward == ForwardAxis::NegativeX) {
            return Dimensions{ .width = extent_y, .height = extent_z, .depth = extent_x };
        } else if (forward == ForwardAxis::PositiveY || forward == ForwardAxis::NegativeY) {
            return Dimensions{ .width = extent_x, .height = extent_z, .depth = extent_y };
        }
    }

    throw std::invalid_argument("ComputeDimensions: invalid arguments 'forward' and 'up'");
}

static glm::vec3 GetForwardVector(ForwardAxis forward)
{
    switch (forward) {
    case ForwardAxis::PositiveX: return glm::vec3(1, 0, 0);
    case ForwardAxis::NegativeX: return glm::vec3(-1, 0, 0);
    case ForwardAxis::PositiveY: return glm::vec3(0, 1, 0);
    case ForwardAxis::NegativeY: return glm::vec3(0, -1, 0);
    case ForwardAxis::PositiveZ: return glm::vec3(0, 0, 1);
    case ForwardAxis::NegativeZ: return glm::vec3(0, 0, -1);
    default: break;
    }

    throw std::invalid_argument("GetForwardVector: invalid argument");
}

static glm::vec3 GetUpVector(UpAxis up)
{
    switch (up) {
    case UpAxis::PositiveX: return glm::vec3(1, 0, 0);
    case UpAxis::NegativeX: return glm::vec3(-1, 0, 0);
    case UpAxis::PositiveY: return glm::vec3(0, 1, 0);
    case UpAxis::NegativeY: return glm::vec3(0, -1, 0);
    case UpAxis::PositiveZ: return glm::vec3(0, 0, 1);
    case UpAxis::NegativeZ: return glm::vec3(0, 0, -1);
    default: break;
    }

    throw std::invalid_argument("GetUpVector: invalid argument");
}

} // namespace

Camera Camera::Create(
    Orientation orientation,
    ForwardAxis forward_axis,
    UpAxis      up_axis,
    ObjectView  object_view,
    AABB        object,
    Degrees     fovy,
    float       aspect,
    float       near,
    float       far)
{
    using namespace glm;

    auto up         = GetUpVector(up_axis);
    auto forward    = GetForwardVector(forward_axis);
    auto right      = orientation == Orientation::RightHanded ? cross(forward, up) : -cross(forward, up);
    auto dimensions = ComputeDimensions(forward_axis, up_axis, object);
    auto elevation  = 0_rad;
    auto azimuth    = 0_rad;
    auto obj_width  = 0.0f;
    auto obj_height = 0.0f;
    auto obj_depth  = 0.0f;

    if (object_view == ObjectView::Front || object_view == ObjectView::Back) {
        azimuth    = object_view == ObjectView::Front ? Radians(0) : Radians(pi<float>());
        obj_width  = dimensions.width;
        obj_height = dimensions.height;
        obj_depth  = dimensions.depth;
    }
    if (object_view == ObjectView::Left || object_view == ObjectView::Right) {
        azimuth    = object_view == ObjectView::Left ? Radians(half_pi<float>()) : Radians(-half_pi<float>());
        obj_width  = dimensions.depth;
        obj_height = dimensions.height;
        obj_depth  = dimensions.width;
    }
    if (object_view == ObjectView::Top || object_view == ObjectView::Bottom) {
        elevation  = object_view == ObjectView::Top ? Radians(half_pi<float>()) : Radians(-half_pi<float>());
        obj_width  = dimensions.width;
        obj_height = dimensions.depth;
        obj_depth  = dimensions.height;
    }

    auto fovy_rad     = ToRadians(fovy);
    auto fovx_rad     = aspect * fovy_rad;
    auto distance     = 0.0f;
    auto min_distance = std::min(std::min(obj_width, obj_height), obj_depth);
    auto max_distance = 500 * min_distance;
    auto perspective  = perspectiveRH(fovy_rad.value, aspect, near, far);

    if (auto obj_aspect = obj_width / obj_height; obj_aspect > aspect) {
        distance = (0.5f * obj_depth) + (0.5f * obj_width) / tanf(0.5f * fovx_rad.value);
    } else {
        distance = (0.5f * obj_depth) + (0.5f * obj_height) / tanf(0.5f * fovy_rad.value);
    }

    auto limits = CameraLimits{

        .elevation = { -90_deg, 90_deg },
        .azimuth   = { -180_deg, 180_deg },
        .distance  = { min_distance, max_distance }
    };

    return Camera(
        elevation,
        azimuth,
        distance,
        forward,
        up,
        right,
        fovy_rad,
        perspective,
        aspect,
        near,
        far,
        object,
        limits);
}

glm::mat4 Camera::GetViewMatrix() const noexcept
{
    using namespace glm;

    bool flip   = m_coords.elevation >= Radians::HalfPi || m_coords.elevation <= -Radians::HalfPi;
    auto center = m_object.Center();
    auto view   = identity<mat4>();
    view        = rotate(view, m_coords.elevation.value, m_basis.right);
    view        = rotate(view, m_coords.azimuth.value, m_basis.up);
    auto eye    = -m_coords.distance * m_basis.forward * mat3(view) + center;

    return lookAtRH(eye, center, flip ? -m_basis.up : m_basis.up);
}

SphericalCoordinates Camera::GetSphericalCoordinates() const noexcept
{
    using namespace glm;

    auto camera_up = CameraUp::Normal;
    auto elevation = m_coords.elevation;

    if (elevation > Radians::HalfPi) {
        elevation = Radians::Pi - elevation;
        camera_up = CameraUp::Inverted;
    } else if (elevation < -Radians::HalfPi) {
        elevation = -Radians::Pi - elevation;
        camera_up = CameraUp::Inverted;
    }

    return SphericalCoordinates{ elevation, m_coords.azimuth, camera_up, m_coords.distance };
}

const CameraLimits& Camera::GetLimits() const noexcept
{
    return m_limits;
}

void Camera::Orbit(Degrees elevation_delta, Degrees azimuth_delta) noexcept
{
    using namespace glm;

    m_coords.azimuth = m_coords.azimuth + ToRadians(azimuth_delta);
    if (m_coords.azimuth > Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth - Radians::TwoPi;
    } else if (m_coords.azimuth < -Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth + Radians::TwoPi;
    }

    m_coords.elevation = m_coords.elevation + ToRadians(elevation_delta);
    if (m_coords.elevation > Radians::Pi) {
        m_coords.elevation = m_coords.elevation - Radians::TwoPi;
    } else if (m_coords.elevation < -Radians::Pi) {
        m_coords.elevation = m_coords.elevation + Radians::TwoPi;
    }
}

void Camera::Zoom(float delta) noexcept
{
    constexpr auto delta_modifier = 0.01f;

    auto distance     = (m_coords.distance - m_limits.distance.min) / m_limits.distance.max;
    auto step         = (0.01f + distance) * delta * delta_modifier;
    distance          = std::clamp(distance + step, 0.0f, 1.0f);
    distance          = (distance * m_limits.distance.max) + m_limits.distance.min;
    m_coords.distance = std::clamp(distance, m_limits.distance.min, m_limits.distance.max);
}

void Camera::UpdateAspect(float aspect) noexcept
{
    m_perspective.aspect = aspect;
    m_perspective.matrix = glm::perspectiveRH(m_perspective.fovy.value, aspect, m_perspective.near, m_perspective.far);
}

void Camera::UpdateSphericalCoordinates(const SphericalCoordinates& coordinates) noexcept
{
    using namespace glm;

    m_coords.azimuth = coordinates.azimuth;
    if (m_coords.azimuth > Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth - Radians::TwoPi;
    } else if (m_coords.azimuth < -Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth + Radians::TwoPi;
    }

    m_coords.elevation = coordinates.elevation;

    if (coordinates.camera_up == CameraUp::Inverted) {
        m_coords.elevation = (m_coords.elevation >= 0_rad) ? (Radians::Pi - m_coords.elevation)
                                                           : (-Radians::Pi - m_coords.elevation);
    }

    if (m_coords.elevation > Radians::Pi) {
        m_coords.elevation = m_coords.elevation - Radians::TwoPi;
    } else if (m_coords.elevation < -Radians::Pi) {
        m_coords.elevation = m_coords.elevation + Radians::TwoPi;
    }

    m_coords.distance = std::clamp(coordinates.distance, m_limits.distance.min, m_limits.distance.max);
}
