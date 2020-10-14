#include "descriptor.hpp"
#include "buffer.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaDescriptorSetLayout_T final {
    VkDescriptorSetLayout descriptor_set_layout;
    VkDevice              device;
};

struct EtnaDescriptorPool_T final {
    VkDescriptorPool descriptor_pool;
    VkDevice         device;
};

} // namespace

namespace etna {

DescriptorSetLayout::operator VkDescriptorSetLayout() const noexcept
{
    return m_state ? m_state->descriptor_set_layout : VkDescriptorSetLayout{};
}

UniqueDescriptorSetLayout DescriptorSetLayout::Create(
    VkDevice                               device,
    const VkDescriptorSetLayoutCreateInfo& create_info)
{
    VkDescriptorSetLayout descriptor_set_layout{};

    if (auto result = vkCreateDescriptorSetLayout(device, &create_info, nullptr, &descriptor_set_layout);
        result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateDescriptorSetLayout error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkDescriptorSetLayout {}", fmt::ptr(descriptor_set_layout));

    return UniqueDescriptorSetLayout(new EtnaDescriptorSetLayout_T{ descriptor_set_layout, device });
}

void DescriptorSetLayout::Destroy() noexcept
{
    assert(m_state);

    vkDestroyDescriptorSetLayout(m_state->device, m_state->descriptor_set_layout, nullptr);

    spdlog::info(COMPONENT "Destroyed VkDescriptorSetLayout {}", fmt::ptr(m_state->descriptor_set_layout));

    delete m_state;

    m_state = nullptr;
}

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder() noexcept
{
    create_info = VkDescriptorSetLayoutCreateInfo{

        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = {},
        .bindingCount = 0,
        .pBindings    = nullptr
    };
}

void DescriptorSetLayoutBuilder::AddDescriptorSetLayoutBinding(
    Binding         binding,
    DescriptorType  descriptor_type,
    uint32_t        descriptor_count,
    ShaderStageMask shader_stage_mask)
{
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding = {

        .binding            = binding,
        .descriptorType     = GetVkFlags(descriptor_type),
        .descriptorCount    = descriptor_count,
        .stageFlags         = GetVkFlags(shader_stage_mask),
        .pImmutableSamplers = nullptr
    };

    m_descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);

    create_info.bindingCount = narrow_cast<uint32_t>(m_descriptor_set_layout_bindings.size());
    create_info.pBindings    = m_descriptor_set_layout_bindings.data();
}

DescriptorPool::operator VkDescriptorPool() const noexcept
{
    return m_state ? m_state->descriptor_pool : VkDescriptorPool{};
}

DescriptorSet DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout descriptor_set_layout)
{
    assert(m_state);

    VkDescriptorSetLayout vk_descriptor_set_layout = descriptor_set_layout;

    VkDescriptorSetAllocateInfo allocate_info = {

        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = m_state->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &vk_descriptor_set_layout
    };

    VkDescriptorSet descriptor_set{};
    vkAllocateDescriptorSets(m_state->device, &allocate_info, &descriptor_set);

    return descriptor_set;
}

UniqueDescriptorPool DescriptorPool::Create(VkDevice device, const VkDescriptorPoolCreateInfo& create_info)
{
    VkDescriptorPool descriptor_pool{};

    if (auto result = vkCreateDescriptorPool(device, &create_info, nullptr, &descriptor_pool); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateDescriptorPool error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkDescriptorPool {}", fmt::ptr(descriptor_pool));

    return UniqueDescriptorPool(new EtnaDescriptorPool_T{ descriptor_pool, device });
}

void DescriptorPool::Destroy() noexcept
{
    assert(m_state);

    vkDestroyDescriptorPool(m_state->device, m_state->descriptor_pool, nullptr);

    spdlog::info(COMPONENT "Destroyed VkDescriptorPool {}", fmt::ptr(m_state->descriptor_pool));

    delete m_state;

    m_state = nullptr;
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
        .descriptorType   = GetVkFlags(descriptor_type),
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

PipelineLayoutBuilder::PipelineLayoutBuilder() noexcept
{
    create_info = VkPipelineLayoutCreateInfo{

        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = {},
        .setLayoutCount         = 0,
        .pSetLayouts            = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };
}

void PipelineLayoutBuilder::AddDescriptorSetLayout(DescriptorSetLayout descriptor_set_layout)
{
    m_descriptor_set_layouts.push_back(descriptor_set_layout);

    create_info.setLayoutCount = narrow_cast<uint32_t>(m_descriptor_set_layouts.size());
    create_info.pSetLayouts    = m_descriptor_set_layouts.data();
}

} // namespace etna
