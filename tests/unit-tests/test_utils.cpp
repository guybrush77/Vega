#include "utils/resource.hpp"

#include <cstring>
#include <doctest/doctest.h>
#include <string_view>

using data_view = std::basic_string_view<unsigned char>;

TEST_CASE("testing resource manager")
{
    const char* data = "Lorem ipsum dolor sit amet.";
    const void* temp = data;
    const auto  view = data_view(static_cast<const unsigned char*>(temp), strlen(data));

    const auto [resource_data, resource_size] = GetResource("test-resource");
    const auto resource_view = data_view(static_cast<const unsigned char*>(resource_data), resource_size);

    CHECK(view == resource_view);
}
