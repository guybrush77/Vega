#pragma once

#include <span>
#include <vulkan/vulkan.hpp>

namespace etna {

using cstring = const char*;

bool AreExtensionsAvailable(std::span<cstring> extensions);

bool AreLayersAvailable(std::span<cstring> layers);

vk::UniqueInstance CreateInstance(std::span<cstring> requested_extensions, std::span<cstring> requested_layers);

} // namespace etna
