#include "etna/pipeline.hpp"

#include "utils/casts.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaPipelineLayout_T final {
    VkPipelineLayout pipeline_layout;
    VkDevice         device;
};

struct EtnaPipeline_T final {
    VkPipeline pipeline;
    VkDevice   device;
};

} // namespace

namespace etna {

PipelineBuilder::PipelineBuilder()
{
    m_vertex_input_state = VkPipelineVertexInputStateCreateInfo{

        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = {},
        .vertexBindingDescriptionCount   = 0,
        .pVertexBindingDescriptions      = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions    = nullptr
    };

    m_input_assembly_state = VkPipelineInputAssemblyStateCreateInfo{

        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = {},
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = false
    };

    m_tessellation_state = VkPipelineTessellationStateCreateInfo{

        .sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = {},
        .patchControlPoints = 0
    };

    m_viewport_state = VkPipelineViewportStateCreateInfo{

        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = {},
        .viewportCount = 0,
        .pViewports    = nullptr,
        .scissorCount  = 0,
        .pScissors     = nullptr
    };

    m_rasterization_state = VkPipelineRasterizationStateCreateInfo{

        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .depthClampEnable        = false,
        .rasterizerDiscardEnable = false,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = false,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = 1.0f
    };

    m_multisample_state = VkPipelineMultisampleStateCreateInfo{

        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = false,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable      = false
    };

    m_depth_stencil_state = VkPipelineDepthStencilStateCreateInfo{

        .sType                 = {},
        .pNext                 = nullptr,
        .flags                 = {},
        .depthTestEnable       = false,
        .depthWriteEnable      = false,
        .depthCompareOp        = VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = false,
        .stencilTestEnable     = false,
        .front                 = VK_STENCIL_OP_KEEP,
        .back                  = VK_STENCIL_OP_KEEP,
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 0.0f
    };

    m_color_blend_state = VkPipelineColorBlendStateCreateInfo{

        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = {},
        .logicOpEnable   = false,
        .logicOp         = VK_LOGIC_OP_CLEAR,
        .attachmentCount = 0,
        .pAttachments    = nullptr,
        .blendConstants  = {}
    };

    m_dynamic_state = VkPipelineDynamicStateCreateInfo{

        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = {},
        .dynamicStateCount = 0,
        .pDynamicStates    = nullptr
    };

    create_info = VkGraphicsPipelineCreateInfo{

        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = {},
        .stageCount          = 0,
        .pStages             = nullptr,
        .pVertexInputState   = &m_vertex_input_state,
        .pInputAssemblyState = &m_input_assembly_state,
        .pTessellationState  = &m_tessellation_state,
        .pViewportState      = &m_viewport_state,
        .pRasterizationState = &m_rasterization_state,
        .pMultisampleState   = &m_multisample_state,
        .pDepthStencilState  = &m_depth_stencil_state,
        .pColorBlendState    = &m_color_blend_state,
        .pDynamicState       = &m_dynamic_state,
        .layout              = {},
        .renderPass          = {},
        .subpass             = {},
        .basePipelineHandle  = {},
        .basePipelineIndex   = -1
    };
}

PipelineBuilder::PipelineBuilder(PipelineLayout layout, RenderPass renderpass) noexcept : PipelineBuilder()
{
    create_info.layout     = layout;
    create_info.renderPass = renderpass;
}

void PipelineBuilder::AddShaderStage(
    ShaderModule    shader_module,
    ShaderStageMask shader_stage_mask,
    const char*     entry_function)
{
    VkPipelineShaderStageCreateInfo shader_stage_create_info = {

        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = {},
        .stage               = static_cast<VkShaderStageFlagBits>(GetVkFlags(shader_stage_mask)),
        .module              = shader_module,
        .pName               = entry_function,
        .pSpecializationInfo = nullptr
    };

    m_shader_stages.push_back(shader_stage_create_info);

    create_info.pStages    = m_shader_stages.data();
    create_info.stageCount = narrow_cast<std::uint32_t>(m_shader_stages.size());
}

void PipelineBuilder::AddVertexInputBindingDescription(
    Binding         binding,
    size_t          stride,
    VertexInputRate vertex_input_rate)
{
    m_binding_descriptions.push_back({ binding, narrow_cast<uint32_t>(stride), GetVkFlags(vertex_input_rate) });

    m_vertex_input_state.vertexBindingDescriptionCount = narrow_cast<uint32_t>(m_binding_descriptions.size());
    m_vertex_input_state.pVertexBindingDescriptions    = m_binding_descriptions.data();
}

void PipelineBuilder::AddVertexInputAttributeDescription(
    Location location,
    Binding  binding,
    Format   format,
    size_t   offset)
{
    m_attribute_descriptions.push_back({ location, binding, GetVkFlags(format), narrow_cast<uint32_t>(offset) });

    m_vertex_input_state.vertexAttributeDescriptionCount = narrow_cast<uint32_t>(m_attribute_descriptions.size());
    m_vertex_input_state.pVertexAttributeDescriptions    = m_attribute_descriptions.data();
}

void PipelineBuilder::AddViewport(Viewport viewport)
{
    m_viewports.push_back(viewport);

    m_viewport_state.viewportCount = narrow_cast<std::uint32_t>(m_viewports.size());
    m_viewport_state.pViewports    = m_viewports.data();
}

void PipelineBuilder::AddScissor(Rect2D scissor)
{
    m_scissors.push_back(scissor);

    m_viewport_state.pScissors    = m_scissors.data();
    m_viewport_state.scissorCount = narrow_cast<std::uint32_t>(m_scissors.size());
}

// TODO
/*
void PipelineBuilder::AddColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState& state)
{
    m_color_blend_attachments.push_back(state);

    m_color_blend_state.pAttachments    = m_color_blend_attachments.data();
    m_color_blend_state.attachmentCount = narrow_cast<std::uint32_t>(m_color_blend_attachments.size());
}
*/

void PipelineBuilder::AddColorBlendAttachmentBaseState()
{
    VkColorComponentFlags write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT;

    auto state = VkPipelineColorBlendAttachmentState{

        .blendEnable         = false,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = write_mask
    };

    m_color_blend_attachments.push_back(state);

    m_color_blend_state.pAttachments    = m_color_blend_attachments.data();
    m_color_blend_state.attachmentCount = narrow_cast<std::uint32_t>(m_color_blend_attachments.size());
}

void PipelineBuilder::AddDynamicState(DynamicState dynamic_state)
{
    m_dynamic_states.push_back(GetVkFlags(dynamic_state));

    m_dynamic_state.pDynamicStates    = m_dynamic_states.data();
    m_dynamic_state.dynamicStateCount = narrow_cast<std::uint32_t>(m_dynamic_states.size());
}

PipelineLayout::operator VkPipelineLayout() const noexcept
{
    return m_state ? m_state->pipeline_layout : VkPipelineLayout{};
}

UniquePipelineLayout PipelineLayout::Create(VkDevice device, const VkPipelineLayoutCreateInfo& create_info)
{
    VkPipelineLayout pipeline_layout{};

    if (auto result = vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreatePipelineLayout error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));

