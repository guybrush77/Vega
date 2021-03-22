#pragma once

#include "core.hpp"

namespace etna {

class Sampler {
  public:
    struct Builder final {
        Builder() noexcept;

        Builder(Filter mag_filter, Filter min_filter, SamplerMipmapMode mipmap_mode) noexcept;

        VkSamplerCreateInfo state{};
    };

    Sampler() noexcept {}
    Sampler(std::nullptr_t) noexcept {}

    operator VkSampler() const noexcept { return m_sampler; }

    explicit operator bool() const noexcept { return m_sampler != nullptr; }

    bool operator==(const Sampler& rhs) const noexcept { return m_sampler == rhs.m_sampler; }
    bool operator!=(const Sampler& rhs) const noexcept { return m_sampler != rhs.m_sampler; }

  private:
    template <typename>
    friend class UniqueHandle;

    friend class Device;

    Sampler(VkSampler sampler, VkDevice device) : m_sampler(sampler), m_device(device) {}

    static auto Create(VkDevice vk_device, const VkSamplerCreateInfo& create_info) -> UniqueSampler;

    void Destroy() noexcept;

    VkSampler m_sampler{};
    VkDevice  m_device{};
};

} // namespace etna
