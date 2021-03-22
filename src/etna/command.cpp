#include "etna/command.hpp"
#include "etna/buffer.hpp"
#include "etna/descriptor.hpp"
#include "etna/image.hpp"
#include "etna/pipeline.hpp"
#include "etna/renderpass.hpp"

#include <algorithm>
#include <array>
#include <cassert>

namespace etna {

UniqueCommandBuffer CommandPool::AllocateCommandBuffer(CommandBufferLevel level)
{
    assert(m_command_pool);

    auto alloc_info = VkCommandBufferAllocateInfo{

        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = m_command_pool,
        .level              = VkEnum(level),
        .commandBufferCount = 1
    };

    return CommandBuffer::Create(m_device, alloc_info);
}

UniqueCommandPool CommandPool::Create(VkDevice vk_device, const VkCommandPoolCreateInfo& create_info)
{
    VkCommandPool vk_command_pool{};

    if (auto result = vkCreateCommandPool(vk_device, &create_info, nullptr, &vk_command_pool); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueCommandPool(CommandPool(vk_command_pool, vk_device));
}

void CommandPool::Destroy() noexcept
{
    assert(m_command_pool);

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    m_command_pool = nullptr;
    m_device       = nullptr;
}

void CommandBuffer::Begin(CommandBufferUsage command_buffer_usage_flags)
{
    assert(m_command_buffer);

    auto begin_info = VkCommandBufferBeginInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = VkEnum(command_buffer_usage_flags),
        .pInheritanceInfo = nullptr
    };

    if (auto result = vkBeginCommandBuffer(m_command_buffer, &begin_info); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
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

    vkCmdBeginRenderPass(m_command_buffer, &begin_info, VkEnum(subpass_contents));
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
    vkCmdBindPipeline(m_command_buffer, VkEnum(pipeline_bind_point), pipeline);
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

    vkCmdBindIndexBuffer(m_command_buffer, vk_buffer, vk_offset, VkEnum(index_type));
}

void CommandBuffer::BindDescriptorSets(
    PipelineBindPoint                    pipeline_bind_point,
    PipelineLayout                       pipeline_layout,
    size_t                               first_set,
    std::initializer_list<DescriptorSet> descriptor_sets,
    std::initializer_list<uint32_t>      dynamic_offsets)
{
    assert(m_command_buffer);

    constexpr size_t kMaxDescriptorSets = 16;

    std::array<VkDescriptorSet, kMaxDescriptorSets> vk_descriptor_sets;

    if (descriptor_sets.size() > vk_descriptor_sets.size()) {
        throw_etna_error(__FILE__, __LINE__, "Too many elements in std::initializer_list<DescriptorSet>");
    }

    std::transform(descriptor_sets.begin(), descriptor_sets.end(), vk_descriptor_sets.begin(), [](DescriptorSet set) {
        return static_cast<VkDescriptorSet>(set);
    });

    vkCmdBindDescriptorSets(
        m_command_buffer,
        VkEnum(pipeline_bind_point),
        pipeline_layout,
        narrow_cast<uint32_t>(first_set),
        narrow_cast<uint32_t>(descriptor_sets.size()),
        vk_descriptor_sets.data(),
        narrow_cast<uint32_t>(dynamic_offsets.size()),
        dynamic_offsets.size() ? dynamic_offsets.begin() : nullptr);
}

void CommandBuffer::Draw(size_t vertex_count, size_t instance_count, size_t first_vertex, size_t first_instance)
{
    assert(m_command_buffer);

    vkCmdDraw(
        m_command_buffer,
        narrow_cast<uint32_t>(vertex_count),
        narrow_cast<uint32_t>(instance_count),
        narrow_cast<uint32_t>(first_vertex),
        narrow_cast<uint32_t>(first_instance));
}

void CommandBuffer::DrawIndexed(
    size_t index_count,
    size_t instance_count,
    size_t first_index,
    size_t vertex_offset,
    size_t first_instance)
{
    assert(m_command_buffer);

    vkCmdDrawIndexed(
        m_command_buffer,
        narrow_cast<uint32_t>(index_count),
        narrow_cast<uint32_t>(instance_count),
        narrow_cast<uint32_t>(first_index),
        narrow_cast<uint32_t>(vertex_offset),
        narrow_cast<uint32_t>(first_instance));
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

        .aspectMask     = VkEnum(aspect_flags),
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1
    };

    VkImageMemoryBarrier image_memory_barrier = {

        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VkEnum(src_access_flags),
        .dstAccessMask       = VkEnum(dst_access_flags),
        .oldLayout           = VkEnum(old_layout),
        .newLayout           = VkEnum(new_layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = subresource_range
    };

    vkCmdPipelineBarrier(
        m_command_buffer,
        VkEnum(src_stage_flags),
        VkEnum(dst_stage_flags),
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

        .srcSubresource = { VkEnum(image_aspect_flags), 0, 0, 1 },
        .srcOffset      = { 0, 0, 0 },
        .dstSubresource = { VkEnum(image_aspect_flags), 0, 0, 1 },
        .dstOffset      = { 0, 0, 0 },
        .extent         = VkExtent3D{ extent.width, extent.height, 1 }
    };

    vkCmdCopyImage(
        m_command_buffer,
        src_image,
        VkEnum(src_image_layout),
        dst_image,
        VkEnum(dst_image_layout),
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

void CommandBuffer::CopyBufferToImage(
    Buffer                                 src_buffer,
    Image2D                                dst_image,
    ImageLayout                            dst_image_layout,
    std::initializer_list<BufferImageCopy> regions)
{
    assert(m_command_buffer);

    auto vk_count   = narrow_cast<uint32_t>(regions.size());
    auto vk_regions = reinterpret_cast<const VkBufferImageCopy*>(regions.begin());

    vkCmdCopyBufferToImage(m_command_buffer, src_buffer, dst_image, VkEnum(dst_image_layout), vk_count, vk_regions);
}

void CommandBuffer::ResetCommandBuffer(CommandBufferReset reset_flags)
{
    assert(m_command_buffer);

    vkResetCommandBuffer(m_command_buffer, VkEnum(reset_flags));
}

void CommandBuffer::SetViewport(Viewport viewport)
{
    assert(m_command_buffer);

    vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);
}

void CommandBuffer::SetScissor(Rect2D scissor)
{
    assert(m_command_buffer);

    vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);
}

UniqueCommandBuffer CommandBuffer::Create(VkDevice vk_device, const VkCommandBufferAllocateInfo& alloc_info)
{
    VkCommandBuffer vk_command_buffer{};

    if (auto result = vkAllocateCommandBuffers(vk_device, &alloc_info, &vk_command_buffer); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueCommandBuffer(CommandBuffer(vk_command_buffer, vk_device, alloc_info.commandPool));
}

void CommandBuffer::Destroy() noexcept
{
    assert(m_command_buffer);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);

    m_command_buffer = nullptr;
    m_device         = nullptr;
    m_command_pool   = nullptr;
}

} // namespace etna
