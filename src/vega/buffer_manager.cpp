#include "buffer_manager.hpp"

#include "etna/command.hpp"

BufferManager::BufferManager(etna::Device device, etna::Queue transfer_queue)
    : m_device(device), m_transfer_queue(transfer_queue)
{
    using namespace etna;

    auto command_pool_flags = CommandPoolCreate::Transient | CommandPoolCreate::ResetCommandBuffer;

    m_command_pool   = m_device.CreateCommandPool(m_transfer_queue.FamilyIndex(), command_pool_flags);
    m_command_buffer = m_command_pool->AllocateCommandBuffer();
}

void BufferManager::CreateBuffer(BufferPtr buffer, etna::BufferUsage buffer_usage)
{
    using namespace etna;

    assert(buffer);

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

void BufferManager::UploadAsync()
{
    using namespace etna;

    auto is_set = [](auto& buffer) { return buffer.get(); };

    if (auto count = std::ranges::count_if(m_records, is_set, &Record::host_buffer); count == 0) {
        return;
    }

    m_command_buffer->ResetCommandBuffer(CommandBufferReset::ReleaseResources);

    m_command_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

    for (auto& [id, usage, host_buffer, gpu_buffer] : m_records) {
        if (gpu_buffer) {
            continue;
        }
        gpu_buffer = m_device.CreateBuffer(host_buffer->Size(), usage | BufferUsage::TransferDst, MemoryUsage::GpuOnly);
        m_command_buffer->CopyBuffer(*host_buffer, *gpu_buffer, host_buffer->Size());
    }

    m_command_buffer->End();

    m_transfer_queue.Submit(*m_command_buffer);
}

void BufferManager::CleanAfterUpload()
{
    for (auto& record : m_records) {
        record.host_buffer.reset();
    }
}
