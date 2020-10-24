#include <cstddef>

struct ResourceResult {
    const unsigned char* data;
    const size_t         size;
};

void AddResource(const char* resource_name, const unsigned char* resource_data, size_t resource_size);

ResourceResult GetResource(const char* resource_name);
