#include "etna/pipeline.hpp"
#include "etna/descriptor.hpp"
#include "etna/renderpass.hpp"
#include "etna/shader.hpp"

#include <cassert>

namespace etna {

PipelineLayout::Builder::Builder() noexcept
{
    state = VkPipelineLayoutCreateInfo{

        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = {},
        .setLayoutCount         = 0,
        .pSetLayouts            = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };
}

void PipelineLayout::Builder::AddDescriptorSetLayout(DescriptorSetLayout descriptor_set_layout)
{
    m_descriptor_set_layouts.push_back(descriptor_set_layout);

    state.setLayoutCount = narrow_cast<uint32_t>(m_descriptor_set_layouts.size());
    state.pSetLayouts    = m_descriptor_set_layouts.data();
}

Pipeline::Builder::Builder()
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

        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = {},
        .depthTestEnable       = false,
        .depthWriteEnable      = false,
        .depthCompareOp        = VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = false,
        .stencilTestEnable     = false,
        .front                 = {},
        .back                  = {},
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

    state = VkGraphicsPipelineCreateInfo{

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

Pipeline::Builder::Builder(PipelineLayout layout, RenderPass renderpass) noexcept : Builder()
{
    state.layout     = layout;
    state.renderPass = renderpass;
}

void Pipeline::Builder::AddShaderStage(
    ShaderModule shader_module,
    ShaderStage  shader_stage_flags,
    const char*  entry_function)
{
    VkPipelineShaderStageCreateInfo shader_stage_create_info = {

        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = {},
        .stage               = static_cast<VkShaderStageFlagBits>(VkEnum(shader_stage_flags)),
        .module              = shader_module,
        .pName               = entry_function,
        .pSpecializationInfo = nullptr
    };

    m_shader_stages.push_back(shader_stage_create_info);

    state.pStages    = m_shader_stages.data();
    state.stageCount = narrow_cast<uint32_t>(m_shader_stages.size());
}

void Pipeline::Builder::AddVertexInputBindingDescription(
    Binding         binding,
    size_t          stride,
    VertexInputRate vertex_input_rate)
{
    m_binding_descriptions.push_back({ binding, narrow_cast<uint32_t>(stride), VkEnum(vertex_input_rate) });

    m_vertex_input_state.vertexBindingDescriptionCount = narrow_cast<uint32_t>(m_binding_descriptions.size());
    m_vertex_input_state.pVertexBindingDescriptions    = m_binding_descriptions.data();
}

void Pipeline::Builder::AddVertexInputAttributeDescription(
    Location location,
    Binding  binding,
    Format   format,
    size_t   offset)
{
    m_attribute_descriptions.push_back({ location, binding, VkEnum(format), narrow_cast<uint32_t>(offset) });

    m_vertex_input_state.vertexAttributeDescriptionCount = narrow_cast<uint32_t>(m_attribute_descriptions.size());
    m_vertex_input_state.pVertexAttributeDescriptions    = m_attribute_descriptions.data();
}

void Pipeline::Builder::AddViewport(Viewport viewport)
{
    m_viewports.push_back(viewport);

    m_viewport_state.viewportCount = narrow_cast<uint32_t>(m_viewports.size());
    m_viewport_state.pViewports    = m_viewports.data();
}

void Pipeline::Builder::AddScissor(Rect2D scissor)
{
    m_scissors.push_back(scissor);

    m_viewport_state.pScissors    = m_scissors.data();
    m_viewport_state.scissorCount = narrow_cast<uint32_t>(m_scissors.size());
}

// TODO
/*
void Pipeline::Builder::AddColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState& state)
{
    m_color_blend_attachments.push_back(state);

    m_color_blend_state.pAttachments    = m_color_blend_attachments.data();
    m_color_blend_state.attachmentCount = narrow_cast<uint32_t>(m_color_blend_attachments.size());
}
*/

void Pipeline::Builder::AddColorBlendAttachmentState()
{
    VkColorComponentFlags write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT;

    auto color_blend_attachment_state = VkPipelineColorBlendAttachmentState{

        .blendEnable         = false,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = write_mask
    };

    m_color_blend_attachments.push_back(color_blend_attachment_state);

    m_color_blend_state.pAttachments    = m_color_blend_attachments.data();
    m_color_blend_state.attachmentCount = narrow_cast<uint32_t>(m_color_blend_attachments.size());
}

void Pipeline::Builder::AddDynamicStates(std::initializer_list<DynamicState> dynamic_states)
{
    auto vk_dynamic_states = reinterpret_cast<const VkDynamicState*>(dynamic_states.begin());
    auto vk_states_size    = narrow_cast<uint32_t>(dynamic_states.size());

    m_dynamic_states.insert(m_dynamic_states.end(), vk_dynamic_states, vk_dynamic_states + vk_states_size);
}

void Pipeline::Builder::SetDepthState(DepthTest depth_test, DepthWrite depth_write, CompareOp compare_op) noexcept
{
    m_depth_stencil_state.depthTestEnable  = depth_test == DepthTest::Enable;
    m_depth_stencil_state.depthWriteEnable = depth_write == DepthWrite::Enable;
    m_depth_stencil_state.depthCompareOp   = VkEnum(compare_op);
}

UniquePipelineLayout PipelineLayout::Create(VkDevice vk_device, const VkPipelineLayoutCreateInfo& create_info)
{
    VkPipelineLayout vk_pipeline_layout{};

    if (auto result = vkCreatePipelineLayout(vk_device, &create_info, nullptr, &vk_pipeline_layout);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniquePipelineLayout(PipelineLayout(vk_pipeline_layout, vk_device));
}

void PipelineLayout::Destroy() noexcept
{
    assert(m_pipeline_layout);

    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);

    m_pipeline_layout = nullptr;
    m_device          = nullptr;
}

UniquePipeline Pipeline::Create(VkDevice vk_device, const VkGraphicsPipelineCreateInfo& create_info)
{
    VkPipeline vk_pipeline{};

    if (auto result = vkCreateGraphicsPipelines(vk_device, {}, 1, &create_info, nullptr, &vk_pipeline);
        result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniquePipeline(Pipeline(vk_pipeline, vk_device));
}

void Pipeline::Destroy() noexcept
{
    assert(m_pipeline);

    vkDestroyPipeline(m_device, m_pipeline, nullptr);

    m_pipeline = nullptr;
    m_device   = nullptr;
}

} // namespace etna
