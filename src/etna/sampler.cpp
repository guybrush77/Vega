#include "etna/sampler.hpp"

#include <cassert>

namespace etna {

Sampler::Builder::Builder() noexcept
{
    state = VkSamplerCreateInfo{

        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .magFilter               = VK_FILTER_NEAREST,
        .minFilter               = VK_FILTER_NEAREST,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_FALSE,
        .maxAnisotropy           = 0.0f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };
}

Sampler::Builder::Builder(Filter mag_filter, Filter min_filter, SamplerMipmapMode mipmap_mode) noexcept
{
    state = VkSamplerCreateInfo{

        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = {},
        .magFilter               = VkEnum(mag_filter),
        .minFilter               = VkEnum(min_filter),
        .mipmapMode              = VkEnum(mipmap_mode),
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_FALSE,
        .maxAnisotropy           = 0.0f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };
}

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
