#pragma once

#include "core.hpp"

#include <vector>

namespace etna {

class DescriptorSetLayout {
  public:
    struct Builder final {
        Builder() noexcept;

        void AddDescriptorSetLayoutBinding(
            Binding        binding,
            DescriptorType descriptor_type,
            uint32_t       descriptor_count,
            ShaderStage    shader_stage_flags);

        VkDescriptorSetLayoutCreateInfo state{};

      private:
        std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings;
    };

    DescriptorSetLayout() noexcept {}
    DescriptorSetLayout(std::nullptr_t) noexcept {}

    operator VkDescriptorSetLayout() const noexcept { return m_descriptor_set_layout; }

    bool operator==(const DescriptorSetLayout&) const = default;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    DescriptorSetLayout(VkDescriptorSetLayout descriptor_set_layout, VkDevice device) noexcept
        : m_descriptor_set_layout(descriptor_set_layout), m_device(device)
    {}

    static auto Create(VkDevice vk_device, const VkDescriptorSetLayoutCreateInfo& create_info)
        -> UniqueDescriptorSetLayout;

    void Destroy() noexcept;

    VkDescriptorSetLayout m_descriptor_set_layout{};
    VkDevice              m_device{};
};

class DescriptorSet {
  public:
    DescriptorSet() noexcept {}
    DescriptorSet(std::nullptr_t) noexcept {}
    DescriptorSet(VkDescriptorSet descriptor_set) noexcept : m_descriptor_set(descriptor_set) {}

    operator VkDescriptorSet() const noexcept { return m_descriptor_set; }

    bool operator==(const DescriptorSet&) const = default;

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
    DescriptorPool(VkDescriptorPool descriptor_pool, VkDevice device)
        : m_descriptor_pool(descriptor_pool), m_device(device)
    {}

    operator VkDescriptorPool() const noexcept { return m_descriptor_pool; }

    bool operator==(const DescriptorPool& rhs) const = default;

    auto AllocateDescriptorSet(DescriptorSetLayout descriptor_set_layout) -> DescriptorSet;
    auto AllocateDescriptorSets(size_t count, DescriptorSetLayout descriptor_set_layout) -> std::vector<DescriptorSet>;

  private:
    template <typename>
    friend class UniqueHandle;

    void Destroy() noexcept;

    VkDescriptorPool m_descriptor_pool{};
    VkDevice         m_device{};
};

} // namespace etna
