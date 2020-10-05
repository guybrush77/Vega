#pragma once

#include "renderpass.hpp"

#include <span>

VK_DEFINE_HANDLE(VmaAllocator)

ETNA_DEFINE_HANDLE(EtnaImage2D)
ETNA_DEFINE_HANDLE(EtnaImageView2D)
ETNA_DEFINE_HANDLE(EtnaFramebuffer)

namespace etna {

class Image2D;
using UniqueImage2D = UniqueHandle<Image2D>;

class Image2D {
  public:
    Image2D() noexcept {}
    Image2D(std::nullptr_t) noexcept {}

    operator VkImage() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Image2D& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Image2D& rhs) const noexcept { return m_state != rhs.m_state; }

    auto UsageMask() const noexcept -> ImageUsageMask;
    auto Format() const noexcept -> Format;
    auto Extent() const noexcept -> Extent2D;
    auto Rect2D() const noexcept -> Rect2D;

    void* MapMemory();
    void  UnmapMemory();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    operator EtnaImage2D() const noexcept { return m_state; }

    Image2D(EtnaImage2D image) : m_state(image) {}

    static auto Create(VmaAllocator allocator, const VkImageCreateInfo& create_info, MemoryUsage memory_usage)
        -> UniqueImage2D;

    void Destroy() noexcept;

    EtnaImage2D m_state = nullptr;
};

class ImageView2D;
using UniqueImageView2D = UniqueHandle<ImageView2D>;

class ImageView2D {
  public:
    ImageView2D() noexcept {}
    ImageView2D(std::nullptr_t) noexcept {}

    operator VkImageView() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const ImageView2D& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const ImageView2D& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    operator EtnaImageView2D() const noexcept { return m_state; }

    ImageView2D(EtnaImageView2D image_view) : m_state(image_view) {}

    static auto Create(VkDevice device, const VkImageViewCreateInfo& create_info) -> UniqueImageView2D;

    void Destroy() noexcept;

    EtnaImageView2D m_state = nullptr;
};

class Framebuffer;
using UniqueFramebuffer = UniqueHandle<Framebuffer>;

class Framebuffer {
  public:
    Framebuffer() noexcept {}
    Framebuffer(std::nullptr_t) noexcept {}

    operator VkFramebuffer() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Framebuffer& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Framebuffer& rhs) const noexcept { return m_state != rhs.m_state; }

    Extent2D Extent2D() const noexcept;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;
    friend class CommandBuffer;

    operator EtnaFramebuffer() const noexcept { return m_state; }

    Framebuffer(EtnaFramebuffer framebuffer) : m_state(framebuffer) {}

    VkRenderPass RenderPass() const noexcept;

    static auto Create(VkDevice device, const VkFramebufferCreateInfo& create_info) -> UniqueFramebuffer;

    void Destroy() noexcept;

    EtnaFramebuffer m_state = nullptr;
};

} // namespace etna
