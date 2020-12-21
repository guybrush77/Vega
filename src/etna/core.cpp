#include "etna/core.hpp"

#include <stdexcept>

namespace etna {

class etna_error : public std::exception {
  public:
    etna_error(const char* /*file*/, int /*line*/, Result /*result*/) noexcept {}

    etna_error(const char* /*file*/, int /*line*/, const char* /*description*/) noexcept {}

    virtual char const* what() const noexcept override { return "TODO"; }
};

const char* to_string(Result value) noexcept
{
    switch (value) {
    case Result::Success: return "Success";
    case Result::NotReady: return "NotReady";
    case Result::Timeout: return "Timeout";
    case Result::EventSet: return "EventSet";
    case Result::EventReset: return "EventReset";
    case Result::Incomplete: return "Incomplete";
    case Result::ErrorOutOfHostMemory: return "ErrorOutOfHostMemory";
    case Result::ErrorOutOfDeviceMemory: return "ErrorOutOfDeviceMemory";
    case Result::ErrorInitializationFailed: return "ErrorInitializationFailed";
    case Result::ErrorDeviceLost: return "ErrorDeviceLost";
    case Result::ErrorMemoryMapFailed: return "ErrorMemoryMapFailed";
    case Result::ErrorLayerNotPresent: return "ErrorLayerNotPresent";
    case Result::ErrorExtensionNotPresent: return "ErrorExtensionNotPresent";
    case Result::ErrorFeatureNotPresent: return "ErrorFeatureNotPresent";
    case Result::ErrorIncompatibleDriver: return "ErrorIncompatibleDriver";
    case Result::ErrorTooManyObjects: return "ErrorTooManyObjects";
    case Result::ErrorFormatNotSupported: return "ErrorFormatNotSupported";
    case Result::ErrorFragmentedPool: return "ErrorFragmentedPool";
    case Result::ErrorUnknown: return "ErrorUnknown";
    case Result::ErrorOutOfPoolMemory: return "ErrorOutOfPoolMemory";
    case Result::ErrorInvalidExternalHandle: return "ErrorInvalidExternalHandle";
    case Result::ErrorFragmentation: return "ErrorFragmentation";
    case Result::ErrorInvalidOpaqueCaptureAddress: return "ErrorInvalidOpaqueCaptureAddress";
    case Result::ErrorSurfaceLostKHR: return "ErrorSurfaceLostKHR";
    case Result::ErrorNativeWindowInUseKHR: return "ErrorNativeWindowInUseKHR";
    case Result::SuboptimalKHR: return "SuboptimalKHR";
    case Result::ErrorOutOfDateKHR: return "ErrorOutOfDateKHR";
    case Result::ErrorIncompatibleDisplayKHR: return "ErrorIncompatibleDisplayKHR";
    case Result::ErrorValidationFailedEXT: return "ErrorValidationFailedEXT";
    case Result::ErrorInvalidShaderNV: return "ErrorInvalidShaderNV";
    case Result::ErrorIncompatibleVersionKHR: return "ErrorIncompatibleVersionKHR";
    case Result::ErrorInvalidDrmFormatModifierPlaneLayoutEXT: return "ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
    case Result::ErrorNotPermittedEXT: return "ErrorNotPermittedEXT";
    case Result::ErrorFullScreenExclusiveModeLostEXT: return "ErrorFullScreenExclusiveModeLostEXT";
    case Result::ThreadIdleKHR: return "ThreadIdleKHR";
    case Result::ThreadDoneKHR: return "ThreadDoneKHR";
    case Result::OperationDeferredKHR: return "OperationDeferredKHR";
    case Result::OperationNotDeferredKHR: return "OperationNotDeferredKHR";
    case Result::ErrorPipelineCompileRequiredEXT: return "ErrorPipelineCompileRequiredEXT";
    default: return "invalid";
    }
}

const char* to_string(PhysicalDeviceType value) noexcept
{
    switch (value) {
    case PhysicalDeviceType::Other: return "Other";
    case PhysicalDeviceType::IntegratedGpu: return "Integrated Gpu";
    case PhysicalDeviceType::DiscreteGpu: return "Discrete Gpu";
    case PhysicalDeviceType::VirtualGpu: return "Virtual Gpu";
    case PhysicalDeviceType::Cpu: return "Cpu";
    default: return "invalid";
    }
}

const char* to_string(Format value) noexcept
{
    switch (value) {
    case Format::Undefined: return "Undefined";
    case Format::R4G4UnormPack8: return "R4G4UnormPack8";
    case Format::R4G4B4A4UnormPack16: return "R4G4B4A4UnormPack16";
    case Format::B4G4R4A4UnormPack16: return "B4G4R4A4UnormPack16";
    case Format::R5G6B5UnormPack16: return "R5G6B5UnormPack16";
    case Format::B5G6R5UnormPack16: return "B5G6R5UnormPack16";
    case Format::R5G5B5A1UnormPack16: return "R5G5B5A1UnormPack16";
    case Format::B5G5R5A1UnormPack16: return "B5G5R5A1UnormPack16";
    case Format::A1R5G5B5UnormPack16: return "A1R5G5B5UnormPack16";
    case Format::R8Unorm: return "R8Unorm";
    case Format::R8Snorm: return "R8Snorm";
    case Format::R8Uscaled: return "R8Uscaled";
    case Format::R8Sscaled: return "R8Sscaled";
    case Format::R8Uint: return "R8Uint";
    case Format::R8Sint: return "R8Sint";
    case Format::R8Srgb: return "R8Srgb";
    case Format::R8G8Unorm: return "R8G8Unorm";
    case Format::R8G8Snorm: return "R8G8Snorm";
    case Format::R8G8Uscaled: return "R8G8Uscaled";
    case Format::R8G8Sscaled: return "R8G8Sscaled";
    case Format::R8G8Uint: return "R8G8Uint";
    case Format::R8G8Sint: return "R8G8Sint";
    case Format::R8G8Srgb: return "R8G8Srgb";
    case Format::R8G8B8Unorm: return "R8G8B8Unorm";
    case Format::R8G8B8Snorm: return "R8G8B8Snorm";
    case Format::R8G8B8Uscaled: return "R8G8B8Uscaled";
    case Format::R8G8B8Sscaled: return "R8G8B8Sscaled";
    case Format::R8G8B8Uint: return "R8G8B8Uint";
    case Format::R8G8B8Sint: return "R8G8B8Sint";
    case Format::R8G8B8Srgb: return "R8G8B8Srgb";
    case Format::B8G8R8Unorm: return "B8G8R8Unorm";
    case Format::B8G8R8Snorm: return "B8G8R8Snorm";
    case Format::B8G8R8Uscaled: return "B8G8R8Uscaled";
    case Format::B8G8R8Sscaled: return "B8G8R8Sscaled";
    case Format::B8G8R8Uint: return "B8G8R8Uint";
    case Format::B8G8R8Sint: return "B8G8R8Sint";
    case Format::B8G8R8Srgb: return "B8G8R8Srgb";
    case Format::R8G8B8A8Unorm: return "R8G8B8A8Unorm";
    case Format::R8G8B8A8Snorm: return "R8G8B8A8Snorm";
    case Format::R8G8B8A8Uscaled: return "R8G8B8A8Uscaled";
    case Format::R8G8B8A8Sscaled: return "R8G8B8A8Sscaled";
    case Format::R8G8B8A8Uint: return "R8G8B8A8Uint";
    case Format::R8G8B8A8Sint: return "R8G8B8A8Sint";
    case Format::R8G8B8A8Srgb: return "R8G8B8A8Srgb";
    case Format::B8G8R8A8Unorm: return "B8G8R8A8Unorm";
    case Format::B8G8R8A8Snorm: return "B8G8R8A8Snorm";
    case Format::B8G8R8A8Uscaled: return "B8G8R8A8Uscaled";
    case Format::B8G8R8A8Sscaled: return "B8G8R8A8Sscaled";
    case Format::B8G8R8A8Uint: return "B8G8R8A8Uint";
    case Format::B8G8R8A8Sint: return "B8G8R8A8Sint";
    case Format::B8G8R8A8Srgb: return "B8G8R8A8Srgb";
    case Format::A8B8G8R8UnormPack32: return "A8B8G8R8UnormPack32";
    case Format::A8B8G8R8SnormPack32: return "A8B8G8R8SnormPack32";
    case Format::A8B8G8R8UscaledPack32: return "A8B8G8R8UscaledPack32";
    case Format::A8B8G8R8SscaledPack32: return "A8B8G8R8SscaledPack32";
    case Format::A8B8G8R8UintPack32: return "A8B8G8R8UintPack32";
    case Format::A8B8G8R8SintPack32: return "A8B8G8R8SintPack32";
    case Format::A8B8G8R8SrgbPack32: return "A8B8G8R8SrgbPack32";
    case Format::A2R10G10B10UnormPack32: return "A2R10G10B10UnormPack32";
    case Format::A2R10G10B10SnormPack32: return "A2R10G10B10SnormPack32";
    case Format::A2R10G10B10UscaledPack32: return "A2R10G10B10UscaledPack32";
    case Format::A2R10G10B10SscaledPack32: return "A2R10G10B10SscaledPack32";
    case Format::A2R10G10B10UintPack32: return "A2R10G10B10UintPack32";
    case Format::A2R10G10B10SintPack32: return "A2R10G10B10SintPack32";
    case Format::A2B10G10R10UnormPack32: return "A2B10G10R10UnormPack32";
    case Format::A2B10G10R10SnormPack32: return "A2B10G10R10SnormPack32";
    case Format::A2B10G10R10UscaledPack32: return "A2B10G10R10UscaledPack32";
    case Format::A2B10G10R10SscaledPack32: return "A2B10G10R10SscaledPack32";
    case Format::A2B10G10R10UintPack32: return "A2B10G10R10UintPack32";
    case Format::A2B10G10R10SintPack32: return "A2B10G10R10SintPack32";
    case Format::R16Unorm: return "R16Unorm";
    case Format::R16Snorm: return "R16Snorm";
    case Format::R16Uscaled: return "R16Uscaled";
    case Format::R16Sscaled: return "R16Sscaled";
    case Format::R16Uint: return "R16Uint";
    case Format::R16Sint: return "R16Sint";
    case Format::R16Sfloat: return "R16Sfloat";
    case Format::R16G16Unorm: return "R16G16Unorm";
    case Format::R16G16Snorm: return "R16G16Snorm";
    case Format::R16G16Uscaled: return "R16G16Uscaled";
    case Format::R16G16Sscaled: return "R16G16Sscaled";
    case Format::R16G16Uint: return "R16G16Uint";
    case Format::R16G16Sint: return "R16G16Sint";
    case Format::R16G16Sfloat: return "R16G16Sfloat";
    case Format::R16G16B16Unorm: return "R16G16B16Unorm";
    case Format::R16G16B16Snorm: return "R16G16B16Snorm";
    case Format::R16G16B16Uscaled: return "R16G16B16Uscaled";
    case Format::R16G16B16Sscaled: return "R16G16B16Sscaled";
    case Format::R16G16B16Uint: return "R16G16B16Uint";
    case Format::R16G16B16Sint: return "R16G16B16Sint";
    case Format::R16G16B16Sfloat: return "R16G16B16Sfloat";
    case Format::R16G16B16A16Unorm: return "R16G16B16A16Unorm";
    case Format::R16G16B16A16Snorm: return "R16G16B16A16Snorm";
    case Format::R16G16B16A16Uscaled: return "R16G16B16A16Uscaled";
    case Format::R16G16B16A16Sscaled: return "R16G16B16A16Sscaled";
    case Format::R16G16B16A16Uint: return "R16G16B16A16Uint";
    case Format::R16G16B16A16Sint: return "R16G16B16A16Sint";
    case Format::R16G16B16A16Sfloat: return "R16G16B16A16Sfloat";
    case Format::R32Uint: return "R32Uint";
    case Format::R32Sint: return "R32Sint";
    case Format::R32Sfloat: return "R32Sfloat";
    case Format::R32G32Uint: return "R32G32Uint";
    case Format::R32G32Sint: return "R32G32Sint";
    case Format::R32G32Sfloat: return "R32G32Sfloat";
    case Format::R32G32B32Uint: return "R32G32B32Uint";
    case Format::R32G32B32Sint: return "R32G32B32Sint";
    case Format::R32G32B32Sfloat: return "R32G32B32Sfloat";
    case Format::R32G32B32A32Uint: return "R32G32B32A32Uint";
    case Format::R32G32B32A32Sint: return "R32G32B32A32Sint";
    case Format::R32G32B32A32Sfloat: return "R32G32B32A32Sfloat";
    case Format::R64Uint: return "R64Uint";
    case Format::R64Sint: return "R64Sint";
    case Format::R64Sfloat: return "R64Sfloat";
    case Format::R64G64Uint: return "R64G64Uint";
    case Format::R64G64Sint: return "R64G64Sint";
    case Format::R64G64Sfloat: return "R64G64Sfloat";
    case Format::R64G64B64Uint: return "R64G64B64Uint";
    case Format::R64G64B64Sint: return "R64G64B64Sint";
    case Format::R64G64B64Sfloat: return "R64G64B64Sfloat";
    case Format::R64G64B64A64Uint: return "R64G64B64A64Uint";
    case Format::R64G64B64A64Sint: return "R64G64B64A64Sint";
    case Format::R64G64B64A64Sfloat: return "R64G64B64A64Sfloat";
    case Format::B10G11R11UfloatPack32: return "B10G11R11UfloatPack32";
    case Format::E5B9G9R9UfloatPack32: return "E5B9G9R9UfloatPack32";
    case Format::D16Unorm: return "D16Unorm";
    case Format::X8D24UnormPack32: return "X8D24UnormPack32";
    case Format::D32Sfloat: return "D32Sfloat";
    case Format::S8Uint: return "S8Uint";
    case Format::D16UnormS8Uint: return "D16UnormS8Uint";
    case Format::D24UnormS8Uint: return "D24UnormS8Uint";
    case Format::D32SfloatS8Uint: return "D32SfloatS8Uint";
    case Format::Bc1RgbUnormBlock: return "Bc1RgbUnormBlock";
    case Format::Bc1RgbSrgbBlock: return "Bc1RgbSrgbBlock";
    case Format::Bc1RgbaUnormBlock: return "Bc1RgbaUnormBlock";
    case Format::Bc1RgbaSrgbBlock: return "Bc1RgbaSrgbBlock";
    case Format::Bc2UnormBlock: return "Bc2UnormBlock";
    case Format::Bc2SrgbBlock: return "Bc2SrgbBlock";
    case Format::Bc3UnormBlock: return "Bc3UnormBlock";
    case Format::Bc3SrgbBlock: return "Bc3SrgbBlock";
    case Format::Bc4UnormBlock: return "Bc4UnormBlock";
    case Format::Bc4SnormBlock: return "Bc4SnormBlock";
    case Format::Bc5UnormBlock: return "Bc5UnormBlock";
    case Format::Bc5SnormBlock: return "Bc5SnormBlock";
    case Format::Bc6HUfloatBlock: return "Bc6HUfloatBlock";
    case Format::Bc6HSfloatBlock: return "Bc6HSfloatBlock";
    case Format::Bc7UnormBlock: return "Bc7UnormBlock";
    case Format::Bc7SrgbBlock: return "Bc7SrgbBlock";
    case Format::Etc2R8G8B8UnormBlock: return "Etc2R8G8B8UnormBlock";
    case Format::Etc2R8G8B8SrgbBlock: return "Etc2R8G8B8SrgbBlock";
    case Format::Etc2R8G8B8A1UnormBlock: return "Etc2R8G8B8A1UnormBlock";
    case Format::Etc2R8G8B8A1SrgbBlock: return "Etc2R8G8B8A1SrgbBlock";
    case Format::Etc2R8G8B8A8UnormBlock: return "Etc2R8G8B8A8UnormBlock";
    case Format::Etc2R8G8B8A8SrgbBlock: return "Etc2R8G8B8A8SrgbBlock";
    case Format::EacR11UnormBlock: return "EacR11UnormBlock";
    case Format::EacR11SnormBlock: return "EacR11SnormBlock";
    case Format::EacR11G11UnormBlock: return "EacR11G11UnormBlock";
    case Format::EacR11G11SnormBlock: return "EacR11G11SnormBlock";
    case Format::Astc4x4UnormBlock: return "Astc4x4UnormBlock";
    case Format::Astc4x4SrgbBlock: return "Astc4x4SrgbBlock";
    case Format::Astc5x4UnormBlock: return "Astc5x4UnormBlock";
    case Format::Astc5x4SrgbBlock: return "Astc5x4SrgbBlock";
    case Format::Astc5x5UnormBlock: return "Astc5x5UnormBlock";
    case Format::Astc5x5SrgbBlock: return "Astc5x5SrgbBlock";
    case Format::Astc6x5UnormBlock: return "Astc6x5UnormBlock";
    case Format::Astc6x5SrgbBlock: return "Astc6x5SrgbBlock";
    case Format::Astc6x6UnormBlock: return "Astc6x6UnormBlock";
    case Format::Astc6x6SrgbBlock: return "Astc6x6SrgbBlock";
    case Format::Astc8x5UnormBlock: return "Astc8x5UnormBlock";
    case Format::Astc8x5SrgbBlock: return "Astc8x5SrgbBlock";
    case Format::Astc8x6UnormBlock: return "Astc8x6UnormBlock";
    case Format::Astc8x6SrgbBlock: return "Astc8x6SrgbBlock";
    case Format::Astc8x8UnormBlock: return "Astc8x8UnormBlock";
    case Format::Astc8x8SrgbBlock: return "Astc8x8SrgbBlock";
    case Format::Astc10x5UnormBlock: return "Astc10x5UnormBlock";
    case Format::Astc10x5SrgbBlock: return "Astc10x5SrgbBlock";
    case Format::Astc10x6UnormBlock: return "Astc10x6UnormBlock";
    case Format::Astc10x6SrgbBlock: return "Astc10x6SrgbBlock";
    case Format::Astc10x8UnormBlock: return "Astc10x8UnormBlock";
    case Format::Astc10x8SrgbBlock: return "Astc10x8SrgbBlock";
    case Format::Astc10x10UnormBlock: return "Astc10x10UnormBlock";
    case Format::Astc10x10SrgbBlock: return "Astc10x10SrgbBlock";
    case Format::Astc12x10UnormBlock: return "Astc12x10UnormBlock";
    case Format::Astc12x10SrgbBlock: return "Astc12x10SrgbBlock";
    case Format::Astc12x12UnormBlock: return "Astc12x12UnormBlock";
    case Format::Astc12x12SrgbBlock: return "Astc12x12SrgbBlock";
    case Format::G8B8G8R8422Unorm: return "G8B8G8R8422Unorm";
    case Format::B8G8R8G8422Unorm: return "B8G8R8G8422Unorm";
    case Format::G8B8R83Plane420Unorm: return "G8B8R83Plane420Unorm";
    case Format::G8B8R82Plane420Unorm: return "G8B8R82Plane420Unorm";
    case Format::G8B8R83Plane422Unorm: return "G8B8R83Plane422Unorm";
    case Format::G8B8R82Plane422Unorm: return "G8B8R82Plane422Unorm";
    case Format::G8B8R83Plane444Unorm: return "G8B8R83Plane444Unorm";
    case Format::R10X6UnormPack16: return "R10X6UnormPack16";
    case Format::R10X6G10X6Unorm2Pack16: return "R10X6G10X6Unorm2Pack16";
    case Format::R10X6G10X6B10X6A10X6Unorm4Pack16: return "R10X6G10X6B10X6A10X6Unorm4Pack16";
    case Format::G10X6B10X6G10X6R10X6422Unorm4Pack16: return "G10X6B10X6G10X6R10X6422Unorm4Pack16";
    case Format::B10X6G10X6R10X6G10X6422Unorm4Pack16: return "B10X6G10X6R10X6G10X6422Unorm4Pack16";
    case Format::G10X6B10X6R10X63Plane420Unorm3Pack16: return "G10X6B10X6R10X63Plane420Unorm3Pack16";
    case Format::G10X6B10X6R10X62Plane420Unorm3Pack16: return "G10X6B10X6R10X62Plane420Unorm3Pack16";
    case Format::G10X6B10X6R10X63Plane422Unorm3Pack16: return "G10X6B10X6R10X63Plane422Unorm3Pack16";
    case Format::G10X6B10X6R10X62Plane422Unorm3Pack16: return "G10X6B10X6R10X62Plane422Unorm3Pack16";
    case Format::G10X6B10X6R10X63Plane444Unorm3Pack16: return "G10X6B10X6R10X63Plane444Unorm3Pack16";
    case Format::R12X4UnormPack16: return "R12X4UnormPack16";
    case Format::R12X4G12X4Unorm2Pack16: return "R12X4G12X4Unorm2Pack16";
    case Format::R12X4G12X4B12X4A12X4Unorm4Pack16: return "R12X4G12X4B12X4A12X4Unorm4Pack16";
    case Format::G12X4B12X4G12X4R12X4422Unorm4Pack16: return "G12X4B12X4G12X4R12X4422Unorm4Pack16";
    case Format::B12X4G12X4R12X4G12X4422Unorm4Pack16: return "B12X4G12X4R12X4G12X4422Unorm4Pack16";
    case Format::G12X4B12X4R12X43Plane420Unorm3Pack16: return "G12X4B12X4R12X43Plane420Unorm3Pack16";
    case Format::G12X4B12X4R12X42Plane420Unorm3Pack16: return "G12X4B12X4R12X42Plane420Unorm3Pack16";
    case Format::G12X4B12X4R12X43Plane422Unorm3Pack16: return "G12X4B12X4R12X43Plane422Unorm3Pack16";
    case Format::G12X4B12X4R12X42Plane422Unorm3Pack16: return "G12X4B12X4R12X42Plane422Unorm3Pack16";
    case Format::G12X4B12X4R12X43Plane444Unorm3Pack16: return "G12X4B12X4R12X43Plane444Unorm3Pack16";
    case Format::G16B16G16R16422Unorm: return "G16B16G16R16422Unorm";
    case Format::B16G16R16G16422Unorm: return "B16G16R16G16422Unorm";
    case Format::G16B16R163Plane420Unorm: return "G16B16R163Plane420Unorm";
    case Format::G16B16R162Plane420Unorm: return "G16B16R162Plane420Unorm";
    case Format::G16B16R163Plane422Unorm: return "G16B16R163Plane422Unorm";
    case Format::G16B16R162Plane422Unorm: return "G16B16R162Plane422Unorm";
    case Format::G16B16R163Plane444Unorm: return "G16B16R163Plane444Unorm";
    case Format::Pvrtc12BppUnormBlockIMG: return "Pvrtc12BppUnormBlockIMG";
    case Format::Pvrtc14BppUnormBlockIMG: return "Pvrtc14BppUnormBlockIMG";
    case Format::Pvrtc22BppUnormBlockIMG: return "Pvrtc22BppUnormBlockIMG";
    case Format::Pvrtc24BppUnormBlockIMG: return "Pvrtc24BppUnormBlockIMG";
    case Format::Pvrtc12BppSrgbBlockIMG: return "Pvrtc12BppSrgbBlockIMG";
    case Format::Pvrtc14BppSrgbBlockIMG: return "Pvrtc14BppSrgbBlockIMG";
    case Format::Pvrtc22BppSrgbBlockIMG: return "Pvrtc22BppSrgbBlockIMG";
    case Format::Pvrtc24BppSrgbBlockIMG: return "Pvrtc24BppSrgbBlockIMG";
    case Format::Astc4x4SfloatBlockEXT: return "Astc4x4SfloatBlockEXT";
    case Format::Astc5x4SfloatBlockEXT: return "Astc5x4SfloatBlockEXT";
    case Format::Astc5x5SfloatBlockEXT: return "Astc5x5SfloatBlockEXT";
    case Format::Astc6x5SfloatBlockEXT: return "Astc6x5SfloatBlockEXT";
    case Format::Astc6x6SfloatBlockEXT: return "Astc6x6SfloatBlockEXT";
    case Format::Astc8x5SfloatBlockEXT: return "Astc8x5SfloatBlockEXT";
    case Format::Astc8x6SfloatBlockEXT: return "Astc8x6SfloatBlockEXT";
    case Format::Astc8x8SfloatBlockEXT: return "Astc8x8SfloatBlockEXT";
    case Format::Astc10x5SfloatBlockEXT: return "Astc10x5SfloatBlockEXT";
    case Format::Astc10x6SfloatBlockEXT: return "Astc10x6SfloatBlockEXT";
    case Format::Astc10x8SfloatBlockEXT: return "Astc10x8SfloatBlockEXT";
    case Format::Astc10x10SfloatBlockEXT: return "Astc10x10SfloatBlockEXT";
    case Format::Astc12x10SfloatBlockEXT: return "Astc12x10SfloatBlockEXT";
    case Format::Astc12x12SfloatBlockEXT: return "Astc12x12SfloatBlockEXT";
    default: return "invalid";
    }
}

const char* to_string(ColorSpaceKHR value) noexcept
{
    switch (value) {
    case ColorSpaceKHR::SrgbNonlinear: return "SrgbNonlinear";
    case ColorSpaceKHR::DisplayP3NonlinearEXT: return "DisplayP3NonlinearEXT";
    case ColorSpaceKHR::ExtendedSrgbLinearEXT: return "ExtendedSrgbLinearEXT";
    case ColorSpaceKHR::DisplayP3LinearEXT: return "DisplayP3LinearEXT";
    case ColorSpaceKHR::DciP3NonlinearEXT: return "DciP3NonlinearEXT";
    case ColorSpaceKHR::Bt709LinearEXT: return "Bt709LinearEXT";
    case ColorSpaceKHR::Bt709NonlinearEXT: return "Bt709NonlinearEXT";
    case ColorSpaceKHR::Bt2020LinearEXT: return "Bt2020LinearEXT";
    case ColorSpaceKHR::Hdr10St2084EXT: return "Hdr10St2084EXT";
    case ColorSpaceKHR::DolbyvisionEXT: return "DolbyvisionEXT";
    case ColorSpaceKHR::Hdr10HlgEXT: return "Hdr10HlgEXT";
    case ColorSpaceKHR::AdobergbLinearEXT: return "AdobergbLinearEXT";
    case ColorSpaceKHR::AdobergbNonlinearEXT: return "AdobergbNonlinearEXT";
    case ColorSpaceKHR::PassThroughEXT: return "PassThroughEXT";
    case ColorSpaceKHR::ExtendedSrgbNonlinearEXT: return "ExtendedSrgbNonlinearEXT";
    case ColorSpaceKHR::DisplayNativeAMD: return "DisplayNativeAMD";
    default: return "invalid";
    }
}

const char* to_string(DebugUtilsMessageSeverity value) noexcept
{
    switch (value) {
    case DebugUtilsMessageSeverity::Verbose: return "Verbose";
    case DebugUtilsMessageSeverity::Info: return "Info";
    case DebugUtilsMessageSeverity::Warning: return "Warning";
    case DebugUtilsMessageSeverity::Error: return "Error";
    default: return "invalid";
    }
}

const char* to_string(DebugUtilsMessageType value) noexcept
{
    switch (value) {
    case DebugUtilsMessageType::General: return "General";
    case DebugUtilsMessageType::Validation: return "Validation";
    case DebugUtilsMessageType::Performance: return "Performance";
    default: return "invalid";
    }
}

const char* to_string(DescriptorPoolFlags value) noexcept
{
    switch (value) {
    case DescriptorPoolFlags::FreeDescriptorSet: return "FreeDescriptorSet";
    case DescriptorPoolFlags::UpdateAfterBind: return "UpdateAfterBind";
    default: return "invalid";
    }
}

void throw_etna_error(const char* file, int line, Result result)
{
    throw etna_error(file, line, result);
}

void throw_etna_error(const char* file, int line, const char* description)
{
    throw etna_error(file, line, description);
}

} // namespace etna
