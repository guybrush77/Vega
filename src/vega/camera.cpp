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

static Dimensions ComputeDimensions(Forward forward, Up up, AABB aabb)
{
    auto extent_x = aabb.ExtentX();
    auto extent_y = aabb.ExtentY();
    auto extent_z = aabb.ExtentZ();

    if (up == Axis::PositiveX || up == Axis::NegativeX) {
        if (forward == Axis::PositiveY || forward == Axis::NegativeY) {
            return Dimensions{ .width = extent_z, .height = extent_x, .depth = extent_y };
        } else if (forward == Axis::PositiveZ || forward == Axis::NegativeZ) {
            return Dimensions{ .width = extent_y, .height = extent_x, .depth = extent_z };
        }
    }
    if (up == Axis::PositiveY || up == Axis::NegativeY) {
        if (forward == Axis::PositiveX || forward == Axis::NegativeX) {
            return Dimensions{ .width = extent_z, .height = extent_y, .depth = extent_x };
        } else if (forward == Axis::PositiveZ || forward == Axis::NegativeZ) {
            return Dimensions{ .width = extent_x, .height = extent_y, .depth = extent_z };
        }
    }
    if (up == Axis::PositiveZ || up == Axis::NegativeZ) {
        if (forward == Axis::PositiveX || forward == Axis::NegativeX) {
            return Dimensions{ .width = extent_y, .height = extent_z, .depth = extent_x };
        } else if (forward == Axis::PositiveY || forward == Axis::NegativeY) {
            return Dimensions{ .width = extent_x, .height = extent_z, .depth = extent_y };
        }
    }

    throw std::invalid_argument("ComputeDimensions: invalid arguments 'forward' and 'up'");
}

static Axis GetAxis(glm::vec3 vec)
{
    const auto mag_x = fabsf(vec.x);
    const auto mag_y = fabsf(vec.y);
    const auto mag_z = fabsf(vec.z);

    if ((mag_x > mag_y) && (mag_x > mag_z)) {
        return vec.x > 0 ? Axis::PositiveX : Axis::NegativeX;
    } else if ((mag_y > mag_x) && (mag_y > mag_z)) {
        return vec.y > 0 ? Axis::PositiveY : Axis::NegativeY;
    } else if ((mag_z > mag_x) && (mag_z > mag_y)) {
        return vec.z > 0 ? Axis::PositiveZ : Axis::NegativeZ;
    }

    throw std::invalid_argument("GetAxis: invalid argument");
}

static std::pair<float, float> ComputeClipPlanes(const AABB& aabb)
{
    auto dim = std::min({ aabb.ExtentX(), aabb.ExtentY(), aabb.ExtentZ() });

    auto len = floorf(log10f(dim)) + 1.0f;
    auto num = powf(10.0f, len);

    return { num / 100.0f, num * 100.0f };
}

} // namespace

Camera Camera::Create(
    Orientation orientation,
    Forward     forward,
    Up          up,
    ObjectView  object_view,
    AABB        object,
    Degrees     fovy,
    float       aspect)
{
    using namespace glm;

    auto cross_prod = cross(forward.Vector(), up.Vector());
    auto right_vec  = orientation == Orientation::RightHanded ? cross_prod : -cross_prod;
    auto right      = Right(GetAxis(right_vec));
    auto dimensions = ComputeDimensions(forward, up, object);
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

    auto basis       = Basis{ forward, up, right, orientation };
    auto [near, far] = ComputeClipPlanes(object);
    auto perspective = Perspective{ fovy_rad, aspect, 0.0f, near, far, far };

    return Camera(basis, elevation, azimuth, distance, perspective, object, limits);
}

glm::mat4 Camera::GetViewMatrix() const noexcept
{
    using namespace glm;

    auto elevation = m_coords.elevation.value;
    auto azimuth   = m_coords.azimuth.value;
    auto forward   = m_basis.forward.Vector();
    auto up        = m_basis.up.Vector();
    auto right     = m_basis.right.Vector();
    auto view      = rotate(rotate(identity<mat4>(), elevation, right), azimuth, up);
    auto center    = m_object.Center();
    auto eye       = -m_coords.distance * forward * mat3(view) + center;
    bool flip      = m_coords.elevation >= Radians::HalfPi || m_coords.elevation <= -Radians::HalfPi;

    view = lookAtRH(eye, center, flip ? -up : up);

    view[3][0] += m_offset.horizontal;
    view[3][1] += m_offset.vertical;

    return view;
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

const Perspective& Camera::GetPerspective() const noexcept
{
    return m_perspective;
}

Basis Camera::GetBasis() const noexcept
{
    return m_basis;
}

AABB Camera::GetObject() const noexcept
{
    return m_object;
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

    auto step     = std::min({ m_object.ExtentX(), m_object.ExtentY(), m_object.ExtentZ() });
    auto distance = (m_coords.distance - m_limits.distance.min) / m_limits.distance.max;
    auto step_x   = step * (0.01f + distance) * delta_x * delta_modifier;
    auto step_y   = step * (0.01f + distance) * delta_y * delta_modifier;

    m_offset.horizontal = m_offset.horizontal + step_x;
    m_offset.vertical   = m_offset.vertical - step_y;
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
