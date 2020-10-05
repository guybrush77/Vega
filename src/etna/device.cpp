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
    uint32_t queue_count  = 0;
};

struct QueueIndices final {
    std::optional<QueueInfo> graphics;
    std::optional<QueueInfo> compute;
    std::optional<QueueInfo> transfer;
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

    std::vector<VkQueueFamilyProperties> available_families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, available_families.data());

    QueueIndices queue_indices;

    for (std::size_t i = 0; i != available_families.size(); ++i) {
        uint32_t family_index = narrow_cast<uint32_t>(i);
        uint32_t queue_count  = available_families[i].queueCount;
        if (false == queue_indices.graphics.has_value()) {
            if (available_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queue_indices.graphics = { family_index, queue_count };
            }
        }
        if (false == queue_indices.compute.has_value()) {
            if (available_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queue_indices.compute = { family_index, queue_count };
            }
        }
        if (false == queue_indices.transfer.has_value()) {
            if (available_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                queue_indices.transfer = { family_index, queue_count };
            }
        }
    }

    return queue_indices;
}

static auto GetDeviceQueueCreateInfos(const QueueIndices& queue_indices)
{
    static float queue_priority = 1.0f;

    if (false == queue_indices.graphics.has_value()) {
        throw_runtime_error("Failed to detect GPU graphics queue!");
    }

    VkDeviceQueueCreateInfo graphics_queue_create_info = {

        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = {},
        .queueFamilyIndex = queue_indices.graphics->family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority
    };

    std::array create_infos = { graphics_queue_create_info };
    return create_infos;
}

} // namespace

namespace etna {

UniqueRenderPass Device::CreateRenderPass(const VkRenderPassCreateInfo& create_info)
{
    assert(m_state);
    return RenderPass::Create(m_state->device, create_info);
}

auto Device::CreateShaderModule(const char* shader_name) -> UniqueShaderModule
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

auto Device::CreateCommandPool(QueueFamily queue_family, CommandPoolCreateMask command_pool_create_mask)
    -> UniqueCommandPool
{
    assert(m_state);

    uint32_t queue_family_index{};
    if (queue_family == QueueFamily::Graphics) {
        queue_family_index = m_state->indices.graphics->family_index;
    }

    auto create_info = VkCommandPoolCreateInfo{

        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = command_pool_create_mask.GetVkFlags(),
        .queueFamilyIndex = queue_family_index
    };

    return CommandPool::Create(m_state->device, create_info);
}

auto Device::CreateFramebuffer(RenderPass renderpass, ImageView2D image_view, Extent2D extent) -> UniqueFramebuffer
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

auto Device::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline
{
    assert(m_state);
    return Pipeline::Create(m_state->device, create_info);
}

auto Device::CreatePipelineLayout(const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout
{
    return PipelineLayout::Create(m_state->device, create_info);
}

auto Device::CreateImage(
    Format             format,
    Extent2D           extent,
    ImageUsageMask image_usage_mask,
    MemoryUsage        memory_usage,
    ImageTiling        image_tiling) -> UniqueImage2D
{
    assert(m_state);

    VkImageCreateInfo create_info = {

        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = GetVkFlags(format),
        .extent      = { extent.width, extent.height, 1 },
        .mipLevels   = 1,
        .arrayLayers = 1,
        .samples     = VK_SAMPLE_COUNT_1_BIT,
        .tiling      = GetVkFlags(image_tiling),
        .usage       = image_usage_mask.GetVkFlags()
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

auto Device::GetQueue(QueueFamily queue_family) const noexcept -> Queue
{
    assert(m_state);

    uint32_t queue_family_index = 0;

    switch (queue_family) {
    case QueueFamily::Graphics:
        queue_family_index = m_state->indices.graphics->family_index;
        break;
    default:
        throw_runtime_error("Device::GetQueue bad argument: queue family unrecognized");
        break;
    }

    VkQueue vk_queue{};

    vkGetDeviceQueue(m_state->device, queue_family_index, 0, &vk_queue);

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

} // namespace etna
