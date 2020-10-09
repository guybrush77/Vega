#include "etna/image.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaImage2D_T final {
    VkImage           image;
    VkFormat          format;
    VkExtent2D        extent;
    VkImageUsageFlags usage;
    VmaAllocator      allocator;
    VmaAllocation     allocation;
};

struct EtnaImageView2D_T final {
    VkImageView image_view;
    VkDevice    device;
};

struct EtnaFramebuffer_T final {
    VkFramebuffer framebuffer;
    VkDevice      device;
    VkRenderPass  renderpass;
    VkExtent2D    extent;
};

} // namespace

namespace etna {

Image2D::operator VkImage() const noexcept
{
    assert(m_state);
    return m_state->image;
}

ImageUsageMask Image2D::UsageMask() const noexcept
{
    assert(m_state);
    return static_cast<ImageUsage>(m_state->usage);
}

Format Image2D::Format() const noexcept
{
    assert(m_state);
    return static_cast<etna::Format>(m_state->format);
}

Extent2D Image2D::Extent() const noexcept
{
    assert(m_state);
    return { m_state->extent.width, m_state->extent.height };
}

Rect2D Image2D::Rect2D() const noexcept
{
    assert(m_state);
    auto [width, height] = m_state->extent;
    return etna::Rect2D{ { 0, 0 }, { width, height } };
}

void* Image2D::MapMemory()
{
    assert(m_state);

    void* data = nullptr;
    if (VK_SUCCESS != vmaMapMemory(m_state->allocator, m_state->allocation, &data)) {
        throw_runtime_error(COMPONENT "Function vmaMapMemory failed");
    }

    return data;
}

void Image2D::UnmapMemory()
{
    assert(m_state);

    vmaUnmapMemory(m_state->allocator, m_state->allocation);
}

UniqueImage2D Image2D::Create(VmaAllocator allocator, const VkImageCreateInfo& create_info, MemoryUsage memory_usage)
{
    VmaAllocationCreateInfo allocation_info{};
    switch (memory_usage) {
    case MemoryUsage::GpuOnly:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case MemoryUsage::CpuOnly:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        break;
    case MemoryUsage::CpuToGpu:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    case MemoryUsage::GpuToCpu:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        break;
    case MemoryUsage::CpuCopy:
        allocation_info.usage = VMA_MEMORY_USAGE_CPU_COPY;
        break;
    case MemoryUsage::GpuLazilyAllocated:
        allocation_info.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        break;
    default:
        allocation_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    };

    VkImage       image;
    VmaAllocation allocation;
    if (VK_SUCCESS != vmaCreateImage(allocator, &create_info, &allocation_info, &image, &allocation, nullptr)) {
        throw_runtime_error("vmaCreateImage failed");
    }

    spdlog::info(COMPONENT "Created VkImage {}", fmt::ptr(image));

    return UniqueImage2D(new EtnaImage2D_T{ image,
                                            create_info.format,
                                            VkExtent2D{ create_info.extent.width, create_info.extent.height },
                                            create_info.usage,
                                            allocator,
                                            allocation });
}

void Image2D::Destroy() noexcept
{
    assert(m_state);

    vmaDestroyImage(m_state->allocator, m_state->image, m_state->allocation);

    spdlog::info(COMPONENT "Destroyed VkImage {}", fmt::ptr(m_state->image));

    delete m_state;

    m_state = nullptr;
}

ImageView2D::operator VkImageView() const noexcept
{
    assert(m_state);
    return m_state->image_view;
}

UniqueImageView2D ImageView2D::Create(VkDevice device, const VkImageViewCreateInfo& create_info)
{
    VkImageView image_view{};

    if (auto result = vkCreateImageView(device, &create_info, nullptr, &image_view); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateImageView error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkImageView {}", fmt::ptr(image_view));

    return UniqueImageView2D(new EtnaImageView2D_T{ image_view, device });
}

void ImageView2D::Destroy() noexcept
{
    assert(m_state);

    vkDestroyImageView(m_state->device, m_state->image_view, nullptr);

    spdlog::info(COMPONENT "Destroyed VkImageView {}", fmt::ptr(m_state->image_view));

    delete m_state;

    m_state = nullptr;
}

Framebuffer::operator VkFramebuffer() const noexcept
{
    assert(m_state);
    return m_state->framebuffer;
}

Extent2D Framebuffer::Extent2D() const noexcept
{
    assert(m_state);
    return { m_state->extent.width, m_state->extent.height };
}

VkRenderPass Framebuffer::RenderPass() const noexcept
{
    assert(m_state);
    return m_state->renderpass;
}

UniqueFramebuffer Framebuffer::Create(VkDevice device, const VkFramebufferCreateInfo& create_info)
{
    VkFramebuffer framebuffer{};

    if (auto result = vkCreateFramebuffer(device, &create_info, nullptr, &framebuffer); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateFramebuffer error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkFramebuffer {}", fmt::ptr(framebuffer));

    return UniqueFramebuffer(new EtnaFramebuffer_T{ framebuffer,
                                                    device,
                                                    create_info.renderPass,
                                                    { create_info.width, create_info.height } });
}

void Framebuffer::Destroy() noexcept
{
    assert(m_state);

    vkDestroyFramebuffer(m_state->device, m_state->framebuffer, nullptr);

    spdlog::info(COMPONENT "Destroyed VkFramebuffer {}", fmt::ptr(m_state->framebuffer));

    delete m_state;

    m_state = nullptr;
}

} // namespace etna