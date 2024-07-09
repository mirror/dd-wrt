//
// Copyright (c) 2022 Yat Ho (lagoho7@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

// C++
#include <memory>

// External
#include <catch2/catch.hpp>

// Small
#include <small/vector.hpp>

TEST_CASE("Shared Ptr Vector") {
    using namespace small;

    STATIC_REQUIRE(!is_relocatable_v<std::shared_ptr<int>>);

    SECTION("Erase in middle") {
        vector<std::shared_ptr<int>> a;

        for (int i = 0; i < 2; ++i) {
            a.emplace_back(std::make_shared<int>(i));
        }

        a.erase(a.begin());

        REQUIRE(*a[0] == 1);
    }
}
