#pragma once

#include "core.hpp"

#include <span>
#include <vector>

ETNA_DEFINE_HANDLE(EtnaInstance)

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

    explicit operator bool() const noexcept { return m_physical_device != nullptr; }

    bool operator==(const PhysicalDevice& rhs) const noexcept { return m_physical_device == rhs.m_physical_device; }
    bool operator!=(const PhysicalDevice& rhs) const noexcept { return m_physical_device != rhs.m_physical_device; }

    // std::vector<QueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties() const;
    // std::vector<ExtensionProperties>   EnumerateDeviceExtensionProperties(const char* layer_name = nullptr) const;

  private:
    VkPhysicalDevice m_physical_device{};
};

class Instance {
  public:
    Instance() noexcept {}
    Instance(std::nullptr_t) noexcept {}

    operator VkInstance() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Instance& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Instance& rhs) const noexcept { return m_state != rhs.m_state; }

    std::vector<PhysicalDevice> EnumeratePhysicalDevices() const;

    UniqueDevice CreateDevice(PhysicalDevice physical_device);

  private:
    template <typename>
    friend class UniqueHandle;

    friend auto CreateInstance(
        const char*            application_name,
        Version                application_version,
        std::span<const char*> extensions,
        std::span<const char*> layers) -> UniqueInstance;

    Instance(EtnaInstance instance) : m_state(instance) {}

    static auto Create(
        const char*            application_name,
        Version                application_version,
        std::span<const char*> extensions,
        std::span<const char*> layers) -> UniqueInstance;

    void Destroy() noexcept;

    operator EtnaInstance() const noexcept { return m_state; }

    EtnaInstance m_state{};
};

bool AreExtensionsAvailable(std::span<const char*> extensions);

bool AreLayersAvailable(std::span<const char*> layers);

} // namespace etna
