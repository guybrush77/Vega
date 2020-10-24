#pragma once

#include "core.hpp"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)

namespace etna {

class Image2D {
  public:
    Image2D() noexcept {}
    Image2D(std::nullptr_t) noexcept {}

    operator VkImage() const noexcept { return m_image; }

    bool operator==(const Image2D&) const = default;

    auto Format() const noexcept { return static_cast<etna::Format>(m_format); }

    void* MapMemory();
    void  UnmapMemory();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Image2D(VkImage image, VmaAllocator allocator, VmaAllocation allocation, VkFormat format) noexcept
        : m_image(image), m_allocator(allocator), m_allocation(allocation), m_format(format)
    {}

    static auto Create(VmaAllocator allocator, const VkImageCreateInfo& create_info, MemoryUsage memory_usage)
        -> UniqueImage2D;

    void Destroy() noexcept;

    VkImage       m_image{};
    VmaAllocator  m_allocator{};
    VmaAllocation m_allocation{};
    VkFormat      m_format{};
};

class ImageView2D {
  public:
    ImageView2D() noexcept {}
    ImageView2D(std::nullptr_t) noexcept {}

    operator VkImageView() const noexcept { return m_image_view; }

    bool operator==(const ImageView2D&) const = default;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    ImageView2D(VkImageView image_view, VkDevice device) noexcept : m_image_view(image_view), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkImageViewCreateInfo& create_info) -> UniqueImageView2D;

    void Destroy() noexcept;

    VkImageView m_image_view{};
    VkDevice    m_device{};
};

class Framebuffer {
  public:
    Framebuffer() noexcept {}
    Framebuffer(std::nullptr_t) noexcept {}

    operator VkFramebuffer() const noexcept { return m_framebuffer; }

    bool operator==(const Framebuffer&) const = default;

    auto RenderPass() const noexcept -> RenderPass;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Framebuffer(VkFramebuffer framebuffer, VkDevice device, VkRenderPass renderpass) noexcept
        : m_framebuffer(framebuffer), m_device(device), m_renderpass(renderpass)
    {}

    static auto Create(VkDevice device, const VkFramebufferCreateInfo& create_info) -> UniqueFramebuffer;

    void Destroy() noexcept;

    VkFramebuffer m_framebuffer{};
    VkDevice      m_device{};
    VkRenderPass  m_renderpass{};
};

} // namespace etna
