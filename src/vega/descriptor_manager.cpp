#include "descriptor_manager.hpp"

#include "utils/misc.hpp"
#include <cstring>
#include <spdlog/spdlog.h>

DescriptorManager::DescriptorManager(
    etna::Device                      device,
    uint32_t                          num_frames,
    etna::DescriptorSetLayout         transforms_set_layout,
    etna::DescriptorSetLayout         textures_set_layout,
    const etna::PhysicalDeviceLimits& gpu_limits)
    : m_device(device), m_transforms_set_layout(transforms_set_layout), m_textures_set_layout(textures_set_layout)
{
    using namespace etna;

    m_offset_multiplier = sizeof(ModelUniform);

    if (auto min_alignment = gpu_limits.minUniformBufferOffsetAlignment; min_alignment > 0) {
        m_offset_multiplier = (m_offset_multiplier + min_alignment - 1) & ~(min_alignment - 1);
    }

    m_descriptor_pool = device.CreateDescriptorPool({

        DescriptorPoolSize{ DescriptorType::CombinedImageSampler, 128 },
        DescriptorPoolSize{ DescriptorType::UniformBuffer, num_frames },
        DescriptorPoolSize{ DescriptorType::UniformBufferDynamic, num_frames } });

    auto transforms_set = m_descriptor_pool->AllocateDescriptorSets(num_frames, m_transforms_set_layout);

    auto model_buffers = device.CreateBuffers(
        num_frames,
        kMaxTransforms * m_offset_multiplier,
        BufferUsage::UniformBuffer,
        MemoryUsage::CpuToGpu);

    auto camera_buffers =
        device.CreateBuffers(num_frames, sizeof(CameraUniform), BufferUsage::UniformBuffer, MemoryUsage::CpuToGpu);

    auto lights_buffers =
        device.CreateBuffers(num_frames, sizeof(LightsUniform), BufferUsage::UniformBuffer, MemoryUsage::CpuToGpu);

    auto write_descriptor_sets = std::vector<WriteDescriptorSet>();

    m_frame_states.reserve(num_frames);
    write_descriptor_sets.reserve(num_frames);

    for (size_t i = 0; i < num_frames; ++i) {
        auto model_buffer_memory  = static_cast<std::byte*>(model_buffers[i]->MapMemory());
        auto camera_buffer_memory = static_cast<std::byte*>(camera_buffers[i]->MapMemory());
        auto lights_buffer_memory = static_cast<std::byte*>(lights_buffers[i]->MapMemory());

        auto frame_state = FrameState{

            transforms_set[i],
            { std::move(model_buffers[i]), model_buffer_memory },
            { std::move(camera_buffers[i]), camera_buffer_memory },
            { std::move(lights_buffers[i]), lights_buffer_memory }
        };

        m_frame_states.push_back(std::move(frame_state));

        write_descriptor_sets.emplace_back(transforms_set[i], Binding{ 0 }, DescriptorType::UniformBufferDynamic);
        write_descriptor_sets.back().AddBuffer(*m_frame_states[i].model.buffer);

        write_descriptor_sets.emplace_back(transforms_set[i], Binding{ 1 }, DescriptorType::UniformBuffer);
        write_descriptor_sets.back().AddBuffer(*m_frame_states[i].camera.buffer);

        write_descriptor_sets.emplace_back(transforms_set[i], Binding{ 2 }, DescriptorType::UniformBuffer);
        write_descriptor_sets.back().AddBuffer(*m_frame_states[i].lights.buffer);
    }

    m_device.UpdateDescriptorSets(write_descriptor_sets);

    auto builder = Sampler::Builder(Filter::Nearest, Filter::Nearest, SamplerMipmapMode::Nearest);

    m_sampler = m_device.CreateSampler(builder.state);
}

DescriptorManager::~DescriptorManager() noexcept
{
    for (auto& frame_state : m_frame_states) {
        frame_state.model.buffer->UnmapMemory();
        frame_state.camera.buffer->UnmapMemory();
        frame_state.lights.buffer->UnmapMemory();
    }
}

etna::DescriptorSet DescriptorManager::GetTextureSet(etna::ImageView2D image_view) const noexcept
{
    if (auto it = m_textures.find(image_view); it != m_textures.end()) {
        return it->second;
    }
    return etna::DescriptorSet{};
}

uint32_t DescriptorManager::Set(size_t frame_index, size_t transform_index, const ModelUniform& model)
{
    utils::throw_runtime_error_if(transform_index >= kMaxTransforms, "Transform index is greater than MaxTransforms");

    auto& frame_state = m_frame_states[frame_index];

    auto offset = transform_index * m_offset_multiplier;

    std::memcpy(frame_state.model.mapped_memory + offset, &model, sizeof(model));

    return etna::narrow_cast<uint32_t>(offset);
}

void DescriptorManager::Set(size_t frame_index, const CameraUniform& camera) noexcept
{
    auto& frame_state = m_frame_states[frame_index];

    std::memcpy(frame_state.camera.mapped_memory, &camera, sizeof(camera));
}

void DescriptorManager::Set(size_t frame_index, const LightsUniform& lights) noexcept
{
    auto& frame_state = m_frame_states[frame_index];

    std::memcpy(frame_state.lights.mapped_memory, &lights, sizeof(lights));
}

void DescriptorManager::Set(etna::ImageView2D image_view) noexcept
{
    if (auto [it, emplaced] = m_textures.try_emplace(image_view, etna::DescriptorSet{}); emplaced) {
        it->second = m_descriptor_pool->AllocateDescriptorSets(1, m_textures_set_layout).front();

        auto descriptor_type      = etna::DescriptorType::CombinedImageSampler;
        auto write_descriptor_set = etna::WriteDescriptorSet(it->second, etna::Binding{ 10 }, descriptor_type);

        write_descriptor_set.AddImage(*m_sampler, image_view, etna::ImageLayout::ShaderReadOnlyOptimal);

        m_device.UpdateDescriptorSets({ write_descriptor_set });
    }
}

void DescriptorManager::Flush(size_t frame_index)
{
    using namespace etna;

    auto& frame_state = m_frame_states[frame_index];

    frame_state.model.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
    frame_state.camera.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
    frame_state.lights.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
}
