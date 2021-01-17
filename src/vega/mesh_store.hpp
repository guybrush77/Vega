#pragma once

#include "etna/buffer.hpp"
#include "etna/device.hpp"
#include "etna/queue.hpp"

#include "scene.hpp"

#include <unordered_map>

struct MeshRecord final {
    struct Vertex final {
        etna::Buffer buffer;
        uint32_t     size;
        uint32_t     count;
    } vertices;
    struct Indexfinal {
        etna::Buffer buffer;
        uint32_t     size;
        uint32_t     count;
    } indices;
};

class MeshStore {
  public:
    MeshStore(etna::Device device, etna::Queue transfer_queue) : m_device(device), m_transfer_queue(transfer_queue) {}

    MeshStore(const MeshStore&) = delete;
    MeshStore& operator=(const MeshStore&) = delete;

    MeshStore(MeshStore&&) = default;
    MeshStore& operator=(MeshStore&&) = default;

    bool Add(MeshPtr mesh);

    void Upload();

    auto GetMeshRecord(MeshPtr mesh) -> MeshRecord;

  private:
    struct MeshRecordPrivate final {
        struct Vertex final {
            etna::UniqueBuffer buffer;
            uint32_t           size;
            uint32_t           count;
        } vertices;
        struct Indexfinal {
            etna::UniqueBuffer buffer;
            uint32_t           size;
            uint32_t           count;
        } indices;
    };

    etna::Device m_device;
    etna::Queue  m_transfer_queue;

    std::unordered_map<MeshPtr, MeshRecordPrivate> m_cpu_buffers;
    std::unordered_map<MeshPtr, MeshRecordPrivate> m_gpu_buffers;
};
