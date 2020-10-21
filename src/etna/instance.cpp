#include "instance.hpp"
#include "device.hpp"
#include "surface.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaInstance_T final {
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debug_messenger;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data)
{
    const char* type_string = nullptr;

    switch (message_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        type_string = "General";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        type_string = "Validation";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        type_string = "Performance";
        break;
    default:
        type_string = "Unknown";
        break;
    }

    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::debug("{}: {}", type_string, callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info("{}: {}", type_string, callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn("{}: {}", type_string, callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error("{}: {}", type_string, callback_data->pMessage);
        break;
    default:
        spdlog::warn("Vulkan message callback type not recognized");
        spdlog::error("{}: {}", type_string, callback_data->pMessage);
        break;
    }

    return VK_FALSE;
}

static VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerCreateInfo() noexcept
{
    uint32_t message_severity_mask = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    uint32_t message_type_mask = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    return VkDebugUtilsMessengerCreateInfoEXT{

        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext           = nullptr,
        .flags           = {},
        .messageSeverity = message_severity_mask,
        .messageType     = message_type_mask,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData       = nullptr
    };
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance                         instance,
    VkDebugUtilsMessengerCreateInfoEXT create_info,
    VkDebugUtilsMessengerEXT*          debug_messenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, &create_info, nullptr, debug_messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

} // namespace

namespace etna {

UniqueInstance CreateInstance(
    const char*            application_name,
    Version                application_version,
    std::span<const char*> extensions,
    std::span<const char*> layers)
{
    return Instance::Create(application_name, application_version, extensions, layers);
}

bool AreExtensionsAvailable(std::span<const char*> extensions)
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(count);

    vkEnumerateInstanceExtensionProperties(nullptr, &count, available_extensions.data());

    auto is_available = [&](std::string_view extension) {
        return std::ranges::count(available_extensions, extension, &VkExtensionProperties::extensionName);
    };

    return std::ranges::all_of(extensions, is_available);
}

bool AreLayersAvailable(std::span<const char*> layers)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    std::vector<VkLayerProperties> available_layers(count);

    vkEnumerateInstanceLayerProperties(&count, available_layers.data());

    auto is_available = [&](std::string_view layer) {
        return std::ranges::count(available_layers, layer, &VkLayerProperties::layerName);
    };

    return std::ranges::all_of(layers, is_available);
}

Instance::operator VkInstance() const noexcept
{
    return m_state ? m_state->instance : VkInstance{};
}

#include <algorithm>

std::vector<PhysicalDevice> Instance::EnumeratePhysicalDevices() const
{
    assert(m_state);

    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_state->instance, &count, nullptr);

    if (count == 0) {
        throw_runtime_error("Failed to detect GPU!");
    }

    std::vector<VkPhysicalDevice> vk_physical_devices(count);
    vkEnumeratePhysicalDevices(m_state->instance, &count, vk_physical_devices.data());

    std::vector<PhysicalDevice> physical_devices(count);
    for (uint32_t i = 0; i < count; ++i) {
        physical_devices[i] = vk_physical_devices[i];
    }

    return physical_devices;
}

UniqueDevice Instance::CreateDevice(PhysicalDevice physical_device, const VkDeviceCreateInfo& device_create_info)
{
    assert(m_state);
    return Device::Create(m_state->instance, physical_device, device_create_info);
}

UniqueInstance Instance::Create(
    const char*            app_name,
    Version                app_version,
    std::span<const char*> requested_extensions,
    std::span<const char*> requested_layers)
{
    if (false == AreExtensionsAvailable(requested_extensions)) {
        throw_runtime_error("Requested Vulkan extensions are not available");
    }

    if (false == AreLayersAvailable(requested_layers)) {
        throw_runtime_error("Requested Vulkan layers are not available");
    }

    auto debug_create_info = GetDebugUtilsMessengerCreateInfo();

    bool enable_debug = std::ranges::count(requested_extensions, std::string_view(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));

    VkApplicationInfo app_info = {

        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = app_name,
        .applicationVersion = VK_MAKE_VERSION(app_version.major, app_version.minor, app_version.patch),
        .pEngineName        = nullptr,
        .engineVersion      = {},
        .apiVersion         = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo instance_create_info = {

        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = enable_debug ? &debug_create_info : nullptr,
        .flags                   = {},
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = narrow_cast<uint32_t>(requested_layers.size()),
        .ppEnabledLayerNames     = requested_layers.data(),
        .enabledExtensionCount   = narrow_cast<uint32_t>(requested_extensions.size()),
        .ppEnabledExtensionNames = requested_extensions.data()
    };

    VkInstance instance{};

    if (auto result = vkCreateInstance(&instance_create_info, nullptr, &instance); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateInstance error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkInstance {}", fmt::ptr(instance));

    VkDebugUtilsMessengerEXT debug_messenger{};

    if (enable_debug) {
        auto result = CreateDebugUtilsMessengerEXT(instance, debug_create_info, &debug_messenger);
        if (result != VK_SUCCESS) {
            throw_runtime_error(fmt::format("vkCreateDebugUtilsMessengerEXT error: {}", result).c_str());
        }
        spdlog::info(COMPONENT "Created VkDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger));
    }

    return UniqueInstance(new EtnaInstance_T{ instance, debug_messenger });
}

void Instance::Destroy() noexcept
{
    assert(m_state);

    auto [instance, debug_messenger] = *m_state;

    if (debug_messenger) {
        auto* func_name = "vkDestroyDebugUtilsMessengerEXT";
        auto  func_ptr  = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, func_name);
        if (func_ptr != nullptr) {
            func_ptr(instance, debug_messenger, nullptr);
            spdlog::info(COMPONENT "Destroyed vkDestroyDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger));
        }
    }

    vkDestroyInstance(instance, nullptr);

    spdlog::info(COMPONENT "Destroyed VkInstance {}", fmt::ptr(instance));

    delete m_state;

    m_state = nullptr;
}

std::vector<QueueFamilyProperties> PhysicalDevice::GetPhysicalDeviceQueueFamilyProperties() const
{
    assert(m_physical_device);

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> vk_properties(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &count, vk_properties.data());

    std::vector<QueueFamilyProperties> properties(count);

    for (size_t i = 0; i < count; ++i) {
        properties[i].queueFlags                  = static_cast<QueueFlags>(vk_properties[i].queueFlags);
        properties[i].queueCount                  = vk_properties[i].queueCount;
        properties[i].timestampValidBits          = vk_properties[i].timestampValidBits;
        properties[i].minImageTransferGranularity = vk_properties[i].minImageTransferGranularity;
    }
    return properties;
}

std::vector<SurfaceFormatKHR> PhysicalDevice::GetPhysicalDeviceSurfaceFormatsKHR(SurfaceKHR surface) const
{
    assert(m_physical_device);

    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> vk_formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &count, vk_formats.data());

    std::vector<SurfaceFormatKHR> formats(count);

    for (size_t i = 0; i < count; ++i) {
        formats[i].format     = static_cast<Format>(vk_formats[i].format);
        formats[i].colorSpace = static_cast<ColorSpaceKHR>(vk_formats[i].colorSpace);
    }

    return formats;
}

std::vector<ExtensionProperties> PhysicalDevice::EnumerateDeviceExtensionProperties(const char* layer_name) const
{
    assert(m_physical_device);

    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(m_physical_device, layer_name, &count, nullptr);

    std::vector<ExtensionProperties> properties(count);
    vkEnumerateDeviceExtensionProperties(m_physical_device, layer_name, &count, properties.data());

    return properties;
}

} // namespace etna
