#include "etna/buffer.hpp"

#include "utils/throw_exception.hpp"

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
    assert(m_state);
    return m_state->buffer;
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

} // namespace etna
