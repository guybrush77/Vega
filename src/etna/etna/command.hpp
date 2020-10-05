#pragma once

#include "etna/image.hpp"
#include "etna/pipeline.hpp"

ETNA_DEFINE_HANDLE(EtnaCommandPool)
ETNA_DEFINE_HANDLE(EtnaCommandBuffer)

namespace etna {

class CommandPool;
class CommandBuffer;
class Device;

using UniqueCommandPool   = UniqueHandle<CommandPool>;
using UniqueCommandBuffer = UniqueHandle<CommandBuffer>;

class CommandPool {
  public:
    CommandPool() noexcept {}
    CommandPool(std::nullptr_t) noexcept {}

    operator VkCommandPool() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const CommandPool& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const CommandPool& rhs) const noexcept { return m_state != rhs.m_state; }

    auto AllocateCommandBuffer(CommandBufferLevel level = CommandBufferLevel::Primary) -> UniqueCommandBuffer;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    CommandPool(EtnaCommandPool command_pool) : m_state(command_pool) {}

    static auto Create(VkDevice device, const VkCommandPoolCreateInfo& create_info) -> UniqueCommandPool;

    void Destroy() noexcept;

    operator EtnaCommandPool() const noexcept { return m_state; }

    EtnaCommandPool m_state{};
};

class CommandBuffer {
  public:
    CommandBuffer() noexcept {}
    CommandBuffer(std::nullptr_t) noexcept {}

    operator VkCommandBuffer() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const CommandBuffer& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const CommandBuffer& rhs) const noexcept { return m_state != rhs.m_state; }

    void Begin(CommandBufferUsageMask command_buffer_usage_mask = {});
    void BeginRenderPass(Framebuffer framebuffer, SubpassContents subpass_contents);
    void EndRenderPass();
    void End();
    void BindPipeline(PipelineBindPoint pipeline_bind_point, Pipeline pipeline);
    void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

    void PipelineBarrier(
        Image2D               image,
        PipelineStageMask src_stage_mask,
        PipelineStageMask dst_stage_mask,
        AccessMask            src_access_mask,
        AccessMask            dst_access_mask,
        ImageLayout           old_layout,
        ImageLayout           new_layout,
        ImageAspectMask       aspect_mask);

    void CopyImage(
        Image2D         src_image,
        ImageLayout     src_image_layout,
        Image2D         dst_image,
        ImageLayout     dst_image_layout,
        ImageAspectMask aspect_mask);

  private:
    template <typename>
    friend class UniqueHandle;

    friend class CommandPool;

    CommandBuffer(EtnaCommandBuffer command_buffer) : m_state(command_buffer) {}

    static auto Create(VkDevice device, const VkCommandBufferAllocateInfo& alloc_info) -> UniqueCommandBuffer;

    void Destroy() noexcept;

    operator EtnaCommandBuffer() const noexcept { return m_state; }

    EtnaCommandBuffer m_state{};
};

} // namespace etna
