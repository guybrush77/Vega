#include "renderpass.hpp"

#include <spdlog/spdlog.h>

#define COMPONENT "Etna: "

namespace {

struct EtnaRenderPass_T final {
    VkRenderPass renderpass;
    VkDevice     device;
};

} // namespace

namespace etna {

RenderPass::operator VkRenderPass() const noexcept
{
    return m_state ? m_state->renderpass : VkRenderPass{};
}

UniqueRenderPass RenderPass::Create(VkDevice device, const VkRenderPassCreateInfo& create_info)
{
    VkRenderPass renderpass{};

    if (auto result = vkCreateRenderPass(device, &create_info, nullptr, &renderpass); result != VK_SUCCESS) {
        throw_runtime_error(fmt::format("vkCreateRenderPass error: {}", result).c_str());
    }

    spdlog::info(COMPONENT "Created VkRenderPass {}", fmt::ptr(renderpass));

    return UniqueRenderPass(new EtnaRenderPass_T{ renderpass, device });
}

void RenderPass::Destroy() noexcept
{
    assert(m_state);

    vkDestroyRenderPass(m_state->device, m_state->renderpass, nullptr);

    spdlog::info(COMPONENT "Destroyed VkRenderPass {}", fmt::ptr(m_state->renderpass));

    delete m_state;

    m_state = nullptr;
}

RenderPassBuilder::RenderPassBuilder() noexcept
{
    create_info = VkRenderPassCreateInfo{

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

SubpassBuilder RenderPassBuilder::CreateSubpassBuilder() const
{
    return SubpassBuilder(this);
}

AttachmentID RenderPassBuilder::AddAttachment(
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

    create_info.attachmentCount = narrow_cast<uint32_t>(m_attachment_descriptions.size());
    create_info.pAttachments    = m_attachment_descriptions.data();

    return AttachmentID(m_attachment_descriptions.size() - 1);
}

ReferenceID RenderPassBuilder::AddReference(AttachmentID attachment_id, ImageLayout image_layout)
{
    m_references.push_back({ attachment_id.value, GetVk(image_layout) });
    return ReferenceID(m_references.size() - 1);
}

void RenderPassBuilder::AddSubpass(VkSubpassDescription subpass_description)
{
    m_subpass_descriptions.push_back(subpass_description);

    create_info.subpassCount = narrow_cast<uint32_t>(m_subpass_descriptions.size());
    create_info.pSubpasses   = m_subpass_descriptions.data();
}

void SubpassBuilder::AddColorAttachment(ReferenceID reference_id)
{
    m_color_attachment_references.push_back(m_renderpass_builder->m_references[reference_id.value]);

    subpass_description.colorAttachmentCount = narrow_cast<uint32_t>(m_color_attachment_references.size());
    subpass_description.pColorAttachments    = m_color_attachment_references.data();
}

void SubpassBuilder::SetDepthStencilAttachment(ReferenceID reference_id)
{
    depth_stencil_attachment_reference = m_renderpass_builder->m_references[reference_id.value];

    subpass_description.pDepthStencilAttachment = &depth_stencil_attachment_reference;
}

SubpassBuilder::SubpassBuilder(const RenderPassBuilder* renderpass_builder) noexcept
    : m_renderpass_builder(renderpass_builder)
{
    subpass_description = VkSubpassDescription{

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
