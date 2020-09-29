#include "utils/resource.hpp"

#include <unordered_map>

namespace {

class ResourceManager final {
  public:
    static ResourceManager& Instance()
    {
        static ResourceManager manager;
        return manager;
    }

    bool AddResource(std::string_view resource_name, const unsigned char* resource_data, size_t resource_size)
    {
        auto [iter, success] = resources.insert({ resource_name, { resource_data, resource_size } });
        return success;
    }

    ResourceResult GetResource(std::string_view resource_name)
    {
        auto iter = resources.find(resource_name);
        return iter != resources.end() ? iter->second : ResourceResult{};
    }

  private:
    ResourceManager()                       = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&)      = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    std::unordered_map<std::string_view, ResourceResult> resources;
};

} // namespace

void AddResource(const char* resource_name, const unsigned char* resource_data, size_t resource_size)
{
    ResourceManager::Instance().AddResource(resource_name, resource_data, resource_size);
}

ResourceResult GetResource(const char* resource_name)
{
    return ResourceManager::Instance().GetResource(resource_name);
}
