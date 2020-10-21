#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "buffer.hpp"
#include "command.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "image.hpp"
#include "pipeline.hpp"
#include "queue.hpp"
#include "renderpass.hpp"
#include "shader.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

UniqueRenderPass Device::CreateRenderPass(const VkRenderPassCreateInfo& create_info)
{
    assert(m_device);
    return RenderPass::Create(m_device, create_info);
}

UniqueDescriptorSetLayout Device::CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& create_info)
{
    assert(m_device);

    return DescriptorSetLayout::Create(m_device, create_info);
}

UniqueDescriptorPool Device::CreateDescriptorPool(DescriptorType descriptor_type, uint32_t size, uint32_t max_sets)
{
    assert(m_device);

    VkDescriptorPoolSize vk_pool_size = DescriptorPoolSize{ GetVk(descriptor_type), size };

    VkDescriptorPoolCreateInfo create_info = {

        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = {},
        .maxSets       = max_sets == 0 ? vk_pool_size.descriptorCount : max_sets,
        .poolSizeCount = 1,
        .pPoolSizes    = &vk_pool_size
    };

    return DescriptorPool::Create(m_device, create_info);
}

UniqueShaderModule Device::CreateShaderModule(const unsigned char* shader_data, size_t shader_size)
{
    assert(m_device);

    VkShaderModuleCreateInfo create_info = {

        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = {},
        .codeSize = shader_size,
        .pCode    = reinterpret_cast<const std::uint32_t*>(shader_data)
    };

    return ShaderModule::Create(m_device, create_info);
}

UniqueBuffer Device::CreateBuffer(std::size_t size, BufferUsage buffer_usage_flags, MemoryUsage memory_usage)
{
    assert(m_device);

    VkBufferCreateInfo create_info = {

        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .size                  = narrow_cast<VkDeviceSize>(size),
        .usage                 = GetVk(buffer_usage_flags),
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr
    };

    return Buffer::Create(m_allocator, create_info, memory_usage);
}

UniqueCommandPool Device::CreateCommandPool(uint32_t queue_family_index, CommandPoolCreate command_pool_create_flags)
{
    assert(m_device);

    auto create_info = VkCommandPoolCreateInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = GetVk(command_pool_create_flags),
        .queueFamilyIndex = queue_family_index
    };

    return CommandPool::Create(m_device, create_info);
}

UniqueFramebuffer Device::CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent)
{
    assert(m_device);

    VkImageView vk_image_view = image_view;

    VkFramebufferCreateInfo create_info = {

        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = {},
        .renderPass      = renderpass,
        .attachmentCount = 1,
        .pAttachments    = &vk_image_view,
        .width           = extent.width,
        .height          = extent.height,
        .layers          = 1
    };

    return Framebuffer::Create(m_device, create_info);
}

auto Device::CreateFramebuffer(
    RenderPass                               renderpass,
    std::initializer_list<const ImageView2D> image_views,
    Extent2D                                 extent) -> UniqueFramebuffer
{
    assert(m_device);

    auto vk_image_views = std::vector<VkImageView>(image_views.begin(), image_views.end());

    VkFramebufferCreateInfo create_info = {

        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = {},
        .renderPass      = renderpass,
        .attachmentCount = narrow_cast<uint32_t>(vk_image_views.size()),
        .pAttachments    = vk_image_views.data(),
        .width           = extent.width,
        .height          = extent.height,
        .layers          = 1
    };

    return Framebuffer::Create(m_device, create_info);
}

UniquePipeline Device::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info)
{
    assert(m_device);
    return Pipeline::Create(m_device, create_info);
}

UniquePipelineLayout Device::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info)
{
    return PipelineLayout::Create(m_device, create_info);
}

UniqueImage2D Device::CreateImage(
    Format      format,
    Extent2D    extent,
    ImageUsage  image_usage_flags,
    MemoryUsage memory_usage,
    ImageTiling image_tiling)
{
    assert(m_device);

    VkImageCreateInfo create_info = {

        .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .imageType             = VK_IMAGE_TYPE_2D,
        .format                = GetVk(format),
        .extent                = { extent.width, extent.height, 1 },
        .mipLevels             = 1,
        .arrayLayers           = 1,
        .samples               = VK_SAMPLE_COUNT_1_BIT,
        .tiling                = GetVk(image_tiling),
        .usage                 = GetVk(image_usage_flags),
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
    };

    return Image2D::Create(m_allocator, create_info, memory_usage);
}

