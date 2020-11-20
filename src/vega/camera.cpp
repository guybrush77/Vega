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

    auto fovy_rad      = ToRadians(fovy);
    auto fovx_rad      = aspect * fovy_rad;
    auto distance      = 0.0f;
    auto min_dimension = std::min({ obj_width, obj_height, obj_depth });
    auto min_distance  = 0.1f * min_dimension;
    auto max_distance  = 1000 * min_dimension;
    auto min_offset    = -10 * min_dimension;
    auto max_offset    = +10 * min_dimension;

    if (auto obj_aspect = obj_width / obj_height; obj_aspect > aspect) {
        distance = (0.5f * obj_depth) + (0.5f * obj_width) / tanf(0.5f * fovx_rad.value);
    } else {
        distance = (0.5f * obj_depth) + (0.5f * obj_height) / tanf(0.5f * fovy_rad.value);
    }

    auto limits = CameraLimits{

        .elevation = { -90_deg, 90_deg },
        .azimuth   = { -180_deg, 180_deg },
        .distance  = { min_distance, max_distance },
        .offset_x  = { min_offset, max_offset },
        .offset_y  = { min_offset, max_offset },
        .fov_y     = { 5_deg, 90_deg }
    };

    distance = std::clamp(distance, limits.distance.min, limits.distance.max);
    fovy_rad = std::clamp(fovy_rad, ToRadians(limits.fov_y.min), ToRadians(limits.fov_y.max));

    return Camera(elevation, azimuth, distance, forward, up, right, fovy_rad, aspect, near, far, object, limits);
}

glm::mat4 Camera::GetViewMatrix() const noexcept
{
    using namespace glm;

    bool flip   = m_coords.elevation >= Radians::HalfPi || m_coords.elevation <= -Radians::HalfPi;
    auto view   = identity<mat4>();
    view        = rotate(view, m_coords.elevation.value, m_basis.right);
    view        = rotate(view, m_coords.azimuth.value, m_basis.up);
    auto pan_x  = vec3(row(view, 0)) * m_offset.horizontal;
    auto pan_y  = vec3(row(view, 2)) * m_offset.vertical;
    auto center = m_object.Center() - pan_x + pan_y;
    auto eye    = -m_coords.distance * m_basis.forward * mat3(view) + center;

    return lookAtRH(eye, center, flip ? -m_basis.up : m_basis.up);
}

glm::mat4 Camera::GetPerspectiveMatrix() const noexcept
{
    auto dim  = std::min({ m_object.ExtentX(), m_object.ExtentY(), m_object.ExtentZ() });
    auto near = std::max(dim * 0.01f, m_perspective.near);
    auto far  = std::min(std::numeric_limits<float>::max(), m_perspective.far);

    return glm::perspectiveRH(m_perspective.fovy.value, m_perspective.aspect, near, far);
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

Offset Camera::GetOffset() const noexcept
{
    return m_offset;
}

Perspective Camera::GetPerspective() const noexcept
{
    return m_perspective;
}

const CameraLimits& Camera::GetLimits() const noexcept
{
    return m_limits;
}

void Camera::Orbit(Degrees delta_elevation, Degrees delta_azimuth) noexcept
{
    using namespace glm;

    m_coords.azimuth = m_coords.azimuth + ToRadians(delta_azimuth);
    if (m_coords.azimuth > Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth - Radians::TwoPi;
    } else if (m_coords.azimuth < -Radians::Pi) {
        m_coords.azimuth = m_coords.azimuth + Radians::TwoPi;
    }

    m_coords.elevation = m_coords.elevation + ToRadians(delta_elevation);
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

void Camera::Track(float delta_x, float delta_y) noexcept
{
    using namespace glm;

    constexpr auto delta_modifier = 0.5f;

    auto step     = std::min(std::min(m_object.ExtentX(), m_object.ExtentY()), m_object.ExtentZ());
    auto distance = (m_coords.distance - m_limits.distance.min) / m_limits.distance.max;
    auto step_x   = step * (0.01f + distance) * delta_x * delta_modifier;
    auto step_y   = step * (0.01f + distance) * delta_y * delta_modifier;

    m_offset.horizontal = m_offset.horizontal + step_x;
    m_offset.vertical   = m_offset.vertical + step_y;
}

void Camera::UpdateAspect(float aspect) noexcept
{
    m_perspective.aspect = aspect;
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

void Camera::UpdateOffset(Offset offset) noexcept
{
    m_offset = offset;
}

void Camera::UpdatePerspective(Perspective perspective) noexcept
{
    m_perspective = perspective;
}
