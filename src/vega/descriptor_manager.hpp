#pragma once

#include "lights.hpp"
#include "platform.hpp"

#include "etna/buffer.hpp"
#include "etna/descriptor.hpp"
#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/sampler.hpp"

BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

#include <map>

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
        etna::DescriptorSetLayout         transforms_set_layout,
        etna::DescriptorSetLayout         textures_set_layout,
        const etna::PhysicalDeviceLimits& gpu_limits);

    ~DescriptorManager() noexcept;

    auto GetTransformsSet(size_t frame_index) const noexcept { return m_frame_states[frame_index].transforms_set; }

    auto GetTextureSet(etna::ImageView2D image_view) const noexcept -> etna::DescriptorSet;

    auto Set(size_t frame_index, size_t transform_index, const ModelUniform& model) -> uint32_t;

    void Set(size_t frame_index, const CameraUniform& camera) noexcept;

    void Set(size_t frame_index, const LightsUniform& lights) noexcept;

    void Set(etna::ImageView2D image_view) noexcept;

    void Flush(size_t frame_index);

  private:
    static constexpr int kMaxTransforms = 128;

    struct FrameState final {
        etna::DescriptorSet transforms_set;

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

    using TextureMap = std::map<etna::ImageView2D, etna::DescriptorSet>;

    etna::Device               m_device;
    etna::DescriptorSetLayout  m_transforms_set_layout;
    etna::DescriptorSetLayout  m_textures_set_layout;
    etna::UniqueDescriptorPool m_descriptor_pool;
    etna::UniqueSampler        m_sampler;
    std::vector<FrameState>    m_frame_states;
    etna::DeviceSize           m_offset_multiplier;
    TextureMap                 m_textures;
};
