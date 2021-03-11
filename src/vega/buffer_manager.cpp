#include "buffer_manager.hpp"

#include "etna/command.hpp"

void BufferManager::CreateBuffer(VertexBufferPtr vertex_buffer)
{
    using namespace etna;

    assert(vertex_buffer);

    auto buffer = m_device.CreateBuffer(vertex_buffer->Size(), BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto buffer_data = buffer->MapMemory();
    memcpy(buffer_data, vertex_buffer->Data(), vertex_buffer->Size());
    buffer->UnmapMemory();

    m_records.push_back({ vertex_buffer, BufferUsage::VertexBuffer, std::move(buffer), {} });
}

void BufferManager::CreateBuffer(IndexBufferPtr index_buffer)
{
    using namespace etna;

    assert(index_buffer);

    auto buffer = m_device.CreateBuffer(index_buffer->Size(), BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto buffer_data = buffer->MapMemory();
    memcpy(buffer_data, index_buffer->Data(), index_buffer->Size());
    buffer->UnmapMemory();

    m_records.push_back({ index_buffer, BufferUsage::IndexBuffer, std::move(buffer), {} });
}

etna::Buffer BufferManager::GetBuffer(VertexBufferPtr vertex_buffer) const noexcept
{
    if (auto it = std::ranges::find(m_records, vertex_buffer, &Record::ptr); it != m_records.end()) {
        return *it->gpu_buffer;
    }
    return {};
}

etna::Buffer BufferManager::GetBuffer(IndexBufferPtr index_buffer) const noexcept
{
    if (auto it = std::ranges::find(m_records, index_buffer, &Record::ptr); it != m_records.end()) {
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

    for (auto& [id, usage, cpu_buffer, gpu_buffer] : m_records) {
        if (gpu_buffer) {
            continue;
        }
        gpu_buffer = m_device.CreateBuffer(cpu_buffer->Size(), usage | BufferUsage::TransferDst, MemoryUsage::GpuOnly);
        cmd_buffer->CopyBuffer(*cpu_buffer, *gpu_buffer, cpu_buffer->Size());
    }

    cmd_buffer->End();

    m_transfer_queue.Submit(*cmd_buffer);

    m_device.WaitIdle(); // TODO

    // TODO: Clear CPU buffers
}
