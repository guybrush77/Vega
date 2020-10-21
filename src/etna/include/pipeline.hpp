#pragma once

#include "core.hpp"

#include <vector>

ETNA_DEFINE_HANDLE(EtnaPipeline)
ETNA_DEFINE_HANDLE(EtnaPipelineLayout)

namespace etna {

class PipelineLayout {
  public:
    struct Builder final {
        Builder() noexcept;

        void AddDescriptorSetLayout(DescriptorSetLayout descriptor_set_layout);

        VkPipelineLayoutCreateInfo state{};

      private:
        std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    };

    PipelineLayout() noexcept {}
    PipelineLayout(std::nullptr_t) noexcept {}

    operator VkPipelineLayout() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const PipelineLayout& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const PipelineLayout& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    PipelineLayout(EtnaPipelineLayout state) : m_state(state) {}

    static auto Create(VkDevice device, const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout;

    void Destroy() noexcept;

    EtnaPipelineLayout m_state{};
};

class Pipeline {
  public:
    struct Builder final {
        Builder();
        Builder(PipelineLayout layout, RenderPass renderpass) noexcept;

        void
        AddShaderStage(ShaderModule shader_module, ShaderStage shader_stage_flags, const char* entry_function = "main");

        void AddVertexInputBindingDescription(
            Binding         binding,
            size_t          stride,
            VertexInputRate vertex_input_rate = VertexInputRate::Vertex);

        void AddVertexInputAttributeDescription(Location location, Binding binding, Format format, size_t offset);

        void AddViewport(Viewport viewport);

        void AddScissor(Rect2D scissor);

        // void AddColorBlendAttachmentState();// TODO

        void AddColorBlendAttachmentState();

        void AddDynamicState(DynamicState dynamic_state);

        void SetDepthState(DepthTest depth_test, DepthWrite depth_write, CompareOp compare_op) noexcept;

        VkGraphicsPipelineCreateInfo state{};

      private:
        std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
        VkPipelineVertexInputStateCreateInfo         m_vertex_input_state;
        VkPipelineInputAssemblyStateCreateInfo       m_input_assembly_state;
        VkPipelineTessellationStateCreateInfo        m_tessellation_state;
        VkPipelineViewportStateCreateInfo            m_viewport_state;
        VkPipelineRasterizationStateCreateInfo       m_rasterization_state;
        VkPipelineMultisampleStateCreateInfo         m_multisample_state;
        VkPipelineDepthStencilStateCreateInfo        m_depth_stencil_state;
        VkPipelineColorBlendStateCreateInfo          m_color_blend_state;
        VkPipelineDynamicStateCreateInfo             m_dynamic_state;

        std::vector<VkVertexInputBindingDescription>     m_binding_descriptions;
        std::vector<VkVertexInputAttributeDescription>   m_attribute_descriptions;
        std::vector<VkViewport>                          m_viewports;
        std::vector<VkRect2D>                            m_scissors;
        std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachments;
        std::vector<VkDynamicState>                      m_dynamic_states;
    };

    Pipeline() noexcept {}
    Pipeline(std::nullptr_t) noexcept {}

    operator VkPipeline() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const Pipeline& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const Pipeline& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Pipeline(EtnaPipeline state) : m_state(state) {}

    static auto Create(VkDevice device, const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline;

    void Destroy() noexcept;

    EtnaPipeline m_state{};
};

} // namespace etna
