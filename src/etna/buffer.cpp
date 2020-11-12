#include "etna/buffer.hpp"

#include <cassert>
#include <vk_mem_alloc.h>

namespace etna {

void* Buffer::MapMemory()
{
    assert(m_buffer);

    void* data = nullptr;
    if (auto result = vmaMapMemory(m_allocator, m_allocation, &data); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return data;
}

void Buffer::UnmapMemory()
{
    assert(m_buffer);

    vmaUnmapMemory(m_allocator, m_allocation);
}

UniqueBuffer Buffer::Create(VmaAllocator allocator, const VkBufferCreateInfo& create_info, MemoryUsage memory_usage)
{
    VmaAllocationCreateInfo allocation_info{};

    allocation_info.usage = static_cast<VmaMemoryUsage>(memory_usage);

    VkBuffer      vk_buffer{};
    VmaAllocation allocation{};
    if (auto result = vmaCreateBuffer(allocator, &create_info, &allocation_info, &vk_buffer, &allocation, nullptr);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueBuffer(Buffer(vk_buffer, allocator, allocation));
}

void Buffer::Destroy() noexcept
{
    assert(m_buffer);

    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);

    m_buffer     = nullptr;
    m_allocator  = nullptr;
    m_allocation = nullptr;
}

} // namespace etna
