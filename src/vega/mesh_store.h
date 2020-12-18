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
    MeshStore(etna::Device device) : m_device(device) {}

    MeshStore(const MeshStore&) = delete;
    MeshStore& operator=(const MeshStore&) = delete;

    MeshStore(MeshStore&&) = default;
    MeshStore& operator=(MeshStore&&) = default;

    bool Add(MeshPtr mesh);

    void Upload(etna::Queue transfer_queue, uint32_t transfer_queue_family_index);

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

    std::unordered_map<MeshPtr, MeshRecordPrivate> m_cpu_buffers;
    std::unordered_map<MeshPtr, MeshRecordPrivate> m_gpu_buffers;
};
