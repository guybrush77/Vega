#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "etna/device.hpp"

#include "utils/casts.hpp"
#include "utils/resource.hpp"
#include "utils/throw_exception.hpp"

#include <array>
#include <optional>
#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct QueueInfo final {
    uint32_t family_index = 0;
    uint32_t queue_flags  = 0;
    uint32_t queue_count  = 0;
};

struct QueueIndices final {
    QueueInfo graphics;
    QueueInfo compute;
    QueueInfo transfer;
};

struct EtnaDevice_T final {
    VkDevice     device;
    QueueIndices indices;
    VmaAllocator allocator;
};

static VkPhysicalDevice GetGpu(VkInstance instance)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    if (count == 0) {
        throw_runtime_error("Failed to detect GPU!");
    }

    std::vector<VkPhysicalDevice> gpus(count);
    vkEnumeratePhysicalDevices(instance, &count, gpus.data());

    // TODO: pick the best device instead of picking the first available
    return gpus[0];
}

static QueueIndices GetQueueIndices(VkPhysicalDevice gpu)
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);

    if (count == 0) {
        throw_runtime_error("Failed to detect GPU device queues!");
    }

    std::vector<VkQueueFamilyProperties> properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, properties.data());

    const auto mask = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    std::optional<QueueInfo> graphics;

    std::optional<QueueInfo> compute;
    std::optional<QueueInfo> dedicated_compute;
    std::optional<QueueInfo> graphics_compute;
    std::optional<QueueInfo> mixed_compute;

    std::optional<QueueInfo> transfer;
    std::optional<QueueInfo> dedicated_transfer;
    std::optional<QueueInfo> graphics_transfer;
    std::optional<QueueInfo> mixed_transfer;

    for (std::size_t i = 0; i != properties.size(); ++i) {
        const auto family_index = narrow_cast<uint32_t>(i);
        const auto queue_flags  = properties[i].queueFlags;
        const auto queue_count  = properties[i].queueCount;
        const auto masked_flags = queue_flags & mask;
        const auto queue_info   = QueueInfo{ family_index, queue_flags, queue_count };

        if (masked_flags & VK_QUEUE_GRAPHICS_BIT) {
            if (!graphics.has_value() || queue_count > graphics->queue_count) {
                graphics = queue_info;
            }
        }

        if (masked_flags == VK_QUEUE_COMPUTE_BIT) {
            if (!dedicated_compute.has_value() || queue_count > dedicated_compute->queue_count) {
                dedicated_compute = queue_info;
            }
        } else if (masked_flags & VK_QUEUE_COMPUTE_BIT) {
            if (masked_flags & VK_QUEUE_GRAPHICS_BIT) {
                if (!graphics_compute.has_value() || queue_count > graphics_compute->queue_count) {
                    graphics_compute = queue_info;
                }
            } else {
                if (!mixed_compute.has_value() || queue_count > mixed_compute->queue_count) {
                    mixed_compute = queue_info;
                }
            }
        }

        if (masked_flags == VK_QUEUE_TRANSFER_BIT) {
            if (!dedicated_transfer.has_value() || queue_count > dedicated_transfer->queue_count) {
                dedicated_transfer = queue_info;
            }
        } else if (masked_flags & VK_QUEUE_TRANSFER_BIT) {
            if (masked_flags & VK_QUEUE_GRAPHICS_BIT) {
                if (!graphics_transfer.has_value() || queue_count > graphics_transfer->queue_count) {
                    graphics_transfer = queue_info;
                }
            } else {
                if (!mixed_transfer.has_value() || queue_count > mixed_transfer->queue_count) {
                    mixed_transfer = queue_info;
                }
            }
        }
    }

    if (false == graphics.has_value()) {
        throw_runtime_error("Failed to detect GPU graphics queue!");
    }

    if (dedicated_compute.has_value()) {
        compute = dedicated_compute;
    } else if (mixed_compute.has_value()) {
        compute = mixed_compute;
    } else if (graphics_compute.has_value()) {
        compute = graphics_compute;
    }

    if (false == compute.has_value()) {
        throw_runtime_error("Failed to detect GPU compute queue!");
    }

    if (dedicated_transfer.has_value()) {
        transfer = dedicated_transfer;
    } else if (mixed_transfer.has_value()) {
        transfer = mixed_transfer;
    } else if (graphics_transfer.has_value()) {
        transfer = graphics_transfer;
    }

    if (false == transfer.has_value()) {
        throw_runtime_error("Failed to detect GPU transfer queue!");
    }

    return { graphics.value(), compute.value(), transfer.value() };
}

static auto GetDeviceQueueCreateInfos(const QueueIndices& queue_indices)
{
    static const float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo graphics_queue_create_info = {

        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .queueFamilyIndex = queue_indices.graphics.family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority
    };

    VkDeviceQueueCreateInfo transfer_queue_create_info = {

        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .queueFamilyIndex = queue_indices.transfer.family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority
    };

    VkDeviceQueueCreateInfo compute_queue_create_info = {

        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .queueFamilyIndex = queue_indices.compute.family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority
    };

    std::array create_infos = { graphics_queue_create_info, transfer_queue_create_info, compute_queue_create_info };
    return create_infos;
}

} // namespace

