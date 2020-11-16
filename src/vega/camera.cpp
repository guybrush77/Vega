#include "camera.hpp"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>
#include <stdexcept>

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
    ForwardAxis forward,
    UpAxis      up,
    ObjectView  camera_view,
    AABB        aabb,
    Degrees     fovy,
    float       aspect)
{
    using namespace glm;

    auto vec_up      = GetUpVector(up);
    auto vec_forward = GetForwardVector(forward);
    auto cross_prod  = cross(vec_forward, vec_up);
    auto vec_right   = orientation == Orientation::RightHanded ? cross_prod : -cross_prod;
    auto dimensions  = ComputeDimensions(forward, up, aabb);
    auto elevation   = Radians(0);
    auto azimuth     = Radians(0);
    auto obj_width   = float{};
    auto obj_height  = float{};
    auto obj_depth   = float{};

    if (camera_view == ObjectView::Front || camera_view == ObjectView::Back) {
        azimuth    = camera_view == ObjectView::Front ? Radians(0) : Radians(pi<float>());
        obj_width  = dimensions.width;
        obj_height = dimensions.height;
        obj_depth  = dimensions.depth;
    }
    if (camera_view == ObjectView::Left || camera_view == ObjectView::Right) {
        azimuth    = camera_view == ObjectView::Left ? Radians(half_pi<float>()) : Radians(-half_pi<float>());
        obj_width  = dimensions.depth;
        obj_height = dimensions.height;
        obj_depth  = dimensions.width;
    }
    if (camera_view == ObjectView::Top || camera_view == ObjectView::Bottom) {
        elevation  = camera_view == ObjectView::Top ? Radians(half_pi<float>()) : Radians(-half_pi<float>());
        obj_width  = dimensions.width;
        obj_height = dimensions.depth;
        obj_depth  = dimensions.height;
    }

    auto fovy_rad = ToRadians(fovy);
    auto fovx_rad = aspect * fovy_rad;
    auto center   = aabb.Center();
    auto distance = 0.0f;

    if (auto obj_aspect = obj_width / obj_height; obj_aspect > aspect) {
        distance = (0.5f * obj_depth) + (0.5f * obj_width) / tanf(0.5f * fovx_rad.value);
    } else {
        distance = (0.5f * obj_depth) + (0.5f * obj_height) / tanf(0.5f * fovy_rad.value);
    }

    auto perspective = glm::perspectiveRH(fovy_rad.value, aspect, 0.1f, 1000.0f);

    return Camera(elevation, azimuth, distance, fovy_rad, aspect, vec_forward, vec_up, vec_right, center, perspective);
}

glm::mat4 Camera::GetViewMatrix() const noexcept
{
    using namespace glm;

    bool flip = m_elevation.value >= half_pi<float>() || m_elevation.value <= -half_pi<float>();
    auto view = identity<mat4>();
    view      = rotate(view, m_elevation.value, m_right);
    view      = rotate(view, m_azimuth.value, m_up);
    auto eye  = -m_distance * m_forward * mat3(view) + m_center;

    return lookAtRH(eye, m_center, flip ? -m_up : m_up);
}

SphericalCoordinates Camera::GetSphericalCoordinates() const noexcept
{
    using namespace glm;

    auto camera_up = CameraUp::Normal;
    auto elevation = m_elevation.value;

    if (elevation > half_pi<float>()) {
        elevation = pi<float>() - elevation;
        camera_up = CameraUp::Inverted;
    } else if (elevation < -half_pi<float>()) {
        elevation = -(pi<float>() + elevation);
        camera_up = CameraUp::Inverted;
    }

    return SphericalCoordinates{ Radians(elevation), Radians(m_azimuth), camera_up };
}

void Camera::Orbit(Degrees elevation_delta, Degrees azimuth_delta) noexcept
{
    using namespace glm;

    m_azimuth = m_azimuth + ToRadians(azimuth_delta);
    if (m_azimuth.value > pi<float>()) {
        m_azimuth.value -= two_pi<float>();
    } else if (m_azimuth.value < -pi<float>()) {
        m_azimuth.value += two_pi<float>();
    }

    m_elevation = m_elevation + ToRadians(elevation_delta);
    if (m_elevation.value > pi<float>()) {
        m_elevation.value -= two_pi<float>();
    } else if (m_elevation.value < -pi<float>()) {
        m_elevation.value += two_pi<float>();
    }
}

void Camera::UpdateAspect(float aspect) noexcept
{
    m_perspective = glm::perspectiveRH(m_fovy.value, aspect, 0.1f, 1000.0f);
}

void Camera::UpdateView(Radians elevation, Radians azimuth, CameraUp camera_up) noexcept
{
    using namespace glm;

    m_azimuth = azimuth;
    if (m_azimuth.value > pi<float>()) {
        m_azimuth.value -= two_pi<float>();
    } else if (m_azimuth.value < -pi<float>()) {
        m_azimuth.value += two_pi<float>();
    }

    if (camera_up == CameraUp::Inverted) {
        if (elevation.value >= 0) {
            elevation.value = pi<float>() - elevation.value;
        } else if (elevation.value < 0) {
            elevation.value = -pi<float>() - elevation.value;
        }
    }

    m_elevation = elevation;
    if (m_elevation.value > pi<float>()) {
        m_elevation.value -= two_pi<float>();
    } else if (m_elevation.value < -pi<float>()) {
        m_elevation.value += two_pi<float>();
    }
}
