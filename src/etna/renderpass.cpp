#include "renderpass.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace etna {

UniqueRenderPass RenderPass::Create(VkDevice vk_device, const VkRenderPassCreateInfo& create_info)
{
    VkRenderPass vk_renderpass{};

    if (auto result = vkCreateRenderPass(vk_device, &create_info, nullptr, &vk_renderpass); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateRenderPass error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkRenderPass {}", fmt::ptr(vk_renderpass));

    return UniqueRenderPass(RenderPass(vk_renderpass, vk_device));
}

void RenderPass::Destroy() noexcept
{
    assert(m_renderpass);

    vkDestroyRenderPass(m_device, m_renderpass, nullptr);

    spdlog::info(COMPONENT "Destroyed VkRenderPass {}", fmt::ptr(m_renderpass));

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

AttachmentID RenderPass::Builder::AddAttachment(
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

ReferenceID RenderPass::Builder::AddReference(AttachmentID attachment_id, ImageLayout image_layout)
{
    m_references.push_back({ attachment_id.value, GetVk(image_layout) });
    return ReferenceID(m_references.size() - 1);
}

Subpass RenderPass::Builder::CreateSubpass() noexcept
{
    return Subpass(this);
}

void RenderPass::Builder::AddSubpass(VkSubpassDescription subpass_description)
{
    m_subpass_descriptions.push_back(subpass_description);

    state.subpassCount = narrow_cast<uint32_t>(m_subpass_descriptions.size());
    state.pSubpasses   = m_subpass_descriptions.data();
}

void Subpass::AddColorAttachment(ReferenceID reference_id)
{
    m_color_attachment_references.push_back(m_renderpass_builder->m_references[reference_id.value]);

    state.colorAttachmentCount = narrow_cast<uint32_t>(m_color_attachment_references.size());
    state.pColorAttachments    = m_color_attachment_references.data();
}

void Subpass::SetDepthStencilAttachment(ReferenceID reference_id)
{
    depth_stencil_attachment_reference = m_renderpass_builder->m_references[reference_id.value];

    state.pDepthStencilAttachment = &depth_stencil_attachment_reference;
}

Subpass::Subpass(const RenderPass::Builder* renderpass_builder) noexcept : m_renderpass_builder(renderpass_builder)
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
