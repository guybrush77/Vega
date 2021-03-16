#include "etna/sampler.hpp"

#include <cassert>

namespace etna {

UniqueSampler Sampler::Create(VkDevice vk_device, const VkSamplerCreateInfo& create_info)
{
    VkSampler vk_sampler{};
    vkCreateSampler(vk_device, &create_info, nullptr, &vk_sampler);

    return UniqueSampler(Sampler(vk_sampler, vk_device));
}

void Sampler::Destroy() noexcept
{
    assert(m_sampler);

    vkDestroySampler(m_device, m_sampler, nullptr);

    m_sampler = nullptr;
    m_device  = nullptr;
}

} // namespace etna
