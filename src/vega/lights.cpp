#include "lights.hpp"

#include <cmath>

Float3 Light::ComputePremultipliedColor() const noexcept
{
    return m_multiplier * m_color;
}

Float3 Light::ComputeDir() const noexcept
{
    float x = std::cos(m_elevation.value) * std::sin(m_azimuth.value);
    float y = std::sin(m_elevation.value);
    float z = std::cos(m_elevation.value) * std::cos(m_azimuth.value);

    return { x, y, z };
}
