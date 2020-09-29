#pragma once

#include <vulkan/vulkan.hpp>

namespace etna {

auto CreateUniqueShaderModule(vk::Device device, const char* shader_name) -> vk::UniqueShaderModule;

} // namespace etna
