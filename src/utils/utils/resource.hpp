#include <stdexcept>
#include <string_view>
#include <unordered_map>

using data_view = std::basic_string_view<unsigned char>;

class ResourceManager final {
  public:
    static ResourceManager& Instance()
    {
        static ResourceManager manager;
        return manager;
    }

    void AddResource(std::string_view resource_name, data_view resource_data)
    {
        auto [iter, success] = resources.insert({ resource_name, resource_data });
        if (!success) {
            throw std::runtime_error("Resource manager failed to add resource");
        }
    }

    data_view GetResource(std::string_view resource_name)
    {
        auto iter = resources.find(resource_name);
        if (iter == resources.end()) {
            throw std::runtime_error("Resource manager failed to find requested resource");
        }
        return iter->second;
    }

  private:
    ResourceManager()                       = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&)      = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    std::unordered_map<std::string_view, data_view> resources;
};
