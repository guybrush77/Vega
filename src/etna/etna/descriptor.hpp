#pragma once

#include "types.hpp"

#include <vector>

ETNA_DEFINE_HANDLE(EtnaDescriptorSetLayout)
ETNA_DEFINE_HANDLE(EtnaDescriptorPool)

namespace etna {

struct DescriptorSetLayoutBuilder final {
    DescriptorSetLayoutBuilder() noexcept;

    constexpr operator VkDescriptorSetLayoutCreateInfo() const noexcept { return create_info; }

    void AddDescriptorSetLayoutBinding(
        Binding         binding,
        DescriptorType  descriptor_type,
        uint32_t        descriptor_count,
        ShaderStageMask shader_stage_mask);

    VkDescriptorSetLayoutCreateInfo create_info{};

  private:
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
};

class DescriptorSetLayout {
  public:
    DescriptorSetLayout() noexcept {}
    DescriptorSetLayout(std::nullptr_t) noexcept {}

    operator VkDescriptorSetLayout() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const DescriptorSetLayout& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const DescriptorSetLayout& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    DescriptorSetLayout(EtnaDescriptorSetLayout state) : m_state(state) {}

    static auto Create(VkDevice device, const VkDescriptorSetLayoutCreateInfo& create_info)
        -> UniqueDescriptorSetLayout;

    void Destroy() noexcept;

    EtnaDescriptorSetLayout m_state{};
};

class DescriptorSet {
  public:
    DescriptorSet() noexcept {}
    DescriptorSet(std::nullptr_t) noexcept {}
    DescriptorSet(VkDescriptorSet descriptor_set) noexcept : m_descriptor_set(descriptor_set) {}

    operator VkDescriptorSet() const noexcept { return m_descriptor_set; }

    explicit operator bool() const noexcept { return m_descriptor_set != nullptr; }

    bool operator==(const DescriptorSet& rhs) const noexcept { return m_descriptor_set == rhs.m_descriptor_set; }
    bool operator!=(const DescriptorSet& rhs) const noexcept { return m_descriptor_set != rhs.m_descriptor_set; }

  private:
    VkDescriptorSet m_descriptor_set{};
};

class WriteDescriptorSet {
  public:
    WriteDescriptorSet(DescriptorSet descriptor_set, Binding binding, DescriptorType descriptor_type) noexcept;

    operator VkWriteDescriptorSet() const noexcept { return state; }

    void AddBuffer(Buffer buffer, size_t offset = 0, size_t size = VK_WHOLE_SIZE);

    VkWriteDescriptorSet state{};

  private:
    std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos;
};

class DescriptorPool {
  public:
    DescriptorPool() noexcept {}
    DescriptorPool(std::nullptr_t) noexcept {}

    operator VkDescriptorPool() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const DescriptorPool& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const DescriptorPool& rhs) const noexcept { return m_state != rhs.m_state; }

    DescriptorSet AllocateDescriptorSet(DescriptorSetLayout descriptor_set_layout);

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    DescriptorPool(EtnaDescriptorPool state) : m_state(state) {}

    static auto Create(VkDevice device, const VkDescriptorPoolCreateInfo& create_info) -> UniqueDescriptorPool;

    void Destroy() noexcept;

    EtnaDescriptorPool m_state{};
};

struct PipelineLayoutBuilder final {
    PipelineLayoutBuilder() noexcept;

    constexpr operator VkPipelineLayoutCreateInfo() const noexcept { return create_info; }

    void AddDescriptorSetLayout(DescriptorSetLayout descriptor_set_layout);

    VkPipelineLayoutCreateInfo create_info{};

  private:
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
};

} // namespace etna
