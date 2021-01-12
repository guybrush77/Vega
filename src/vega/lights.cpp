#include "lights.hpp"

#include <cmath>

Float3 Light::ComputePremultipliedColor() const noexcept
{
    return m_multiplier * m_color;
}

Float3 Light::ComputeDir() const noexcept
{
    float x = std::cosf(m_elevation.value) * std::sinf(m_azimuth.value);
    float y = std::sinf(m_elevation.value);
    float z = std::cosf(m_elevation.value) * std::cosf(m_azimuth.value);

    return { x, y, z };
}
