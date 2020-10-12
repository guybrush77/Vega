#pragma once

#include "buffer.hpp"
#include "command.hpp"
#include "queue.hpp"
#include "renderpass.hpp"
#include "shader.hpp"

ETNA_DEFINE_HANDLE(EtnaDevice)

namespace etna {

class Device;
class Instance;

using UniqueDevice = UniqueHandle<Device>;

class Device {
  public:
    Device() noexcept {}
    Device(std::nullptr_t) noexcept {}

    operator VkDevice() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Device& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Device& rhs) const noexcept { return m_state != rhs.m_state; }

    auto CreateCommandPool(QueueFamily queue_family, CommandPoolCreateMask command_pool_create_mask = {})
        -> UniqueCommandPool;

    auto CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent) -> UniqueFramebuffer;

    auto CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline;

    auto CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout;

    auto CreateRenderPass(const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass;

    auto CreateShaderModule(const char* shader_name) -> UniqueShaderModule;

    auto CreateBuffer(std::size_t size, BufferUsageMask buffer_usage_mask, MemoryUsage memory_usage) -> UniqueBuffer;

    auto CreateImage(
        Format         format,
        Extent2D       extent,
        ImageUsageMask image_usage_mask,
        MemoryUsage    memory_usage,
        ImageTiling    image_tiling) -> UniqueImage2D;

    auto CreateImageView(Image2D image) -> UniqueImageView2D;

    auto GetQueue(QueueFamily queue_family) const noexcept -> Queue;

    void WaitIdle();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Instance;

    Device(EtnaDevice device) : m_state(device) {}

    static auto Create(VkInstance instance) -> UniqueDevice;

    void Destroy() noexcept;

    uint32_t GetQueueFamilyIndex(QueueFamily queue_family) const noexcept;

    operator EtnaDevice() const noexcept { return m_state; }

    EtnaDevice m_state{};
};

} // namespace etna
