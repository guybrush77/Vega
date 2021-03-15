#include "buffer_manager.hpp"

#include "etna/command.hpp"

void BufferManager::CreateBuffer(BufferPtr buffer, etna::BufferUsage buffer_usage)
{
    using namespace etna;

    assert(vertex_buffer);

    if (std::ranges::find(m_records, buffer->GetID(), &Record::id) != m_records.end()) {
        return;
    }

    auto host_buffer = m_device.CreateBuffer(buffer->Size(), BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto buffer_data = host_buffer->MapMemory();
    memcpy(buffer_data, buffer->Data(), buffer->Size());
    host_buffer->UnmapMemory();

    m_records.push_back({ buffer->GetID(), buffer_usage, std::move(host_buffer), {} });
}

etna::Buffer BufferManager::GetBuffer(BufferPtr buffer) const noexcept
{
    if (auto it = std::ranges::find(m_records, buffer->GetID(), &Record::id); it != m_records.end()) {
        return *it->gpu_buffer;
    }
    return {};
}

void BufferManager::Upload()
{
    using namespace etna;

    auto cmd_pool   = m_device.CreateCommandPool(m_transfer_queue.FamilyIndex(), CommandPoolCreate::Transient);
    auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

    cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

    for (auto& [id, usage, host_buffer, gpu_buffer] : m_records) {
        if (gpu_buffer) {
            continue;
        }
        gpu_buffer = m_device.CreateBuffer(host_buffer->Size(), usage | BufferUsage::TransferDst, MemoryUsage::GpuOnly);
        cmd_buffer->CopyBuffer(*host_buffer, *gpu_buffer, host_buffer->Size());
    }

    cmd_buffer->End();

    m_transfer_queue.Submit(*cmd_buffer);

    m_device.WaitIdle(); // TODO

    // TODO: Clear CPU buffers
}
