#pragma once

#include <span>
#include <vulkan/vulkan.hpp>

namespace etna {

using UniqueDebugMessenger = vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;

bool AreExtensionsAvailable(std::span<const char*> extensions);

bool AreLayersAvailable(std::span<const char*> layers);

auto CreateUniqueInstance(std::span<const char*> extensions, std::span<const char*> layers) -> vk::UniqueInstance;

auto CreateUniqueDebugMessenger(vk::Instance instance) -> UniqueDebugMessenger;

} // namespace etna
