#include "descriptor.hpp"
#include "buffer.hpp"

#include <cassert>

namespace etna {

UniqueDescriptorSetLayout DescriptorSetLayout::Create(
    VkDevice                               vk_device,
    const VkDescriptorSetLayoutCreateInfo& create_info)
{
    VkDescriptorSetLayout vk_descriptor_set_layout{};

    if (auto result = vkCreateDescriptorSetLayout(vk_device, &create_info, nullptr, &vk_descriptor_set_layout);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueDescriptorSetLayout(DescriptorSetLayout(vk_descriptor_set_layout, vk_device));
}

void DescriptorSetLayout::Destroy() noexcept
{
    assert(m_descriptor_set_layout);

    vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);

    m_descriptor_set_layout = nullptr;
    m_device                = nullptr;
}

DescriptorSetLayout::Builder::Builder() noexcept
{
    state = VkDescriptorSetLayoutCreateInfo{

        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = {},
        .bindingCount = 0,
        .pBindings    = nullptr
    };
}

void DescriptorSetLayout::Builder::AddDescriptorSetLayoutBinding(
    Binding        binding,
    DescriptorType descriptor_type,
    uint32_t       descriptor_count,
    ShaderStage    shader_stage_flags)
{
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {

        .binding            = binding,
        .descriptorType     = GetVk(descriptor_type),
        .descriptorCount    = descriptor_count,
        .stageFlags         = GetVk(shader_stage_flags),
        .pImmutableSamplers = nullptr
    };

    m_descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);

    state.bindingCount = narrow_cast<uint32_t>(m_descriptor_set_layout_bindings.size());
    state.pBindings    = m_descriptor_set_layout_bindings.data();
}

DescriptorSet DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout descriptor_set_layout)
{
    assert(m_descriptor_pool);

    VkDescriptorSetLayout vk_descriptor_set_layout = descriptor_set_layout;

    VkDescriptorSetAllocateInfo allocate_info = {

        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = m_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &vk_descriptor_set_layout
    };

    VkDescriptorSet descriptor_set{};
    vkAllocateDescriptorSets(m_device, &allocate_info, &descriptor_set);

    return descriptor_set;
}

std::vector<DescriptorSet> DescriptorPool::AllocateDescriptorSets(
    size_t              count,
    DescriptorSetLayout descriptor_set_layout)
{
    assert(m_descriptor_pool);

    std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts(count, descriptor_set_layout);

    VkDescriptorSetAllocateInfo allocate_info = {

        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = m_descriptor_pool,
        .descriptorSetCount = narrow_cast<uint32_t>(count),
        .pSetLayouts        = vk_descriptor_set_layouts.data()
    };

    std::vector<VkDescriptorSet> vk_descriptor_sets(count);
    vkAllocateDescriptorSets(m_device, &allocate_info, vk_descriptor_sets.data());

    return std::vector<DescriptorSet>(vk_descriptor_sets.begin(), vk_descriptor_sets.end());
}

void DescriptorPool::Destroy() noexcept
{
    assert(m_descriptor_pool);

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

    m_descriptor_pool = nullptr;
    m_device          = nullptr;
}

WriteDescriptorSet::WriteDescriptorSet(
    DescriptorSet  descriptor_set,
    Binding        binding,
    DescriptorType descriptor_type) noexcept
{
    state = VkWriteDescriptorSet{

        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext            = nullptr,
        .dstSet           = descriptor_set,
        .dstBinding       = binding,
        .dstArrayElement  = 0,
        .descriptorCount  = 0,
        .descriptorType   = GetVk(descriptor_type),
        .pImageInfo       = nullptr,
        .pBufferInfo      = nullptr,
        .pTexelBufferView = nullptr
    };
}

void WriteDescriptorSet::AddBuffer(Buffer buffer, size_t offset, size_t size)
{
    m_descriptor_buffer_infos.push_back({ buffer, offset, size });

    state.descriptorCount = narrow_cast<uint32_t>(m_descriptor_buffer_infos.size());
    state.pBufferInfo     = m_descriptor_buffer_infos.data();
}

} // namespace etna
