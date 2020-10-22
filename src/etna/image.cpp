#include "image.hpp"

#include "renderpass.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#define COMPONENT "Etna: "

namespace etna {

void* Image2D::MapMemory()
{
    assert(m_image);

    void* data = nullptr;
    if (VK_SUCCESS != vmaMapMemory(m_allocator, m_allocation, &data)) {
        throw_runtime_error(COMPONENT "Function vmaMapMemory failed");
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
    if (VK_SUCCESS != vmaCreateImage(allocator, &create_info, &allocation_info, &image, &allocation, nullptr)) {
        throw_runtime_error("vmaCreateImage failed");
    }

    spdlog::info(COMPONENT "Created VkImage {}", fmt::ptr(image));

    return UniqueImage2D(Image2D(image, allocator, allocation, create_info.format));
}

void Image2D::Destroy() noexcept
{
    assert(m_image);
    assert(m_allocator);

    vmaDestroyImage(m_allocator, m_image, m_allocation);

    spdlog::info(COMPONENT "Destroyed VkImage {}", fmt::ptr(m_image));

    m_image      = nullptr;
    m_allocator  = nullptr;
    m_allocation = nullptr;
    m_format     = {};
}

UniqueImageView2D ImageView2D::Create(VkDevice vk_device, const VkImageViewCreateInfo& create_info)
{
    VkImageView vk_image_view{};

    if (auto result = vkCreateImageView(vk_device, &create_info, nullptr, &vk_image_view); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateImageView error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkImageView {}", fmt::ptr(vk_image_view));

    return UniqueImageView2D(ImageView2D(vk_image_view, vk_device));
}

void ImageView2D::Destroy() noexcept
{
    assert(m_image_view);

    vkDestroyImageView(m_device, m_image_view, nullptr);

    spdlog::info(COMPONENT "Destroyed VkImageView {}", fmt::ptr(m_image_view));

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
        throw_runtime_error(fmt::format("vkCreateFramebuffer error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkFramebuffer {}", fmt::ptr(framebuffer));

    auto renderpass = create_info.renderPass;

    return UniqueFramebuffer(Framebuffer(framebuffer, device, renderpass));
}

void Framebuffer::Destroy() noexcept
{
    assert(m_renderpass);

    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);

    spdlog::info(COMPONENT "Destroyed VkFramebuffer {}", fmt::ptr(m_framebuffer));

    m_framebuffer = nullptr;
    m_device      = nullptr;
    m_renderpass  = nullptr;
}

} // namespace etna
