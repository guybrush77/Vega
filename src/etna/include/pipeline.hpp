#pragma once

#include "core.hpp"

#include <vector>

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

    operator VkPipelineLayout() const noexcept { return m_pipeline_layout; }

    explicit operator bool() const noexcept { return m_pipeline_layout != nullptr; }

    bool operator==(const PipelineLayout& rhs) const noexcept { return m_pipeline_layout == rhs.m_pipeline_layout; }
    bool operator!=(const PipelineLayout& rhs) const noexcept { return m_pipeline_layout != rhs.m_pipeline_layout; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    PipelineLayout(VkPipelineLayout pipeline_layout, VkDevice device) noexcept
        : m_pipeline_layout(pipeline_layout), m_device(device)
    {}

    static auto Create(VkDevice vk_device, const VkPipelineLayoutCreateInfo& create_info) -> UniquePipelineLayout;

    void Destroy() noexcept;

    VkPipelineLayout m_pipeline_layout{};
    VkDevice         m_device{};
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

        void AddDynamicStates(std::initializer_list<DynamicState> dynamic_states);

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

    operator VkPipeline() const noexcept { return m_pipeline; }

    explicit operator bool() const noexcept { return m_pipeline != nullptr; }

    bool operator==(const Pipeline& rhs) const noexcept { return m_pipeline == rhs.m_pipeline; }
    bool operator!=(const Pipeline& rhs) const noexcept { return m_pipeline != rhs.m_pipeline; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Pipeline(VkPipeline pipeline, VkDevice device) noexcept : m_pipeline(pipeline), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkGraphicsPipelineCreateInfo& create_info) -> UniquePipeline;

    void Destroy() noexcept;

    VkPipeline m_pipeline{};
    VkDevice   m_device{};
};

} // namespace etna
