#pragma once

#include "types.hpp"

ETNA_DEFINE_HANDLE(EtnaShaderModule)

namespace etna {

class ShaderModule {
  public:
    ShaderModule() noexcept {}
    ShaderModule(std::nullptr_t) noexcept {}

    operator VkShaderModule() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const ShaderModule& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const ShaderModule& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    ShaderModule(EtnaShaderModule state) noexcept : m_state(state) {}

    static auto Create(VkDevice device, const VkShaderModuleCreateInfo& create_info) -> UniqueShaderModule;

    void Destroy() noexcept;

    EtnaShaderModule m_state{};
};

} // namespace etna
