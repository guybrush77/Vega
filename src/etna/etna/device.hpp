#pragma once

#include <vulkan/vulkan.hpp>

namespace etna {

auto CreateUniqueDevice(vk::Instance instance) -> vk::UniqueDevice;

} // namespace etna
