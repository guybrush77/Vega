#include "etna/shader.hpp"

#include "utils/resource.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

vk::UniqueShaderModule CreateUniqueShaderModule(vk::Device device, const char* shader_name)
{
    auto [shader_data, shader_size] = GetResource(shader_name);

    vk::ShaderModuleCreateInfo create_info;
    {
        create_info.codeSize = shader_size;
        create_info.pCode    = reinterpret_cast<const std::uint32_t*>(shader_data);
    }

    auto shader_module = device.createShaderModuleUnique(create_info);

    spdlog::info(COMPONENT "Created VkShaderModule {}", shader_module.get());

    return shader_module;
}

} // namespace etna