namespace etna {

UniqueRenderPass Device::CreateRenderPass(const VkRenderPassCreateInfo& create_info)
{
    assert(m_state);
    return RenderPass::Create(m_state->device, create_info);
}

UniqueShaderModule Device::CreateShaderModule(const char* shader_name)
{
    assert(m_state);

    auto [shader_data, shader_size] = GetResource(shader_name);

    VkShaderModuleCreateInfo create_info = {

        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = {},
        .codeSize = shader_size,
        .pCode    = reinterpret_cast<const std::uint32_t*>(shader_data)
    };

    return ShaderModule::Create(m_state->device, create_info);
}

UniqueBuffer Device::CreateBuffer(std::size_t size, BufferUsageMask buffer_usage_mask, MemoryUsage memory_usage)
{
    assert(m_state);

    VkBufferCreateInfo create_info = {

        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .size                  = narrow_cast<VkDeviceSize>(size),
        .usage                 = GetVkFlags(buffer_usage_mask),
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr
    };

    return Buffer::Create(m_state->allocator, create_info, memory_usage);
}

UniqueCommandPool Device::CreateCommandPool(QueueFamily queue_family, CommandPoolCreateMask command_pool_create_mask)
{
    assert(m_state);

    auto create_info = VkCommandPoolCreateInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = GetVkFlags(command_pool_create_mask),
        .queueFamilyIndex = GetQueueFamilyIndex(queue_family)
    };

    return CommandPool::Create(m_state->device, create_info);
}

UniqueFramebuffer Device::CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent)
{
    assert(m_state);

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

    return Framebuffer::Create(m_state->device, create_info);
}

UniquePipeline Device::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info)
{
    assert(m_state);
    return Pipeline::Create(m_state->device, create_info);
}

UniquePipelineLayout Device::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info)
{
    return PipelineLayout::Create(m_state->device, create_info);
}

UniqueImage2D Device::CreateImage(
    Format         format,
    Extent2D       extent,
    ImageUsageMask image_usage_mask,
    MemoryUsage    memory_usage,
    ImageTiling    image_tiling)
{
    assert(m_state);

    VkImageCreateInfo create_info = {

        .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .imageType             = VK_IMAGE_TYPE_2D,
        .format                = GetVkFlags(format),
        .extent                = { extent.width, extent.height, 1 },
        .mipLevels             = 1,
        .arrayLayers           = 1,
        .samples               = VK_SAMPLE_COUNT_1_BIT,
        .tiling                = GetVkFlags(image_tiling),
        .usage                 = GetVkFlags(image_usage_mask),
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
    };

    return Image2D::Create(m_state->allocator, create_info, memory_usage);
}

UniqueImageView2D Device::CreateImageView(Image2D image)
{
    assert(m_state);

    VkImageAspectFlags aspect_mask{};

    if (image.UsageMask() & ImageUsage::ColorAttachment) {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    auto create_info = VkImageViewCreateInfo{

        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .image            = image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = GetVkFlags(image.Format()),
        .components       = {},
        .subresourceRange = { aspect_mask, 0, 1, 0, 1 }
    };

    return ImageView2D::Create(m_state->device, create_info);
}

Queue Device::GetQueue(QueueFamily queue_family) const noexcept
{
    assert(m_state);

    VkQueue vk_queue{};

    vkGetDeviceQueue(m_state->device, GetQueueFamilyIndex(queue_family), 0, &vk_queue);

    return Queue(vk_queue);
}

void Device::WaitIdle()
{
    assert(m_state);
    vkDeviceWaitIdle(m_state->device);
}

UniqueDevice Device::Create(VkInstance instance)
{
    auto gpu                = GetGpu(instance);
    auto queue_indices      = GetQueueIndices(gpu);
    auto queue_create_infos = GetDeviceQueueCreateInfos(queue_indices);

    VkDeviceCreateInfo device_create_info = {

        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .queueCreateInfoCount    = narrow_cast<std::uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos       = queue_create_infos.data(),
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = nullptr,
        .enabledExtensionCount   = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures        = nullptr
    };

    VkDevice device{};

    if (auto result = vkCreateDevice(gpu, &device_create_info, nullptr, &device); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateDevice error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkDevice {}", fmt::ptr(device));

    VmaAllocatorCreateInfo allocator_create_info = {

        .flags                       = {},
        .physicalDevice              = gpu,
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

    return UniqueDevice(new EtnaDevice_T{ device, queue_indices, allocator });
}

void Device::Destroy() noexcept
{
    assert(m_state);

    vmaDestroyAllocator(m_state->allocator);
    spdlog::info(COMPONENT "Destroyed VmaAllocator {}", fmt::ptr(m_state->allocator));

    vkDestroyDevice(m_state->device, nullptr);
    spdlog::info(COMPONENT "Destroyed VkDevice {}", fmt::ptr(m_state->device));

    delete m_state;
    m_state = nullptr;
}

uint32_t Device::GetQueueFamilyIndex(QueueFamily queue_family) const noexcept
{
    assert(m_state);

    uint32_t family_index{};

    switch (queue_family) {
    case QueueFamily::Graphics:
        family_index = m_state->indices.graphics.family_index;
        break;
    case QueueFamily::Compute:
        family_index = m_state->indices.compute.family_index;
        break;
    case QueueFamily::Transfer:
        family_index = m_state->indices.transfer.family_index;
        break;
    default:
        throw_runtime_error("Device::GetQueueFamilyIndex bad argument: queue family unrecognized");
        break;
    }

    return family_index;
}

} // namespace etna
