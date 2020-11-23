#pragma once

#include "platform.hpp"

#include "utils/math.hpp"

BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

#include <array>

template <typename>
auto ToStringArray();

#define DEFINE_ENUM_TO_STRING(Type, ...) \
    template <> \
    inline constexpr auto ToStringArray<Type>() noexcept \
    { \
        return std::array{ __VA_ARGS__ }; \
    } \
    inline constexpr const char* ToString(Type arg) noexcept { return ToStringArray<Type>()[static_cast<size_t>(arg)]; }

//
// CameraUp
//
enum class CameraUp { Normal, Inverted };

DEFINE_ENUM_TO_STRING(CameraUp, "Normal", "Inverted")

//
// Orientation
//
enum class Orientation { RightHanded, LeftHanded };

DEFINE_ENUM_TO_STRING(Orientation, "RightHanded", "LeftHanded")

//
// Orientation
//
enum class Axis { PositiveX, NegativeX, PositiveY, NegativeY, PositiveZ, NegativeZ };

DEFINE_ENUM_TO_STRING(Axis, "Positive X", "Negative X", "Positive Y", "Negative Y", "Positive Z", "Negative Z");

//
// ObjectView
//
enum class ObjectView { Front, Back, Left, Right, Top, Bottom };

DEFINE_ENUM_TO_STRING(ObjectView, "Front", "Back", "Left", "Right", "Top", "Bottom");

template <typename T>
struct AxisBase {
    static constexpr T FromInt(int value) { return T(static_cast<Axis>(value)); }

    bool operator==(const T& rhs) const noexcept { return axis == rhs.axis; }
    bool operator==(Axis rhs) const noexcept { return axis == rhs; }

    constexpr int ToInt() const noexcept { return static_cast<int>(axis); }

    constexpr glm::vec3 Vector() const noexcept
    {
        switch (axis) {
        case Axis::PositiveX: return glm::vec3(1, 0, 0);
        case Axis::NegativeX: return glm::vec3(-1, 0, 0);
        case Axis::PositiveY: return glm::vec3(0, 1, 0);
        case Axis::NegativeY: return glm::vec3(0, -1, 0);
        case Axis::PositiveZ: return glm::vec3(0, 0, 1);
        case Axis::NegativeZ: return glm::vec3(0, 0, -1);
        default: break;
        }
        return {};
    }

    Axis axis{};
};

struct Forward final : AxisBase<Forward> {
    explicit Forward(Axis axis) noexcept : AxisBase{ axis } {}
};

struct Up final : AxisBase<Up> {
    explicit Up(Axis axis) noexcept : AxisBase{ axis } {}
};

struct Right final : AxisBase<Right> {
    explicit Right(Axis axis) noexcept : AxisBase{ axis } {}
};

struct Basis final {
    Forward     forward;
    Up          up;
    Right       right;
    Orientation orientation;
};

struct SphericalCoordinates final {
    Radians  elevation;
    Radians  azimuth;
    CameraUp camera_up;
    float    distance;
};

struct Offset final {
    float horizontal;
    float vertical;
};

struct Perspective final {
    Radians fovy;
    float   aspect;
    float   near;
    float   far;
};

struct CameraLimits final {
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
    struct {
        float min;
        float max;
    } offset_x;
    struct {
        float min;
        float max;
    } offset_y;
    struct {
        Degrees min;
        Degrees max;
    } fov_y;
};

class Camera {
  public:
    static Camera Create(
        Orientation orientation,
        Forward     forward,
        Up          up,
        ObjectView  object_view,
        AABB        object,
        Degrees     fovy,
        float       aspect,
        float       near = 0,
        float       far  = std::numeric_limits<float>::infinity());

    virtual ~Camera() noexcept = default;

    auto GetViewMatrix() const noexcept -> glm::mat4;
    auto GetPerspectiveMatrix() const noexcept -> glm::mat4;
    auto GetSphericalCoordinates() const noexcept -> SphericalCoordinates;
    auto GetOffset() const noexcept -> Offset;
    auto GetPerspective() const noexcept -> Perspective;
    auto GetBasis() const noexcept -> Basis;
    auto GetObject() const noexcept -> AABB;
    auto GetLimits() const noexcept -> const CameraLimits&;

    void Orbit(Degrees delta_elevation, Degrees delta_azimuth) noexcept;
    void Zoom(float delta) noexcept;
    void Track(float delta_x, float delta_y) noexcept;

    void UpdateAspect(float aspect) noexcept;
    void UpdateSphericalCoordinates(const SphericalCoordinates& coordinates) noexcept;
    void UpdateOffset(Offset offset) noexcept;
    void UpdatePerspective(Perspective perspective) noexcept;

  private:
    Camera(
        Basis        basis,
        Radians      elevation,
        Radians      azimuth,
        float        distance,
        Perspective  perspective,
        AABB         object,
        CameraLimits limits)
        : m_basis{ basis }, m_coords{ elevation, azimuth, distance }, m_offset{},
          m_perspective{ perspective }, m_object{ object }, m_limits{ limits }
    {}

    struct Coordinates final {
        Radians elevation{};
        Radians azimuth{};
        float   distance{};
    };

    Basis        m_basis;
    Coordinates  m_coords;
    Offset       m_offset;
    Perspective  m_perspective;
    AABB         m_object;
    CameraLimits m_limits;
};
