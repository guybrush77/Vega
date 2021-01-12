#pragma once

#include "lights.hpp"
#include "platform.hpp"

#include "etna/buffer.hpp"
#include "etna/descriptor.hpp"
#include "etna/device.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

struct ModelUniform final {
    glm::mat4 model;
};

struct CameraUniform final {
    glm::mat4 view;
    glm::mat4 projection;
};

struct LightDescription final {
    glm::vec4 color;
    glm::vec4 dir;
};

struct LightsUniform final {
    LightDescription key;
    LightDescription fill;
};

class DescriptorManager {
  public:
    DescriptorManager() noexcept = default;

    DescriptorManager(const DescriptorManager&) = delete;
    DescriptorManager& operator=(const DescriptorManager&) = delete;

    DescriptorManager(DescriptorManager&&) = default;
    DescriptorManager& operator=(DescriptorManager&&) = default;

    DescriptorManager(
        etna::Device                      device,
        uint32_t                          num_frames,
        etna::DescriptorSetLayout         descriptor_set_layout,
        const etna::PhysicalDeviceLimits& gpu_limits);

    ~DescriptorManager() noexcept;

    auto DescriptorSet(size_t frame_index) const noexcept { return m_frame_states[frame_index].descriptor_set; }
    auto DescriptorSetLayout() const noexcept { return m_descriptor_set_layout; }

    auto Set(size_t frame_index, size_t transform_index, const ModelUniform& model) noexcept -> uint32_t;

    void Set(size_t frame_index, const CameraUniform& camera) noexcept;

    void Set(size_t frame_index, const LightsUniform& lights) noexcept;

    void UpdateDescriptorSet(size_t frame_index);

  private:
    static constexpr int kMaxTransforms = 64;

    struct FrameState final {
        etna::DescriptorSet descriptor_set;

        struct Model final {
            etna::UniqueBuffer buffer{};
            std::byte*         mapped_memory{};
        } model;

        struct Camera final {
            etna::UniqueBuffer buffer{};
            std::byte*         mapped_memory{};
        } camera;

        struct Lights final {
            etna::UniqueBuffer buffer{};
            std::byte*         mapped_memory{};
        } lights;
    };

    etna::Device               m_device;
    etna::DescriptorSetLayout  m_descriptor_set_layout;
    etna::UniqueDescriptorPool m_descriptor_pool;
    std::vector<FrameState>    m_frame_states;
    etna::DeviceSize           m_offset_multiplier;
};
