#pragma once

#include "core.hpp"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

namespace etna {

class Buffer {
  public:
    Buffer() noexcept {}
    Buffer(std::nullptr_t) noexcept {}

    operator VkBuffer() const noexcept { return m_buffer; }

    explicit operator bool() const noexcept { return m_buffer != nullptr; }

    bool operator==(const Buffer& rhs) const noexcept { return m_buffer == rhs.m_buffer; }
    bool operator!=(const Buffer& rhs) const noexcept { return m_buffer != rhs.m_buffer; }

    auto Size() const noexcept { return m_size; }

    void* MapMemory();
    void  UnmapMemory();

    void FlushMappedMemoryRanges(std::initializer_list<MappedMemoryRange> memory_ranges);

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Buffer(VkBuffer buffer, VkDeviceSize size, VmaAllocator allocator, VmaAllocation allocation) noexcept
        : m_buffer(buffer), m_size(size), m_allocator(allocator), m_allocation(allocation)
    {}

    static auto Create(VmaAllocator allocator, const VkBufferCreateInfo& create_info, MemoryUsage memory_usage)
        -> UniqueBuffer;

    void Destroy() noexcept;

    VkBuffer      m_buffer{};
    VkDeviceSize  m_size{};
    VmaAllocator  m_allocator{};
    VmaAllocation m_allocation{};
};

} // namespace etna
