#pragma once

#include "platform.hpp"
#include <math.h>
BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>

END_DISABLE_WARNINGS

struct AABB final {
    glm::vec3 min;
    glm::vec3 max;

    auto Center() const noexcept { return 0.5f * (min + max); }
    auto ExtentX() const noexcept { return max.x - min.x; }
    auto ExtentY() const noexcept { return max.y - min.y; }
    auto ExtentZ() const noexcept { return max.z - min.z; }
};

struct Radians final {
    constexpr Radians() noexcept = default;
    constexpr explicit Radians(float value) noexcept : value(value) {}

    Radians operator+=(Radians rhs) noexcept { value += rhs.value; }
    Radians operator-=(Radians rhs) noexcept { value -= rhs.value; }

    float value = 0;
};

inline constexpr Radians operator*(float lhs, Radians rhs) noexcept
{
    return Radians(lhs * rhs.value);
}

inline constexpr Radians operator+(Radians lhs, Radians rhs) noexcept
{
    return Radians(lhs.value + rhs.value);
}

inline Radians operator-(Radians lhs, Radians rhs) noexcept
{
    return Radians(lhs.value - rhs.value);
}

struct Degrees final {
    constexpr Degrees() noexcept = default;
    constexpr explicit Degrees(float value) noexcept : value(value) {}

    Degrees operator+=(Degrees rhs) noexcept { value += rhs.value; }
    Degrees operator-=(Degrees rhs) noexcept { value -= rhs.value; }

    float value = 0;
};

inline constexpr Degrees operator+(Degrees lhs, Degrees rhs) noexcept
{
    return Degrees(lhs.value + rhs.value);
}

inline constexpr Degrees operator-(Degrees lhs, Degrees rhs) noexcept
{
    return Degrees(lhs.value - rhs.value);
}

inline constexpr Radians ToRadians(Degrees degrees) noexcept
{
    return Radians(0.01745329252f * degrees.value);
}

inline constexpr Degrees ToDegrees(Radians radians) noexcept
{
    return Degrees(57.29577951f * radians.value);
}