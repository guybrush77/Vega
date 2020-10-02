#include "etna/image.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#define COMPONENT "Etna: "

struct EtnaImage2D_T final {
    VkImage           image;
    VkFormat          format;
    uint32_t          width;
    uint32_t          height;
    VkImageUsageFlags usage;
    VmaAllocator      allocator;
    VmaAllocation     allocation;
};

namespace etna {

Image2D::operator vk::Image() const noexcept
{
    assert(m_image);
    return m_image->image;
}

vk::ImageUsageFlags Image2D::UsageFlags() const noexcept
{
    assert(m_image);
    return static_cast<vk::ImageUsageFlags>(m_image->usage);
}

vk::Format Image2D::Format() const noexcept
{
    assert(m_image);
    return static_cast<vk::Format>(m_image->format);
}

vk::Extent2D Image2D::Extent() const noexcept
{
    assert(m_image);
    return vk::Extent2D(m_image->width, m_image->height);
}

vk::Viewport Image2D::Viewport() const noexcept
{
    assert(m_image);
    return vk::Viewport(0, 0, narrow_cast<float>(m_image->width), narrow_cast<float>(m_image->height), 0, 1);
}

vk::Rect2D Image2D::Rect2D() const noexcept
{
    assert(m_image);
    return vk::Rect2D({ 0, 0 }, { m_image->width, m_image->height });
}

void Image2D::destroy(EtnaImage2D image)
{
    vmaDestroyImage(image->allocator, image->image, image->allocation);
    delete image;
}

etna::UniqueImage2D CreateUniqueImage2D(
    etna::Allocator     allocator,
    vk::Format          format,
    vk::Extent2D        extent,
    vk::ImageUsageFlags usage,
    etna::MemoryUsage   memory_usage,
    vk::ImageTiling     tiling)
{
    const VkFormat          vk_format = static_cast<VkFormat>(format);
    const VkImageUsageFlags vk_usage  = static_cast<VkImageUsageFlags>(usage);

    VkImageCreateInfo create_info{};
    {
        create_info.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.imageType   = VK_IMAGE_TYPE_2D;
        create_info.format      = vk_format;
        create_info.extent      = { extent.width, extent.height, 1 };
        create_info.mipLevels   = 1;
        create_info.arrayLayers = 1;
        create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
        create_info.tiling      = static_cast<VkImageTiling>(tiling);
        create_info.usage       = static_cast<VkImageUsageFlags>(usage);
    }

    VmaAllocationCreateInfo allocation_info{};
    switch (memory_usage) {
    case etna::MemoryUsage::eGpuOnly:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case etna::MemoryUsage::eCpuOnly:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        break;
    case etna::MemoryUsage::eCpuToGpu:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case etna::MemoryUsage::eGpuToCpu:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        break;
    case etna::MemoryUsage::eCpuCopy:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_COPY;
        break;
    case etna::MemoryUsage::eGpuLazilyAllocated:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        break;
    default:
        allocation_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    };

    VkImage       vk_image;
    VmaAllocation vk_allocation;
    if (VK_SUCCESS != vmaCreateImage(allocator, &create_info, &allocation_info, &vk_image, &vk_allocation, nullptr)) {
        throw_runtime_error("vmaCreateImage failed");
    }

    return vk::UniqueHandle<etna::Image2D, vk::DispatchLoaderStatic>(
        new EtnaImage2D_T{ vk_image, vk_format, extent.width, extent.height, vk_usage, allocator, vk_allocation });
}

vk::UniqueImageView CreateUniqueImageView(vk::Device device, etna::Image2D image)
{
    vk::ImageAspectFlags aspect_flags;

    if (image.UsageFlags() & vk::ImageUsageFlagBits::eColorAttachment) {
        aspect_flags = vk::ImageAspectFlagBits::eColor;
    }

    vk::ImageViewCreateInfo create_info;
    {
        create_info.image                           = image;
        create_info.viewType                        = vk::ImageViewType::e2D;
        create_info.format                          = image.Format();
        create_info.subresourceRange.aspectMask     = aspect_flags;
        create_info.subresourceRange.baseMipLevel   = 0;
        create_info.subresourceRange.levelCount     = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount     = 1;
    }

    auto image_view = device.createImageViewUnique(create_info);

    return image_view;
}

vk::UniqueFramebuffer CreateUniqueFrameBuffer(
    vk::Device                     device,
    vk::RenderPass                 renderpass,
    vk::Extent2D                   extent,
    std::span<const vk::ImageView> attachments)
{
    vk::FramebufferCreateInfo create_info;
    {
        create_info.renderPass      = renderpass;
        create_info.attachmentCount = narrow_cast<uint32_t>(attachments.size());
        create_info.pAttachments    = attachments.data();
        create_info.width           = extent.width;
        create_info.height          = extent.height;
        create_info.layers          = 1;
    }
    auto framebuffer = device.createFramebufferUnique(create_info);

    return framebuffer;
}

vk::UniqueFramebuffer CreateUniqueFrameBuffer(
    vk::Device                           device,
    vk::RenderPass                       renderpass,
    vk::Extent2D                         extent,
    std::initializer_list<vk::ImageView> attachments)
{
    auto s = std::span(attachments.begin(), attachments.size());
    return CreateUniqueFrameBuffer(device, renderpass, extent, s);
}

} // namespace etna
