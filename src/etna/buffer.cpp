#include "etna/buffer.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaBuffer_T final {
    VkBuffer      buffer;
    VmaAllocator  allocator;
    VmaAllocation allocation;
};

} // namespace

namespace etna {

Buffer::operator VkBuffer() const noexcept
{
    return m_state ? m_state->buffer : VkBuffer{};
}

void* Buffer::MapMemory()
{
    assert(m_state);

    void* data = nullptr;
    if (VK_SUCCESS != vmaMapMemory(m_state->allocator, m_state->allocation, &data)) {
        throw_runtime_error(COMPONENT "Function vmaMapMemory failed");
    }

    return data;
}

void Buffer::UnmapMemory()
{
    assert(m_state);

    vmaUnmapMemory(m_state->allocator, m_state->allocation);
}

UniqueBuffer Buffer::Create(VmaAllocator allocator, const VkBufferCreateInfo& create_info, MemoryUsage memory_usage)
{
    VmaAllocationCreateInfo allocation_info{};

    allocation_info.usage = static_cast<VmaMemoryUsage>(memory_usage);

    VkBuffer      buffer{};
    VmaAllocation allocation{};
    if (VK_SUCCESS != vmaCreateBuffer(allocator, &create_info, &allocation_info, &buffer, &allocation, nullptr)) {
        throw_runtime_error("vmaCreateBuffer failed");
    }

    spdlog::info(COMPONENT "Created VkBuffer {}", fmt::ptr(buffer));

    return UniqueBuffer(new EtnaBuffer_T{ buffer, allocator, allocation });
}

void Buffer::Destroy() noexcept
{
    assert(m_state);

    vmaDestroyBuffer(m_state->allocator, m_state->buffer, m_state->allocation);

    spdlog::info(COMPONENT "Destroyed VkBuffer {}", fmt::ptr(m_state->buffer));

    delete m_state;

    m_state = nullptr;
}

} // namespace etna
