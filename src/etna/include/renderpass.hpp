#pragma once

#include "core.hpp"

#include <deque>
#include <span>
#include <vector>

namespace etna {

class RenderPass {
  public:
    struct SubpassBuilder;

    struct Builder final {
        Builder() noexcept;

        SubpassBuilder GetSubpassBuilder();

        [[nodiscard]] AttachmentID AddAttachmentDescription(
            Format            format,
            AttachmentLoadOp  load_op,
            AttachmentStoreOp store_op,
            ImageLayout       initial_layout,
            ImageLayout       final_layout);

        [[nodiscard]] ReferenceID AddAttachmentReference(AttachmentID attachment_id, ImageLayout image_layout);

        SubpassID AddSubpass(VkSubpassDescription subpass_description);

        void AddSubpassDependency(
            SubpassID     src_subpass,
            SubpassID     dst_subpass,
            PipelineStage src_stage_mask,
            PipelineStage dst_stage_mask,
            Access        src_access_mask,
            Access        dst_access_mask,
            Dependency    dependency_flags = {}) noexcept;

        VkRenderPassCreateInfo state{};

      private:
        friend struct SubpassBuilder;

        std::vector<VkAttachmentDescription> m_attachment_descriptions;
        std::deque<VkAttachmentReference>    m_references;
        std::vector<VkSubpassDescription>    m_subpass_descriptions;
        std::vector<VkSubpassDependency>     m_subpass_dependencies;
    };

    struct SubpassBuilder final {
        void AddColorAttachment(ReferenceID reference_id);
        void SetDepthStencilAttachment(ReferenceID reference_id);

        VkSubpassDescription state{};

      private:
        friend struct RenderPass::Builder;

        SubpassBuilder(const RenderPass::Builder* renderpass_builder) noexcept;

        std::vector<VkAttachmentReference> m_color_attachment_references;
        VkAttachmentReference              depth_stencil_attachment_reference;

        const RenderPass::Builder* m_renderpass_builder{};
    };

    RenderPass() noexcept {}
    RenderPass(std::nullptr_t) noexcept {}

    operator VkRenderPass() const noexcept { return m_renderpass; }

    bool operator==(const RenderPass&) const = default;

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;
    friend class Framebuffer;

    RenderPass(VkRenderPass renderpass, VkDevice device) noexcept : m_renderpass(renderpass), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass;

    void Destroy() noexcept;

    VkRenderPass m_renderpass{};
    VkDevice     m_device{};
};

} // namespace etna
