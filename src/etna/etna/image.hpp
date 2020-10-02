#pragma once

#include "allocator.hpp"

#include <cassert>
#include <span>
#include <vulkan/vulkan.hpp>

VK_DEFINE_HANDLE(EtnaImage2D)

namespace etna {

class Image2D {
  public:
    Image2D() noexcept {}
    Image2D(std::nullptr_t) noexcept : m_image(nullptr) {}
    Image2D(EtnaImage2D image) noexcept : m_image(image) {}

    auto operator<=>(Image2D const&) const = default;

    operator EtnaImage2D() const noexcept { return m_image; }
    operator vk::Image() const noexcept;

    explicit operator bool() const noexcept { return m_image != nullptr; }

    vk::ImageUsageFlags UsageFlags() const noexcept;
    vk::Format          Format() const noexcept;
    vk::Extent2D        Extent() const noexcept;
    vk::Viewport        Viewport() const noexcept;
    vk::Rect2D          Rect2D() const noexcept;

    void destroy(EtnaImage2D image);

  private:
    EtnaImage2D m_image = nullptr;
};

template <typename Dispatch>
class vk::UniqueHandleTraits<Image2D, Dispatch> {
  public:
    using deleter = typename etna::Image2D;
};

using UniqueImage2D = vk::UniqueHandle<Image2D, vk::DispatchLoaderStatic>;

auto CreateUniqueImage2D(
    Allocator           allocator,
    vk::Format          format,
    vk::Extent2D        extent,
    vk::ImageUsageFlags usage,
    etna::MemoryUsage   memory_usage,
    vk::ImageTiling     tiling = vk::ImageTiling::eOptimal) -> UniqueImage2D;

auto CreateUniqueImageView(vk::Device device, etna::Image2D image) -> vk::UniqueImageView;

auto CreateUniqueFrameBuffer(
    vk::Device                     device,
    vk::RenderPass                 renderpass,
    vk::Extent2D                   extent,
    std::span<const vk::ImageView> attachments) -> vk::UniqueFramebuffer;

auto CreateUniqueFrameBuffer(
    vk::Device                           device,
    vk::RenderPass                       renderpass,
    vk::Extent2D                         extent,
    std::initializer_list<vk::ImageView> attachments) -> vk::UniqueFramebuffer;

} // namespace etna
