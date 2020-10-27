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

using etna::ArrayView;

struct Small {
    int  value;
    bool operator==(const Small&) const = default;
};

struct Big {
    std::array<int, 1000> value;

    bool operator==(const Big&) const = default;
};

TEST_CASE("testing etna::Array class")
{
    // Empty view
    {
        ArrayView<int> view;

        CHECK(view.empty() == true);
        CHECK(view.size() == 0);

        ArrayView<Big> bigs;

        CHECK(view.empty() == true);
        CHECK(view.size() == 0);
    }

    // View of Small constructed from Rvalue init list
    {
        ArrayView<Small> view({ Small{ 1 }, Small{ 2 } });
        ArrayView<Small> other({ Small{ 1 }, Small{ 3 } });

        CHECK(view[0].value == 1);
        CHECK(view[1].value == 2);
        CHECK(view.empty() == false);
        CHECK(view.size() == 2);
        CHECK(view == view);
        CHECK(view != other);
    }

    // View of Small constructed from Lvalue init list
    {
        auto             l = { Small{ 1 }, Small{ 2 } };
        ArrayView<Small> view(l);
        ArrayView<Small> other({ Small{ 1 } });

        CHECK(view[0] == Small{ 1 });
        CHECK(view[1] == Small{ 2 });
        CHECK(view.empty() == false);
        CHECK(view.size() == 2);
        CHECK(view == view);
        CHECK(view != other);
    }

    // View of Big constructed from Rvalue init list
    {
        ArrayView<Big> view({ Big{ 1 }, Big{ 2 } });
        ArrayView<Big> other({ Big{ 1 }, Big{ 2 } });

        CHECK(view[0] == Big{ 1 });
        CHECK(view[1] == Big{ 2 });
        CHECK(view.empty() == false);
        CHECK(view.size() == 2);
        CHECK(view == view);
        CHECK(view == other);
    }

    // View of Big constructed from Lvalue init list
    {
        auto           l = { Big{ 1 }, Big{ 2 } };
        ArrayView<Big> view(l);
        ArrayView<Big> view_other({ Big{ 2 } });

        CHECK(view[0] == Big{ 1 });
        CHECK(view[1] == Big{ 2 });
        CHECK(view.empty() == false);
        CHECK(view.size() == 2);
        CHECK(view == view);
        CHECK(view != view_other);
    }

    // View of Small constructed from C array
    {
        Small            smalls[]{ 1 };
        ArrayView<Small> view(smalls);

        Small            others[]{ 1, 2 };
        ArrayView<Small> view_other(others);

        CHECK(view[0].value == 1);
        smalls[0].value = 5;
        CHECK(view[0].value == 5);
        CHECK(view.empty() == false);
        CHECK(view.size() == 1);
        CHECK(view == view);
        CHECK(view != view_other);
    }

    // View of Big constructed from C array
    {
        Big            bigs[1]{ 1 };
        ArrayView<Big> view(bigs);

        Big            others[1]{ 2 };
        ArrayView<Big> view_others(others);

        CHECK(view[0] == bigs[0]);
        CHECK(view.empty() == false);
        CHECK(view.size() == 1);
        CHECK(view == view);
        CHECK(view != view_others);
    }
}
