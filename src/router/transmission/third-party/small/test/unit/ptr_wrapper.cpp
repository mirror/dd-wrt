//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

// C++
#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <string>
#include <string_view>

// External
#include <catch2/catch.hpp>

// Small
#include <small/vector.hpp>

TEST_CASE("Pointer wrapper") {
    using namespace small;
    using namespace small::detail;

    SECTION("Constructor") {
        SECTION("Empty") {
            [[maybe_unused]] pointer_wrapper<int *> p;
        }

        SECTION("From pointer") {
            int a = 2;
            pointer_wrapper<int *> p(&a);
            REQUIRE(p.base() == &a);
        }

        SECTION("From another pointer wrapper") {
            int a = 2;
            pointer_wrapper<int *> p1(&a);
            REQUIRE(p1.base() == &a);

            pointer_wrapper<int *> p2(p1);
            REQUIRE(p2.base() == &a);
        }
    }

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    pointer_wrapper<int *> begin(&a[0]);
    pointer_wrapper<int *> end(&a[0] + 9);

    SECTION("Element access") {
        REQUIRE(begin != end);
        REQUIRE(*begin == 1);
        REQUIRE(*std::prev(end) == 9);
        REQUIRE(begin.base() == &a[0]);
        REQUIRE(begin[0] == 1);
        REQUIRE(begin[1] == 2);
        REQUIRE(begin[2] == 3);
    }

    SECTION("Modifiers") {
        ++begin;
        REQUIRE(*begin == 2);
        begin++;
        REQUIRE(*begin == 3);
        --begin;
        REQUIRE(*begin == 2);
        begin--;
        REQUIRE(*begin == 1);
        auto it = begin + 1;
        REQUIRE(*it == 2);
        it = begin + 2;
        REQUIRE(*it == 3);
        begin += 2;
        REQUIRE(*begin == 3);
        it = begin - 1;
        REQUIRE(*it == 2);
        it = begin - 2;
        REQUIRE(*it == 1);
        begin -= 2;
        REQUIRE(*begin == 1);
    }

    SECTION("Algorithms") {
        int b[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        pointer_wrapper<int *> b_begin(&b[0]);
        pointer_wrapper<int *> b_end(&b[0] + 9);

        std::copy(begin, end, b_begin);
        REQUIRE(std::equal(begin, end, b_begin, b_end));
    }
}