#include "buffer.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#define COMPONENT "Etna: "

namespace etna {

void* Buffer::MapMemory()
{
    assert(m_buffer);

    void* data = nullptr;
    if (VK_SUCCESS != vmaMapMemory(m_allocator, m_allocation, &data)) {
        throw_runtime_error(COMPONENT "Function vmaMapMemory failed");
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
    if (VK_SUCCESS != vmaCreateBuffer(allocator, &create_info, &allocation_info, &vk_buffer, &allocation, nullptr)) {
        throw_runtime_error("vmaCreateBuffer failed");
    }

    spdlog::info(COMPONENT "Created VkBuffer {}", fmt::ptr(vk_buffer));

    return UniqueBuffer(Buffer(vk_buffer, allocator, allocation));
}

void Buffer::Destroy() noexcept
{
    assert(m_buffer);

    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);

    spdlog::info(COMPONENT "Destroyed VkBuffer {}", fmt::ptr(m_buffer));

    m_buffer     = nullptr;
    m_allocator  = nullptr;
    m_allocation = nullptr;
}

} // namespace etna
