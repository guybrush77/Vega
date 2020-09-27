#include "etna/instance.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data)
{
    const auto severity = static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity);
    const auto type     = static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(message_type);

    switch (severity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
        spdlog::debug("{}: {}", vk::to_string(type), callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        spdlog::info("{}: {}", vk::to_string(type), callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        spdlog::warn("{}: {}", vk::to_string(type), callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        spdlog::error("{}: {}", vk::to_string(type), callback_data->pMessage);
        break;
    default:
        spdlog::warn("Vulkan message callback type not recognized");
        break;
    }

    return VK_FALSE;
}

class Context {
  public:
    static Context& Get()
    {
        static Context context;
        return context;
    }

    void SetDebugEnabled(VkInstance instance, bool debug_enabled)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_info_map[instance].is_debug_enabled = debug_enabled;
    }

    bool IsDebugEnabled(VkInstance instance)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_info_map[instance].is_debug_enabled;
    }

  private:
    struct Info {
        bool is_debug_enabled;
    };

    struct Hash {
        std::size_t operator()(VkInstance instance) { return reinterpret_cast<std::size_t>(instance); }
    };

    using info_map_t = std::unordered_map<VkInstance, Info>;

    std::mutex m_lock;
    info_map_t m_info_map;
};

static vk::DebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo()
{
    vk::DebugUtilsMessengerCreateInfoEXT create_info;
    {
        create_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                      //vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        create_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                  vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                  vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        create_info.pfnUserCallback = VulkanDebugCallback;

        create_info.pUserData = nullptr;
    }

    return create_info;
}

} // namespace

namespace etna {
bool AreExtensionsAvailable(std::span<const char*> extensions)
{
    auto available_extensions = vk::enumerateInstanceExtensionProperties();

    auto is_available = [&](std::string_view extension) {
        return std::ranges::count(available_extensions, extension, &vk::ExtensionProperties::extensionName);
    };

    return std::ranges::all_of(extensions, is_available);
}

bool AreLayersAvailable(std::span<const char*> layers)
{
    auto available_layers = vk::enumerateInstanceLayerProperties();

    auto is_available = [&](std::string_view layer) {
        return std::ranges::count(available_layers, layer, &vk::LayerProperties::layerName);
    };

    return std::ranges::all_of(layers, is_available);
}

vk::UniqueInstance CreateUniqueInstance(
    std::span<const char*> requested_extensions,
    std::span<const char*> requested_layers)
{
    if (false == AreExtensionsAvailable(requested_extensions))
        throw_runtime_error("Requested Vulkan extensions are not available");

    if (false == AreLayersAvailable(requested_layers)) {
        throw_runtime_error("Requested Vulkan layers are not available");
    }

    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = GetDebugUtilsMessengerCreateInfo();
    bool enable_debug = std::ranges::count(requested_extensions, std::string_view(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));

    vk::ApplicationInfo app_info;
    {
        app_info.pApplicationName   = "Vega";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app_info.apiVersion         = VK_API_VERSION_1_0;
    }

    vk::InstanceCreateInfo create_info;
    {
        create_info.pNext                   = enable_debug ? &debug_create_info : nullptr;
        create_info.pApplicationInfo        = &app_info;
        create_info.enabledLayerCount       = narrow_cast<uint32_t>(requested_layers.size());
        create_info.ppEnabledLayerNames     = requested_layers.data();
        create_info.enabledExtensionCount   = narrow_cast<uint32_t>(requested_extensions.size());
        create_info.ppEnabledExtensionNames = requested_extensions.data();
    }

    auto instance = vk::createInstanceUnique(create_info);

    Context::Get().SetDebugEnabled(instance.get(), enable_debug);

    spdlog::info(COMPONENT "Created VkInstance {}", instance.get());

    return instance;
}

UniqueDebugMessenger CreateUniqueDebugMessenger(vk::Instance instance)
{
    if (false == Context::Get().IsDebugEnabled(instance)) {
        spdlog::info(COMPONENT "Cannot create debug utils messenger. Required extension not found.");
        return {};
    }

    static auto dispatch_loader = std::make_unique<vk::DispatchLoaderDynamic>(instance, vkGetInstanceProcAddr);
    const auto  create_info     = GetDebugUtilsMessengerCreateInfo();

    auto debug_messenger = instance.createDebugUtilsMessengerEXTUnique(create_info, nullptr, *dispatch_loader);

    spdlog::info(COMPONENT "Created VkDebugUtilsMessengerEXT {}", debug_messenger.get());

    return debug_messenger;
}

} // namespace etna
