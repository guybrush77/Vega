#include "etna/pipeline.hpp"

#include "utils/casts.hpp"

#define COMPONENT "Etna: "

namespace {} // namespace

namespace etna {

vk::UniquePipelineLayout CreateUniquePipelineLayout(vk::Device device)
{
    vk::PipelineLayoutCreateInfo create_info;
    return device.createPipelineLayoutUnique(create_info);
}

auto GetRenderPassBeginInfo(
    vk::RenderPass  renderpass,
    vk::Rect2D      render_area,
    vk::Framebuffer framebuffer,
    vk::Image       image) -> vk::RenderPassBeginInfo
{
    static const vk::ClearValue clear_value;

    vk::RenderPassBeginInfo begin_info;
    {
        begin_info.renderPass      = renderpass;
        begin_info.framebuffer     = framebuffer;
        begin_info.renderArea      = render_area;
        begin_info.clearValueCount = 1;
        begin_info.pClearValues    = &clear_value;
    }

    return begin_info;
}

AttachmentID RenderPassBuilder::AddAttachment(
    vk::Format            format,
    vk::AttachmentLoadOp  load_op,
    vk::AttachmentStoreOp store_op,
    vk::ImageLayout       initial_layout,
    vk::ImageLayout       final_layout)
{
    vk::AttachmentDescription description;
    {
        description.format         = format;
        description.samples        = vk::SampleCountFlagBits::e1;
        description.loadOp         = load_op;
        description.storeOp        = store_op;
        description.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        description.initialLayout  = initial_layout;
        description.finalLayout    = final_layout;
    }
    m_attachment_descriptions.push_back(description);

    create_info.attachmentCount = static_cast<std::uint32_t>(m_attachment_descriptions.size());
    create_info.pAttachments    = m_attachment_descriptions.data();

    return static_cast<std::uint32_t>(m_attachment_descriptions.size() - 1);
}

ReferenceID RenderPassBuilder::AddReference(AttachmentID attachment_id, vk::ImageLayout image_layout)
{
    m_references.push_back({ attachment_id, image_layout });
    return m_references.size() - 1;
}

void RenderPassBuilder::AddSubpass(std::span<const ReferenceID> reference_ids)
{
    std::vector<vk::AttachmentReference> references;
    for (auto reference_id : reference_ids)
        references.push_back(m_references[reference_id]);

    m_subpass_references.push_front(std::move(references));

    vk::SubpassDescription subpass_description;
    {
        subpass_description.colorAttachmentCount = narrow_cast<std::uint32_t>(m_subpass_references.front().size());
        subpass_description.pColorAttachments    = m_subpass_references.front().data();
    }
    m_subpass_descriptions.push_back(subpass_description);

    create_info.subpassCount = narrow_cast<std::uint32_t>(m_subpass_descriptions.size());
    create_info.pSubpasses   = m_subpass_descriptions.data();
}

void RenderPassBuilder::AddSubpass(std::initializer_list<ReferenceID> reference_ids)
{
    AddSubpass(std::span(reference_ids.begin(), reference_ids.size()));
}

PipelineBuilder::PipelineBuilder()
{
    m_input_assembly_state.topology               = vk::PrimitiveTopology::eTriangleList;
    m_input_assembly_state.primitiveRestartEnable = false;

    m_rasterization_state.depthClampEnable        = false;
    m_rasterization_state.rasterizerDiscardEnable = false;
    m_rasterization_state.polygonMode             = vk::PolygonMode::eFill;
    m_rasterization_state.cullMode                = vk::CullModeFlagBits::eNone;
    m_rasterization_state.frontFace               = vk::FrontFace::eClockwise;
    m_rasterization_state.depthBiasEnable         = false;
    m_rasterization_state.depthBiasConstantFactor = 0.0f;
    m_rasterization_state.depthBiasClamp          = 0.0f;
    m_rasterization_state.depthBiasSlopeFactor    = 0.0f;
    m_rasterization_state.lineWidth               = 1.0f;

    m_multisample_state.rasterizationSamples  = vk::SampleCountFlagBits::e1;
    m_multisample_state.sampleShadingEnable   = false;
    m_multisample_state.minSampleShading      = 1.0f;
    m_multisample_state.pSampleMask           = nullptr;
    m_multisample_state.alphaToCoverageEnable = false;
    m_multisample_state.alphaToOneEnable      = false;

    m_depth_stencil_state.depthTestEnable       = false;
    m_depth_stencil_state.depthWriteEnable      = false;
    m_depth_stencil_state.depthCompareOp        = vk::CompareOp::eLess;
    m_depth_stencil_state.depthBoundsTestEnable = false;
    m_depth_stencil_state.stencilTestEnable     = false;
    m_depth_stencil_state.front                 = vk::StencilOpState{};
    m_depth_stencil_state.back                  = vk::StencilOpState{};
    m_depth_stencil_state.minDepthBounds        = 0.0f;
    m_depth_stencil_state.maxDepthBounds        = 0.0f;

    m_color_blend_state.logicOpEnable     = false;
    m_color_blend_state.logicOp           = vk::LogicOp::eClear;
    m_color_blend_state.attachmentCount   = 0;
    m_color_blend_state.pAttachments      = nullptr;
    m_color_blend_state.blendConstants[0] = 0.0f;
    m_color_blend_state.blendConstants[1] = 0.0f;
    m_color_blend_state.blendConstants[2] = 0.0f;
    m_color_blend_state.blendConstants[3] = 0.0f;

    create_info.flags               = {};
    create_info.stageCount          = 0;
    create_info.pStages             = nullptr;
    create_info.pVertexInputState   = &m_vertex_input_state;
    create_info.pInputAssemblyState = &m_input_assembly_state;
    create_info.pTessellationState  = &m_tessellation_state;
    create_info.pViewportState      = &m_viewport_state;
    create_info.pRasterizationState = &m_rasterization_state;
    create_info.pMultisampleState   = &m_multisample_state;
    create_info.pDepthStencilState  = &m_depth_stencil_state;
    create_info.pColorBlendState    = &m_color_blend_state;
    create_info.pDynamicState       = &m_dynamic_state;
    create_info.basePipelineHandle  = nullptr;
    create_info.basePipelineIndex   = -1;
}

PipelineBuilder::PipelineBuilder(vk::PipelineLayout layout, vk::RenderPass renderpass) noexcept : PipelineBuilder()
{
    create_info.layout     = layout;
    create_info.renderPass = renderpass;
}

void PipelineBuilder::AddShaderStage(vk::ShaderModule module, vk::ShaderStageFlagBits stage, const char* entry_function)
{
    m_shader_stages.push_back({ {}, stage, module, entry_function });

    create_info.pStages    = m_shader_stages.data();
    create_info.stageCount = narrow_cast<std::uint32_t>(m_shader_stages.size());
}

void PipelineBuilder::AddViewport(vk::Viewport viewport)
{
    m_viewports.push_back(viewport);

    m_viewport_state.viewportCount = narrow_cast<std::uint32_t>(m_viewports.size());
    m_viewport_state.pViewports    = m_viewports.data();
}

void PipelineBuilder::AddScissor(vk::Rect2D scissor)
{
    m_scissors.push_back(scissor);

    m_viewport_state.pScissors    = m_scissors.data();
    m_viewport_state.scissorCount = narrow_cast<std::uint32_t>(m_scissors.size());
}

void PipelineBuilder::AddColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState& state)
{
    m_color_blend_attachments.push_back(state);

    m_color_blend_state.pAttachments    = m_color_blend_attachments.data();
    m_color_blend_state.attachmentCount = narrow_cast<std::uint32_t>(m_color_blend_attachments.size());
}

void PipelineBuilder::AddColorBlendAttachmentBaseState()
{
    vk::PipelineColorBlendAttachmentState state;
    state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    AddColorBlendAttachmentState(state);
}

void PipelineBuilder::AddDynamicState(vk::DynamicState dynamic_state)
{
    m_dynamic_states.push_back(dynamic_state);

    m_dynamic_state.pDynamicStates    = m_dynamic_states.data();
    m_dynamic_state.dynamicStateCount = narrow_cast<std::uint32_t>(m_dynamic_states.size());
}

} // namespace etna
