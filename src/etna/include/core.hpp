#pragma once

#include <type_traits>
#include <vulkan/vulkan_core.h>

#define ETNA_DEFINE_HANDLE(handle)                                                                                     \
    namespace {                                                                                                        \
    using handle = struct handle##_T*;                                                                                 \
    }

#define ETNA_DEFINE_VK_ENUM(type)                                                                                      \
    inline constexpr auto GetVk(type val) noexcept { return static_cast<Vk##type>(val); }

#define ETNA_DEFINE_VK_FLAGS(type)                                                                                     \
    inline constexpr auto GetVk(type val) noexcept { return static_cast<Vk##type##Flags>(val); }                       \
    template <>                                                                                                        \
    struct composable_flags<##type##> : std::true_type {};                                                             \
    using type##Mask = Mask<##type##>;

#define ETNA_DEFINE_VK_FLAGS_SUFFIXED(type)                                                                            \
    inline constexpr auto GetVk(type val) noexcept { return static_cast<Vk##type>(val); }                              \
    template <>                                                                                                        \
    struct composable_flags<##type##> : std::true_type {};                                                             \
    using type##Mask = Mask<##type##>;

template <typename>
struct etna_vertex_attribute_type_trait;

#define DECLARE_VERTEX_ATTRIBUTE_TYPE(attribute_type, vulkan_type)                                                     \
    template <>                                                                                                        \
    struct etna_vertex_attribute_type_trait<attribute_type> {                                                          \
        static constexpr etna::Format value = vulkan_type;                                                             \
    };

#define formatof(vertex_type, field)                                                                                   \
    etna_vertex_attribute_type_trait<decltype(std::declval<vertex_type>().field)>::value

namespace etna {

using Location = uint32_t;
using Binding  = uint32_t;

template <typename>
struct composable_flags : std::false_type {};

template <typename>
class Mask;

enum class QueueFamily { Graphics, Transfer, Compute };

enum class MemoryUsage { Unknown, GpuOnly, CpuOnly, CpuToGpu, GpuToCpu, CpuCopy, GpuLazilyAllocated };

enum class AttachmentLoadOp {
    Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
    Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE
};

ETNA_DEFINE_VK_ENUM(AttachmentLoadOp)

enum class AttachmentStoreOp { Store = VK_ATTACHMENT_STORE_OP_STORE, DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE };

ETNA_DEFINE_VK_ENUM(AttachmentStoreOp)

enum class Format {
    Undefined                               = VK_FORMAT_UNDEFINED,
    R4G4UnormPack8                          = VK_FORMAT_R4G4_UNORM_PACK8,
    R4G4B4A4UnormPack16                     = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    B4G4R4A4UnormPack16                     = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    R5G6B5UnormPack16                       = VK_FORMAT_R5G6B5_UNORM_PACK16,
    B5G6R5UnormPack16                       = VK_FORMAT_B5G6R5_UNORM_PACK16,
    R5G5B5A1UnormPack16                     = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
    B5G5R5A1UnormPack16                     = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
    A1R5G5B5UnormPack16                     = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
    R8Unorm                                 = VK_FORMAT_R8_UNORM,
    R8Snorm                                 = VK_FORMAT_R8_SNORM,
    R8Uscaled                               = VK_FORMAT_R8_USCALED,
    R8Sscaled                               = VK_FORMAT_R8_SSCALED,
    R8Uint                                  = VK_FORMAT_R8_UINT,
    R8Sint                                  = VK_FORMAT_R8_SINT,
    R8Srgb                                  = VK_FORMAT_R8_SRGB,
    R8G8Unorm                               = VK_FORMAT_R8G8_UNORM,
    R8G8Snorm                               = VK_FORMAT_R8G8_SNORM,
    R8G8Uscaled                             = VK_FORMAT_R8G8_USCALED,
    R8G8Sscaled                             = VK_FORMAT_R8G8_SSCALED,
    R8G8Uint                                = VK_FORMAT_R8G8_UINT,
    R8G8Sint                                = VK_FORMAT_R8G8_SINT,
    R8G8Srgb                                = VK_FORMAT_R8G8_SRGB,
    R8G8B8Unorm                             = VK_FORMAT_R8G8B8_UNORM,
    R8G8B8Snorm                             = VK_FORMAT_R8G8B8_SNORM,
    R8G8B8Uscaled                           = VK_FORMAT_R8G8B8_USCALED,
    R8G8B8Sscaled                           = VK_FORMAT_R8G8B8_SSCALED,
    R8G8B8Uint                              = VK_FORMAT_R8G8B8_UINT,
    R8G8B8Sint                              = VK_FORMAT_R8G8B8_SINT,
    R8G8B8Srgb                              = VK_FORMAT_R8G8B8_SRGB,
    B8G8R8Unorm                             = VK_FORMAT_B8G8R8_UNORM,
    B8G8R8Snorm                             = VK_FORMAT_B8G8R8_SNORM,
    B8G8R8Uscaled                           = VK_FORMAT_B8G8R8_USCALED,
    B8G8R8Sscaled                           = VK_FORMAT_B8G8R8_SSCALED,
    B8G8R8Uint                              = VK_FORMAT_B8G8R8_UINT,
    B8G8R8Sint                              = VK_FORMAT_B8G8R8_SINT,
    B8G8R8Srgb                              = VK_FORMAT_B8G8R8_SRGB,
    R8G8B8A8Unorm                           = VK_FORMAT_R8G8B8A8_UNORM,
    R8G8B8A8Snorm                           = VK_FORMAT_R8G8B8A8_SNORM,
    R8G8B8A8Uscaled                         = VK_FORMAT_R8G8B8A8_USCALED,
    R8G8B8A8Sscaled                         = VK_FORMAT_R8G8B8A8_SSCALED,
    R8G8B8A8Uint                            = VK_FORMAT_R8G8B8A8_UINT,
    R8G8B8A8Sint                            = VK_FORMAT_R8G8B8A8_SINT,
    R8G8B8A8Srgb                            = VK_FORMAT_R8G8B8A8_SRGB,
    B8G8R8A8Unorm                           = VK_FORMAT_B8G8R8A8_UNORM,
    B8G8R8A8Snorm                           = VK_FORMAT_B8G8R8A8_SNORM,
    B8G8R8A8Uscaled                         = VK_FORMAT_B8G8R8A8_USCALED,
    B8G8R8A8Sscaled                         = VK_FORMAT_B8G8R8A8_SSCALED,
    B8G8R8A8Uint                            = VK_FORMAT_B8G8R8A8_UINT,
    B8G8R8A8Sint                            = VK_FORMAT_B8G8R8A8_SINT,
    B8G8R8A8Srgb                            = VK_FORMAT_B8G8R8A8_SRGB,
    A8B8G8R8UnormPack32                     = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    A8B8G8R8SnormPack32                     = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
    A8B8G8R8UscaledPack32                   = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
    A8B8G8R8SscaledPack32                   = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
    A8B8G8R8UintPack32                      = VK_FORMAT_A8B8G8R8_UINT_PACK32,
    A8B8G8R8SintPack32                      = VK_FORMAT_A8B8G8R8_SINT_PACK32,
    A8B8G8R8SrgbPack32                      = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
    A2R10G10B10UnormPack32                  = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    A2R10G10B10SnormPack32                  = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
    A2R10G10B10UscaledPack32                = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
    A2R10G10B10SscaledPack32                = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
    A2R10G10B10UintPack32                   = VK_FORMAT_A2R10G10B10_UINT_PACK32,
    A2R10G10B10SintPack32                   = VK_FORMAT_A2R10G10B10_SINT_PACK32,
    A2B10G10R10UnormPack32                  = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    A2B10G10R10SnormPack32                  = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
    A2B10G10R10UscaledPack32                = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
    A2B10G10R10SscaledPack32                = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
    A2B10G10R10UintPack32                   = VK_FORMAT_A2B10G10R10_UINT_PACK32,
    A2B10G10R10SintPack32                   = VK_FORMAT_A2B10G10R10_SINT_PACK32,
    R16Unorm                                = VK_FORMAT_R16_UNORM,
    R16Snorm                                = VK_FORMAT_R16_SNORM,
    R16Uscaled                              = VK_FORMAT_R16_USCALED,
    R16Sscaled                              = VK_FORMAT_R16_SSCALED,
    R16Uint                                 = VK_FORMAT_R16_UINT,
    R16Sint                                 = VK_FORMAT_R16_SINT,
    R16Sfloat                               = VK_FORMAT_R16_SFLOAT,
    R16G16Unorm                             = VK_FORMAT_R16G16_UNORM,
    R16G16Snorm                             = VK_FORMAT_R16G16_SNORM,
    R16G16Uscaled                           = VK_FORMAT_R16G16_USCALED,
    R16G16Sscaled                           = VK_FORMAT_R16G16_SSCALED,
    R16G16Uint                              = VK_FORMAT_R16G16_UINT,
    R16G16Sint                              = VK_FORMAT_R16G16_SINT,
    R16G16Sfloat                            = VK_FORMAT_R16G16_SFLOAT,
    R16G16B16Unorm                          = VK_FORMAT_R16G16B16_UNORM,
    R16G16B16Snorm                          = VK_FORMAT_R16G16B16_SNORM,
    R16G16B16Uscaled                        = VK_FORMAT_R16G16B16_USCALED,
    R16G16B16Sscaled                        = VK_FORMAT_R16G16B16_SSCALED,
    R16G16B16Uint                           = VK_FORMAT_R16G16B16_UINT,
    R16G16B16Sint                           = VK_FORMAT_R16G16B16_SINT,
    R16G16B16Sfloat                         = VK_FORMAT_R16G16B16_SFLOAT,
    R16G16B16A16Unorm                       = VK_FORMAT_R16G16B16A16_UNORM,
    R16G16B16A16Snorm                       = VK_FORMAT_R16G16B16A16_SNORM,
    R16G16B16A16Uscaled                     = VK_FORMAT_R16G16B16A16_USCALED,
    R16G16B16A16Sscaled                     = VK_FORMAT_R16G16B16A16_SSCALED,
    R16G16B16A16Uint                        = VK_FORMAT_R16G16B16A16_UINT,
    R16G16B16A16Sint                        = VK_FORMAT_R16G16B16A16_SINT,
    R16G16B16A16Sfloat                      = VK_FORMAT_R16G16B16A16_SFLOAT,
    R32Uint                                 = VK_FORMAT_R32_UINT,
    R32Sint                                 = VK_FORMAT_R32_SINT,
    R32Sfloat                               = VK_FORMAT_R32_SFLOAT,
    R32G32Uint                              = VK_FORMAT_R32G32_UINT,
    R32G32Sint                              = VK_FORMAT_R32G32_SINT,
    R32G32Sfloat                            = VK_FORMAT_R32G32_SFLOAT,
    R32G32B32Uint                           = VK_FORMAT_R32G32B32_UINT,
    R32G32B32Sint                           = VK_FORMAT_R32G32B32_SINT,
    R32G32B32Sfloat                         = VK_FORMAT_R32G32B32_SFLOAT,
    R32G32B32A32Uint                        = VK_FORMAT_R32G32B32A32_UINT,
    R32G32B32A32Sint                        = VK_FORMAT_R32G32B32A32_SINT,
    R32G32B32A32Sfloat                      = VK_FORMAT_R32G32B32A32_SFLOAT,
    R64Uint                                 = VK_FORMAT_R64_UINT,
    R64Sint                                 = VK_FORMAT_R64_SINT,
    R64Sfloat                               = VK_FORMAT_R64_SFLOAT,
    R64G64Uint                              = VK_FORMAT_R64G64_UINT,
    R64G64Sint                              = VK_FORMAT_R64G64_SINT,
    R64G64Sfloat                            = VK_FORMAT_R64G64_SFLOAT,
    R64G64B64Uint                           = VK_FORMAT_R64G64B64_UINT,
    R64G64B64Sint                           = VK_FORMAT_R64G64B64_SINT,
    R64G64B64Sfloat                         = VK_FORMAT_R64G64B64_SFLOAT,
    R64G64B64A64Uint                        = VK_FORMAT_R64G64B64A64_UINT,
    R64G64B64A64Sint                        = VK_FORMAT_R64G64B64A64_SINT,
    R64G64B64A64Sfloat                      = VK_FORMAT_R64G64B64A64_SFLOAT,
    B10G11R11UfloatPack32                   = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
    E5B9G9R9UfloatPack32                    = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
    D16Unorm                                = VK_FORMAT_D16_UNORM,
    X8D24UnormPack32                        = VK_FORMAT_X8_D24_UNORM_PACK32,
    D32Sfloat                               = VK_FORMAT_D32_SFLOAT,
    S8Uint                                  = VK_FORMAT_S8_UINT,
    D16UnormS8Uint                          = VK_FORMAT_D16_UNORM_S8_UINT,
    D24UnormS8Uint                          = VK_FORMAT_D24_UNORM_S8_UINT,
    D32SfloatS8Uint                         = VK_FORMAT_D32_SFLOAT_S8_UINT,
    Bc1RgbUnormBlock                        = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    Bc1RgbSrgbBlock                         = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
    Bc1RgbaUnormBlock                       = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    Bc1RgbaSrgbBlock                        = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
    Bc2UnormBlock                           = VK_FORMAT_BC2_UNORM_BLOCK,
    Bc2SrgbBlock                            = VK_FORMAT_BC2_SRGB_BLOCK,
    Bc3UnormBlock                           = VK_FORMAT_BC3_UNORM_BLOCK,
    Bc3SrgbBlock                            = VK_FORMAT_BC3_SRGB_BLOCK,
    Bc4UnormBlock                           = VK_FORMAT_BC4_UNORM_BLOCK,
    Bc4SnormBlock                           = VK_FORMAT_BC4_SNORM_BLOCK,
    Bc5UnormBlock                           = VK_FORMAT_BC5_UNORM_BLOCK,
    Bc5SnormBlock                           = VK_FORMAT_BC5_SNORM_BLOCK,
    Bc6HUfloatBlock                         = VK_FORMAT_BC6H_UFLOAT_BLOCK,
    Bc6HSfloatBlock                         = VK_FORMAT_BC6H_SFLOAT_BLOCK,
    Bc7UnormBlock                           = VK_FORMAT_BC7_UNORM_BLOCK,
    Bc7SrgbBlock                            = VK_FORMAT_BC7_SRGB_BLOCK,
    Etc2R8G8B8UnormBlock                    = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    Etc2R8G8B8SrgbBlock                     = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
    Etc2R8G8B8A1UnormBlock                  = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
    Etc2R8G8B8A1SrgbBlock                   = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
    Etc2R8G8B8A8UnormBlock                  = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
    Etc2R8G8B8A8SrgbBlock                   = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
    EacR11UnormBlock                        = VK_FORMAT_EAC_R11_UNORM_BLOCK,
    EacR11SnormBlock                        = VK_FORMAT_EAC_R11_SNORM_BLOCK,
    EacR11G11UnormBlock                     = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
    EacR11G11SnormBlock                     = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
    Astc4x4UnormBlock                       = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
    Astc4x4SrgbBlock                        = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
    Astc5x4UnormBlock                       = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
    Astc5x4SrgbBlock                        = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
    Astc5x5UnormBlock                       = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
    Astc5x5SrgbBlock                        = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
    Astc6x5UnormBlock                       = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
    Astc6x5SrgbBlock                        = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
    Astc6x6UnormBlock                       = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
    Astc6x6SrgbBlock                        = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
    Astc8x5UnormBlock                       = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
    Astc8x5SrgbBlock                        = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
    Astc8x6UnormBlock                       = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
    Astc8x6SrgbBlock                        = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
    Astc8x8UnormBlock                       = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
    Astc8x8SrgbBlock                        = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
    Astc10x5UnormBlock                      = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
    Astc10x5SrgbBlock                       = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
    Astc10x6UnormBlock                      = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
    Astc10x6SrgbBlock                       = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
    Astc10x8UnormBlock                      = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
    Astc10x8SrgbBlock                       = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
    Astc10x10UnormBlock                     = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
    Astc10x10SrgbBlock                      = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
    Astc12x10UnormBlock                     = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
    Astc12x10SrgbBlock                      = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
    Astc12x12UnormBlock                     = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
    Astc12x12SrgbBlock                      = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
    G8B8G8R8422Unorm                        = VK_FORMAT_G8B8G8R8_422_UNORM,
    B8G8R8G8422Unorm                        = VK_FORMAT_B8G8R8G8_422_UNORM,
    G8B8R83Plane420Unorm                    = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    G8B8R82Plane420Unorm                    = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    G8B8R83Plane422Unorm                    = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
    G8B8R82Plane422Unorm                    = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
    G8B8R83Plane444Unorm                    = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
    R10X6UnormPack16                        = VK_FORMAT_R10X6_UNORM_PACK16,
    R10X6G10X6Unorm2Pack16                  = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
    R10X6G10X6B10X6A10X6Unorm4Pack16        = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
    G10X6B10X6G10X6R10X6422Unorm4Pack16     = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
    B10X6G10X6R10X6G10X6422Unorm4Pack16     = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
    G10X6B10X6R10X63Plane420Unorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
    G10X6B10X6R10X62Plane420Unorm3Pack16    = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
    G10X6B10X6R10X63Plane422Unorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
    G10X6B10X6R10X62Plane422Unorm3Pack16    = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
    G10X6B10X6R10X63Plane444Unorm3Pack16    = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
    R12X4UnormPack16                        = VK_FORMAT_R12X4_UNORM_PACK16,
    R12X4G12X4Unorm2Pack16                  = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
    R12X4G12X4B12X4A12X4Unorm4Pack16        = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
    G12X4B12X4G12X4R12X4422Unorm4Pack16     = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
    B12X4G12X4R12X4G12X4422Unorm4Pack16     = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
    G12X4B12X4R12X43Plane420Unorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
    G12X4B12X4R12X42Plane420Unorm3Pack16    = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
    G12X4B12X4R12X43Plane422Unorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
    G12X4B12X4R12X42Plane422Unorm3Pack16    = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
    G12X4B12X4R12X43Plane444Unorm3Pack16    = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
    G16B16G16R16422Unorm                    = VK_FORMAT_G16B16G16R16_422_UNORM,
    B16G16R16G16422Unorm                    = VK_FORMAT_B16G16R16G16_422_UNORM,
    G16B16R163Plane420Unorm                 = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
    G16B16R162Plane420Unorm                 = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
    G16B16R163Plane422Unorm                 = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
    G16B16R162Plane422Unorm                 = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
    G16B16R163Plane444Unorm                 = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
    Pvrtc12BppUnormBlockIMG                 = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
    Pvrtc14BppUnormBlockIMG                 = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
    Pvrtc22BppUnormBlockIMG                 = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
    Pvrtc24BppUnormBlockIMG                 = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
    Pvrtc12BppSrgbBlockIMG                  = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
    Pvrtc14BppSrgbBlockIMG                  = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
    Pvrtc22BppSrgbBlockIMG                  = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
    Pvrtc24BppSrgbBlockIMG                  = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
    Astc4x4SfloatBlockEXT                   = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
    Astc5x4SfloatBlockEXT                   = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
    Astc5x5SfloatBlockEXT                   = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
    Astc6x5SfloatBlockEXT                   = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
    Astc6x6SfloatBlockEXT                   = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
    Astc8x5SfloatBlockEXT                   = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
    Astc8x6SfloatBlockEXT                   = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
    Astc8x8SfloatBlockEXT                   = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
    Astc10x5SfloatBlockEXT                  = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
    Astc10x6SfloatBlockEXT                  = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
    Astc10x8SfloatBlockEXT                  = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
    Astc10x10SfloatBlockEXT                 = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
    Astc12x10SfloatBlockEXT                 = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
    Astc12x12SfloatBlockEXT                 = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
    G8B8G8R8422UnormKHR                     = VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
    B8G8R8G8422UnormKHR                     = VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
    G8B8R83Plane420UnormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
    G8B8R82Plane420UnormKHR                 = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
    G8B8R83Plane422UnormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
    G8B8R82Plane422UnormKHR                 = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
    G8B8R83Plane444UnormKHR                 = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
    R10X6UnormPack16KHR                     = VK_FORMAT_R10X6_UNORM_PACK16_KHR,
    R10X6G10X6Unorm2Pack16KHR               = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
    R10X6G10X6B10X6A10X6Unorm4Pack16KHR     = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
    G10X6B10X6G10X6R10X6422Unorm4Pack16KHR  = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
    B10X6G10X6R10X6G10X6422Unorm4Pack16KHR  = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
    G10X6B10X6R10X63Plane420Unorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
    G10X6B10X6R10X62Plane420Unorm3Pack16KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
    G10X6B10X6R10X63Plane422Unorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
    G10X6B10X6R10X62Plane422Unorm3Pack16KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
    G10X6B10X6R10X63Plane444Unorm3Pack16KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
    R12X4UnormPack16KHR                     = VK_FORMAT_R12X4_UNORM_PACK16_KHR,
    R12X4G12X4Unorm2Pack16KHR               = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
    R12X4G12X4B12X4A12X4Unorm4Pack16KHR     = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
    G12X4B12X4G12X4R12X4422Unorm4Pack16KHR  = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
    B12X4G12X4R12X4G12X4422Unorm4Pack16KHR  = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
    G12X4B12X4R12X43Plane420Unorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
    G12X4B12X4R12X42Plane420Unorm3Pack16KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
    G12X4B12X4R12X43Plane422Unorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
    G12X4B12X4R12X42Plane422Unorm3Pack16KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
    G12X4B12X4R12X43Plane444Unorm3Pack16KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
    G16B16G16R16422UnormKHR                 = VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
    B16G16R16G16422UnormKHR                 = VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
    G16B16R163Plane420UnormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
    G16B16R162Plane420UnormKHR              = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
    G16B16R163Plane422UnormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
    G16B16R162Plane422UnormKHR              = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
    G16B16R163Plane444UnormKHR              = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR
};

ETNA_DEFINE_VK_ENUM(Format)

enum class VertexInputRate { Vertex = VK_VERTEX_INPUT_RATE_VERTEX, Instance = VK_VERTEX_INPUT_RATE_INSTANCE };

ETNA_DEFINE_VK_ENUM(VertexInputRate)

enum class IndexType {
    Uint16   = VK_INDEX_TYPE_UINT16,
    Uint32   = VK_INDEX_TYPE_UINT32,
    NoneKHR  = VK_INDEX_TYPE_NONE_KHR,
    Uint8EXT = VK_INDEX_TYPE_UINT8_EXT,
    NoneNV   = VK_INDEX_TYPE_NONE_NV
};

ETNA_DEFINE_VK_ENUM(IndexType)

enum class ImageLayout {
    Undefined                                = VK_IMAGE_LAYOUT_UNDEFINED,
    General                                  = VK_IMAGE_LAYOUT_GENERAL,
    ColorAttachmentOptimal                   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    DepthStencilAttachmentOptimal            = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    DepthStencilReadOnlyOptimal              = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    ShaderReadOnlyOptimal                    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    TransferSrcOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    TransferDstOptimal                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    Preinitialized                           = VK_IMAGE_LAYOUT_PREINITIALIZED,
    DepthReadOnlyStencilAttachmentOptimal    = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
    DepthAttachmentStencilReadOnlyOptimal    = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
    DepthAttachmentOptimal                   = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    DepthReadOnlyOptimal                     = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
    StencilAttachmentOptimal                 = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
    StencilReadOnlyOptimal                   = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
    PresentSrcKHR                            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    SharedPresentKHR                         = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
    ShadingRateOptimalNV                     = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
    FragmentDensityMapOptimalEXT             = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
    DepthReadOnlyStencilAttachmentOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR,
    DepthAttachmentStencilReadOnlyOptimalKHR = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR,
    DepthAttachmentOptimalKHR                = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,
    DepthReadOnlyOptimalKHR                  = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,
    StencilAttachmentOptimalKHR              = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR,
    StencilReadOnlyOptimalKHR                = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR
};

ETNA_DEFINE_VK_ENUM(ImageLayout)

enum class DynamicState {
    Viewport                     = VK_DYNAMIC_STATE_VIEWPORT,
    Scissor                      = VK_DYNAMIC_STATE_SCISSOR,
    LineWidth                    = VK_DYNAMIC_STATE_LINE_WIDTH,
    DepthBias                    = VK_DYNAMIC_STATE_DEPTH_BIAS,
    BlendConstants               = VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    DepthBounds                  = VK_DYNAMIC_STATE_DEPTH_BOUNDS,
    StencilCompareMask           = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
    StencilWriteMask             = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
    StencilReference             = VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    ViewportWScalingNV           = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV,
    DiscardRectangleEXT          = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT,
    SampleLocationsEXT           = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT,
    ViewportShadingRatePaletteNV = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV,
    ViewportCoarseSampleOrderNV  = VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV,
    ExclusiveScissorNV           = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV,
    LineStippleEXT               = VK_DYNAMIC_STATE_LINE_STIPPLE_EXT
};

ETNA_DEFINE_VK_ENUM(DynamicState)

enum class CompareOp {
    Never          = VK_COMPARE_OP_NEVER,
    Less           = VK_COMPARE_OP_LESS,
    Equal          = VK_COMPARE_OP_EQUAL,
    LessOrEqual    = VK_COMPARE_OP_LESS_OR_EQUAL,
    Greater        = VK_COMPARE_OP_GREATER,
    NotEqual       = VK_COMPARE_OP_NOT_EQUAL,
    GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
    Always         = VK_COMPARE_OP_ALWAYS
};

ETNA_DEFINE_VK_ENUM(CompareOp)

enum class DescriptorType {
    Sampler                  = VK_DESCRIPTOR_TYPE_SAMPLER,
    CombinedImageSampler     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    SampledImage             = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    StorageImage             = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    UniformTexelBuffer       = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    StorageTexelBuffer       = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    UniformBuffer            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    StorageBuffer            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    UniformBufferDynamic     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    StorageBufferDynamic     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    InputAttachment          = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    InlineUniformBlockEXT    = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
    AccelerationStructureKHR = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
    AccelerationStructureNV  = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV
};

ETNA_DEFINE_VK_ENUM(DescriptorType)

enum class ImageTiling {
    Optimal              = VK_IMAGE_TILING_OPTIMAL,
    Linear               = VK_IMAGE_TILING_LINEAR,
    DrmFormatModifierEXT = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT
};

ETNA_DEFINE_VK_ENUM(ImageTiling)

enum class CommandBufferLevel {
    Primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
};

ETNA_DEFINE_VK_ENUM(CommandBufferLevel)

enum class SubpassContents {
    Inline                  = VK_SUBPASS_CONTENTS_INLINE,
    SecondaryCommandBuffers = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
};

ETNA_DEFINE_VK_ENUM(SubpassContents)

enum class PipelineBindPoint {
    Graphics      = VK_PIPELINE_BIND_POINT_GRAPHICS,
    Compute       = VK_PIPELINE_BIND_POINT_COMPUTE,
    RayTracingKHR = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    RayTracingNV  = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
};

ETNA_DEFINE_VK_ENUM(PipelineBindPoint)

enum class ImageUsage : VkImageUsageFlags {
    TransferSrc            = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    TransferDst            = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    Sampled                = VK_IMAGE_USAGE_SAMPLED_BIT,
    Storage                = VK_IMAGE_USAGE_STORAGE_BIT,
    ColorAttachment        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    DepthStencilAttachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    TransientAttachment    = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    InputAttachment        = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    ShadingRateImageNV     = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
    FragmentDensityMapEXT  = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT
};

ETNA_DEFINE_VK_FLAGS(ImageUsage)

enum class BufferUsage : VkBufferUsageFlags {
    TransferSrc                       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    TransferDst                       = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    UniformTexelBuffer                = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
    StorageTexelBuffer                = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
    UniformBuffer                     = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    StorageBuffer                     = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    IndexBuffer                       = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VertexBuffer                      = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    IndirectBuffer                    = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
    ShaderDeviceAddress               = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    TransformFeedbackBufferEXT        = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
    TransformFeedbackCounterBufferEXT = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
    ConditionalRenderingEXT           = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
    RayTracingKHR                     = VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR,
    RayTracingNV                      = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
    ShaderDeviceAddressEXT            = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT,
    ShaderDeviceAddressKHR            = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR
};

ETNA_DEFINE_VK_FLAGS(BufferUsage)

enum class CommandBufferUsage : VkCommandBufferUsageFlags {
    OneTimeSubmit      = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    RenderPassContinue = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    SimultaneousUse    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
};

ETNA_DEFINE_VK_FLAGS(CommandBufferUsage)

enum class CommandPoolCreate : VkCommandPoolCreateFlags {
    Transient          = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    ResetCommandBuffer = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    Protected          = VK_COMMAND_POOL_CREATE_PROTECTED_BIT
};

ETNA_DEFINE_VK_FLAGS(CommandPoolCreate)

enum class ShaderStage : VkShaderStageFlags {
    Vertex                 = VK_SHADER_STAGE_VERTEX_BIT,
    TessellationControl    = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    Geometry               = VK_SHADER_STAGE_GEOMETRY_BIT,
    Fragment               = VK_SHADER_STAGE_FRAGMENT_BIT,
    Compute                = VK_SHADER_STAGE_COMPUTE_BIT,
    AllGraphics            = VK_SHADER_STAGE_ALL_GRAPHICS,
    All                    = VK_SHADER_STAGE_ALL,
    RaygenKHR              = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    AnyHitKHR              = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
    ClosestHitKHR          = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    MissKHR                = VK_SHADER_STAGE_MISS_BIT_KHR,
    IntersectionKHR        = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
    CallableKHR            = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
    TaskNV                 = VK_SHADER_STAGE_TASK_BIT_NV,
    MeshNV                 = VK_SHADER_STAGE_MESH_BIT_NV,
    RaygenNV               = VK_SHADER_STAGE_RAYGEN_BIT_NV,
    AnyHitNV               = VK_SHADER_STAGE_ANY_HIT_BIT_NV,
    ClosestHitNV           = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV,
    MissNV                 = VK_SHADER_STAGE_MISS_BIT_NV,
    IntersectionNV         = VK_SHADER_STAGE_INTERSECTION_BIT_NV,
    CallableNV             = VK_SHADER_STAGE_CALLABLE_BIT_NV
};

ETNA_DEFINE_VK_FLAGS(ShaderStage)

enum class PipelineStage : VkPipelineStageFlags {
    TopOfPipe                     = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    DrawIndirect                  = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
    VertexInput                   = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
    VertexShader                  = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
    TessellationControlShader     = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
    TessellationEvaluationShader  = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
    GeometryShader                = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
    FragmentShader                = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    EarlyFragmentTests            = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    LateFragmentTests             = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    ColorAttachmentOutput         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    ComputeShader                 = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    Transfer                      = VK_PIPELINE_STAGE_TRANSFER_BIT,
    BottomOfPipe                  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    Host                          = VK_PIPELINE_STAGE_HOST_BIT,
    AllGraphics                   = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
    AllCommands                   = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    TransformFeedbackEXT          = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
    ConditionalRenderingEXT       = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
    RayTracingShaderKHR           = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
    AccelerationStructureBuildKHR = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
    ShadingRateImageNV            = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
    TaskShaderNV                  = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
    MeshShaderNV                  = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
    FragmentDensityProcessEXT     = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
    CommandPreprocessNV           = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
    RayTracingShaderNV            = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
    AccelerationStructureBuildNV  = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
};

ETNA_DEFINE_VK_FLAGS(PipelineStage)

enum class Access : VkAccessFlags {
    IndirectCommandRead               = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
    IndexRead                         = VK_ACCESS_INDEX_READ_BIT,
    VertexAttributeRead               = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
    UniformRead                       = VK_ACCESS_UNIFORM_READ_BIT,
    InputAttachmentRead               = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
    ShaderRead                        = VK_ACCESS_SHADER_READ_BIT,
    ShaderWrite                       = VK_ACCESS_SHADER_WRITE_BIT,
    ColorAttachmentRead               = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
    ColorAttachmentWrite              = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    DepthStencilAttachmentRead        = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    DepthStencilAttachmentWrite       = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    TransferRead                      = VK_ACCESS_TRANSFER_READ_BIT,
    TransferWrite                     = VK_ACCESS_TRANSFER_WRITE_BIT,
    HostRead                          = VK_ACCESS_HOST_READ_BIT,
    HostWrite                         = VK_ACCESS_HOST_WRITE_BIT,
    MemoryRead                        = VK_ACCESS_MEMORY_READ_BIT,
    MemoryWrite                       = VK_ACCESS_MEMORY_WRITE_BIT,
    TransformFeedbackWriteEXT         = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
    TransformFeedbackCounterReadEXT   = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
    TransformFeedbackCounterWriteEXT  = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
    ConditionalRenderingReadEXT       = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
    ColorAttachmentReadNoncoherentEXT = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
    AccelerationStructureReadKHR      = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
    AccelerationStructureWriteKHR     = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
    ShadingRateImageReadNV            = VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV,
    FragmentDensityMapReadEXT         = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
    CommandPreprocessReadNV           = VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV,
    CommandPreprocessWriteNV          = VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV,
    AccelerationStructureReadNV       = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV,
    AccelerationStructureWriteNV      = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV
};

ETNA_DEFINE_VK_FLAGS(Access)

enum class QueueFlags : VkQueueFlags {
    Graphics      = VK_QUEUE_GRAPHICS_BIT,
    Compute       = VK_QUEUE_COMPUTE_BIT,
    Transfer      = VK_QUEUE_TRANSFER_BIT,
    SparseBinding = VK_QUEUE_SPARSE_BINDING_BIT,
    Protected     = VK_QUEUE_PROTECTED_BIT
};

ETNA_DEFINE_VK_FLAGS_SUFFIXED(QueueFlags)

enum class ImageAspect : VkImageAspectFlags {
    Color           = VK_IMAGE_ASPECT_COLOR_BIT,
    Depth           = VK_IMAGE_ASPECT_DEPTH_BIT,
    Stencil         = VK_IMAGE_ASPECT_STENCIL_BIT,
    Metadata        = VK_IMAGE_ASPECT_METADATA_BIT,
    Plane0          = VK_IMAGE_ASPECT_PLANE_0_BIT,
    Plane1          = VK_IMAGE_ASPECT_PLANE_1_BIT,
    Plane2          = VK_IMAGE_ASPECT_PLANE_2_BIT,
    MemoryPlane0EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
    MemoryPlane1EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
    MemoryPlane2EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
    MemoryPlane3EXT = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT,
    Plane0KHR       = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR,
    Plane1KHR       = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
    Plane2KHR       = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR
};

ETNA_DEFINE_VK_FLAGS(ImageAspect)

template <typename T>
class Mask final {
  public:
    using mask_type = std::underlying_type_t<T>;

    constexpr Mask() noexcept = default;
    constexpr Mask(T value) noexcept : m_value(static_cast<mask_type>(value)) {}

    constexpr bool operator==(Mask<T> rhs) const noexcept { return m_value == rhs.m_value; }
    constexpr bool operator!=(Mask<T> rhs) const noexcept { return m_value != rhs.m_value; }

    constexpr explicit operator bool() const noexcept { return m_value; }

    constexpr auto operator|(T rhs) const noexcept { return Mask<T>(m_value | static_cast<mask_type>(rhs)); }
    constexpr auto operator&(T rhs) const noexcept { return Mask<T>(m_value & static_cast<mask_type>(rhs)); }

    constexpr operator T() const noexcept { return static_cast<T>(m_value); }

    constexpr auto GetVk() const noexcept { return m_value; }

  private:
    template <typename T>
    requires composable_flags<T>::value friend constexpr auto operator|(T, T) noexcept;

    constexpr Mask(mask_type value) noexcept : m_value(value) {}

    mask_type m_value{};
};

template <typename T>
inline auto GetVk(Mask<T> mask) noexcept
{
    return mask.GetVk();
}

template <typename T>
requires composable_flags<T>::value inline constexpr auto operator|(T lhs, T rhs) noexcept
{
    using mask_type = std::underlying_type_t<T>;
    return Mask<T>(static_cast<mask_type>(lhs) | static_cast<mask_type>(rhs));
}

using Offset2D            = VkOffset2D;
using Extent2D            = VkExtent2D;
using Extent3D            = VkExtent3D;
using Rect2D              = VkRect2D;
using Viewport            = VkViewport;
using ExtensionProperties = VkExtensionProperties;
using DescriptorPoolSize  = VkDescriptorPoolSize;

struct ClearColor final {
    constexpr ClearColor(float r, float g, float b, float a = 1.0f) noexcept
    {
        value.float32[0] = r;
        value.float32[1] = g;
        value.float32[2] = b;
        value.float32[3] = a;
    }

    static const ClearColor Black;
    static const ClearColor Transparent;
    static const ClearColor White;

    VkClearColorValue value{};
};

inline const ClearColor ClearColor::Black       = ClearColor(0, 0, 0);
inline const ClearColor ClearColor::Transparent = ClearColor(0, 0, 0, 0);
inline const ClearColor ClearColor::White       = ClearColor(1, 1, 1);

struct ClearDepthStencil final {
    constexpr ClearDepthStencil(float depth, uint32_t stencil) : value{ depth, stencil } {}

    static const ClearDepthStencil Default;

    VkClearDepthStencilValue value{};
};

inline const ClearDepthStencil ClearDepthStencil::Default = ClearDepthStencil(1.0f, 0);

struct ClearValue final {
    constexpr ClearValue(ClearColor color) noexcept : color(color), depth_stencil(0, 0), is_color(true) {}

    constexpr ClearValue(ClearDepthStencil depth_stencil) noexcept
        : color(0, 0, 0), depth_stencil(depth_stencil), is_color(false)
    {}

    constexpr operator VkClearValue() const noexcept
    {
        VkClearValue clear_value;
        if (is_color) {
            clear_value.color = color.value;
        } else {
            clear_value.depthStencil = depth_stencil.value;
        }
        return clear_value;
    }

  private:
    ClearColor        color;
    ClearDepthStencil depth_stencil;
    bool              is_color;
};

template <typename T>
class UniqueHandle {
  public:
    constexpr UniqueHandle() = default;

    constexpr explicit UniqueHandle(const T& value) noexcept : m_value(value) {}

    UniqueHandle(const UniqueHandle&) = delete;

    constexpr UniqueHandle(UniqueHandle&& other) noexcept : m_value(other.release()) {}

    ~UniqueHandle() noexcept
    {
        if (m_value) {
            m_value.Destroy();
        }
    }

    UniqueHandle& operator=(const UniqueHandle&) = delete;

    constexpr UniqueHandle& operator=(UniqueHandle&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    constexpr explicit operator bool() const noexcept { return m_value.operator bool(); }

    constexpr const T* operator->() const noexcept { return &m_value; }

    constexpr T* operator->() noexcept { return &m_value; }

    constexpr const T& operator*() const noexcept { return m_value; }

    constexpr T& operator*() noexcept { return m_value; }

    constexpr const T& get() const noexcept { return m_value; }

    constexpr T& get() noexcept { return m_value; }

    constexpr void reset(const T& value = T()) noexcept
    {
        if (m_value != value) {
            if (m_value) {
                m_value.Destroy();
            }
            m_value = value;
        }
    }

    constexpr T release() noexcept { return std::exchange(m_value, nullptr); }

    constexpr void swap(UniqueHandle<T>& rhs) noexcept { std::swap(m_value, rhs.m_value); }

  private:
    T m_value{};
};

void throw_runtime_error(const char* description);

template <class T, class U>
struct have_same_sign : std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value> {};

template <class DstT, class SrcT>
constexpr DstT narrow_cast(SrcT src)
{
    DstT dst = static_cast<DstT>(src);

    if (static_cast<SrcT>(dst) != src) {
        throw_runtime_error("narrow_cast failed");
    }

    if (!have_same_sign<DstT, SrcT>::value && ((dst < DstT{}) != (src < SrcT{}))) {
        throw_runtime_error("narrow_cast failed");
    }

    return dst;
}

struct AttachmentID final {
    explicit constexpr AttachmentID(uint32_t val) noexcept : value(val) {}
    explicit constexpr AttachmentID(size_t val) : value(narrow_cast<uint32_t>(val)) {}
    uint32_t value;
};

struct ReferenceID final {
    explicit constexpr ReferenceID(size_t val) noexcept : value(val) {}
    size_t value;
};

class Buffer;
class CommandBuffer;
class CommandPool;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Device;
class Framebuffer;
class Image2D;
class ImageView2D;
class Instance;
class Pipeline;
class PipelineLayout;
class Queue;
class RenderPass;
class ShaderModule;
class SurfaceKHR;
class WriteDescriptorSet;

using UniqueBuffer              = UniqueHandle<Buffer>;
using UniqueCommandBuffer       = UniqueHandle<CommandBuffer>;
using UniqueCommandPool         = UniqueHandle<CommandPool>;
using UniqueDescriptorPool      = UniqueHandle<DescriptorPool>;
using UniqueDescriptorSetLayout = UniqueHandle<DescriptorSetLayout>;
using UniqueDevice              = UniqueHandle<Device>;
using UniqueFramebuffer         = UniqueHandle<Framebuffer>;
using UniqueImage2D             = UniqueHandle<Image2D>;
using UniqueInstance            = UniqueHandle<Instance>;
using UniqueImageView2D         = UniqueHandle<ImageView2D>;
using UniquePipeline            = UniqueHandle<Pipeline>;
using UniquePipelineLayout      = UniqueHandle<PipelineLayout>;
using UniqueRenderPass          = UniqueHandle<RenderPass>;
using UniqueShaderModule        = UniqueHandle<ShaderModule>;
using UniqueSurfaceKHR          = UniqueHandle<SurfaceKHR>;

} // namespace etna
