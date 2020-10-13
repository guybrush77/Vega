#pragma once

#include "types.hpp"

VK_DEFINE_HANDLE(VmaAllocator)

ETNA_DEFINE_HANDLE(EtnaBuffer)

namespace etna {

class Buffer {
  public:
    Buffer() noexcept {}
    Buffer(std::nullptr_t) noexcept {}

    operator VkBuffer() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Buffer& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Buffer& rhs) const noexcept { return m_state != rhs.m_state; }

    void* MapMemory();
    void  UnmapMemory();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    operator EtnaBuffer() const noexcept { return m_state; }

    Buffer(EtnaBuffer buffer) : m_state(buffer) {}

    static auto Create(VmaAllocator allocator, const VkBufferCreateInfo& create_info, MemoryUsage memory_usage)
        -> UniqueBuffer;

    void Destroy() noexcept;

    EtnaBuffer m_state{};
};

} // namespace etna
