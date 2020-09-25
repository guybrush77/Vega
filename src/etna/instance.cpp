#include "etna/instance.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

#include <algorithm>

namespace etna {

bool AreExtensionsAvailable(std::span<cstring> extensions)
{
    auto available_extensions = vk::enumerateInstanceExtensionProperties();

    auto available = [&available_extensions](std::string_view extension) {
        return std::ranges::count(available_extensions, extension, &vk::ExtensionProperties::extensionName);
    };

    return std::ranges::all_of(extensions, available);
}

bool AreLayersAvailable(std::span<cstring> layers)
{
    auto available_layers = vk::enumerateInstanceLayerProperties();

    auto available = [&available_layers](std::string_view layer) {
        return std::ranges::count(available_layers, layer, &vk::LayerProperties::layerName);
    };

    return std::ranges::all_of(layers, available);
}

vk::UniqueInstance CreateInstance(std::span<cstring> requested_extensions, std::span<cstring> requested_layers)
{
    if (false == AreExtensionsAvailable(requested_extensions)) {
        throw_runtime_error("Requested Vulkan extensions are not available");
    }

    if (false == AreLayersAvailable(requested_layers)) {
        throw_runtime_error("Requested Vulkan layers are not available");
    }

    vk::ApplicationInfo app_info;
    {
        app_info.pApplicationName   = "Vega";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app_info.apiVersion         = VK_API_VERSION_1_0;
    }

    vk::InstanceCreateInfo create_info;
    {
        create_info.pApplicationInfo        = &app_info;
        create_info.enabledLayerCount       = narrow_cast<uint32_t>(requested_layers.size());
        create_info.ppEnabledLayerNames     = requested_layers.data();
        create_info.enabledExtensionCount   = narrow_cast<uint32_t>(requested_extensions.size());
        create_info.ppEnabledExtensionNames = requested_extensions.data();
    }

    return vk::createInstanceUnique(create_info);
}

} // namespace etna
