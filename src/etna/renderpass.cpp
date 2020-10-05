#include "etna/renderpass.hpp"

#include "utils/casts.hpp"
#include "utils/throw_exception.hpp"

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

auto RenderPass::Create(VkDevice device, const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass
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

AttachmentID RenderPassBuilder::AddAttachment(
    Format            format,
    AttachmentLoadOp  load_op,
    AttachmentStoreOp store_op,
    ImageLayout       initial_layout,
    ImageLayout       final_layout)
{
    VkAttachmentDescription description = {

        .flags          = {},
        .format         = GetVkFlags(format),
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = GetVkFlags(load_op),
        .storeOp        = GetVkFlags(store_op),
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = GetVkFlags(initial_layout),
        .finalLayout    = GetVkFlags(final_layout)
    };

    m_attachment_descriptions.push_back(description);

    create_info.attachmentCount = narrow_cast<uint32_t>(m_attachment_descriptions.size());
    create_info.pAttachments    = m_attachment_descriptions.data();

    return narrow_cast<AttachmentID>(m_attachment_descriptions.size() - 1);
}

ReferenceID RenderPassBuilder::AddReference(AttachmentID attachment_id, ImageLayout image_layout)
{
    m_references.push_back({ attachment_id, GetVkFlags(image_layout) });
    return m_references.size() - 1;
}

void RenderPassBuilder::AddSubpass(ReferenceID reference_id)
{
    AddSubpass({ reference_id });
}

void RenderPassBuilder::AddSubpass(std::span<const ReferenceID> reference_ids)
{
    std::vector<VkAttachmentReference> references;
    for (auto reference_id : reference_ids)
        references.push_back(m_references[reference_id]);

    m_subpass_references.push_front(std::move(references));

    VkSubpassDescription subpass_description = {

        .flags                   = {},
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = nullptr,
        .colorAttachmentCount    = narrow_cast<uint32_t>(m_subpass_references.front().size()),
        .pColorAttachments       = m_subpass_references.front().data(),
        .pResolveAttachments     = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = nullptr
    };

    m_subpass_descriptions.push_back(subpass_description);

    create_info.subpassCount = narrow_cast<std::uint32_t>(m_subpass_descriptions.size());
    create_info.pSubpasses   = m_subpass_descriptions.data();
}

void RenderPassBuilder::AddSubpass(std::initializer_list<ReferenceID> reference_ids)
{
    AddSubpass(std::span(reference_ids.begin(), reference_ids.size()));
}

} // namespace etna
