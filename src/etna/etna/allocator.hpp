#pragma once

#include <vulkan/vulkan.hpp>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

namespace etna {

using Allocator = VmaAllocator;

enum class MemoryUsage { eGpuOnly, eCpuOnly, eCpuToGpu, eGpuToCpu, eCpuCopy, eGpuLazilyAllocated };

class UniqueAllocator final {
  public:
    UniqueAllocator() noexcept = default;
    UniqueAllocator(vk::Instance instance, vk::PhysicalDevice gpu, vk::Device device);
    ~UniqueAllocator() noexcept;

    UniqueAllocator(const UniqueAllocator&) = delete;
    UniqueAllocator& operator=(const UniqueAllocator&) = delete;

    UniqueAllocator(UniqueAllocator&& rhs) noexcept;
    UniqueAllocator& operator=(UniqueAllocator&& rhs) noexcept;

    Allocator get() const noexcept;
    Allocator operator*() const noexcept;

  private:
    Allocator m_allocator = nullptr;
};

} // namespace etna
