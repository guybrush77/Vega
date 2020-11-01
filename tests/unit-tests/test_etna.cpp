#include "core.hpp"

#include <array>
#include <doctest/doctest.h>

using etna::narrow_cast;

TEST_CASE("testing etna::narrow_cast function")
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
