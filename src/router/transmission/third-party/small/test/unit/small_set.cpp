//
// Copyright (c) 2024 Yat Ho (lagoho7@gmail.com)
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/set.hpp>
#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <utility>
#include <catch2/catch.hpp>
#include <string_view>

TEST_CASE("Small Set") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_set, std::initializer_list<int> il) -> bool {
        return std::equal(sm_set.begin(), sm_set.end(), il.begin(), il.end());
    };

    using small_set_type = set<int, 5>;
    using small_set_type_2 = set<std::pair<int, int>, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_set_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_set_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 4, 5, 7 };
            small_set_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 4, 5, 7 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_set_type e = { 1, 2 };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From another flat set") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };

            small_set_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From iterators") {
            small_set_type a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 4 });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("Swap") {
            small_set_type a = { 1, 3, 5, 7 };
            small_set_type b = { 9, 11, 13 };

            std::initializer_list<int> ar = { 1, 3, 5, 7 };
            std::initializer_list<int> br = { 9, 11, 13 };

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 4);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin().operator*() == 1);
        REQUIRE(std::prev(a.end()).operator*() == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin().operator*() == 1);
        REQUIRE(std::prev(a.cend()).operator*() == 3);

        REQUIRE(a.rbegin().operator*() == 3);
        REQUIRE(std::prev(a.rend()).operator*() == 1);

        REQUIRE(a.crbegin().operator*() == 3);
        REQUIRE(std::prev(a.crend()).operator*() == 1);
    }

    SECTION("Capacity") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 1);
    }

    SECTION("Element access with non-trivial key") {
        small_set_type_2 a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        REQUIRE(a[0] == std::pair{ 1, 1 });
        REQUIRE(a[1] == std::pair{ 2, 2 });
        REQUIRE(a[2] == std::pair{ 3, 3 });
        REQUIRE(a.at(0) == std::pair{ 1, 1 });
        REQUIRE(a.at(1) == std::pair{ 2, 2 });
        REQUIRE(a.at(2) == std::pair{ 3, 3 });
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == std::pair{ 1, 1 });
        REQUIRE(a.back() == std::pair{ 3, 3 });
        REQUIRE(*a.data() == std::pair{ 1, 1 });
        REQUIRE(*(a.data() + 1) == std::pair{ 2, 2 });
        REQUIRE(*(a.data() + 2) == std::pair{ 3, 3 });
        REQUIRE(*(a.data() + a.size() - 1) == std::pair{ 3, 3 });
        REQUIRE(*(a.data() + a.size() - 2) == std::pair{ 2, 2 });
        REQUIRE(*(a.data() + a.size() - 3) == std::pair{ 1, 1 });
    }

    SECTION("Modifiers") {
        small_set_type a = { 1, 2, 3 };
        a.insert({ 4, 4 });
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        int p = 5;
        a.insert(std::move(p));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        auto it = a.emplace_hint(a.upper_bound(10), 10);
        REQUIRE(it.operator*() == (a.begin() + 4).operator*());
        REQUIRE(a.back() == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 10 }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<int> src = { 6, 5, 7 };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 5, 6, 7 }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7 }));

        a.insert({ 4, 5, 6 });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 4, 5, 6, 7 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 4, 5, 6, 7 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 6, 7 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Find in small set") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.find(1) == a.begin());
        REQUIRE(a.find(4) == a.end());
    }

    SECTION("Find in big set") {
        small_set_type a = {};
        for (int i = 0; i < 1000; ++i) {
            a.insert(i);
        }

        REQUIRE(a.find(0) == a.begin());
        REQUIRE(a.find(1000) == a.end());
    }

    SECTION("Element access errors") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE_THROWS(a.at(4));
    }

    SECTION("Relational Operators") {
        small_set_type a = { 1, 2, 3 };
        small_set_type b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Max Size Set") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_set, std::initializer_list<int> il) -> bool {
        return std::equal(sm_set.begin(), sm_set.end(), il.begin(), il.end());
    };

    using max_size_set_type = max_size_set<int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            max_size_set_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            max_size_set_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 4, 5, 7 };
            max_size_set_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 4, 5, 7 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            max_size_set_type e = { 1, 2 };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            max_size_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From another flat set") {
            max_size_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };

            max_size_set_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From iterators") {
            max_size_set_type a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("From initializer list") {
            max_size_set_type a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 4 });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 6 }));
        }

        SECTION("Swap") {
            max_size_set_type a = { 1, 3, 5, 7 };
            max_size_set_type b = { 9, 11, 13 };

            std::initializer_list<int> ar = { 1, 3, 5, 7 };
            std::initializer_list<int> br = { 9, 11, 13 };

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 4);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        max_size_set_type a = { 1, 2, 3 };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin().operator*() == 1);
        REQUIRE(std::prev(a.end()).operator*() == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin().operator*() == 1);
        REQUIRE(std::prev(a.cend()).operator*() == 3);

        REQUIRE(a.rbegin().operator*() == 3);
        REQUIRE(std::prev(a.rend()).operator*() == 1);

        REQUIRE(a.crbegin().operator*() == 3);
        REQUIRE(std::prev(a.crend()).operator*() == 1);
    }

    SECTION("Capacity") {
        max_size_set_type a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        REQUIRE_THROWS(a.reserve(10));
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        max_size_set_type a = { 1, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 1);
    }

    SECTION("Modifiers") {
        max_size_set_type a = { 1, 2, 3 };
        a.insert({ 4, 4 });
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        int p = 5;
        // NOLINTNEXTLINE(performance-move-const-arg)
        a.insert(std::move(p));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        a.emplace(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        auto it = a.emplace_hint(a.upper_bound(10), 10);
        REQUIRE(it.operator*() == (a.begin() + 4).operator*());
        REQUIRE(a.back() == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 10 }));

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));

        std::initializer_list<int> src = { 6, 5 };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 5, 6 }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2 }));

        a.insert({ 4, 5, 6 });
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 4, 5, 6 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 4, 5, 6 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 6 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        max_size_set_type a = { 1, 2, 3 };
        REQUIRE_THROWS(a.at(4));
    }

    SECTION("Relational Operators") {
        max_size_set_type a = { 1, 2, 3 };
        max_size_set_type b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Multi Set") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_set, std::initializer_list<int> il) -> bool {
        return std::equal(sm_set.begin(), sm_set.end(), il.begin(), il.end());
    };

    using small_set_type = multiset<int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_set_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_set_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 4, 5, 7, 7 };
            small_set_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 4);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 4, 5, 7, 7 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_set_type e = { 1, 2, 2 };
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 5, 4 };
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 5, 6 }));
        }

        SECTION("From another flat set") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 6, 5, 4 };

            small_set_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 4);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(a, { 4, 5, 6, 6 }));
        }

        SECTION("From iterators") {
            small_set_type a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 4, 5, 6 }));
        }

        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 5, 4 });
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 4, 5, 5, 6 }));
        }

        SECTION("Swap") {
            small_set_type a = { 1, 1, 3, 5, 7 };
            small_set_type b = { 9, 11, 13 };

            std::initializer_list<int> ar = { 1, 1, 3, 5, 7 };
            std::initializer_list<int> br = { 9, 11, 13 };

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 5);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 5);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 5);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin().operator*() == 1);
        REQUIRE(std::prev(a.end()).operator*() == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin().operator*() == 1);
        REQUIRE(std::prev(a.cend()).operator*() == 3);

        REQUIRE(a.rbegin().operator*() == 3);
        REQUIRE(std::prev(a.rend()).operator*() == 1);

        REQUIRE(a.crbegin().operator*() == 3);
        REQUIRE(std::prev(a.crend()).operator*() == 1);
    }

    SECTION("Capacity") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_set_type a = { 1, 2, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 2);
        REQUIRE(*(a.data() + 3) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 2);
        REQUIRE(*(a.data() + a.size() - 4) == 1);
    }

    SECTION("Modifiers") {
        small_set_type a = { 1, 2, 3 };
        a.insert({ 4, 4 });
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));
        REQUIRE(a.count(4) == 2);

        // NOLINTNEXTLINE(performance-move-const-arg)
        int p = 5;
        a.insert(std::move(p));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));

        auto it = a.emplace_hint(a.lower_bound(10), 10);
        REQUIRE(it.operator*() == (a.begin() + 5).operator*());
        REQUIRE(a.back() == 10);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4, 10 }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 2);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));
        REQUIRE(a.count(4) == 0);

        std::initializer_list<int> src = { 6, 5, 7 };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 5, 6, 7 }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7 }));

        a.insert({ 4, 5, 6 });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 4, 5, 6, 7 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 4, 5, 6, 7 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 6, 7 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE_THROWS(a.at(4));
    }

    SECTION("Relational Operators") {
        small_set_type a = { 1, 2, 3 };
        small_set_type b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Unordered Set") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_set, std::initializer_list<int> il) -> bool {
        return std::equal(sm_set.begin(), sm_set.end(), il.begin(), il.end());
    };

    using small_set_type = unordered_set<int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_set_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_set_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 4, 5, 7 };
            small_set_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 4, 5, 7 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_set_type e = { 1, 2 };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From another flat set") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };

            small_set_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From iterators") {
            small_set_type a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 4 });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("Swap") {
            small_set_type a = { 1, 3, 5, 7 };
            small_set_type b = { 9, 11, 13 };

            std::initializer_list<int> ar = { 1, 3, 5, 7 };
            std::initializer_list<int> br = { 9, 11, 13 };

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 4);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin().operator*() == 1);
        REQUIRE(std::prev(a.end()).operator*() == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin().operator*() == 1);
        REQUIRE(std::prev(a.cend()).operator*() == 3);

        REQUIRE(a.rbegin().operator*() == 3);
        REQUIRE(std::prev(a.rend()).operator*() == 1);

        REQUIRE(a.crbegin().operator*() == 3);
        REQUIRE(std::prev(a.crend()).operator*() == 1);
    }

    SECTION("Capacity") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 1);
    }

    SECTION("Modifiers") {
        small_set_type a = { 1, 2, 3 };
        a.insert({ 4, 4 });
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        int p = 5;
        a.insert(std::move(p));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        auto it = a.emplace_hint(a.begin() + 3, 10);
        REQUIRE(it.operator*() == (a.begin() + 3).operator*());
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 10, 4 }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<int> src = { 6, 5, 7 };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 6, 5, 7 }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7 }));

        a.insert({ 4, 5, 6 });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7, 4, 5, 6 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 7, 4, 5, 6 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 5, 6 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Find in small set") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.find(1) == a.begin());
        REQUIRE(a.find(4) == a.end());
    }

    SECTION("Find in big set") {
        small_set_type a = {};
        for (int i = 0; i < 1000; ++i) {
            a.insert(i);
        }

        REQUIRE(a.find(0) == a.begin());
        REQUIRE(a.find(1000) == a.end());
    }

    SECTION("Element access errors") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE_THROWS(a.at(4));
    }

    SECTION("Relational Operators") {
        small_set_type a = { 1, 2, 3 };
        small_set_type b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Unordered Multi Set") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_set, std::initializer_list<int> il) -> bool {
        return std::equal(sm_set.begin(), sm_set.end(), il.begin(), il.end());
    };

    using small_set_type = unordered_multiset<int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_set_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_set_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 4, 5, 7, 7 };
            small_set_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 4);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 4, 5, 7, 7 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_set_type e = { 1, 2, 2 };
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 5, 5, 4 };
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 5, 4 }));
        }

        SECTION("From another flat set") {
            small_set_type a;
            REQUIRE(a.empty());
            a = { 6, 6, 5, 4 };

            small_set_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 4);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(a, { 6, 6, 5, 4 }));
        }

        SECTION("From iterators") {
            small_set_type a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4, 4 }));
        }

        SECTION("From initializer list") {
            small_set_type a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 5, 4 });
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 5, 4 }));
        }

        SECTION("Swap") {
            small_set_type a = { 1, 1, 3, 5, 7 };
            small_set_type b = { 9, 11, 13 };

            std::initializer_list<int> ar = { 1, 1, 3, 5, 7 };
            std::initializer_list<int> br = { 9, 11, 13 };

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 5);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 5);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 5);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        small_set_type a = { 1, 2, 3 };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin().operator*() == 1);
        REQUIRE(std::prev(a.end()).operator*() == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin().operator*() == 1);
        REQUIRE(std::prev(a.cend()).operator*() == 3);

        REQUIRE(a.rbegin().operator*() == 3);
        REQUIRE(std::prev(a.rend()).operator*() == 1);

        REQUIRE(a.crbegin().operator*() == 3);
        REQUIRE(std::prev(a.crend()).operator*() == 1);
    }

    SECTION("Capacity") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_set_type a = { 1, 2, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 2);
        REQUIRE(*(a.data() + 3) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 2);
        REQUIRE(*(a.data() + a.size() - 4) == 1);
    }

    SECTION("Modifiers") {
        small_set_type a = { 1, 2, 3 };
        a.insert({ 4, 4 });
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));
        REQUIRE(a.count(4) == 2);

        // NOLINTNEXTLINE(performance-move-const-arg)
        int p = 5;
        a.insert(std::move(p));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4, 5 }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 4 }));

        auto it = a.emplace_hint(a.begin() + 4, 10);
        REQUIRE(it.operator*() == (a.begin() + 4).operator*());
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 10, 4 }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 2);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));
        REQUIRE(a.count(4) == 0);

        std::initializer_list<int> src = { 6, 5, 7 };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 6, 5, 7 }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7 }));

        a.insert({ 4, 5, 6 });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 7, 4, 5, 6 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 7, 4, 5, 6 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 5, 6 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        small_set_type a = { 1, 2, 3 };
        REQUIRE_THROWS(a.at(4));
    }

    SECTION("Relational Operators") {
        small_set_type a = { 1, 2, 3 };
        small_set_type b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}
