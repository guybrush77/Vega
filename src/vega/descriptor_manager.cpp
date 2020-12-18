#include "descriptor_manager.hpp"

DescriptorManager::DescriptorManager(
    etna::Device                      device,
    uint32_t                          num_frames,
    etna::DescriptorSetLayout         descriptor_set_layout,
    const etna::PhysicalDeviceLimits& gpu_limits)
    : m_device(device), m_descriptor_set_layout(descriptor_set_layout)
{
    using namespace etna;

    m_offset_multiplier = sizeof(ModelTransform);

    if (auto min_alignment = gpu_limits.minUniformBufferOffsetAlignment; min_alignment > 0) {
        m_offset_multiplier = (m_offset_multiplier + min_alignment - 1) & ~(min_alignment - 1);
    }

    m_descriptor_pool =
        device.CreateDescriptorPool({ DescriptorPoolSize{ DescriptorType::UniformBuffer, num_frames } });

    auto descriptor_sets = m_descriptor_pool->AllocateDescriptorSets(num_frames, descriptor_set_layout);

    auto model_buffers = device.CreateBuffers(
        num_frames,
        kMaxTransforms * m_offset_multiplier,
        BufferUsage::UniformBuffer,
        MemoryUsage::CpuToGpu);

    auto camera_buffers =
        device.CreateBuffers(num_frames, sizeof(CameraTransform), BufferUsage::UniformBuffer, MemoryUsage::CpuToGpu);

    auto write_descriptor_sets = std::vector<WriteDescriptorSet>();

    m_frame_states.reserve(num_frames);
    write_descriptor_sets.reserve(num_frames);

    for (size_t i = 0; i < num_frames; ++i) {
        auto model_buffer_memory  = static_cast<std::byte*>(model_buffers[i]->MapMemory());
        auto camera_buffer_memory = static_cast<std::byte*>(camera_buffers[i]->MapMemory());

        auto frame_state = FrameState{

            descriptor_sets[i],
            { std::move(model_buffers[i]), model_buffer_memory },
            { std::move(camera_buffers[i]), camera_buffer_memory }
        };

        m_frame_states.push_back(std::move(frame_state));

        write_descriptor_sets.emplace_back(descriptor_sets[i], Binding{ 0 }, DescriptorType::UniformBufferDynamic);
        write_descriptor_sets.back().AddBuffer(*m_frame_states[i].model.buffer);

        write_descriptor_sets.emplace_back(descriptor_sets[i], Binding{ 1 }, DescriptorType::UniformBuffer);
        write_descriptor_sets.back().AddBuffer(*m_frame_states[i].camera.buffer);
    }

    m_device.UpdateDescriptorSets(write_descriptor_sets);
}

DescriptorManager::~DescriptorManager() noexcept
{
    for (auto& frame_state : m_frame_states) {
        frame_state.model.buffer->UnmapMemory();
        frame_state.camera.buffer->UnmapMemory();
    }
}

uint32_t
DescriptorManager::Set(size_t frame_index, size_t transform_index, const ModelTransform& model_transform) noexcept
{
    auto& frame_state = m_frame_states[frame_index];

    auto offset = transform_index * m_offset_multiplier;

    memcpy(frame_state.model.mapped_memory + offset, &model_transform, m_offset_multiplier);

    return etna::narrow_cast<uint32_t>(offset);
}

void DescriptorManager::Set(size_t frame_index, const CameraTransform& camera_transform) noexcept
{
    auto& frame_state = m_frame_states[frame_index];

    memcpy(frame_state.camera.mapped_memory, &camera_transform, sizeof(camera_transform));
}

void DescriptorManager::UpdateDescriptorSet(size_t frame_index)
{
    using namespace etna;

    auto& frame_state = m_frame_states[frame_index];

    frame_state.model.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
    frame_state.camera.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
}
