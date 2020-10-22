#pragma once

#include "core.hpp"

#include <deque>
#include <span>
#include <vector>

namespace etna {

struct Subpass;

class RenderPass {
  public:
    struct Builder final {
        Builder() noexcept;

        AttachmentID AddAttachment(
            Format            format,
            AttachmentLoadOp  load_op,
            AttachmentStoreOp store_op,
            ImageLayout       initial_layout,
            ImageLayout       final_layout);

        ReferenceID AddReference(AttachmentID attachment_id, ImageLayout image_layout);

        Subpass CreateSubpass() noexcept;

        void AddSubpass(VkSubpassDescription subpass_description);

        VkRenderPassCreateInfo state{};

      private:
        friend struct Subpass;

        std::vector<VkAttachmentDescription> m_attachment_descriptions;
        std::deque<VkAttachmentReference>    m_references;
        std::vector<VkSubpassDescription>    m_subpass_descriptions;
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

struct Subpass final {
    void AddColorAttachment(ReferenceID reference_id);
    void SetDepthStencilAttachment(ReferenceID reference_id);

    VkSubpassDescription state{};

  private:
    friend struct RenderPass::Builder;

    Subpass(const RenderPass::Builder* renderpass_builder) noexcept;

    std::vector<VkAttachmentReference> m_color_attachment_references;
    VkAttachmentReference              depth_stencil_attachment_reference;

    const RenderPass::Builder* m_renderpass_builder{};
};

} // namespace etna
