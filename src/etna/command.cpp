#include "command.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "pipeline.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaCommandPool_T final {
    VkCommandPool command_pool;
    VkDevice      device;
};

struct EtnaCommandBuffer_T final {
    VkCommandBuffer command_buffer;
    VkDevice        device;
    VkCommandPool   command_pool;
};

} // namespace

namespace etna {

CommandPool::operator VkCommandPool() const noexcept
{
    return m_state ? m_state->command_pool : VkCommandPool{};
}

UniqueCommandBuffer CommandPool::AllocateCommandBuffer(CommandBufferLevel level)
{
    assert(m_state);

    auto alloc_info = VkCommandBufferAllocateInfo{

        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = m_state->command_pool,
        .level              = GetVk(level),
        .commandBufferCount = 1
    };

    return CommandBuffer::Create(m_state->device, alloc_info);
}

UniqueCommandPool CommandPool::Create(VkDevice device, const VkCommandPoolCreateInfo& create_info)
{
    VkCommandPool command_pool{};

    if (auto result = vkCreateCommandPool(device, &create_info, nullptr, &command_pool); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateCommandPool error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkCommandPool {}", fmt::ptr(command_pool));

    return UniqueCommandPool(new EtnaCommandPool_T{ command_pool, device });
}

void CommandPool::Destroy() noexcept
{
    assert(m_state);

    vkDestroyCommandPool(m_state->device, m_state->command_pool, nullptr);

    spdlog::info(COMPONENT "Destroyed VkCommandPool {}", fmt::ptr(m_state->command_pool));

    delete m_state;

    m_state = nullptr;
}

CommandBuffer::operator VkCommandBuffer() const noexcept
{
    return m_state ? m_state->command_buffer : VkCommandBuffer{};
}

void CommandBuffer::Begin(CommandBufferUsage command_buffer_usage_flags)
{
    assert(m_state);

    auto begin_info = VkCommandBufferBeginInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = GetVk(command_buffer_usage_flags),
        .pInheritanceInfo = nullptr
    };

    if (auto result = vkBeginCommandBuffer(m_state->command_buffer, &begin_info); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkBeginCommandBuffer error: {}", result).c_str());
    }
}

void CommandBuffer::BeginRenderPass(
    Framebuffer                             framebuffer,
    std::initializer_list<const ClearValue> clear_values,
    SubpassContents                         subpass_contents)
{
    assert(m_state);

    std::vector<VkClearValue> vk_clear_values(clear_values.begin(), clear_values.end());

    auto vk_render_area = VkRect2D{ VkOffset2D{ 0, 0 }, framebuffer.Extent2D() };

    VkRenderPassBeginInfo begin_info = {

        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext           = nullptr,
        .renderPass      = framebuffer.RenderPass(),
        .framebuffer     = framebuffer,
        .renderArea      = vk_render_area,
        .clearValueCount = narrow_cast<uint32_t>(vk_clear_values.size()),
        .pClearValues    = vk_clear_values.data()
    };

    vkCmdBeginRenderPass(m_state->command_buffer, &begin_info, GetVk(subpass_contents));
}

void CommandBuffer::EndRenderPass()
{
    assert(m_state);
    vkCmdEndRenderPass(m_state->command_buffer);
}

void CommandBuffer::End()
{
    assert(m_state);
    vkEndCommandBuffer(m_state->command_buffer);
}

void CommandBuffer::BindPipeline(PipelineBindPoint pipeline_bind_point, Pipeline pipeline)
{
    assert(m_state);
    vkCmdBindPipeline(m_state->command_buffer, GetVk(pipeline_bind_point), pipeline);
}

void CommandBuffer::BindVertexBuffers(Buffer buffer)
{
    assert(m_state);

    VkBuffer     vk_buffer = buffer;
    VkDeviceSize vk_offset = 0;

    vkCmdBindVertexBuffers(m_state->command_buffer, 0, 1, &vk_buffer, &vk_offset);
}

void CommandBuffer::BindIndexBuffer(Buffer buffer, IndexType index_type, size_t offset)
{
    assert(m_state);

    VkBuffer     vk_buffer = buffer;
    VkDeviceSize vk_offset = narrow_cast<VkDeviceSize>(offset);

    vkCmdBindIndexBuffer(m_state->command_buffer, vk_buffer, vk_offset, GetVk(index_type));
}

void CommandBuffer::BindDescriptorSet(
    PipelineBindPoint pipeline_bind_point,
    PipelineLayout    pipeline_layout,
    DescriptorSet     descriptor_set)
{
    assert(m_state);

    VkDescriptorSet vk_descriptor_set = descriptor_set;

    vkCmdBindDescriptorSets(
        m_state->command_buffer,
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
    assert(m_state);

    vkCmdDraw(m_state->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::DrawIndexed(
    uint32_t index_count,
    uint32_t instance_count,
    uint32_t first_index,
    int32_t  vertex_offset,
    uint32_t first_instance)
{
    assert(m_state);

    vkCmdDrawIndexed(m_state->command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
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
    assert(m_state);

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
        m_state->command_buffer,
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
    ImageAspect image_aspect_flags)
{
    assert(m_state);

    auto [width, height] = src_image.Extent2D();

    VkImageCopy image_copy = {

        .srcSubresource = { GetVk(image_aspect_flags), 0, 0, 1 },
        .srcOffset      = { 0, 0, 0 },
        .dstSubresource = { GetVk(image_aspect_flags), 0, 0, 1 },
        .dstOffset      = { 0, 0, 0 },
        .extent         = { width, height, 1 }
    };

    vkCmdCopyImage(
        m_state->command_buffer,
        src_image,
        GetVk(src_image_layout),
        dst_image,
        GetVk(dst_image_layout),
        1,
        &image_copy);
}

void CommandBuffer::CopyBuffer(Buffer src_buffer, Buffer dst_buffer, size_t size)
{
    assert(m_state);

    VkBufferCopy buffer_copy = {

        .srcOffset = 0,
        .dstOffset = 0,
        .size      = narrow_cast<VkDeviceSize>(size)
    };

    vkCmdCopyBuffer(m_state->command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);
}

UniqueCommandBuffer CommandBuffer::Create(VkDevice device, const VkCommandBufferAllocateInfo& alloc_info)
{
    VkCommandBuffer command_buffer{};

    if (auto result = vkAllocateCommandBuffers(device, &alloc_info, &command_buffer); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateCommandPool error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Allocated VkCommandBuffer {}", fmt::ptr(command_buffer));

    return UniqueCommandBuffer(new EtnaCommandBuffer_T{ command_buffer, device, alloc_info.commandPool });
}

void CommandBuffer::Destroy() noexcept
{
    assert(m_state);

    vkFreeCommandBuffers(m_state->device, m_state->command_pool, 1, &m_state->command_buffer);

    spdlog::info(COMPONENT "Destroyed VkCommandBuffer {}", fmt::ptr(m_state->command_buffer));

    delete m_state;

    m_state = nullptr;
}

} // namespace etna