    return UniquePipelineLayout(new EtnaPipelineLayout_T{ pipeline_layout, device });
}

void PipelineLayout::Destroy() noexcept
{
    assert(m_state);

    vkDestroyPipelineLayout(m_state->device, m_state->pipeline_layout, nullptr);

    spdlog::info(COMPONENT "Destroyed VkPipelineLayout {}", fmt::ptr(m_state->pipeline_layout));

    delete m_state;

    m_state = nullptr;
}

Pipeline::operator VkPipeline() const noexcept
{
    return m_state ? m_state->pipeline : VkPipeline{};
}

UniquePipeline Pipeline::Create(VkDevice device, const VkGraphicsPipelineCreateInfo& create_info)
{
    VkPipeline pipeline{};

    if (auto res = vkCreateGraphicsPipelines(device, {}, 1, &create_info, nullptr, &pipeline); res != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateGraphicsPipelines error: {}", res).c_str());
    }

    spdlog::info(COMPONENT "Created VkPipeline {}", fmt::ptr(pipeline));

    return UniquePipeline(new EtnaPipeline_T{ pipeline, device });
}

void Pipeline::Destroy() noexcept
{
    assert(m_state);

    vkDestroyPipeline(m_state->device, m_state->pipeline, nullptr);

    spdlog::info(COMPONENT "Destroyed VkPipeline {}", fmt::ptr(m_state->pipeline));

    delete m_state;

    m_state = nullptr;
}

} // namespace etna
