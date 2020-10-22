#include "command.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "renderpass.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

UniqueCommandBuffer CommandPool::AllocateCommandBuffer(CommandBufferLevel level)
{
    assert(m_command_pool);

    auto alloc_info = VkCommandBufferAllocateInfo{

        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = m_command_pool,
        .level              = GetVk(level),
        .commandBufferCount = 1
    };

    return CommandBuffer::Create(m_device, alloc_info);
}

UniqueCommandPool CommandPool::Create(VkDevice vk_device, const VkCommandPoolCreateInfo& create_info)
{
    VkCommandPool vk_command_pool{};

    if (auto result = vkCreateCommandPool(vk_device, &create_info, nullptr, &vk_command_pool); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateCommandPool error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkCommandPool {}", fmt::ptr(vk_command_pool));

    return UniqueCommandPool(CommandPool(vk_command_pool, vk_device));
}

void CommandPool::Destroy() noexcept
{
    assert(m_command_pool);

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    spdlog::info(COMPONENT "Destroyed VkCommandPool {}", fmt::ptr(m_command_pool));

    m_command_pool = nullptr;
    m_device       = nullptr;
}

void CommandBuffer::Begin(CommandBufferUsage command_buffer_usage_flags)
{
    assert(m_command_buffer);

    auto begin_info = VkCommandBufferBeginInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = GetVk(command_buffer_usage_flags),
        .pInheritanceInfo = nullptr
    };

    if (auto result = vkBeginCommandBuffer(m_command_buffer, &begin_info); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkBeginCommandBuffer error: {}", result).c_str());
    }
}

void CommandBuffer::BeginRenderPass(
    Framebuffer                             framebuffer,
    Rect2D                                  render_area,
    std::initializer_list<const ClearValue> clear_values,
    SubpassContents                         subpass_contents)
{
    assert(m_command_buffer);

    std::vector<VkClearValue> vk_clear_values(clear_values.begin(), clear_values.end());

    VkRenderPassBeginInfo begin_info = {

        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext           = nullptr,
        .renderPass      = framebuffer.RenderPass(),
        .framebuffer     = framebuffer,
        .renderArea      = render_area,
        .clearValueCount = narrow_cast<uint32_t>(vk_clear_values.size()),
        .pClearValues    = vk_clear_values.data()
    };

    vkCmdBeginRenderPass(m_command_buffer, &begin_info, GetVk(subpass_contents));
}

void CommandBuffer::EndRenderPass()
{
    assert(m_command_buffer);
    vkCmdEndRenderPass(m_command_buffer);
}

void CommandBuffer::End()
{
    assert(m_command_buffer);
    vkEndCommandBuffer(m_command_buffer);
}

void CommandBuffer::BindPipeline(PipelineBindPoint pipeline_bind_point, Pipeline pipeline)
{
    assert(m_command_buffer);
    vkCmdBindPipeline(m_command_buffer, GetVk(pipeline_bind_point), pipeline);
}

void CommandBuffer::BindVertexBuffers(Buffer buffer)
{
    assert(m_command_buffer);

    VkBuffer     vk_buffer = buffer;
    VkDeviceSize vk_offset = 0;

    vkCmdBindVertexBuffers(m_command_buffer, 0, 1, &vk_buffer, &vk_offset);
}

void CommandBuffer::BindIndexBuffer(Buffer buffer, IndexType index_type, size_t offset)
{
    assert(m_command_buffer);

    VkBuffer     vk_buffer = buffer;
    VkDeviceSize vk_offset = narrow_cast<VkDeviceSize>(offset);

    vkCmdBindIndexBuffer(m_command_buffer, vk_buffer, vk_offset, GetVk(index_type));
}

void CommandBuffer::BindDescriptorSet(
    PipelineBindPoint pipeline_bind_point,
    PipelineLayout    pipeline_layout,
    DescriptorSet     descriptor_set)
{
    assert(m_command_buffer);

    VkDescriptorSet vk_descriptor_set = descriptor_set;

    vkCmdBindDescriptorSets(
        m_command_buffer,
        GetVk(pipeline_bind_point),
        pipeline_layout,
        0,
        1,
        &vk_descriptor_set,
        0,
        nullptr);
}

void CommandBuffer::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    assert(m_command_buffer);

    vkCmdDraw(m_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::DrawIndexed(
    uint32_t index_count,
    uint32_t instance_count,
    uint32_t first_index,
    int32_t  vertex_offset,
    uint32_t first_instance)
{
    assert(m_command_buffer);

    vkCmdDrawIndexed(m_command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::PipelineBarrier(
    Image2D       image,
    PipelineStage src_stage_flags,
    PipelineStage dst_stage_flags,
    Access        src_access_flags,
    Access        dst_access_flags,
    ImageLayout   old_layout,
    ImageLayout   new_layout,
    ImageAspect   aspect_flags)
{
    assert(m_command_buffer);

    VkImageSubresourceRange subresource_range = {

        .aspectMask     = GetVk(aspect_flags),
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1
    };

    VkImageMemoryBarrier image_memory_barrier = {

        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = GetVk(src_access_flags),
        .dstAccessMask       = GetVk(dst_access_flags),
        .oldLayout           = GetVk(old_layout),
        .newLayout           = GetVk(new_layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = subresource_range
    };

    vkCmdPipelineBarrier(
        m_command_buffer,
        GetVk(src_stage_flags),
        GetVk(dst_stage_flags),
        {},
        0,
        nullptr,
        0,
        nullptr,
        1,
        &image_memory_barrier);
}

void CommandBuffer::CopyImage(
    Image2D     src_image,
    ImageLayout src_image_layout,
    Image2D     dst_image,
    ImageLayout dst_image_layout,
    Extent2D    extent,
    ImageAspect image_aspect_flags)
{
    assert(m_command_buffer);

    VkImageCopy image_copy = {

        .srcSubresource = { GetVk(image_aspect_flags), 0, 0, 1 },
        .srcOffset      = { 0, 0, 0 },
        .dstSubresource = { GetVk(image_aspect_flags), 0, 0, 1 },
        .dstOffset      = { 0, 0, 0 },
        .extent         = VkExtent3D{ extent.width, extent.height, 1 }
    };

    vkCmdCopyImage(
        m_command_buffer,
        src_image,
        GetVk(src_image_layout),
        dst_image,
        GetVk(dst_image_layout),
        1,
        &image_copy);
}

void CommandBuffer::CopyBuffer(Buffer src_buffer, Buffer dst_buffer, size_t size)
{
    assert(m_command_buffer);

    VkBufferCopy buffer_copy = {

        .srcOffset = 0,
        .dstOffset = 0,
        .size      = narrow_cast<VkDeviceSize>(size)
    };

    vkCmdCopyBuffer(m_command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);
}

UniqueCommandBuffer CommandBuffer::Create(VkDevice vk_device, const VkCommandBufferAllocateInfo& alloc_info)
{
    VkCommandBuffer vk_command_buffer{};

    if (auto result = vkAllocateCommandBuffers(vk_device, &alloc_info, &vk_command_buffer); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateCommandPool error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Allocated VkCommandBuffer {}", fmt::ptr(vk_command_buffer));

    return UniqueCommandBuffer(CommandBuffer(vk_command_buffer, vk_device, alloc_info.commandPool));
}

void CommandBuffer::Destroy() noexcept
{
    assert(m_command_buffer);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);

    spdlog::info(COMPONENT "Destroyed VkCommandBuffer {}", fmt::ptr(m_command_buffer));

    m_command_buffer = nullptr;
    m_device         = nullptr;
    m_command_pool   = nullptr;
}

} // namespace etna
