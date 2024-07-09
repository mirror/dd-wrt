//
// Copyright (c) 2024 Yat Ho (lagoho7@gmail.com)
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/map.hpp>
#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <utility>
#include <catch2/catch.hpp>
#include <string_view>

TEST_CASE("Small Map") {
    using namespace small;

    auto equal_il =
        [](const auto& sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using small_map_type = map<int, int, 5>;
    using small_map_type_2 = map<std::pair<int, int>, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_map_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                { 4, 5 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 7, 8 }
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_map_type e = {
                { 1, 2 },
                { 2, 3 }
            };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    { 1, 2 },
                    { 2, 3 }
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 4 },
                { 4, 5 }
            };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 5, 4 },
                    { 6, 7 }
            }));
        }

        SECTION("From another flat map") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 6 },
                { 4, 5 }
            };

            small_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 6, 7 }
            }));
        }

        SECTION("From iterators") {
            small_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                { 6, 4 },
                { 5, 6 },
                { 4, 6 }
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 6 },
                    { 5, 6 },
                    { 6, 4 }
            }));
        }

        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a.assign({
                { 6, 5 },
                { 5, 2 },
                { 4, 2 }
            });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 2 },
                    { 5, 2 },
                    { 6, 5 }
            }));
        }

        SECTION("Swap") {
            small_map_type a = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type b = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            std::initializer_list<std::pair<const int, int>> br = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

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
        small_map_type a = {
            { 1, 2 },
            { 2, 3 },
            { 3, 3 }
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
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
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 2);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 2);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Element access non-trivial key") {
        small_map_type_2 a = {
            { { 1, 1 }, 1 },
            { { 2, 2 }, 2 },
            { { 3, 3 }, 3 }
        };
        REQUIRE(a[{ 1, 1 }] == 1);
        REQUIRE(a[{ 2, 2 }] == 2);
        REQUIRE(a[{ 3, 3 }] == 3);
        REQUIRE(a[std::pair{ 1, 1 }] == 1);
        REQUIRE(a[std::pair{ 2, 2 }] == 2);
        REQUIRE(a[std::pair{ 3, 3 }] == 3);
        REQUIRE(a.at({ 1, 1 }) == 1);
        REQUIRE(a.at({ 2, 2 }) == 2);
        REQUIRE(a.at({ 3, 3 }) == 3);
        REQUIRE(a.at(std::pair{ 1, 1 }) == 1);
        REQUIRE(a.at(std::pair{ 2, 2 }) == 2);
        REQUIRE(a.at(std::pair{ 3, 3 }) == 3);
        REQUIRE_THROWS(a.at({ 4, 1 }));
        REQUIRE_THROWS(a.at({ 5, 1 }));
        REQUIRE(a.front().first == std::pair{ 1, 1 });
        REQUIRE(a.back().first == std::pair{ 3, 3 });
        REQUIRE(a.data()->first == std::pair{ 1, 1 });
        REQUIRE((a.data() + 1)->first == std::pair{ 2, 2 });
        REQUIRE((a.data() + 2)->first == std::pair{ 3, 3 });
        REQUIRE((a.data() + a.size() - 1)->first == std::pair{ 3, 3 });
        REQUIRE((a.data() + a.size() - 2)->first == std::pair{ 2, 2 });
        REQUIRE((a.data() + a.size() - 3)->first == std::pair{ 1, 1 });
    }

    SECTION("Modifiers") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        auto it = a.emplace_hint(a.upper_bound(10), 10, 10);
        REQUIRE(it->first == (a.begin() + 4)->first);
        REQUIRE(a.back().first == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                {  3,  3 },
                {  4,  4 },
                { 10, 10 }
        }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 }
        }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<std::pair<const int, int>> src = {
            { 6, 6 },
            { 5, 5 },
            { 7, 7 }
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 }
        }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 }
        }));

        a.insert({
            { 4, 4 },
            { 5, 5 },
            { 6, 6 }
        });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 }
        }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 }
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 },
                { 7, 7 }
        }));

        bool ok = false;
        std::tie(it, ok) = a.try_emplace(1, 8);
        REQUIRE_FALSE(ok);
        REQUIRE(it == a.begin());
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 },
                { 7, 7 }
        }));

        std::tie(it, ok) = a.try_emplace(2, 2);
        REQUIRE(ok);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 6, 6 },
                { 7, 7 }
        }));

        it = a.try_emplace(a.upper_bound(10), 10, 10);
        REQUIRE(it == std::prev(a.end()));
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                {  6,  6 },
                {  7,  7 },
                { 10, 10 }
        }));
        REQUIRE(a.erase(10) == 1);

        std::tie(it, ok) = a.insert_or_assign(1, 8);
        REQUIRE_FALSE(ok);
        REQUIRE(it == a.begin());
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 8 },
                { 2, 2 },
                { 6, 6 },
                { 7, 7 }
        }));

        std::tie(it, ok) = a.insert_or_assign(3, 3);
        REQUIRE(ok);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 8 },
                { 2, 2 },
                { 3, 3 },
                { 6, 6 },
                { 7, 7 }
        }));

        it = a.insert_or_assign(a.upper_bound(10), 10, 10);
        REQUIRE(it == std::prev(a.end()));
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  8 },
                {  2,  2 },
                {  3,  3 },
                {  6,  6 },
                {  7,  7 },
                { 10, 10 }
        }));

        it = a.insert_or_assign(a.upper_bound(10), 10, 10);
        REQUIRE(it == std::prev(a.end()));
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  8 },
                {  2,  2 },
                {  3,  3 },
                {  6,  6 },
                {  7,  7 },
                { 10, 10 }
        }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("try_emplace() has no effect if didn't insert") {
        struct A
        {
            A() = default;
            A(A&& a) noexcept {
                *this = std::move(a);
            }
            A(A const& a) {
                *this = a;
            }
            A&
            operator=(A&& a) noexcept {
                moved = a.moved;
                copied = a.copied;
                a.moved = true;
                return *this;
            }
            A&
            operator=(A const& a) {
                moved = a.moved;
                copied = a.copied;
                a.copied = true;
                return *this;
            }

            mutable bool moved = false;
            mutable bool copied = false;
        };
        small::map<int, A> a = {};

        A obj = {};
        a.try_emplace(1, obj);
        REQUIRE(a.size() == 1);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE(obj.copied);

        obj = {};
        a.try_emplace(2, std::move(obj));
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE(obj.moved);
        REQUIRE_FALSE(obj.copied);

        obj = {};
        a.try_emplace(1, obj);
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE_FALSE(obj.copied);

        obj = {};
        a.try_emplace(2, std::move(obj));
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE_FALSE(obj.copied);
    }

    SECTION("Element access errors") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        try {
            a.at(4);
        }
        catch (std::exception& e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Find in small map") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };

        REQUIRE(a.find(1) == a.begin());
        REQUIRE(a.find(4) == a.end());
    }

    SECTION("Find in big map") {
        small_map_type a = {};
        for (int i = 0; i < 1000; ++i) {
            a[i] = i;
        }

        REQUIRE(a.find(0) == a.begin());
        REQUIRE(a.find(1000) == a.end());
    }

    SECTION("Relational Operators") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        small_map_type b = {
            { 2, 2 },
            { 4, 4 },
            { 5, 5 }
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Max Size Map") {
    using namespace small;

    auto equal_il =
        [](const auto& sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using max_size_map_type = max_size_map<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            max_size_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            max_size_map_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                { 4, 5 },
                { 5, 6 },
                { 7, 8 }
            };
            max_size_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 7, 8 }
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            max_size_map_type e = {
                { 1, 2 },
                { 2, 3 }
            };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    { 1, 2 },
                    { 2, 3 }
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 4 },
                { 4, 5 }
            };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 5, 4 },
                    { 6, 7 }
            }));
        }

        SECTION("From another flat map") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 6 },
                { 4, 5 }
            };

            max_size_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 6, 7 }
            }));
        }

        SECTION("From iterators") {
            max_size_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                { 6, 4 },
                { 5, 6 },
                { 4, 6 }
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 6 },
                    { 5, 6 },
                    { 6, 4 }
            }));
        }

        SECTION("From initializer list") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a.assign({
                { 6, 5 },
                { 5, 2 },
                { 4, 2 }
            });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 2 },
                    { 5, 2 },
                    { 6, 5 }
            }));
        }

        SECTION("Swap") {
            max_size_map_type a = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            max_size_map_type b = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            std::initializer_list<std::pair<const int, int>> br = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

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
        max_size_map_type a = {
            { 1, 2 },
            { 2, 3 },
            { 3, 3 }
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        max_size_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
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
        max_size_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 2);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 2);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        max_size_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        auto it = a.emplace_hint(a.upper_bound(10), 10, 10);
        REQUIRE(it->first == (a.begin() + 4)->first);
        REQUIRE(a.back().first == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                {  3,  3 },
                {  4,  4 },
                { 10, 10 }
        }));

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 }
        }));

        std::initializer_list<std::pair<const int, int>> src = {
            { 6, 6 },
            { 5, 5 }
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 5, 5 },
                { 6, 6 }
        }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(7) == 0);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 }
        }));

        a.insert({
            { 4, 4 },
            { 5, 5 },
            { 6, 6 }
        });
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 }
        }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 }
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 }
        }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        max_size_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        try {
            a.at(4);
        }
        catch (std::exception& e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Relational Operators") {
        max_size_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        max_size_map_type b = {
            { 2, 2 },
            { 4, 4 },
            { 5, 5 }
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Multi Map") {
    using namespace small;

    auto equal_il =
        [](const auto& sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using small_map_type = multimap<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_map_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                { 4, 5 },
                { 5, 6 },
                { 7, 8 },
                { 7, 9 }
            };
            small_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 4);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 7, 8 },
                    { 7, 9 }
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_map_type e = {
                { 1, 2 },
                { 2, 3 },
                { 2, 5 }
            };
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    { 1, 2 },
                    { 2, 3 },
                    { 2, 5 }
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 4 },
                { 4, 5 },
                { 4, 8 }
            };
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 4, 8 },
                    { 5, 4 },
                    { 6, 7 }
            }));
        }

        SECTION("From another flat map") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 6 },
                { 5, 4 },
                { 4, 5 }
            };

            small_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 4);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 5, 4 },
                    { 6, 7 }
            }));
        }

        SECTION("From iterators") {
            small_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                { 6, 4 },
                { 5, 6 },
                { 4, 6 },
                { 4, 5 }
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 6 },
                    { 4, 5 },
                    { 5, 6 },
                    { 6, 4 }
            }));
        }

        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a.assign({
                { 6, 5 },
                { 5, 2 },
                { 4, 2 },
                { 4, 3 }
            });
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 2 },
                    { 4, 3 },
                    { 5, 2 },
                    { 6, 5 }
            }));
        }

        SECTION("Swap") {
            small_map_type a = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type b = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            std::initializer_list<std::pair<const int, int>> br = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

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
        small_map_type a = {
            { 1, 2 },
            { 2, 3 },
            { 3, 3 },
            { 3, 4 }
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 5 }
        };
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_map_type a = {
            { 1, 1 },
            { 1, 2 },
            { 3, 3 }
        };
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 1);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 1);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        auto it = a.emplace_hint(a.lower_bound(10), 10, 10);
        REQUIRE(it->first == (a.begin() + 4)->first);
        REQUIRE(a.back().first == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                {  3,  3 },
                {  4,  4 },
                { 10, 10 }
        }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 }
        }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<std::pair<const int, int>> src = {
            { 6, 6 },
            { 5, 5 },
            { 7, 7 }
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 }
        }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 }
        }));

        a.insert({
            { 4, 4 },
            { 5, 5 },
            { 6, 6 },
            { 7, 8 }
        });
        REQUIRE(a.size() == 7);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 7);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 },
                { 7, 8 }
        }));
        REQUIRE(a.count(7) == 2);

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 7 },
                { 7, 8 }
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 },
                { 7, 7 },
                { 7, 8 }
        }));

        REQUIRE(a.erase(7) == 2);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 }
        }));
        REQUIRE(a.count(7) == 0);

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        try {
            a.at(4);
        }
        catch (std::exception& e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Relational Operators") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        small_map_type b = {
            { 2, 2 },
            { 4, 4 },
            { 5, 5 }
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Unordered Map") {
    using namespace small;

    auto equal_il =
        [](const auto& sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using small_map_type = unordered_map<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_map_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                { 4, 5 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 7, 8 }
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_map_type e = {
                { 1, 2 },
                { 2, 3 }
            };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    { 1, 2 },
                    { 2, 3 }
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 4 },
                { 4, 5 }
            };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 6, 7 },
                    { 5, 4 },
                    { 4, 5 }
            }));
        }

        SECTION("From another flat map") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 6 },
                { 4, 5 }
            };

            small_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    { 6, 7 },
                    { 5, 6 },
                    { 4, 5 }
            }));
        }

        SECTION("From iterators") {
            small_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                { 6, 4 },
                { 5, 6 },
                { 4, 6 }
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 6, 4 },
                    { 5, 6 },
                    { 4, 6 }
            }));
        }

        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a.assign({
                { 6, 5 },
                { 5, 2 },
                { 4, 2 }
            });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 6, 5 },
                    { 5, 2 },
                    { 4, 2 }
            }));
        }

        SECTION("Swap") {
            small_map_type a = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type b = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            std::initializer_list<std::pair<const int, int>> br = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

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
        small_map_type a = {
            { 1, 2 },
            { 2, 3 },
            { 3, 3 }
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
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
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 2);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 2);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        auto it = a.emplace_hint(a.begin() + 3, 10, 10);
        REQUIRE(it->first == (a.begin() + 3)->first);
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                {  3,  3 },
                { 10, 10 },
                {  4,  4 }
        }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 }
        }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<std::pair<const int, int>> src = {
            { 6, 6 },
            { 5, 5 },
            { 7, 7 }
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 6, 6 },
                { 5, 5 },
                { 7, 7 }
        }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 }
        }));

        a.insert({
            { 4, 4 },
            { 5, 5 },
            { 6, 6 }
        });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 }
        }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 7, 7 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 }
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 5, 5 },
                { 6, 6 }
        }));

        bool ok = false;
        std::tie(it, ok) = a.try_emplace(1, 8);
        REQUIRE_FALSE(ok);
        REQUIRE(it == a.begin());
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 5, 5 },
                { 6, 6 }
        }));

        std::tie(it, ok) = a.try_emplace(2, 2);
        REQUIRE(ok);
        REQUIRE(it == std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 5, 5 },
                { 6, 6 },
                { 2, 2 }
        }));

        it = a.try_emplace(a.begin() + 2, 10, 10);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  5,  5 },
                { 10, 10 },
                {  6,  6 },
                {  2,  2 }
        }));
        REQUIRE(a.erase(10) == 1);

        std::tie(it, ok) = a.insert_or_assign(1, 8);
        REQUIRE_FALSE(ok);
        REQUIRE(it == a.begin());
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 8 },
                { 5, 5 },
                { 6, 6 },
                { 2, 2 }
        }));

        std::tie(it, ok) = a.insert_or_assign(3, 3);
        REQUIRE(ok);
        REQUIRE(it == std::prev(a.end()));
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 8 },
                { 5, 5 },
                { 6, 6 },
                { 2, 2 },
                { 3, 3 }
        }));

        it = a.insert_or_assign(a.begin() + 2, 10, 10);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  8 },
                {  5,  5 },
                { 10, 10 },
                {  6,  6 },
                {  2,  2 },
                {  3,  3 }
        }));

        it = a.insert_or_assign(a.begin() + 3, 10, 12);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  8 },
                {  5,  5 },
                { 10, 12 },
                {  6,  6 },
                {  2,  2 },
                {  3,  3 }
        }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("try_emplace() has no effect if didn't insert") {
        struct A
        {
            A() = default;
            A(A&& a) noexcept {
                *this = std::move(a);
            }
            A(A const& a) {
                *this = a;
            }
            A&
            operator=(A&& a) noexcept {
                moved = a.moved;
                copied = a.copied;
                a.moved = true;
                return *this;
            }
            A&
            operator=(A const& a) {
                moved = a.moved;
                copied = a.copied;
                a.copied = true;
                return *this;
            }

            mutable bool moved = false;
            mutable bool copied = false;
        };
        small::map<int, A> a = {};

        A obj = {};
        a.try_emplace(1, obj);
        REQUIRE(a.size() == 1);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE(obj.copied);

        obj = {};
        a.try_emplace(2, std::move(obj));
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE(obj.moved);
        REQUIRE_FALSE(obj.copied);

        obj = {};
        a.try_emplace(1, obj);
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE_FALSE(obj.copied);

        obj = {};
        a.try_emplace(2, std::move(obj));
        REQUIRE(a.size() == 2);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(obj.moved);
        REQUIRE_FALSE(obj.copied);
    }

    SECTION("Element access errors") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        try {
            a.at(4);
        }
        catch (std::exception& e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Find in small map") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };

        REQUIRE(a.find(1) == a.begin());
        REQUIRE(a.find(4) == a.end());
    }

    SECTION("Find in big map") {
        small_map_type a = {};
        for (int i = 0; i < 1000; ++i) {
            a[i] = i;
        }

        REQUIRE(a.find(0) == a.begin());
        REQUIRE(a.find(1000) == a.end());
    }

    SECTION("Relational Operators") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        small_map_type b = {
            { 2, 2 },
            { 4, 4 },
            { 5, 5 }
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Small Unordered Multi Map") {
    using namespace small;

    auto equal_il =
        [](const auto& sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using small_map_type = unordered_multimap<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            small_map_type a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                { 4, 5 },
                { 5, 6 },
                { 7, 8 },
                { 7, 9 }
            };
            small_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 4);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    { 4, 5 },
                    { 5, 6 },
                    { 7, 8 },
                    { 7, 9 }
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_map_type e = {
                { 1, 2 },
                { 2, 3 },
                { 2, 5 }
            };
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    { 1, 2 },
                    { 2, 3 },
                    { 2, 5 }
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 4, 5 },
                { 4, 8 },
                { 5, 4 },
                { 6, 7 }
            };
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 4, 5 },
                    { 4, 8 },
                    { 5, 4 },
                    { 6, 7 }
            }));
        }

        SECTION("From another flat map") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                { 6, 7 },
                { 5, 6 },
                { 5, 4 },
                { 4, 5 }
            };

            small_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 4);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    { 6, 7 },
                    { 5, 6 },
                    { 5, 4 },
                    { 4, 5 }
            }));
        }

        SECTION("From iterators") {
            small_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                { 6, 4 },
                { 5, 6 },
                { 4, 6 },
                { 4, 5 }
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 6, 4 },
                    { 5, 6 },
                    { 4, 6 },
                    { 4, 5 }
            }));
        }

        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a.assign({
                { 6, 5 },
                { 5, 2 },
                { 4, 2 },
                { 4, 3 }
            });
            REQUIRE(a.size() == 4);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    { 6, 5 },
                    { 5, 2 },
                    { 4, 2 },
                    { 4, 3 }
            }));
        }

        SECTION("Swap") {
            small_map_type a = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            small_map_type b = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                { 1, 2 },
                { 3, 4 },
                { 5, 6 },
                { 7, 8 }
            };
            std::initializer_list<std::pair<const int, int>> br = {
                {  9, 10 },
                { 11, 12 },
                { 13, 14 }
            };

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
        small_map_type a = {
            { 1, 2 },
            { 2, 3 },
            { 3, 3 },
            { 3, 4 }
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 },
            { 4, 5 }
        };
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.reserve(10);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 10);

        a.shrink_to_fit();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_map_type a = {
            { 1, 1 },
            { 1, 2 },
            { 3, 3 }
        };
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 1);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 1);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(4) == 1);

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));
        REQUIRE(a.count(5) == 0);

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 },
                { 5, 5 }
        }));
        REQUIRE(a.count(5) == 1);

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 4, 4 }
        }));

        auto it = a.emplace_hint(a.begin() + 2, 10, 10);
        REQUIRE(it->first == (a.begin() + 2)->first);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {  1,  1 },
                {  2,  2 },
                { 10, 10 },
                {  3,  3 },
                {  4,  4 }
        }));
        REQUIRE(a.count(10) == 1);

        REQUIRE(a.erase(10) == 1);
        REQUIRE(a.erase(4) == 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 }
        }));
        REQUIRE(a.count(10) == 0);

        std::initializer_list<std::pair<const int, int>> src = {
            { 6, 6 },
            { 5, 5 },
            { 7, 7 }
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 3, 3 },
                { 6, 6 },
                { 5, 5 },
                { 7, 7 }
        }));

        REQUIRE(a.erase(3) == 1);
        REQUIRE(a.erase(5) == 1);
        REQUIRE(a.erase(6) == 1);
        REQUIRE(a.erase(8) == 0);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 }
        }));

        a.insert({
            { 4, 4 },
            { 5, 5 },
            { 6, 6 },
            { 7, 8 }
        });
        REQUIRE(a.size() == 7);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 7);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 2, 2 },
                { 7, 7 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 8 }
        }));
        REQUIRE(a.count(7) == 2);

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 7, 7 },
                { 4, 4 },
                { 5, 5 },
                { 6, 6 },
                { 7, 8 }
        }));

        it = a.erase(a.begin() + 2, a.begin() + 4);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 7, 7 },
                { 6, 6 },
                { 7, 8 }
        }));

        REQUIRE(a.erase(7) == 2);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1, 1 },
                { 6, 6 }
        }));
        REQUIRE(a.count(7) == 0);

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        try {
            a.at(4);
        }
        catch (std::exception& e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Relational Operators") {
        small_map_type a = {
            { 1, 1 },
            { 2, 2 },
            { 3, 3 }
        };
        small_map_type b = {
            { 2, 2 },
            { 4, 4 },
            { 5, 5 }
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}
