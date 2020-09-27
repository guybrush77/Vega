#include "etna/device.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

#include <optional>
#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct QueueFamilies {
    std::uint32_t graphics = 0;
};

static vk::PhysicalDevice GetGpu(vk::Instance instance)
{
    auto gpus = instance.enumeratePhysicalDevices();

    // TODO: pick the best device instead of picking the first available
    return gpus[0];
}

static QueueFamilies GetQueueFamilies(vk::PhysicalDevice gpu)
{
    std::optional<std::uint32_t> graphics_index;

    auto available_families = gpu.getQueueFamilyProperties();

    for (std::size_t i = 0; i != available_families.size(); ++i) {
        if (false == graphics_index.has_value()) {
            if (available_families[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphics_index = narrow_cast<std::uint32_t>(i);
            }
        }
    }
    if (false == graphics_index.has_value()) {
        throw_runtime_error("GPU does not support Vulkan graphics");
    }

    return { graphics_index.value() };
}

static auto GetDeviceQueueCreateInfos(vk::PhysicalDevice gpu)
{
    static float queue_priority = 1.0f;

    auto queue_families = GetQueueFamilies(gpu);

    vk::DeviceQueueCreateInfo graphics_create_info;
    {
        graphics_create_info.queueFamilyIndex = queue_families.graphics;
        graphics_create_info.queueCount       = 1;
        graphics_create_info.pQueuePriorities = &queue_priority;
    }

    std::array create_infos = { graphics_create_info };
    return create_infos;
}

} // namespace

namespace etna {

vk::UniqueDevice CreateUniqueDevice(vk::Instance instance)
{
    auto gpu                = GetGpu(instance);
    auto queue_create_infos = GetDeviceQueueCreateInfos(gpu);

    vk::DeviceCreateInfo create_info;
    {
        create_info.queueCreateInfoCount    = narrow_cast<std::uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos       = queue_create_infos.data();
        create_info.enabledLayerCount       = 0;
        create_info.ppEnabledLayerNames     = nullptr;
        create_info.enabledExtensionCount   = 0;
        create_info.ppEnabledExtensionNames = nullptr;
        create_info.pEnabledFeatures        = nullptr;
    }

    auto device = gpu.createDeviceUnique(create_info);

    spdlog::info(COMPONENT "Created VkDevice {}", device.get());

    return device;
}

} // namespace etna
