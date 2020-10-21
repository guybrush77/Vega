#pragma once

#include "core.hpp"

VK_DEFINE_HANDLE(VmaAllocator)

namespace etna {

struct DeviceBuilder final {
    DeviceBuilder() noexcept;

    constexpr operator VkDeviceCreateInfo() const noexcept { return create_info; }

    void AddQueue(uint32_t queue_family_index, uint32_t queue_count);

    void AddEnabledLayer(const char* layer_name);

    VkDeviceCreateInfo create_info{};

  private:
    std::vector<VkDeviceQueueCreateInfo> m_device_queues;
    std::vector<const char*>             m_enabled_layer_names;
};

class Device {
  public:
    Device() noexcept {}
    Device(std::nullptr_t) noexcept {}

    operator VkDevice() const noexcept { return m_device; }

    explicit operator bool() const noexcept { return m_device != nullptr; }

    bool operator==(const Device& rhs) const noexcept { return m_device == rhs.m_device; }
    bool operator!=(const Device& rhs) const noexcept { return m_device != rhs.m_device; }

    auto CreateDeviceBuilder() const noexcept { return DeviceBuilder{}; }

    auto CreateCommandPool(uint32_t queue_family_index, CommandPoolCreate command_pool_create_flags = {})
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

    auto CreateShaderModule(const unsigned char* shader_data, size_t shader_size) -> UniqueShaderModule;

    auto CreateBuffer(std::size_t size, BufferUsage buffer_usage_flags, MemoryUsage memory_usage) -> UniqueBuffer;

    auto CreateImage(
        Format      format,
        Extent2D    extent,
        ImageUsage  image_usage_flags,
        MemoryUsage memory_usage,
        ImageTiling image_tiling) -> UniqueImage2D;

    auto CreateImageView(Image2D image, ImageAspect image_aspect_flags) -> UniqueImageView2D;

    auto GetQueue(uint32_t queue_family_index) const noexcept -> Queue;

    void UpdateDescriptorSet(const WriteDescriptorSet& write_descriptor_set);

    void WaitIdle();

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Instance;

    Device(VkDevice device, VmaAllocator allocator) noexcept : m_device(device), m_allocator(allocator) {}

    static auto Create(VkInstance instance, VkPhysicalDevice physical_device, const VkDeviceCreateInfo& create_info)
        -> UniqueDevice;

    void Destroy() noexcept;

    VkDevice     m_device{};
    VmaAllocator m_allocator{};
};

} // namespace etna
