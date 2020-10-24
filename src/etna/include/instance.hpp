#pragma once

#include "core.hpp"

#include <span>
#include <vector>

namespace etna {

struct Version {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
};

auto CreateInstance(
    const char*            application_name,
    Version                application_version,
    std::span<const char*> extensions,
    std::span<const char*> layers) -> UniqueInstance;

class PhysicalDevice {
  public:
    PhysicalDevice() noexcept {}
    PhysicalDevice(std::nullptr_t) noexcept {}
    PhysicalDevice(VkPhysicalDevice physical_device) noexcept : m_physical_device(physical_device) {}

    operator VkPhysicalDevice() const noexcept { return m_physical_device; }

    bool operator==(const PhysicalDevice&) const = default;

    FormatProperties                   GetPhysicalDeviceFormatProperties(Format format) const;
    std::vector<QueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties() const;
    SurfaceCapabilitiesKHR             GetPhysicalDeviceSurfaceCapabilitiesKHR(SurfaceKHR surface) const;
    std::vector<SurfaceFormatKHR>      GetPhysicalDeviceSurfaceFormatsKHR(SurfaceKHR surface) const;
    std::vector<PresentModeKHR>        GetPhysicalDeviceSurfacePresentModesKHR(SurfaceKHR surface) const;
    bool                               GetPhysicalDeviceSurfaceSupportKHR(uint32_t queue_idx, SurfaceKHR surface) const;
    std::vector<ExtensionProperties>   EnumerateDeviceExtensionProperties(const char* layer_name = nullptr) const;

  private:
    VkPhysicalDevice m_physical_device{};
};

class Instance {
  public:
    Instance() noexcept {}
    Instance(std::nullptr_t) noexcept {}

    operator VkInstance() const noexcept { return m_instance; }

    bool operator==(const Instance& rhs) const = default;

    std::vector<PhysicalDevice> EnumeratePhysicalDevices() const;

    UniqueDevice CreateDevice(PhysicalDevice physical_device, const VkDeviceCreateInfo& device_create_info);

  private:
    template <typename>
    friend class UniqueHandle;

    friend auto CreateInstance(
        const char*            application_name,
        Version                application_version,
        std::span<const char*> extensions,
        std::span<const char*> layers) -> UniqueInstance;

    Instance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
        : m_instance(instance), m_debug_messenger(debug_messenger)
    {}

    static auto Create(
        const char*            application_name,
        Version                application_version,
        std::span<const char*> extensions,
        std::span<const char*> layers) -> UniqueInstance;

    void Destroy() noexcept;

    VkInstance               m_instance{};
    VkDebugUtilsMessengerEXT m_debug_messenger{};
};

bool AreExtensionsAvailable(std::span<const char*> extensions);

bool AreLayersAvailable(std::span<const char*> layers);

} // namespace etna
