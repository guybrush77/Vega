#include "mesh_store.h"

#include "etna/command.hpp"

bool MeshStore::Add(MeshPtr mesh)
{
    using namespace etna;

    if (m_cpu_buffers.count(mesh)) {
        return false;
    }

    auto vertices_size  = mesh->GetVertices().GetSize();
    auto vertices_data  = mesh->GetVertices().GetData();
    auto vertices_count = mesh->GetVertices().GetCount();

    auto indices_size  = mesh->GetIndices().GetSize();
    auto indices_data  = mesh->GetIndices().GetData();
    auto indices_count = mesh->GetIndices().GetCount();

    auto cpu_vertex_buffer = m_device.CreateBuffer(vertices_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);
    auto cpu_index_buffer  = m_device.CreateBuffer(indices_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto vertex_buffer_data = cpu_vertex_buffer->MapMemory();
    memcpy(vertex_buffer_data, vertices_data, vertices_size);
    cpu_vertex_buffer->UnmapMemory();

    auto index_buffer_data = cpu_index_buffer->MapMemory();
    memcpy(index_buffer_data, indices_data, indices_size);
    cpu_index_buffer->UnmapMemory();

    auto cpu_buffers = MeshRecordPrivate{ { std::move(cpu_vertex_buffer), vertices_size, vertices_count },
                                          { std::move(cpu_index_buffer), indices_size, indices_count } };

    auto rv = m_cpu_buffers.insert({ mesh, std::move(cpu_buffers) });

    return rv.second;
}

void MeshStore::Upload(etna::Queue transfer_queue, uint32_t transfer_queue_family_index)
{
    using namespace etna;

    auto cmd_pool   = m_device.CreateCommandPool(transfer_queue_family_index, CommandPoolCreate::Transient);
    auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

    cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

    for (const auto& [mesh_id, cpu_mesh] : m_cpu_buffers) {
        auto vertices_size  = cpu_mesh.vertices.size;
        auto vertices_count = cpu_mesh.vertices.count;
        auto indices_size   = cpu_mesh.indices.size;
        auto indices_count  = cpu_mesh.indices.count;

        auto gpu_vertex_buffer = m_device.CreateBuffer(
            vertices_size,
            BufferUsage::VertexBuffer | BufferUsage::TransferDst,
            MemoryUsage::GpuOnly);

        auto gpu_index_buffer = m_device.CreateBuffer(
            indices_size,
            BufferUsage::IndexBuffer | BufferUsage::TransferDst,
            MemoryUsage::GpuOnly);

        auto gpu_buffers = MeshRecordPrivate{ { std::move(gpu_vertex_buffer), vertices_size, vertices_count },
                                              { std::move(gpu_index_buffer), indices_size, indices_count } };

        auto [it, success] = m_gpu_buffers.insert({ mesh_id, std::move(gpu_buffers) });

        const auto& gpu_mesh = it->second;

        cmd_buffer->CopyBuffer(*cpu_mesh.vertices.buffer, *gpu_mesh.vertices.buffer, vertices_size);
        cmd_buffer->CopyBuffer(*cpu_mesh.indices.buffer, *gpu_mesh.indices.buffer, indices_size);
    }

    cmd_buffer->End();

    transfer_queue.Submit(*cmd_buffer);

    m_device.WaitIdle();

    m_cpu_buffers.clear();
}

MeshRecord MeshStore::GetMeshRecord(MeshPtr mesh)
{
    if (auto it = m_gpu_buffers.find(mesh); it != m_gpu_buffers.end()) {
        const auto& buffers = it->second;
        return MeshRecord{ { *buffers.vertices.buffer, buffers.vertices.size, buffers.vertices.count },
                           { *buffers.indices.buffer, buffers.indices.size, buffers.indices.count } };
    }
    return MeshRecord{};
}
