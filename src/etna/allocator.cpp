#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "etna/allocator.hpp"

#include "utils/throw_exception.hpp"

etna::UniqueAllocator::UniqueAllocator(vk::Instance instance, vk::PhysicalDevice gpu, vk::Device device)
{
    VmaAllocatorCreateInfo create_info{};
    {
        create_info.physicalDevice = gpu;
        create_info.device         = device;
        create_info.instance       = instance;
    }

    etna::Allocator allocator;
    if (VK_SUCCESS != vmaCreateAllocator(&create_info, &allocator)) {
        throw_runtime_error("Failed to create Vulkan Memory Allocator");
    }

    m_allocator = allocator;
}

etna::UniqueAllocator::~UniqueAllocator() noexcept
{
    vmaDestroyAllocator(m_allocator);
}

etna::UniqueAllocator::UniqueAllocator(UniqueAllocator&& rhs) noexcept
{
    std::swap(m_allocator, rhs.m_allocator);
}

etna::UniqueAllocator& etna::UniqueAllocator::operator=(etna::UniqueAllocator&& rhs) noexcept
{
    m_allocator = std::exchange(rhs.m_allocator, nullptr);
    return *this;
}

etna::Allocator etna::UniqueAllocator::get() const noexcept
{
    assert(m_allocator);
    return m_allocator;
}

etna::Allocator etna::UniqueAllocator::operator*() const noexcept
{
    return get();
}
