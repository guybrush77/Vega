#pragma once

#include "allocator.hpp"

#include <vulkan/vulkan.hpp>

namespace etna {

auto GetGraphicsQueueFamilyIndex(vk::Device device) -> uint32_t;

auto AllocateUniqueCommandBuffer(vk::Device             device,
                                 vk::CommandPool        command_pool,
                                 vk::CommandBufferLevel command_buffer_level = vk::CommandBufferLevel::ePrimary)
    -> vk::UniqueCommandBuffer;

auto CreateUniqueDevice(vk::Instance instance) -> vk::UniqueDevice;

auto CreateUniqueAllocator(vk::Device device) -> etna::UniqueAllocator;

} // namespace etna