UniqueImageView2D Device::CreateImageView(Image2D image, ImageAspect image_aspect_flags)
{
    assert(m_device);

    auto vk_aspect_flags = GetVk(image_aspect_flags);

    auto create_info = VkImageViewCreateInfo{

        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .image            = image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = GetVk(image.Format()),
        .components       = {},
        .subresourceRange = { vk_aspect_flags, 0, 1, 0, 1 }
    };

    return ImageView2D::Create(m_device, create_info);
}

Queue Device::GetQueue(uint32_t queue_family_index) const noexcept
{
    assert(m_device);

    VkQueue vk_queue{};

    vkGetDeviceQueue(m_device, queue_family_index, 0, &vk_queue);

    return Queue(vk_queue);
}

void Device::UpdateDescriptorSet(const WriteDescriptorSet& write_descriptor_set)
{
    assert(m_device);

    VkWriteDescriptorSet vk_write_descriptor_set = write_descriptor_set;

    vkUpdateDescriptorSets(m_device, 1, &vk_write_descriptor_set, 0, nullptr);
}

void Device::WaitIdle()
{
    assert(m_device);
    vkDeviceWaitIdle(m_device);
}

UniqueDevice
Device::Create(VkInstance instance, VkPhysicalDevice physical_device, const VkDeviceCreateInfo& create_info)
{
    VkDevice device{};

    if (auto result = vkCreateDevice(physical_device, &create_info, nullptr, &device); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateDevice error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkDevice {}", fmt::ptr(device));

    VmaAllocatorCreateInfo allocator_create_info = {

        .flags                       = {},
        .physicalDevice              = physical_device,
        .device                      = device,
        .preferredLargeHeapBlockSize = {},
        .pAllocationCallbacks        = nullptr,
        .pDeviceMemoryCallbacks      = nullptr,
        .frameInUseCount             = 0,
        .pHeapSizeLimit              = nullptr,
        .pVulkanFunctions            = nullptr,
        .pRecordSettings             = nullptr,
        .instance                    = instance,
        .vulkanApiVersion            = {}
    };

    VmaAllocator allocator{};
    if (auto result = vmaCreateAllocator(&allocator_create_info, &allocator); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vmaCreateAllocator error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VmaAllocator {}", fmt::ptr(allocator));

    return UniqueDevice(Device(device, allocator));
}

void Device::Destroy() noexcept
{
    assert(m_device);

    vmaDestroyAllocator(m_allocator);
    spdlog::info(COMPONENT "Destroyed VmaAllocator {}", fmt::ptr(m_allocator));

    vkDestroyDevice(m_device, nullptr);
    spdlog::info(COMPONENT "Destroyed VkDevice {}", fmt::ptr(m_device));

    m_device    = nullptr;
    m_allocator = nullptr;
}

Device::Builder::Builder() noexcept
{
    state = VkDeviceCreateInfo{

        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .queueCreateInfoCount    = 0,
        .pQueueCreateInfos       = nullptr,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures        = nullptr
    };
}

void Device::Builder::AddQueue(uint32_t queue_family_index, uint32_t queue_count)
{
    static const float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo device_queue_create_info = {

        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .queueFamilyIndex = queue_family_index,
        .queueCount       = queue_count,
        .pQueuePriorities = &queue_priority
    };

    m_device_queues.push_back(device_queue_create_info);

    state.queueCreateInfoCount = narrow_cast<uint32_t>(m_device_queues.size());
    state.pQueueCreateInfos    = m_device_queues.data();
}

void Device::Builder::AddEnabledLayer(const char* layer_name)
{
    m_enabled_layer_names.push_back(layer_name);

    state.enabledLayerCount   = narrow_cast<uint32_t>(m_enabled_layer_names.size());
    state.ppEnabledLayerNames = m_enabled_layer_names.data();
}

} // namespace etna
