#pragma once

#include "types.hpp"

#include <forward_list>
#include <span>
#include <vector>

ETNA_DEFINE_HANDLE(EtnaRenderPass)

namespace etna {

class RenderPass;

using UniqueRenderPass = UniqueHandle<RenderPass>;

class RenderPass {
  public:
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

using AttachmentID = std::uint32_t;
using ReferenceID  = std::size_t;

struct RenderPassBuilder final {
    RenderPassBuilder() noexcept;

    AttachmentID AddAttachment(
        Format            format,
        AttachmentLoadOp  load_op,
        AttachmentStoreOp store_op,
        ImageLayout       initial_layout,
        ImageLayout       final_layout);

    ReferenceID AddReference(AttachmentID attachment_id, ImageLayout image_layout);

    void AddSubpass(ReferenceID reference_id);

    void AddSubpass(std::span<const ReferenceID> reference_ids);

    void AddSubpass(std::initializer_list<ReferenceID> reference_ids);

    VkRenderPassCreateInfo create_info{};

  private:
    std::vector<VkAttachmentDescription>                  m_attachment_descriptions;
    std::vector<VkAttachmentReference>                    m_references;
    std::forward_list<std::vector<VkAttachmentReference>> m_subpass_references;
    std::vector<VkSubpassDescription>                     m_subpass_descriptions;
};

} // namespace etna
