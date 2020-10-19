#pragma once

#include "core.hpp"

ETNA_DEFINE_HANDLE(EtnaDevice)

namespace etna {

class Device {
  public:
    Device() noexcept {}
    Device(std::nullptr_t) noexcept {}

    operator VkDevice() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Device& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Device& rhs) const noexcept { return m_state != rhs.m_state; }

    auto CreateCommandPool(QueueFamily queue_family, CommandPoolCreate command_pool_create_flags = {})
        -> UniqueCommandPool;

    auto CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent) -> UniqueFramebuffer;
    auto CreateFramebuffer(RenderPass renderpass, std::initializer_list<const ImageView2D> image_views, Extent2D extent)
        -> UniqueFramebuffer;

    auto CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline;

    auto CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout;

    auto CreateRenderPass(const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass;

    auto CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& create_info) -> UniqueDescriptorSetLayout;

    auto CreateDescriptorPool(DescriptorType descriptor_type, uint32_t size, uint32_t max_sets = 0)
        -> UniqueDescriptorPool;

    auto CreateShaderModule(const char* shader_name) -> UniqueShaderModule;

    auto CreateBuffer(std::size_t size, BufferUsage buffer_usage_flags, MemoryUsage memory_usage) -> UniqueBuffer;

    auto CreateImage(
        Format      format,
        Extent2D    extent,
        ImageUsage  image_usage_flags,
        MemoryUsage memory_usage,
        ImageTiling image_tiling) -> UniqueImage2D;

    auto CreateImageView(Image2D image, ImageAspect image_aspect_flags) -> UniqueImageView2D;

    auto GetQueue(QueueFamily queue_family) const noexcept -> Queue;

    void UpdateDescriptorSet(const WriteDescriptorSet& write_descriptor_set);

    void WaitIdle();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Instance;

    Device(EtnaDevice device) : m_state(device) {}

    static auto Create(VkInstance instance, VkSurfaceKHR surface) -> UniqueDevice;

    void Destroy() noexcept;

    uint32_t GetQueueFamilyIndex(QueueFamily queue_family) const noexcept;

    operator EtnaDevice() const noexcept { return m_state; }

    EtnaDevice m_state{};
};

} // namespace etna
