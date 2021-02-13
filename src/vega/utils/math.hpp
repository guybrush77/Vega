#pragma once

#include <compare>

struct Float3 final {
    constexpr Float3() noexcept : x(0), y(0), z(0) {}
    constexpr Float3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

    union {
        float x, r;
    };
    union {
        float y, g;
    };
    union {
        float z, b;
    };
};

inline Float3 operator+(Float3 lhs, Float3 rhs) noexcept
{
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline Float3 operator*(float scalar, Float3 rhs) noexcept
{
    return { scalar * rhs.x, scalar * rhs.y, scalar * rhs.z };
}

struct AABB final {
    Float3 min;
    Float3 max;

    void Expand(const Float3& point) noexcept
    {
        if (point.x < min.x)
            min.x = point.x;
        if (point.x > max.x)
            max.x = point.x;

        if (point.y < min.y)
            min.y = point.y;
        if (point.y > max.y)
            max.y = point.y;

        if (point.z < min.z)
            min.z = point.z;
        if (point.z > max.z)
            max.z = point.z;
    }

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

inline constexpr Radians operator*(Radians lhs, float rhs) noexcept
{
    return Radians(lhs.value * rhs);
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

    Degrees  operator-() noexcept { return Degrees(-value); }
    Degrees& operator+=(Degrees rhs) noexcept
    {
        value += rhs.value;
        return *this;
    }
    Degrees& operator-=(Degrees rhs) noexcept
    {
        value -= rhs.value;
        return *this;
    }

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