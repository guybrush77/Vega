#pragma once

#include "utils/math.hpp"

class Light final {
  public:
    auto GetColor() const noexcept { return m_color; }
    auto GetMultiplier() const noexcept { return m_multiplier; }
    auto GetElevation() const noexcept { return m_elevation; }
    auto GetAzimuth() const noexcept { return m_azimuth; }

    auto& ColorRef() noexcept { return m_color.x; }
    auto& MultiplierRef() noexcept { return m_multiplier; }
    auto& ElevationRef() noexcept { return m_elevation.value; }
    auto& AzimuthRef() noexcept { return m_azimuth.value; }

    auto ComputePremultipliedColor() const noexcept -> Float3;
    auto ComputeDir() const noexcept -> Float3;

  private:
    Float3  m_color{ 1.0f, 1.0f, 1.0f };
    float   m_multiplier{ 1.0f };
    Radians m_elevation{ 0_rad };
    Radians m_azimuth{ 0_rad };
};

using LightRef = Light&;

class Lights final {
  public:
    LightRef KeyRef() noexcept { return m_key; }
    LightRef FillRef() noexcept { return m_fill; }

  private:
    Light m_key{};
    Light m_fill{};
};
