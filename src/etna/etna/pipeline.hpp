#pragma once

#include <forward_list>
#include <span>
#include <vulkan/vulkan.hpp>

namespace etna {

auto CreateUniquePipelineLayout(vk::Device device) -> vk::UniquePipelineLayout;

auto GetRenderPassBeginInfo(
    vk::RenderPass  renderpass,
    vk::Rect2D      render_area,
    vk::Framebuffer framebuffer,
    vk::Image       image) -> vk::RenderPassBeginInfo;

using AttachmentID = std::uint32_t;
using ReferenceID  = std::size_t;

struct RenderPassBuilder final {
    AttachmentID AddAttachment(
        vk::Format            format,
        vk::AttachmentLoadOp  load_op,
        vk::AttachmentStoreOp store_op,
        vk::ImageLayout       initial_layout,
        vk::ImageLayout       final_layout);

    ReferenceID AddReference(AttachmentID attachment_id, vk::ImageLayout image_layout);

    void AddSubpass(std::span<const ReferenceID> reference_ids);

    void AddSubpass(std::initializer_list<ReferenceID> reference_ids);

    vk::RenderPassCreateInfo create_info;

  private:
    std::vector<vk::AttachmentDescription>                  m_attachment_descriptions;
    std::vector<vk::AttachmentReference>                    m_references;
    std::forward_list<std::vector<vk::AttachmentReference>> m_subpass_references;
    std::vector<vk::SubpassDescription>                     m_subpass_descriptions;
};

struct PipelineBuilder final {
    PipelineBuilder();
    PipelineBuilder(vk::PipelineLayout layout, vk::RenderPass renderpass) noexcept;

    void AddShaderStage(vk::ShaderModule module, vk::ShaderStageFlagBits stage, const char* entry_function = "main");

    void AddViewport(vk::Viewport viewport);

    void AddScissor(vk::Rect2D scissor);

    void AddColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState& state);

    void AddColorBlendAttachmentBaseState();

    void AddDynamicState(vk::DynamicState dynamic_state);

    vk::GraphicsPipelineCreateInfo create_info;

  private:
    std::vector<vk::PipelineShaderStageCreateInfo> m_shader_stages;
    vk::PipelineVertexInputStateCreateInfo         m_vertex_input_state;
    vk::PipelineInputAssemblyStateCreateInfo       m_input_assembly_state;
    vk::PipelineTessellationStateCreateInfo        m_tessellation_state;
    vk::PipelineViewportStateCreateInfo            m_viewport_state;
    vk::PipelineRasterizationStateCreateInfo       m_rasterization_state;
    vk::PipelineMultisampleStateCreateInfo         m_multisample_state;
    vk::PipelineDepthStencilStateCreateInfo        m_depth_stencil_state;
    vk::PipelineColorBlendStateCreateInfo          m_color_blend_state;
    vk::PipelineDynamicStateCreateInfo             m_dynamic_state;

    std::vector<vk::Viewport>                          m_viewports;
    std::vector<vk::Rect2D>                            m_scissors;
    std::vector<vk::PipelineColorBlendAttachmentState> m_color_blend_attachments;
    std::vector<vk::DynamicState>                      m_dynamic_states;
};

} // namespace etna
