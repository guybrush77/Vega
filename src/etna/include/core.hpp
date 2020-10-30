#pragma once

#include <type_traits>
#include <utility>
#include <vulkan/vulkan_core.h>

#define ETNA_DEFINE_ENUM_ANALOGUE(type)                                                                                \
    inline constexpr auto GetVk(type val) noexcept { return static_cast<Vk##type>(val); }

#define ETNA_DEFINE_FLAGS_ANALOGUE(type, vk_type)                                                                      \
    inline constexpr auto GetVk(type val) noexcept { return static_cast<vk_type>(val); }                               \
    template <>                                                                                                        \
    struct composable_flags<type> : std::true_type {};                                                                 \
    using type##Mask = Mask<type>;

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

template <typename T>
concept EnumClass = std::integral_constant<bool, std::is_enum_v<T> && !std::is_convertible_v<T, int>>::value;

template <EnumClass>
class Mask;

enum class MemoryUsage { Unknown, GpuOnly, CpuOnly, CpuToGpu, GpuToCpu, CpuCopy, GpuLazilyAllocated };

enum class DepthTest { Disable, Enable };
enum class DepthWrite { Disable, Enable };

enum class AttachmentLoadOp {
    Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
    Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DontCare = VK_ATTACHMENT_LOAD_OP_DONT_CARE
};

ETNA_DEFINE_ENUM_ANALOGUE(AttachmentLoadOp)

enum class Result {
    Success                                     = VK_SUCCESS,
    NotReady                                    = VK_NOT_READY,
    Timeout                                     = VK_TIMEOUT,
    EventSet                                    = VK_EVENT_SET,
    EventReset                                  = VK_EVENT_RESET,
    Incomplete                                  = VK_INCOMPLETE,
    ErrorOutOfHostMemory                        = VK_ERROR_OUT_OF_HOST_MEMORY,
    ErrorOutOfDeviceMemory                      = VK_ERROR_OUT_OF_DEVICE_MEMORY,
    ErrorInitializationFailed                   = VK_ERROR_INITIALIZATION_FAILED,
    ErrorDeviceLost                             = VK_ERROR_DEVICE_LOST,
    ErrorMemoryMapFailed                        = VK_ERROR_MEMORY_MAP_FAILED,
    ErrorLayerNotPresent                        = VK_ERROR_LAYER_NOT_PRESENT,
    ErrorExtensionNotPresent                    = VK_ERROR_EXTENSION_NOT_PRESENT,
    ErrorFeatureNotPresent                      = VK_ERROR_FEATURE_NOT_PRESENT,
    ErrorIncompatibleDriver                     = VK_ERROR_INCOMPATIBLE_DRIVER,
    ErrorTooManyObjects                         = VK_ERROR_TOO_MANY_OBJECTS,
    ErrorFormatNotSupported                     = VK_ERROR_FORMAT_NOT_SUPPORTED,
    ErrorFragmentedPool                         = VK_ERROR_FRAGMENTED_POOL,
    ErrorUnknown                                = VK_ERROR_UNKNOWN,
    ErrorOutOfPoolMemory                        = VK_ERROR_OUT_OF_POOL_MEMORY,
    ErrorInvalidExternalHandle                  = VK_ERROR_INVALID_EXTERNAL_HANDLE,
    ErrorFragmentation                          = VK_ERROR_FRAGMENTATION,
    ErrorInvalidOpaqueCaptureAddress            = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    ErrorSurfaceLostKHR                         = VK_ERROR_SURFACE_LOST_KHR,
    ErrorNativeWindowInUseKHR                   = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    SuboptimalKHR                               = VK_SUBOPTIMAL_KHR,
    ErrorOutOfDateKHR                           = VK_ERROR_OUT_OF_DATE_KHR,
    ErrorIncompatibleDisplayKHR                 = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    ErrorValidationFailedEXT                    = VK_ERROR_VALIDATION_FAILED_EXT,
    ErrorInvalidShaderNV                        = VK_ERROR_INVALID_SHADER_NV,
    ErrorIncompatibleVersionKHR                 = VK_ERROR_INCOMPATIBLE_VERSION_KHR,
    ErrorInvalidDrmFormatModifierPlaneLayoutEXT = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
    ErrorNotPermittedEXT                        = VK_ERROR_NOT_PERMITTED_EXT,
    ErrorFullScreenExclusiveModeLostEXT         = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    ThreadIdleKHR                               = VK_THREAD_IDLE_KHR,
    ThreadDoneKHR                               = VK_THREAD_DONE_KHR,
    OperationDeferredKHR                        = VK_OPERATION_DEFERRED_KHR,
    OperationNotDeferredKHR                     = VK_OPERATION_NOT_DEFERRED_KHR,
    ErrorPipelineCompileRequiredEXT             = VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
    ErrorOutOfPoolMemoryKHR                     = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
    ErrorInvalidExternalHandleKHR               = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
    ErrorFragmentationEXT                       = VK_ERROR_FRAGMENTATION_EXT,
    ErrorInvalidDeviceAddressEXT                = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
    ErrorInvalidOpaqueCaptureAddressKHR         = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR
};

ETNA_DEFINE_ENUM_ANALOGUE(Result)

const char* to_string(Result value);

enum class AttachmentStoreOp { Store = VK_ATTACHMENT_STORE_OP_STORE, DontCare = VK_ATTACHMENT_STORE_OP_DONT_CARE };

ETNA_DEFINE_ENUM_ANALOGUE(AttachmentStoreOp)

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

ETNA_DEFINE_ENUM_ANALOGUE(Format)

enum class ColorSpaceKHR {
    SrgbNonlinear             = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    DisplayP3NonlinearEXT     = VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
    ExtendedSrgbLinearEXT     = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
    DisplayP3LinearEXT        = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
    DciP3NonlinearEXT         = VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
    Bt709LinearEXT            = VK_COLOR_SPACE_BT709_LINEAR_EXT,
    Bt709NonlinearEXT         = VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
    Bt2020LinearEXT           = VK_COLOR_SPACE_BT2020_LINEAR_EXT,
    Hdr10St2084EXT            = VK_COLOR_SPACE_HDR10_ST2084_EXT,
    DolbyvisionEXT            = VK_COLOR_SPACE_DOLBYVISION_EXT,
    Hdr10HlgEXT               = VK_COLOR_SPACE_HDR10_HLG_EXT,
    AdobergbLinearEXT         = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
    AdobergbNonlinearEXT      = VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
    PassThroughEXT            = VK_COLOR_SPACE_PASS_THROUGH_EXT,
    ExtendedSrgbNonlinearEXT  = VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
    DisplayNativeAMD          = VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,
    VkColorspaceSrgbNonlinear = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
    DciP3LinearEXT            = VK_COLOR_SPACE_DCI_P3_LINEAR_EXT
};

ETNA_DEFINE_ENUM_ANALOGUE(ColorSpaceKHR)

enum class PresentModeKHR {
    Immediate               = VK_PRESENT_MODE_IMMEDIATE_KHR,
    Mailbox                 = VK_PRESENT_MODE_MAILBOX_KHR,
    Fifo                    = VK_PRESENT_MODE_FIFO_KHR,
    FifoRelaxed             = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    SharedDemandRefresh     = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
    SharedContinuousRefresh = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
};

ETNA_DEFINE_ENUM_ANALOGUE(PresentModeKHR)

enum class VertexInputRate { Vertex = VK_VERTEX_INPUT_RATE_VERTEX, Instance = VK_VERTEX_INPUT_RATE_INSTANCE };

ETNA_DEFINE_ENUM_ANALOGUE(VertexInputRate)

enum class IndexType {
    Uint16   = VK_INDEX_TYPE_UINT16,
    Uint32   = VK_INDEX_TYPE_UINT32,
    NoneKHR  = VK_INDEX_TYPE_NONE_KHR,
    Uint8EXT = VK_INDEX_TYPE_UINT8_EXT,
    NoneNV   = VK_INDEX_TYPE_NONE_NV
};

ETNA_DEFINE_ENUM_ANALOGUE(IndexType)

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

ETNA_DEFINE_ENUM_ANALOGUE(ImageLayout)

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

ETNA_DEFINE_ENUM_ANALOGUE(DynamicState)

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

ETNA_DEFINE_ENUM_ANALOGUE(CompareOp)

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

ETNA_DEFINE_ENUM_ANALOGUE(DescriptorType)

enum class ImageTiling {
    Optimal              = VK_IMAGE_TILING_OPTIMAL,
    Linear               = VK_IMAGE_TILING_LINEAR,
    DrmFormatModifierEXT = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT
};

ETNA_DEFINE_ENUM_ANALOGUE(ImageTiling)

enum class CommandBufferLevel {
    Primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY
};

ETNA_DEFINE_ENUM_ANALOGUE(CommandBufferLevel)

enum class SubpassContents {
    Inline                  = VK_SUBPASS_CONTENTS_INLINE,
    SecondaryCommandBuffers = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
};

ETNA_DEFINE_ENUM_ANALOGUE(SubpassContents)

enum class PipelineBindPoint {
    Graphics      = VK_PIPELINE_BIND_POINT_GRAPHICS,
    Compute       = VK_PIPELINE_BIND_POINT_COMPUTE,
    RayTracingKHR = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
    RayTracingNV  = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
};

ETNA_DEFINE_ENUM_ANALOGUE(PipelineBindPoint)

enum class DebugUtilsMessageSeverity : VkDebugUtilsMessageSeverityFlagsEXT {
    Verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
    Info    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
    Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    Error   = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
};

ETNA_DEFINE_FLAGS_ANALOGUE(DebugUtilsMessageSeverity, VkDebugUtilsMessageSeverityFlagsEXT)

const char* to_string(DebugUtilsMessageSeverity value) noexcept;

enum class DebugUtilsMessageType : VkDebugUtilsMessageTypeFlagsEXT {
    General     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
    Validation  = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    Performance = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
};

ETNA_DEFINE_FLAGS_ANALOGUE(DebugUtilsMessageType, VkDebugUtilsMessageTypeFlagsEXT)

const char* to_string(DebugUtilsMessageType value) noexcept;

enum class FormatFeature : VkFormatFeatureFlags {
    SampledImage                            = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
    StorageImage                            = VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
    StorageImageAtomic                      = VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,
    UniformTexelBuffer                      = VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,
    StorageTexelBuffer                      = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT,
    StorageTexelBufferAtomic                = VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT,
    VertexBuffer                            = VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,
    ColorAttachment                         = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,
    ColorAttachmentBlend                    = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,
    DepthStencilAttachment                  = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
    BlitSrc                                 = VK_FORMAT_FEATURE_BLIT_SRC_BIT,
    BlitDst                                 = VK_FORMAT_FEATURE_BLIT_DST_BIT,
    SampledImageFilterLinear                = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
    TransferSrc                             = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
    TransferDst                             = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
    MidpointChromaSamples                   = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
    SampledImageYcbcrConversionLinearFilter = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,
    SampledImageYcbcrConversionSeparateReconstructionFilter =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
    SampledImageYcbcrConversionChromaReconstructionExplicit =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
    SampledImageYcbcrConversionChromaReconstructionExplicitForceable =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,
    Disjoint                                   = VK_FORMAT_FEATURE_DISJOINT_BIT,
    CositedChromaSamples                       = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,
    SampledImageFilterMinmax                   = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT,
    SampledImageFilterCubicIMG                 = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG,
    AccelerationStructureVertexBufferKHR       = VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR,
    FragmentDensityMapEXT                      = VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT,
    TransferSrcKHR                             = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR,
    TransferDstKHR                             = VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR,
    SampledImageFilterMinmaxEXT                = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT,
    MidpointChromaSamplesKHR                   = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR,
    SampledImageYcbcrConversionLinearFilterKHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR,
    SampledImageYcbcrConversionSeparateReconstructionFilterKHR =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR,
    SampledImageYcbcrConversionChromaReconstructionExplicitKHR =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR,
    SampledImageYcbcrConversionChromaReconstructionExplicitForceableKHR =
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR,
    DisjointKHR                = VK_FORMAT_FEATURE_DISJOINT_BIT_KHR,
    CositedChromaSamplesKHR    = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT_KHR,
    SampledImageFilterCubicEXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT
};

ETNA_DEFINE_FLAGS_ANALOGUE(FormatFeature, VkFormatFeatureFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(ImageUsage, VkImageUsageFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(BufferUsage, VkBufferUsageFlags)

enum class CommandBufferUsage : VkCommandBufferUsageFlags {
    OneTimeSubmit      = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    RenderPassContinue = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    SimultaneousUse    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
};

ETNA_DEFINE_FLAGS_ANALOGUE(CommandBufferUsage, VkCommandBufferUsageFlags)

enum class CommandPoolCreate : VkCommandPoolCreateFlags {
    Transient          = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    ResetCommandBuffer = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    Protected          = VK_COMMAND_POOL_CREATE_PROTECTED_BIT
};

ETNA_DEFINE_FLAGS_ANALOGUE(CommandPoolCreate, VkCommandPoolCreateFlags)

enum class FenceCreate : VkFenceCreateFlags { Signaled = VK_FENCE_CREATE_SIGNALED_BIT };

ETNA_DEFINE_FLAGS_ANALOGUE(FenceCreate, VkCommandPoolCreateFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(ShaderStage, VkShaderStageFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(PipelineStage, VkPipelineStageFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(Access, VkAccessFlags)

enum class Dependency : VkDependencyFlags {
    ByRegion       = VK_DEPENDENCY_BY_REGION_BIT,
    DeviceGroup    = VK_DEPENDENCY_DEVICE_GROUP_BIT,
    ViewLocal      = VK_DEPENDENCY_VIEW_LOCAL_BIT,
    ViewLocalKHR   = VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR,
    DeviceGroupKHR = VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR
};

ETNA_DEFINE_FLAGS_ANALOGUE(Dependency, VkDependencyFlags)

enum class QueueFlags : VkQueueFlags {
    Graphics      = VK_QUEUE_GRAPHICS_BIT,
    Compute       = VK_QUEUE_COMPUTE_BIT,
    Transfer      = VK_QUEUE_TRANSFER_BIT,
    SparseBinding = VK_QUEUE_SPARSE_BINDING_BIT,
    Protected     = VK_QUEUE_PROTECTED_BIT
};

ETNA_DEFINE_FLAGS_ANALOGUE(QueueFlags, VkQueueFlags)

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

ETNA_DEFINE_FLAGS_ANALOGUE(ImageAspect, VkImageAspectFlags)

enum class SurfaceTransformKHR : VkSurfaceTransformFlagsKHR {
    Identity                  = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    Rotate90                  = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
    Rotate180                 = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
    Rotate270                 = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
    HorizontalMirror          = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
    HorizontalMirrorRotate90  = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
    HorizontalMirrorRotate180 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
    HorizontalMirrorRotate270 = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
    Inherit                   = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR
};

ETNA_DEFINE_FLAGS_ANALOGUE(SurfaceTransformKHR, VkSurfaceTransformFlagsKHR)

enum class CompositeAlphaKHR : VkCompositeAlphaFlagsKHR {
    Opaque         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    PreMultiplied  = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    PostMultiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    Inherit        = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
};

ETNA_DEFINE_FLAGS_ANALOGUE(CompositeAlphaKHR, VkCompositeAlphaFlagsKHR)

template <EnumClass E>
class Mask final {
  public:
    using mask_type = std::underlying_type_t<E>;

    constexpr Mask() noexcept = default;
    constexpr Mask(E value) noexcept : m_value(static_cast<mask_type>(value)) {}

    constexpr explicit operator bool() const noexcept { return m_value; }

    constexpr bool operator==(Mask<E> rhs) const noexcept { return m_value == rhs.m_value; }
    constexpr bool operator!=(Mask<E> rhs) const noexcept { return m_value != rhs.m_value; }

    constexpr bool operator==(E rhs) const noexcept { return m_value == static_cast<mask_type>(rhs); }
    constexpr bool operator!=(E rhs) const noexcept { return m_value != static_cast<mask_type>(rhs); }

    constexpr auto operator|(E rhs) const noexcept { return Mask<E>(m_value | static_cast<mask_type>(rhs)); }
    constexpr auto operator&(E rhs) const noexcept { return Mask<E>(m_value & static_cast<mask_type>(rhs)); }

    constexpr operator E() const noexcept { return static_cast<E>(m_value); }

    constexpr auto GetVk() const noexcept { return m_value; }

  private:
    template <EnumClass T>
    requires composable_flags<T>::value friend constexpr auto operator|(T, T) noexcept;

    explicit constexpr Mask(mask_type value) noexcept : m_value(value) {}

    mask_type m_value{};
};

template <EnumClass E>
inline auto GetVk(Mask<E> mask) noexcept
{
    return mask.GetVk();
}

template <EnumClass E>
requires composable_flags<E>::value inline constexpr auto operator|(E lhs, E rhs) noexcept
{
    return Mask(lhs) | rhs;
}

template <EnumClass E>
requires composable_flags<E>::value inline constexpr auto operator|(E lhs, Mask<E> rhs) noexcept
{
    return rhs | lhs;
}

template <EnumClass E>
requires composable_flags<E>::value inline constexpr auto operator&(E lhs, E rhs) noexcept
{
    return Mask(lhs) & rhs;
}

template <EnumClass E>
requires composable_flags<E>::value inline constexpr auto operator&(E lhs, Mask<E> rhs) noexcept
{
    return rhs & lhs;
}

using Offset2D            = VkOffset2D;
using Extent2D            = VkExtent2D;
using Extent3D            = VkExtent3D;
using Rect2D              = VkRect2D;
using Viewport            = VkViewport;
using ExtensionProperties = VkExtensionProperties;
using DescriptorPoolSize  = VkDescriptorPoolSize;

struct QueueFamilyProperties final {
    QueueFlags queueFlags;
    uint32_t   queueCount;
    uint32_t   timestampValidBits;
    Extent3D   minImageTransferGranularity;

    constexpr operator VkQueueFamilyProperties() const noexcept
    {
        return { GetVk(queueFlags), queueCount, timestampValidBits, minImageTransferGranularity };
    }
};

struct SurfaceFormatKHR final {
    Format        format;
    ColorSpaceKHR colorSpace;

    bool operator==(const SurfaceFormatKHR&) const = default;

    constexpr operator VkSurfaceFormatKHR() const noexcept { return { GetVk(format), GetVk(colorSpace) }; }
};

struct FormatProperties {
    FormatFeature linearTilingFeatures;
    FormatFeature optimalTilingFeatures;
    FormatFeature bufferFeatures;

    bool operator==(const FormatProperties&) const = default;

    constexpr operator VkFormatProperties() const noexcept
    {
        return { GetVk(linearTilingFeatures), GetVk(optimalTilingFeatures), GetVk(bufferFeatures) };
    }
};

struct SurfaceCapabilitiesKHR final {
    uint32_t            minImageCount;
    uint32_t            maxImageCount;
    Extent2D            currentExtent;
    Extent2D            minImageExtent;
    Extent2D            maxImageExtent;
    uint32_t            maxImageArrayLayers;
    SurfaceTransformKHR supportedTransforms;
    SurfaceTransformKHR currentTransform;
    CompositeAlphaKHR   supportedCompositeAlpha;
    ImageUsage          supportedUsageFlags;
};

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

void throw_etna_error(const char* file, int line, Result result);
void throw_etna_error(const char* file, int line, const char* description);

template <typename T, typename U>
struct have_same_sign : std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value> {};

template <typename DstT, typename SrcT>
constexpr DstT narrow_cast(SrcT src)
{
    if constexpr (std::is_same_v<SrcT, DstT>) {
        return src;
    }

    DstT dst = static_cast<DstT>(src);

    if (static_cast<SrcT>(dst) != src) {
        throw_etna_error(__FILE__, __LINE__, "narrow_cast failed");
    }

    if (!have_same_sign<DstT, SrcT>::value && ((dst < DstT{}) != (src < SrcT{}))) {
        throw_etna_error(__FILE__, __LINE__, "narrow_cast failed");
    }

    return dst;
}

namespace detail {
template <typename T, size_t N>
struct ArrayViewBuffer {
    T data[N];
};
template <typename T>
struct ArrayViewBuffer<T, 0> {
    inline static T* data = nullptr;
};
} // namespace detail

template <typename T>
class Return final {
  public:
    constexpr Return() noexcept = default;
    constexpr explicit Return(T value, Result result = Result::Success) : m_value(std::move(value)), m_result(result) {}
    constexpr explicit Return(Result result) : m_result(result) {}

    explicit operator bool() const noexcept { return m_result == Result::Success; }

    bool operator==(const Return&) const noexcept = default;

    T value() const
    {
        if (m_result != Result::Success) {
            throw_etna_error(__FILE__, __LINE__, to_string(m_result));
        }
        return m_value;
    }

    T value_or(const T& value) const noexcept { return m_value == Result::Success ? m_value : value; }

    Result result() const noexcept { return m_result; }

  private:
    T      m_value{};
    Result m_result{};
};

template <typename T>
struct ArrayView final {
    using value_type      = T;
    using size_type       = uint32_t;
    using pointer         = value_type*;
    using reference       = value_type&;
    using const_pointer   = const pointer;
    using const_reference = const reference;
    using const_iterator  = const value_type*;

    ArrayView() noexcept {}

    ArrayView(std::initializer_list<T> items) noexcept
    {
        if (items.size() > 0) {
            m.free = items.size() > kBufferElems;
            m.data = m.free ? new T[items.size()] : buffer.data;
            m.size = narrow_cast<size_type>(items.size());

            int index = 0;
            for (const value_type& value : items) {
                m.data[index++] = value;
            }
        }
    }

    template <size_t N>
    ArrayView(T (&arr)[N]) noexcept : m{ arr, N }
    {}

    ~ArrayView() noexcept
    {
        static_assert(sizeof(ArrayView<T>) <= kMaxTypeSize);
        if (m.free) {
            delete[] m.data;
        }
    }

    const_reference operator[](size_t index) const noexcept { return *(m.data + index); }

    bool operator==(const ArrayView& rhs) const noexcept
    {
        if (this != &rhs) {
            if (m.size != rhs.m.size) {
                return false;
            }
            for (size_type i = 0; i < m.size; ++i) {
                if (m.data[i] != rhs.m.data[i]) {
                    return false;
                }
            }
        }
        return true;
    }

    const_iterator begin() const noexcept { return m.data; }
    const_iterator end() const noexcept { return m.data + m.size; }

    size_type size() const noexcept { return m.size; }
    bool      empty() const noexcept { return m.size == 0; }

  private:
    struct {
        pointer   data{};
        size_type size{};
        bool      free{};
    } m;

    static constexpr size_t kMaxTypeSize = 64; // in bytes
    static constexpr size_t kBufferElems = (kMaxTypeSize - sizeof(m)) / sizeof(T);

    detail::ArrayViewBuffer<T, kBufferElems> buffer;
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

struct AttachmentID final {
    explicit constexpr AttachmentID(uint32_t val) noexcept : value(val) {}
    explicit constexpr AttachmentID(size_t val) : value(narrow_cast<uint32_t>(val)) {}
    uint32_t value;
};

struct ReferenceID final {
    explicit constexpr ReferenceID(size_t val) noexcept : value(val) {}
    size_t value;
};

struct SubpassID final {
    static const SubpassID External;

    explicit constexpr SubpassID(uint32_t val) noexcept : value(val) {}
    explicit constexpr SubpassID(size_t val) : value(narrow_cast<uint32_t>(val)) {}
    uint32_t value;
};

inline const SubpassID SubpassID::External = SubpassID{ VK_SUBPASS_EXTERNAL };

class Buffer;
class CommandBuffer;
class CommandPool;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class Device;
class Fence;
class Framebuffer;
class Image2D;
class ImageView2D;
class Instance;
class Pipeline;
class PipelineLayout;
class Queue;
class RenderPass;
class Semaphore;
class ShaderModule;
class SurfaceKHR;
class SwapchainKHR;
class WriteDescriptorSet;

using UniqueBuffer              = UniqueHandle<Buffer>;
using UniqueCommandBuffer       = UniqueHandle<CommandBuffer>;
using UniqueCommandPool         = UniqueHandle<CommandPool>;
using UniqueDescriptorPool      = UniqueHandle<DescriptorPool>;
using UniqueDescriptorSetLayout = UniqueHandle<DescriptorSetLayout>;
using UniqueDevice              = UniqueHandle<Device>;
using UniqueFence               = UniqueHandle<Fence>;
using UniqueFramebuffer         = UniqueHandle<Framebuffer>;
using UniqueImage2D             = UniqueHandle<Image2D>;
using UniqueInstance            = UniqueHandle<Instance>;
using UniqueImageView2D         = UniqueHandle<ImageView2D>;
using UniquePipeline            = UniqueHandle<Pipeline>;
using UniquePipelineLayout      = UniqueHandle<PipelineLayout>;
using UniqueRenderPass          = UniqueHandle<RenderPass>;
using UniqueSemaphore           = UniqueHandle<Semaphore>;
using UniqueShaderModule        = UniqueHandle<ShaderModule>;
using UniqueSurfaceKHR          = UniqueHandle<SurfaceKHR>;
using UniqueSwapchainKHR        = UniqueHandle<SwapchainKHR>;

} // namespace etna
