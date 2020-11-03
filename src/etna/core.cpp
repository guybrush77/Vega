#include "include/core.hpp"

#include <stdexcept>

namespace etna {

class etna_error : public std::exception {
  public:
    etna_error(const char* /*file*/, int /*line*/, Result /*result*/) noexcept {}

    etna_error(const char* /*file*/, int /*line*/, const char* /*description*/) noexcept {}

    virtual char const* what() const noexcept override { return "TODO"; }
};

const char* to_string(Result value)
{
    switch (value) {
    case Result::Success:
        return "Success";
    case Result::NotReady:
        return "NotReady";
    case Result::Timeout:
        return "Timeout";
    case Result::EventSet:
        return "EventSet";
    case Result::EventReset:
        return "EventReset";
    case Result::Incomplete:
        return "Incomplete";
    case Result::ErrorOutOfHostMemory:
        return "ErrorOutOfHostMemory";
    case Result::ErrorOutOfDeviceMemory:
        return "ErrorOutOfDeviceMemory";
    case Result::ErrorInitializationFailed:
        return "ErrorInitializationFailed";
    case Result::ErrorDeviceLost:
        return "ErrorDeviceLost";
    case Result::ErrorMemoryMapFailed:
        return "ErrorMemoryMapFailed";
    case Result::ErrorLayerNotPresent:
        return "ErrorLayerNotPresent";
    case Result::ErrorExtensionNotPresent:
        return "ErrorExtensionNotPresent";
    case Result::ErrorFeatureNotPresent:
        return "ErrorFeatureNotPresent";
    case Result::ErrorIncompatibleDriver:
        return "ErrorIncompatibleDriver";
    case Result::ErrorTooManyObjects:
        return "ErrorTooManyObjects";
    case Result::ErrorFormatNotSupported:
        return "ErrorFormatNotSupported";
    case Result::ErrorFragmentedPool:
        return "ErrorFragmentedPool";
    case Result::ErrorUnknown:
        return "ErrorUnknown";
    case Result::ErrorOutOfPoolMemory:
        return "ErrorOutOfPoolMemory";
    case Result::ErrorInvalidExternalHandle:
        return "ErrorInvalidExternalHandle";
    case Result::ErrorFragmentation:
        return "ErrorFragmentation";
    case Result::ErrorInvalidOpaqueCaptureAddress:
        return "ErrorInvalidOpaqueCaptureAddress";
    case Result::ErrorSurfaceLostKHR:
        return "ErrorSurfaceLostKHR";
    case Result::ErrorNativeWindowInUseKHR:
        return "ErrorNativeWindowInUseKHR";
    case Result::SuboptimalKHR:
        return "SuboptimalKHR";
    case Result::ErrorOutOfDateKHR:
        return "ErrorOutOfDateKHR";
    case Result::ErrorIncompatibleDisplayKHR:
        return "ErrorIncompatibleDisplayKHR";
    case Result::ErrorValidationFailedEXT:
        return "ErrorValidationFailedEXT";
    case Result::ErrorInvalidShaderNV:
        return "ErrorInvalidShaderNV";
    case Result::ErrorIncompatibleVersionKHR:
        return "ErrorIncompatibleVersionKHR";
    case Result::ErrorInvalidDrmFormatModifierPlaneLayoutEXT:
        return "ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
    case Result::ErrorNotPermittedEXT:
        return "ErrorNotPermittedEXT";
    case Result::ErrorFullScreenExclusiveModeLostEXT:
        return "ErrorFullScreenExclusiveModeLostEXT";
    case Result::ThreadIdleKHR:
        return "ThreadIdleKHR";
    case Result::ThreadDoneKHR:
        return "ThreadDoneKHR";
    case Result::OperationDeferredKHR:
        return "OperationDeferredKHR";
    case Result::OperationNotDeferredKHR:
        return "OperationNotDeferredKHR";
    case Result::ErrorPipelineCompileRequiredEXT:
        return "ErrorPipelineCompileRequiredEXT";
    default:
        return "invalid";
    }
}

const char* to_string(DebugUtilsMessageSeverity value) noexcept
{
    switch (value) {
    case DebugUtilsMessageSeverity::Verbose:
        return "Verbose";
    case DebugUtilsMessageSeverity::Info:
        return "Info";
    case DebugUtilsMessageSeverity::Warning:
        return "Warning";
    case DebugUtilsMessageSeverity::Error:
        return "Error";
    default:
        return "invalid";
    }
}

const char* to_string(DebugUtilsMessageType value) noexcept
{
    switch (value) {
    case DebugUtilsMessageType::General:
        return "General";
    case DebugUtilsMessageType::Validation:
        return "Validation";
    case DebugUtilsMessageType::Performance:
        return "Performance";
    default:
        return "invalid";
    }
}

const char* to_string(DescriptorPoolFlags value)
{
    switch (value) {
    case DescriptorPoolFlags::FreeDescriptorSet:
        return "FreeDescriptorSet";
    case DescriptorPoolFlags::UpdateAfterBind:
        return "UpdateAfterBind";
    default:
        return "invalid";
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
