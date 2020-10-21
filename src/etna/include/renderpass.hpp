#pragma once

#include "core.hpp"

#include <deque>
#include <span>
#include <vector>

ETNA_DEFINE_HANDLE(EtnaRenderPass)

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

    operator VkRenderPass() const noexcept;

    explicit operator bool() const noexcept { return m_state != nullptr; }

    bool operator==(const RenderPass& rhs) const noexcept { return m_state == rhs.m_state; }
    bool operator!=(const RenderPass& rhs) const noexcept { return m_state != rhs.m_state; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    RenderPass(EtnaRenderPass state) : m_state(state) {}

    static auto Create(VkDevice device, const VkRenderPassCreateInfo& create_info) -> UniqueRenderPass;

    void Destroy() noexcept;

    EtnaRenderPass m_state{};
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
