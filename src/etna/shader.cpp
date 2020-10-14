#include "shader.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaShaderModule_T final {
    VkShaderModule shader_module;
    VkDevice       device;
};

} // namespace

namespace etna {

ShaderModule::operator VkShaderModule() const noexcept
{
    return m_state ? m_state->shader_module : VkShaderModule{};
}

UniqueShaderModule ShaderModule::Create(VkDevice device, const VkShaderModuleCreateInfo& create_info)
{
    VkShaderModule shader_module{};
    vkCreateShaderModule(device, &create_info, nullptr, &shader_module);

    spdlog::info(COMPONENT "Created VkShaderModule {}", fmt::ptr(shader_module));

    return UniqueShaderModule(new EtnaShaderModule_T{ shader_module, device });
}

void ShaderModule::Destroy() noexcept
{
    assert(m_state);
    vkDestroyShaderModule(m_state->device, m_state->shader_module, nullptr);

    spdlog::info(COMPONENT "Destroyed VkShaderModule {}", fmt::ptr(m_state->shader_module));

    delete m_state;

    m_state = nullptr;
}

} // namespace etna
