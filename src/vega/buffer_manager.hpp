#pragma once

#include "etna/buffer.hpp"
#include "etna/device.hpp"
#include "etna/queue.hpp"

#include "scene.hpp"

class BufferManager {
  public:
    BufferManager(etna::Device device, etna::Queue transfer_queue) : m_device(device), m_transfer_queue(transfer_queue)
    {}

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    BufferManager(BufferManager&&) = default;
    BufferManager& operator=(BufferManager&&) = default;

    void CreateBuffer(VertexBufferPtr vertex_buffer);
    void CreateBuffer(IndexBufferPtr index_buffer);

    auto GetBuffer(VertexBufferPtr vertex_buffer) const noexcept -> etna::Buffer;
    auto GetBuffer(IndexBufferPtr index_buffer) const noexcept -> etna::Buffer;

    void Upload();

  private:
    struct Record final {
        void*              ptr{};
        etna::BufferUsage  usage{};
        etna::UniqueBuffer cpu_buffer{};
        etna::UniqueBuffer gpu_buffer{};
    };

    etna::Device m_device;
    etna::Queue  m_transfer_queue;

    std::vector<Record> m_records;
};
