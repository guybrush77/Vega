#pragma once

#include "core.hpp"

#include <span>
#include <vector>

VK_DEFINE_HANDLE(VmaAllocator)

namespace etna {

class Device {
  public:
    struct Builder final {
        Builder() noexcept;

        void AddQueue(uint32_t queue_family_index, uint32_t queue_count);
        void AddEnabledLayer(const char* layer_name);
        void AddEnabledExtension(const char* extension_name);

        VkDeviceCreateInfo state{};

      private:
        std::vector<VkDeviceQueueCreateInfo> m_device_queues;
        std::vector<const char*>             m_enabled_layer_names;
        std::vector<const char*>             m_enabled_extension_names;
    };

    Device() noexcept {}
    Device(std::nullptr_t) noexcept {}

    operator VkDevice() const noexcept { return m_device; }

    bool operator==(const Device&) const = default;

    auto AcquireNextImageKHR(SwapchainKHR swapchain, Semaphore semaphore, Fence fence) -> Return<uint32_t>;

    auto CreateCommandPool(uint32_t queue_family_index, CommandPoolCreate command_pool_create_flags = {})
        -> UniqueCommandPool;

    auto CreateFence(FenceCreate fence_flags = {}) -> UniqueFence;

    auto CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent) -> UniqueFramebuffer;
    auto CreateFramebuffer(RenderPass renderpass, std::initializer_list<const ImageView2D> image_views, Extent2D extent)
        -> UniqueFramebuffer;

    auto CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline;

    auto CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout;

    auto CreateRenderPass(const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass;

    auto CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& create_info) -> UniqueDescriptorSetLayout;

    auto CreateDescriptorPool(DescriptorPoolFlags flags, std::span<DescriptorPoolSize> pool_sizes, size_t max_sets = 0)
        -> UniqueDescriptorPool;

    auto CreateDescriptorPool(
        DescriptorPoolFlags                       flags,
        std::initializer_list<DescriptorPoolSize> pool_sizes,
        size_t                                    max_sets = 0) -> UniqueDescriptorPool;

    auto CreateDescriptorPool(std::span<DescriptorPoolSize> pool_sizes, size_t max_sets = 0) -> UniqueDescriptorPool;

    auto CreateDescriptorPool(std::initializer_list<DescriptorPoolSize> pool_sizes, size_t max_sets = 0)
        -> UniqueDescriptorPool;

    auto CreateShaderModule(const unsigned char* shader_data, size_t shader_size) -> UniqueShaderModule;

    auto CreateSwapchainKHR(
        SurfaceKHR       surface,
        uint32_t         min_image_count,
        SurfaceFormatKHR surface_format,
        Extent2D         extent,
        ImageUsage       image_usage,
        PresentModeKHR   present_mode) -> UniqueSwapchainKHR;

    auto CreateBuffer(std::size_t size, BufferUsage buffer_usage_flags, MemoryUsage memory_usage) -> UniqueBuffer;

    auto CreateBuffers(std::size_t count, std::size_t size, BufferUsage buffer_usage_flags, MemoryUsage memory_usage)
        -> std::vector<UniqueBuffer>;

    auto CreateImage(
        Format      format,
        Extent2D    extent,
        ImageUsage  image_usage_flags,
        MemoryUsage memory_usage,
        ImageTiling image_tiling) -> UniqueImage2D;

    auto CreateImageView(Image2D image, ImageAspect image_aspect_flags) -> UniqueImageView2D;

    auto CreateSemaphore() -> UniqueSemaphore;

    auto GetQueue(uint32_t queue_family_index) const noexcept -> Queue;

    auto GetSwapchainImagesKHR(SwapchainKHR swapchain) const -> std::vector<Image2D>;

    void ResetFence(Fence fence);
    void ResetFences(std::initializer_list<Fence> fences);

    void UpdateDescriptorSets(std::initializer_list<WriteDescriptorSetRef> write_descriptor_sets);
    void UpdateDescriptorSets(std::span<WriteDescriptorSet> write_descriptor_sets);

    void WaitForFence(Fence fence, uint64_t timeout = UINT64_MAX);
    void WaitForFences(std::initializer_list<Fence> fences, WaitAll wait_all, uint64_t timeout = UINT64_MAX);

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
