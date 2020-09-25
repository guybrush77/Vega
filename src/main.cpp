#include "etna/instance.hpp"

int main()
{
#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    if (enable_validation_layers) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    auto instance = etna::CreateInstance(extensions, layers);

    return 0;
}
