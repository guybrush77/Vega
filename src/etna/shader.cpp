#include "etna/shader.hpp"

#include <cassert>

namespace etna {

UniqueShaderModule ShaderModule::Create(VkDevice vk_device, const VkShaderModuleCreateInfo& create_info)
{
    VkShaderModule vk_shader_module{};
    vkCreateShaderModule(vk_device, &create_info, nullptr, &vk_shader_module);

    return UniqueShaderModule(ShaderModule(vk_shader_module, vk_device));
}

void ShaderModule::Destroy() noexcept
{
    assert(m_shader_module);

    vkDestroyShaderModule(m_device, m_shader_module, nullptr);

    m_shader_module = nullptr;
    m_device        = nullptr;
}

} // namespace etna
