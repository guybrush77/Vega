#pragma once

#include "core.hpp"

#include <span>

ETNA_DEFINE_HANDLE(EtnaInstance)

namespace etna {

struct Version {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;
};

class Device;
class Instance;

using UniqueDevice   = UniqueHandle<Device>;
using UniqueInstance = UniqueHandle<Instance>;

auto CreateInstance(
    const char*            application_name,
    Version                application_version,
    std::span<const char*> extensions,
    std::span<const char*> layers) -> UniqueInstance;

class Instance {
  public:
    Instance() noexcept {}
    Instance(std::nullptr_t) noexcept {}

    operator VkInstance() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Instance& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Instance& rhs) const noexcept { return m_state != rhs.m_state; }

    UniqueDevice CreateDevice();
    UniqueDevice CreateDevice(SurfaceKHR surface);

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
