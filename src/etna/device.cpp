#include "etna/device.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

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

static vk::PhysicalDevice GetGpu(vk::Instance instance)
{
    auto gpus = instance.enumeratePhysicalDevices();

    // TODO: pick the best device instead of picking the first available
    return gpus[0];
}

static QueueIndices GetQueueIndices(vk::PhysicalDevice gpu)
{
    QueueIndices queue_indices;

    auto available_families = gpu.getQueueFamilyProperties();

    for (std::size_t i = 0; i != available_families.size(); ++i) {
        uint32_t family_index = narrow_cast<uint32_t>(i);
        uint32_t queue_count  = available_families[i].queueCount;
        if (false == queue_indices.graphics.has_value()) {
            if (available_families[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                queue_indices.graphics = { family_index, queue_count };
            }
        }
        if (false == queue_indices.compute.has_value()) {
            if (available_families[i].queueFlags & vk::QueueFlagBits::eCompute) {
                queue_indices.compute = { family_index, queue_count };
            }
        }
        if (false == queue_indices.transfer.has_value()) {
            if (available_families[i].queueFlags & vk::QueueFlagBits::eTransfer) {
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
        throw_runtime_error("GPU does not support Vulkan graphics");
    }

    vk::DeviceQueueCreateInfo graphics_create_info;
    {
        graphics_create_info.queueFamilyIndex = queue_indices.graphics->family_index;
        graphics_create_info.queueCount       = 1;
        graphics_create_info.pQueuePriorities = &queue_priority;
    }

    std::array create_infos = { graphics_create_info };
    return create_infos;
}

struct DeviceInfo final {
    vk::Instance       instance{};
    vk::PhysicalDevice gpu{};
    QueueIndices       queue_indices{};
};

class Context final {
  public:
    static Context& Instance()
    {
        static Context context;
        return context;
    }

    void Set(VkDevice device, DeviceInfo device_info) noexcept
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_info_map[device] = std::move(device_info);
    }

    DeviceInfo Get(VkDevice device) noexcept
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_info_map[device];
    }

  private:
    struct Hash {
        std::size_t operator()(VkDevice device) { return reinterpret_cast<std::size_t>(device); }
    };

    using info_map_t = std::unordered_map<VkDevice, DeviceInfo>;

    std::mutex m_lock;
    info_map_t m_info_map;
};

} // namespace

namespace etna {

auto GetGraphicsQueueFamilyIndex(vk::Device device) -> uint32_t
{
    return Context::Instance().Get(device).queue_indices.graphics->family_index;
}

auto AllocateUniqueCommandBuffer(vk::Device             device,
                                 vk::CommandPool        command_pool,
                                 vk::CommandBufferLevel command_buffer_level) -> vk::UniqueCommandBuffer
{
    vk::CommandBufferAllocateInfo allocate_info;
    {
        allocate_info.commandPool        = command_pool;
        allocate_info.level              = command_buffer_level;
        allocate_info.commandBufferCount = 1;
    }

    auto command_buffer = std::move(device.allocateCommandBuffersUnique(allocate_info).front());

    return command_buffer;
}

vk::UniqueDevice CreateUniqueDevice(vk::Instance instance)
{
    auto gpu                = GetGpu(instance);
    auto queue_indices      = GetQueueIndices(gpu);
    auto queue_create_infos = GetDeviceQueueCreateInfos(queue_indices);

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

    Context::Instance().Set(device.get(), { instance, gpu, std::move(queue_indices) });

    spdlog::info(COMPONENT "Created VkDevice {}", device.get());

    return device;
}

etna::UniqueAllocator CreateUniqueAllocator(vk::Device device)
{
    DeviceInfo device_info = Context::Instance().Get(device);

    return etna::UniqueAllocator(device_info.instance, device_info.gpu, device);
}

} // namespace etna
