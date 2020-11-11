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

static Dimensions ComputeDimensions(CameraForward forward, CameraUp up, AABB aabb)
{
    auto extent_x = aabb.ExtentX();
    auto extent_y = aabb.ExtentY();
    auto extent_z = aabb.ExtentZ();

    if (up == CameraUp::PositiveX || up == CameraUp::NegativeX) {
        if (forward == CameraForward::PositiveY || forward == CameraForward::NegativeY) {
            return Dimensions{ .width = extent_z, .height = extent_x, .depth = extent_y };
        } else if (forward == CameraForward::PositiveZ || forward == CameraForward::NegativeZ) {
            return Dimensions{ .width = extent_y, .height = extent_x, .depth = extent_z };
        }
    }
    if (up == CameraUp::PositiveY || up == CameraUp::NegativeY) {
        if (forward == CameraForward::PositiveX || forward == CameraForward::NegativeX) {
            return Dimensions{ .width = extent_z, .height = extent_y, .depth = extent_x };
        } else if (forward == CameraForward::PositiveZ || forward == CameraForward::NegativeZ) {
            return Dimensions{ .width = extent_x, .height = extent_y, .depth = extent_z };
        }
    }
    if (up == CameraUp::PositiveZ || up == CameraUp::NegativeZ) {
        if (forward == CameraForward::PositiveX || forward == CameraForward::NegativeX) {
            return Dimensions{ .width = extent_y, .height = extent_z, .depth = extent_x };
        } else if (forward == CameraForward::PositiveY || forward == CameraForward::NegativeY) {
            return Dimensions{ .width = extent_x, .height = extent_z, .depth = extent_y };
        }
    }

    throw std::invalid_argument("ComputeDimensions: invalid arguments 'forward' and 'up'");
}

} // namespace

Camera Camera::Create(
    Orientation    orientation,
    CameraForward  forward,
    CameraUp       up,
    ObjectView     camera_view,
    etna::Extent2D extent,
    AABB           aabb,
    Degrees        fovy)
{
    using namespace glm;

    auto vec_up = vec3{};

    switch (up) {
    case CameraUp::PositiveX: vec_up = vec3(1, 0, 0); break;
    case CameraUp::NegativeX: vec_up = vec3(-1, 0, 0); break;
    case CameraUp::PositiveY: vec_up = vec3(0, 1, 0); break;
    case CameraUp::NegativeY: vec_up = vec3(0, -1, 0); break;
    case CameraUp::PositiveZ: vec_up = vec3(0, 0, 1); break;
    case CameraUp::NegativeZ: vec_up = vec3(0, 0, -1); break;
    default: break;
    }

    auto vec_forward = vec3{};

    switch (forward) {
    case CameraForward::PositiveX: vec_forward = vec3(1, 0, 0); break;
    case CameraForward::NegativeX: vec_forward = vec3(-1, 0, 0); break;
    case CameraForward::PositiveY: vec_forward = vec3(0, 1, 0); break;
    case CameraForward::NegativeY: vec_forward = vec3(0, -1, 0); break;
    case CameraForward::PositiveZ: vec_forward = vec3(0, 0, 1); break;
    case CameraForward::NegativeZ: vec_forward = vec3(0, 0, -1); break;
    default: break;
    }

    auto cross_prod = cross(vec_forward, vec_up);
    auto vec_right  = orientation == Orientation::RightHanded ? cross_prod : -cross_prod;
    auto dimensions = ComputeDimensions(forward, up, aabb);
    auto eye        = vec3{};
    auto obj_width  = float{};
    auto obj_height = float{};
    auto obj_depth  = float{};

    if (camera_view == ObjectView::Front || camera_view == ObjectView::Back) {
        eye        = camera_view == ObjectView::Front ? -vec_forward : vec_forward;
        obj_width  = dimensions.width;
        obj_height = dimensions.height;
        obj_depth  = dimensions.depth;
    }
    if (camera_view == ObjectView::Left || camera_view == ObjectView::Right) {
        eye        = camera_view == ObjectView::Left ? -vec_right : vec_right;
        obj_width  = dimensions.depth;
        obj_height = dimensions.height;
        obj_depth  = dimensions.width;
    }
    if (camera_view == ObjectView::Top || camera_view == ObjectView::Bottom) {
        eye        = camera_view == ObjectView::Bottom ? -vec_up : vec_up;
        vec_up     = camera_view == ObjectView::Bottom ? -vec_forward : vec_forward;
        obj_width  = dimensions.width;
        obj_height = dimensions.depth;
        obj_depth  = dimensions.height;
    }

    auto aspect   = etna::narrow_cast<float>(extent.width) / extent.height;
    auto fovy_rad = radians(fovy.value);
    auto fovx_rad = aspect * fovy_rad;
    auto center   = aabb.Center();
    auto distance = 0.0f;

    if (auto obj_aspect = obj_width / obj_height; obj_aspect > aspect) {
        distance = (0.5f * obj_depth) + (0.5f * obj_width) / tanf(0.5f * fovx_rad);
    } else {
        distance = (0.5f * obj_depth) + (0.5f * obj_height) / tanf(0.5f * fovy_rad);
    }

    eye = distance * eye + center;

    auto view        = lookAtRH(eye, center, vec_up);
    auto perspective = glm::perspectiveRH(fovy_rad, aspect, 1.0f, 1000.0f);

    return Camera(fovy_rad, distance, center, view, perspective);
}

glm::vec3 Camera::Position() const noexcept
{
    using namespace glm;

    auto up      = vec3(row(m_view, 1));
    auto forward = vec3(row(m_view, 2));
    return m_distance * forward + m_center;
}

void Camera::Orbit(Degrees horizontal, Degrees vertical) noexcept
{
    using namespace glm;

    auto right = vec3(row(m_view, 0));
    auto up    = vec3(row(m_view, 1));

    m_view = rotate(m_view, radians(horizontal.value), up);
    m_view = rotate(m_view, radians(vertical.value), right);

    auto eye = Position();

    m_view = lookAtRH(eye, m_center, up);
}

void Camera::UpdateExtent(etna::Extent2D extent) noexcept
{
    auto aspect = etna::narrow_cast<float>(extent.width) / extent.height;

    m_perspective = glm::perspectiveRH(m_fovy, aspect, 1.0f, 1000.0f);
}
