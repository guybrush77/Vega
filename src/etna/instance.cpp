#include "etna/instance.hpp"
#include "etna/device.hpp"
#include "etna/surface.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string_view>

namespace etna {

UniqueInstance CreateInstance(
    const char*                          application_name,
    Version                              application_version,
    std::span<const char*>               extensions,
    std::span<const char*>               layers,
    PFN_vkDebugUtilsMessengerCallbackEXT debug_utils_messenger_callback,
    etna::DebugUtilsMessageSeverity      debug_utils_message_severity,
    etna::DebugUtilsMessageType          debug_utils_message_type)
{
    return Instance::Create(
        application_name,
        application_version,
        extensions,
        layers,
        debug_utils_messenger_callback,
        debug_utils_message_severity,
        debug_utils_message_type);
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

std::vector<PhysicalDevice> Instance::EnumeratePhysicalDevices() const
{
    assert(m_instance);

    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);

    if (count == 0) {
        return {};
    }

    std::vector<VkPhysicalDevice> vk_physical_devices(count);
    vkEnumeratePhysicalDevices(m_instance, &count, vk_physical_devices.data());

    std::vector<PhysicalDevice> physical_devices(count);
    for (uint32_t i = 0; i < count; ++i) {
        physical_devices[i] = vk_physical_devices[i];
    }

    return physical_devices;
}

UniqueDevice Instance::CreateDevice(PhysicalDevice physical_device, const VkDeviceCreateInfo& device_create_info)
{
    assert(m_instance);

    return Device::Create(m_instance, physical_device, device_create_info);
}

UniqueInstance Instance::Create(
    const char*                          application_name,
    Version                              application_version,
    std::span<const char*>               requested_extensions,
    std::span<const char*>               requested_layers,
    PFN_vkDebugUtilsMessengerCallbackEXT debug_utils_messenger_callback,
    DebugUtilsMessageSeverity            debug_utils_message_severity,
    DebugUtilsMessageType                debug_utils_message_type)
{
    if (false == AreExtensionsAvailable(requested_extensions)) {
        throw_etna_error(__FILE__, __LINE__, "Requested Vulkan extensions are not available");
    }

    if (false == AreLayersAvailable(requested_layers)) {
        throw_etna_error(__FILE__, __LINE__, "Requested Vulkan layers are not available");
    }

    auto debug_create_info = VkDebugUtilsMessengerCreateInfoEXT{

        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext           = nullptr,
        .flags           = {},
        .messageSeverity = VkEnum(debug_utils_message_severity),
        .messageType     = VkEnum(debug_utils_message_type),
        .pfnUserCallback = debug_utils_messenger_callback,
        .pUserData       = nullptr
    };

    bool enable_debug = std::ranges::count(requested_extensions, std::string_view(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));

    if (enable_debug) {
        if (debug_create_info.pfnUserCallback == nullptr) {
            throw_etna_error(__FILE__, __LINE__, "debug_utils_messenger_callback may not be null");
        }
        if (debug_utils_message_severity == DebugUtilsMessageSeverity{}) {
            debug_create_info.messageSeverity = VkEnum(
                DebugUtilsMessageSeverity::Verbose | DebugUtilsMessageSeverity::Info |
                DebugUtilsMessageSeverity::Warning | DebugUtilsMessageSeverity::Error);
        }
        if (debug_utils_message_type == DebugUtilsMessageType{}) {
            debug_create_info.messageType = VkEnum(
                DebugUtilsMessageType::General | DebugUtilsMessageType::Validation |
                DebugUtilsMessageType::Performance);
        }
    }

    auto vk_application_version =
        VK_MAKE_VERSION(application_version.major, application_version.minor, application_version.patch);

    VkApplicationInfo app_info = {

        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = application_name,
        .applicationVersion = vk_application_version,
        .pEngineName        = nullptr,
        .engineVersion      = {},
        .apiVersion         = VK_API_VERSION_1_1
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

    VkInstance vk_instance{};

    if (auto result = vkCreateInstance(&instance_create_info, nullptr, &vk_instance); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    VkDebugUtilsMessengerEXT vk_debug_messenger{};

    if (enable_debug) {
        using fn_type = PFN_vkCreateDebugUtilsMessengerEXT;
        auto* fn_name = "vkCreateDebugUtilsMessengerEXT";
        auto* fn_ptr  = (fn_type)vkGetInstanceProcAddr(vk_instance, fn_name);
        if (fn_ptr == nullptr) {
            throw_etna_error(__FILE__, __LINE__, Result::ErrorExtensionNotPresent);
        }
        if (auto result = fn_ptr(vk_instance, &debug_create_info, nullptr, &vk_debug_messenger); result != VK_SUCCESS) {
            throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
        }
    }

    return UniqueInstance(Instance(vk_instance, vk_debug_messenger));
}

void Instance::Destroy() noexcept
{
    assert(m_instance);

    if (m_debug_messenger) {
        auto* func_name = "vkDestroyDebugUtilsMessengerEXT";
        auto  func_ptr  = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, func_name);
        if (func_ptr != nullptr) {
            func_ptr(m_instance, m_debug_messenger, nullptr);
        }
    }

    vkDestroyInstance(m_instance, nullptr);

    m_instance        = nullptr;
    m_debug_messenger = nullptr;
}

PhysicalDeviceProperties PhysicalDevice::GetPhysicalDeviceProperties() const
{
    assert(m_physical_device);

    VkPhysicalDeviceProperties vk_properties{};

    vkGetPhysicalDeviceProperties(m_physical_device, &vk_properties);

    auto properties = PhysicalDeviceProperties{

        vk_properties.apiVersion,
        vk_properties.driverVersion,
        vk_properties.vendorID,
        vk_properties.deviceID,
        static_cast<PhysicalDeviceType>(vk_properties.deviceType),
        {},
        {},
        vk_properties.limits,
        vk_properties.sparseProperties
    };

    std::memcpy(properties.deviceName, vk_properties.deviceName, sizeof(properties.deviceName));
    std::memcpy(properties.pipelineCacheUUID, vk_properties.pipelineCacheUUID, sizeof(properties.pipelineCacheUUID));

    return properties;
}

FormatProperties PhysicalDevice::GetPhysicalDeviceFormatProperties(Format format) const
{
    assert(m_physical_device);

    VkFormat           vk_format = VkEnum(format);
    VkFormatProperties vk_format_properties{};

    vkGetPhysicalDeviceFormatProperties(m_physical_device, vk_format, &vk_format_properties);

    auto linear_tiling_features  = static_cast<FormatFeature>(vk_format_properties.linearTilingFeatures);
    auto optimal_tiling_features = static_cast<FormatFeature>(vk_format_properties.optimalTilingFeatures);
    auto buffer_features         = static_cast<FormatFeature>(vk_format_properties.bufferFeatures);

    return FormatProperties{ linear_tiling_features, optimal_tiling_features, buffer_features };
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

SurfaceCapabilitiesKHR PhysicalDevice::GetPhysicalDeviceSurfaceCapabilitiesKHR(SurfaceKHR surface) const
{
    VkSurfaceCapabilitiesKHR vk_capabilities{};

    if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &vk_capabilities);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return SurfaceCapabilitiesKHR{

        .minImageCount           = vk_capabilities.minImageCount,
        .maxImageCount           = vk_capabilities.maxImageCount,
        .currentExtent           = vk_capabilities.currentExtent,
        .minImageExtent          = vk_capabilities.minImageExtent,
        .maxImageExtent          = vk_capabilities.maxImageExtent,
        .maxImageArrayLayers     = vk_capabilities.maxImageArrayLayers,
        .supportedTransforms     = static_cast<SurfaceTransformKHR>(vk_capabilities.supportedTransforms),
        .currentTransform        = static_cast<SurfaceTransformKHR>(vk_capabilities.currentTransform),
        .supportedCompositeAlpha = static_cast<CompositeAlphaKHR>(vk_capabilities.supportedCompositeAlpha),
        .supportedUsageFlags     = static_cast<ImageUsage>(vk_capabilities.supportedUsageFlags)
    };
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

std::vector<PresentModeKHR> PhysicalDevice::GetPhysicalDeviceSurfacePresentModesKHR(SurfaceKHR surface) const
{
    assert(m_physical_device);

    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &count, nullptr);

    std::vector<VkPresentModeKHR> vk_present_modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &count, vk_present_modes.data());

    std::vector<PresentModeKHR> present_modes(count);

    for (size_t i = 0; i < count; ++i) {
        present_modes[i] = static_cast<PresentModeKHR>(vk_present_modes[i]);
    }

    return present_modes;
}

bool PhysicalDevice::GetPhysicalDeviceSurfaceSupportKHR(uint32_t queue_idx, SurfaceKHR surface) const
{
    assert(m_physical_device);

    VkBool32 is_supported{};

    if (auto result = vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, queue_idx, surface, &is_supported);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return is_supported;
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
