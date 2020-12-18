#pragma once

#include "platform.hpp"

#include "etna/buffer.hpp"
#include "etna/descriptor.hpp"
#include "etna/device.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

struct ModelTransform final {
    glm::mat4 model;
};

struct CameraTransform final {
    glm::mat4 view;
    glm::mat4 projection;
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

    auto Set(size_t frame_index, size_t transform_index, const ModelTransform& model_transform) noexcept -> uint32_t;

    void Set(size_t frame_index, const CameraTransform& camera_transform) noexcept;

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
    };

    etna::Device               m_device;
    etna::DescriptorSetLayout  m_descriptor_set_layout;
    etna::UniqueDescriptorPool m_descriptor_pool;
    std::vector<FrameState>    m_frame_states;
    etna::DeviceSize           m_offset_multiplier;
};
