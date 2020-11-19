#pragma once

#include "platform.hpp"

BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>

END_DISABLE_WARNINGS

#include <compare>

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

    constexpr auto operator<=>(const Radians&) const = default;

    constexpr Radians operator-() const noexcept { return Radians(-value); }

    static const Radians HalfPi;
    static const Radians Pi;
    static const Radians TwoPi;

    float value = 0;
};

inline const Radians Radians::HalfPi = Radians(1.57079632679f);
inline const Radians Radians::Pi     = Radians(3.14159265359f);
inline const Radians Radians::TwoPi  = Radians(6.28318530718f);

inline constexpr Radians operator""_rad(unsigned long long int value) noexcept
{
    return Radians(static_cast<float>(value));
}

inline constexpr Radians operator*(float lhs, Radians rhs) noexcept
{
    return Radians(lhs * rhs.value);
}

inline constexpr Radians operator+(Radians lhs, Radians rhs) noexcept
{
    return Radians(lhs.value + rhs.value);
}

inline constexpr Radians operator-(Radians lhs, Radians rhs) noexcept
{
    return Radians(lhs.value - rhs.value);
}

struct Degrees final {
    constexpr Degrees() noexcept = default;
    constexpr explicit Degrees(float value) noexcept : value(value) {}

    Degrees operator-() noexcept { return Degrees(-value); }
    Degrees operator+=(Degrees rhs) noexcept { value += rhs.value; }
    Degrees operator-=(Degrees rhs) noexcept { value -= rhs.value; }

    float value = 0;
};

inline constexpr Degrees operator""_deg(unsigned long long int value) noexcept
{
    return Degrees(static_cast<float>(value));
}

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