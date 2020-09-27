#include "etna/device.hpp"
#include "etna/instance.hpp"

#include <spdlog/spdlog.h>

int main()
{
#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    if (enable_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    auto instance = etna::CreateUniqueInstance(extensions, layers);
    auto debug    = etna::CreateUniqueDebugMessenger(instance.get());
    auto device   = etna::CreateUniqueDevice(instance.get());

    return 0;
}
