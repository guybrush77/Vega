#pragma once

#include "core.hpp"

namespace etna {

class ShaderModule {
  public:
    ShaderModule() noexcept {}
    ShaderModule(std::nullptr_t) noexcept {}

    operator VkShaderModule() const noexcept { return m_shader_module; }

    explicit operator bool() const noexcept { return m_shader_module != nullptr; }

    bool operator==(const ShaderModule& rhs) const noexcept { return m_shader_module == rhs.m_shader_module; }
    bool operator!=(const ShaderModule& rhs) const noexcept { return m_shader_module != rhs.m_shader_module; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    ShaderModule(VkShaderModule shader_module, VkDevice device) : m_shader_module(shader_module), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkShaderModuleCreateInfo& create_info) -> UniqueShaderModule;

    void Destroy() noexcept;

    VkShaderModule m_shader_module{};
    VkDevice       m_device{};
};

} // namespace etna
