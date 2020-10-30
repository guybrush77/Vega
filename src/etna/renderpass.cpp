#include "renderpass.hpp"

#include <cassert>

namespace etna {

UniqueRenderPass RenderPass::Create(VkDevice vk_device, const VkRenderPassCreateInfo& create_info)
{
    VkRenderPass vk_renderpass{};

    if (auto result = vkCreateRenderPass(vk_device, &create_info, nullptr, &vk_renderpass); result != VK_SUCCESS) {
        throw_etna_error(__FILE__, __LINE__, static_cast<Result>(result));
    }

    return UniqueRenderPass(RenderPass(vk_renderpass, vk_device));
}

void RenderPass::Destroy() noexcept
{
    assert(m_renderpass);

    vkDestroyRenderPass(m_device, m_renderpass, nullptr);

    m_renderpass = nullptr;
    m_device     = nullptr;
}

RenderPass::Builder::Builder() noexcept
{
    state = VkRenderPassCreateInfo{

        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext           = nullptr,
        .flags           = {},
        .attachmentCount = 0,
        .pAttachments    = nullptr,
        .subpassCount    = 0,
        .pSubpasses      = nullptr,
        .dependencyCount = 0,
        .pDependencies   = nullptr
    };
}

RenderPass::SubpassBuilder RenderPass::Builder::GetSubpassBuilder()
{
    return SubpassBuilder(this);
}

AttachmentID RenderPass::Builder::AddAttachmentDescription(
    Format            format,
    AttachmentLoadOp  load_op,
    AttachmentStoreOp store_op,
    ImageLayout       initial_layout,
    ImageLayout       final_layout)
{
    VkAttachmentDescription description = {

        .flags          = {},
        .format         = GetVk(format),
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = GetVk(load_op),
        .storeOp        = GetVk(store_op),
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = GetVk(initial_layout),
        .finalLayout    = GetVk(final_layout)
    };

    m_attachment_descriptions.push_back(description);

    state.attachmentCount = narrow_cast<uint32_t>(m_attachment_descriptions.size());
    state.pAttachments    = m_attachment_descriptions.data();

    return AttachmentID(m_attachment_descriptions.size() - 1);
}

ReferenceID RenderPass::Builder::AddAttachmentReference(AttachmentID attachment_id, ImageLayout image_layout)
{
    m_references.push_back({ attachment_id.value, GetVk(image_layout) });
    return ReferenceID(m_references.size() - 1);
}

SubpassID RenderPass::Builder::AddSubpass(VkSubpassDescription subpass_description)
{
    m_subpass_descriptions.push_back(subpass_description);

    state.subpassCount = narrow_cast<uint32_t>(m_subpass_descriptions.size());
    state.pSubpasses   = m_subpass_descriptions.data();
    return SubpassID(m_subpass_descriptions.size() - 1);
}

void RenderPass::Builder::AddSubpassDependency(
    SubpassID     src_subpass,
    SubpassID     dst_subpass,
    PipelineStage src_stage_mask,
    PipelineStage dst_stage_mask,
    Access        src_access_mask,
    Access        dst_access_mask,
    Dependency    dependency_flags) noexcept
{
    VkSubpassDependency dependency = {

        .srcSubpass      = src_subpass.value,
        .dstSubpass      = dst_subpass.value,
        .srcStageMask    = GetVk(src_stage_mask),
        .dstStageMask    = GetVk(dst_stage_mask),
        .srcAccessMask   = GetVk(src_access_mask),
        .dstAccessMask   = GetVk(dst_access_mask),
        .dependencyFlags = GetVk(dependency_flags)
    };

    m_subpass_dependencies.push_back(dependency);

    state.dependencyCount = narrow_cast<uint32_t>(m_subpass_dependencies.size());
    state.pDependencies   = m_subpass_dependencies.data();
}

void RenderPass::SubpassBuilder::AddColorAttachment(ReferenceID reference_id)
{
    m_color_attachment_references.push_back(m_renderpass_builder->m_references[reference_id.value]);

    state.colorAttachmentCount = narrow_cast<uint32_t>(m_color_attachment_references.size());
    state.pColorAttachments    = m_color_attachment_references.data();
}

void RenderPass::SubpassBuilder::SetDepthStencilAttachment(ReferenceID reference_id)
{
    depth_stencil_attachment_reference = m_renderpass_builder->m_references[reference_id.value];

    state.pDepthStencilAttachment = &depth_stencil_attachment_reference;
}

RenderPass::SubpassBuilder::SubpassBuilder(const RenderPass::Builder* renderpass_builder) noexcept
    : m_renderpass_builder(renderpass_builder)
{
    state = VkSubpassDescription{

        .flags                   = {},
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = nullptr,
        .colorAttachmentCount    = 0,
        .pColorAttachments       = nullptr,
        .pResolveAttachments     = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = nullptr
    };
}

} // namespace etna
