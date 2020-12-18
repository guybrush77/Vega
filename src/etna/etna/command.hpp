#pragma once

#include "core.hpp"

namespace etna {

class CommandPool {
  public:
    CommandPool() noexcept {}
    CommandPool(std::nullptr_t) noexcept {}

    operator VkCommandPool() const noexcept { return m_command_pool; }

    bool operator==(const CommandPool&) const = default;

    auto AllocateCommandBuffer(CommandBufferLevel level = CommandBufferLevel::Primary) -> UniqueCommandBuffer;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    CommandPool(VkCommandPool command_pool, VkDevice device) noexcept : m_command_pool(command_pool), m_device(device)
    {}

    static auto Create(VkDevice vk_device, const VkCommandPoolCreateInfo& create_info) -> UniqueCommandPool;

    void Destroy() noexcept;

    VkCommandPool m_command_pool{};
    VkDevice      m_device{};
};

class CommandBuffer {
  public:
    CommandBuffer() noexcept {}
    CommandBuffer(std::nullptr_t) noexcept {}

    operator VkCommandBuffer() const noexcept { return m_command_buffer; }

    bool operator==(const CommandBuffer&) const = default;

    void Begin(CommandBufferUsage command_buffer_usage_flags = {});

    void BeginRenderPass(
        Framebuffer                             framebuffer,
        Rect2D                                  render_area,
        std::initializer_list<const ClearValue> clear_values,
        SubpassContents                         subpass_contents = SubpassContents::Inline);

    void EndRenderPass();

    void End();

    void BindPipeline(PipelineBindPoint pipeline_bind_point, Pipeline pipeline);

    void BindVertexBuffers(Buffer buffer);

    void BindIndexBuffer(Buffer buffer, IndexType index_type, size_t offset = 0);

    void BindDescriptorSet(
        PipelineBindPoint               pipeline_bind_point,
        PipelineLayout                  pipeline_layout,
        DescriptorSet                   descriptor_set,
        std::initializer_list<uint32_t> dynamic_offsets);

    void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex = 0, uint32_t first_instance = 0);

    void DrawIndexed(
        uint32_t index_count,
        uint32_t instance_count,
        uint32_t first_index    = 0,
        int32_t  vertex_offset  = 0,
        uint32_t first_instance = 0);

    void PipelineBarrier(
        Image2D       image,
        PipelineStage src_stage_flags,
        PipelineStage dst_stage_flags,
        Access        src_access_flags,
        Access        dst_access_flags,
        ImageLayout   old_layout,
        ImageLayout   new_layout,
        ImageAspect   aspect_flags);

    void CopyImage(
        Image2D     src_image,
        ImageLayout src_image_layout,
        Image2D     dst_image,
        ImageLayout dst_image_layout,
        Extent2D    extent,
        ImageAspect image_aspect_flags);

    void CopyBuffer(Buffer src_buffer, Buffer dst_buffer, size_t size);

    void ResetCommandBuffer();

    void SetViewport(Viewport viewport);

    void SetScissor(Rect2D scissor);

  private:
    template <typename>
    friend class UniqueHandle;

    friend class CommandPool;

    CommandBuffer(VkCommandBuffer command_buffer, VkDevice device, VkCommandPool command_pool) noexcept
        : m_command_buffer(command_buffer), m_device(device), m_command_pool(command_pool)
    {}

    static auto Create(VkDevice vk_device, const VkCommandBufferAllocateInfo& alloc_info) -> UniqueCommandBuffer;

    void Destroy() noexcept;

    VkCommandBuffer m_command_buffer{};
    VkDevice        m_device{};
    VkCommandPool   m_command_pool{};
};

} // namespace etna
