#include "etna/image.hpp"
#include "etna/renderpass.hpp"

#include <cassert>
#include <vk_mem_alloc.h>

namespace etna {

void* Image2D::MapMemory()
{
    assert(m_image);

    void* data = nullptr;
    if (auto result = vmaMapMemory(m_allocator, m_allocation, &data); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return data;
}

void Image2D::UnmapMemory()
{
    assert(m_image);

    vmaUnmapMemory(m_allocator, m_allocation);
}

UniqueImage2D Image2D::Create(VmaAllocator allocator, const VkImageCreateInfo& create_info, MemoryUsage memory_usage)
{
    VmaAllocationCreateInfo allocation_info{};

    allocation_info.usage = static_cast<VmaMemoryUsage>(memory_usage);

    VkImage       image{};
    VmaAllocation allocation{};
    if (auto result = vmaCreateImage(allocator, &create_info, &allocation_info, &image, &allocation, nullptr);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueImage2D(Image2D(image, allocator, allocation, create_info.format));
}

void Image2D::Destroy() noexcept
{
    assert(m_image);
    assert(m_allocator);

    vmaDestroyImage(m_allocator, m_image, m_allocation);

    m_image      = nullptr;
    m_allocator  = nullptr;
    m_allocation = nullptr;
    m_format     = {};
}

UniqueImageView2D ImageView2D::Create(VkDevice vk_device, const VkImageViewCreateInfo& create_info)
{
    VkImageView vk_image_view{};

    if (auto result = vkCreateImageView(vk_device, &create_info, nullptr, &vk_image_view); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueImageView2D(ImageView2D(vk_image_view, vk_device));
}

void ImageView2D::Destroy() noexcept
{
    assert(m_image_view);

    vkDestroyImageView(m_device, m_image_view, nullptr);

    m_image_view = nullptr;
    m_device     = nullptr;
}

RenderPass Framebuffer::RenderPass() const noexcept
{
    return etna::RenderPass(m_renderpass, m_device);
}

UniqueFramebuffer Framebuffer::Create(VkDevice device, const VkFramebufferCreateInfo& create_info)
{
    VkFramebuffer framebuffer{};

    if (auto result = vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    auto renderpass = create_info.renderPass;

    return UniqueFramebuffer(Framebuffer(framebuffer, device, renderpass));
}

void Framebuffer::Destroy() noexcept
{
    assert(m_renderpass);

    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);

    m_framebuffer = nullptr;
    m_device      = nullptr;
    m_renderpass  = nullptr;
}

} // namespace etna
