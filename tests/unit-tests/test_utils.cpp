#include "core.hpp"
#include "utils/resource.hpp"

#include <doctest/doctest.h>
#include <string_view>

using data_view = std::basic_string_view<unsigned char>;
using etna::narrow_cast;

TEST_CASE("testing narrow_cast function")
{
    CHECK_NOTHROW(narrow_cast<char>(0));
    CHECK_NOTHROW(narrow_cast<int>(0));
    CHECK_NOTHROW(narrow_cast<float>(0));

    CHECK_NOTHROW(narrow_cast<char>(1));
    CHECK_NOTHROW(narrow_cast<int>(1));
    CHECK_NOTHROW(narrow_cast<float>(1));

    CHECK_NOTHROW(narrow_cast<char>(-1));
    CHECK_NOTHROW(narrow_cast<int>(-1));
    CHECK_NOTHROW(narrow_cast<float>(-1));

    CHECK_NOTHROW(narrow_cast<int>(1.0));
    CHECK_NOTHROW(narrow_cast<int>(1.0f));
    CHECK_NOTHROW(narrow_cast<unsigned int>(1.0));
    CHECK_NOTHROW(narrow_cast<unsigned int>(1.0f));

    CHECK_THROWS(narrow_cast<unsigned int>(-1));
    CHECK_THROWS(narrow_cast<unsigned int>(-1.0));
    CHECK_THROWS(narrow_cast<unsigned int>(-1.0f));

    CHECK_THROWS(narrow_cast<char>(500'000));
    CHECK_THROWS(narrow_cast<int>(5'000'000'000));

    CHECK_THROWS(narrow_cast<int>(3.14f));
    CHECK_THROWS(narrow_cast<int>(3.14));

    CHECK_THROWS(narrow_cast<float>(1'000'000'001));
}

TEST_CASE("testing resource manager")
{
    const char* data = "Lorem ipsum dolor sit amet.";
    const void* temp = data;
    const auto  view = data_view(static_cast<const unsigned char*>(temp), strlen(data));

    const auto [resource_data, resource_size] = GetResource("test-resource");
    const auto resource_view = data_view(static_cast<const unsigned char*>(resource_data), resource_size);

    CHECK(view == resource_view);
}
