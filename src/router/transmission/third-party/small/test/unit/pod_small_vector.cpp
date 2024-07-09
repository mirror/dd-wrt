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

TEST_CASE("POD Small Vector") {
    using namespace small;

    STATIC_REQUIRE(is_relocatable_v<int>);
    auto equal_il =
        [](const auto &sm_vector, std::initializer_list<int> il) -> bool {
        return std::
            equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
    };

    SECTION("Constructor") {
        SECTION("Asserts") {
            REQUIRE(std::is_copy_constructible_v<vector<int, 5>>);
            REQUIRE(std::is_copy_assignable_v<vector<int, 5>>);
            REQUIRE(std::is_move_constructible_v<vector<int, 5>>);
            REQUIRE(std::is_move_assignable_v<vector<int, 5>>);

            REQUIRE(
                std::is_copy_constructible_v<vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_copy_assignable_v<vector<std::pair<int, int>, 5>>);
            REQUIRE(
                std::is_move_constructible_v<vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_move_assignable_v<vector<std::pair<int, int>, 5>>);
        }

        SECTION("Default") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            vector<int, 5, std::allocator<int>> a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("With size") {
            std::allocator<int> alloc;
            vector<int, 5> b(3, alloc);
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(b.get_allocator() == alloc);
        }

        SECTION("From value") {
            std::allocator<int> alloc;
            vector<int, 5> c(3, 1, alloc);
            REQUIRE(c.size() == 3);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(c, { 1, 1, 1 }));
            REQUIRE(c.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<int> alloc;
            std::vector<int> dv = { 6, 5, 4 };
            vector<int, 5> d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 6, 5, 4 }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            vector<int, 5> e = { 1, 2 };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2 }));
        }

        SECTION("From ranges") {
            std::vector<int> v = { 1, 2, 3 };
            vector<int, 5> e(v);
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2, 3 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From another small vector") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };

            vector<int, 5> b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
        }

        SECTION("From iterators") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From size and value") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign(3, 1);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 1, 1, 1 }));
        }

        SECTION("From initializer list") {
            vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 4 });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("Fill") {
            vector<int, 5> a(3, 1);
            REQUIRE_FALSE(a.empty());
            a.fill(2);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 2, 2, 2 }));
        }

        SECTION("Swap") {
            vector<int, 5> a(4, 1);
            vector<int, 5> b(3, 2);

            std::initializer_list<int> ar = { 1, 1, 1, 1 };
            std::initializer_list<int> br = { 2, 2, 2 };

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
        vector<int, 5> a = { 1, 2, 3 };

        REQUIRE(a.begin() == a.data());
        REQUIRE(a.end() == a.data() + a.size());
        REQUIRE_FALSE(a.end() == a.data() + a.capacity());
        REQUIRE(*a.begin() == 1);
        REQUIRE(*std::prev(a.end()) == 3);

        REQUIRE(a.cbegin() == a.data());
        REQUIRE(a.cend() == a.data() + a.size());
        REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
        REQUIRE(*a.cbegin() == 1);
        REQUIRE(*std::prev(a.cend()) == 3);

        REQUIRE(*a.rbegin() == 3);
        REQUIRE(*std::prev(a.rend()) == 1);

        REQUIRE(*a.crbegin() == 3);
        REQUIRE(*std::prev(a.crend()) == 1);
    }

    SECTION("Capacity") {
        vector<int, 5> a = { 1, 2, 3 };
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

        a.resize(4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);
    }

    SECTION("Element access") {
        vector<int, 5> a = { 1, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(3) == 4);
        REQUIRE_THROWS(a.at(4) == 5);
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
        vector<int, 5> a = { 1, 2, 3 };
        a.push_back(4);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        a.push_back(std::move(5));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        a.emplace_back(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        auto it = a.emplace(a.begin() + 2, 10);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 10, 3, 4 }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 10 }));

        it = a.insert(a.begin() + 1, 20);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 20, 2, 10 }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        it = a.insert(a.begin() + 2, std::move(30));
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 20, 30, 2, 10 }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 20 }));

        it = a.insert(a.begin() + 1, 2, 10);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 10, 10, 20 }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 10 }));

        std::initializer_list<int> src = { 2, 4, 8 };
        it = a.insert(a.begin() + 1, src.begin(), src.end());
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 4, 8, 10 }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2 }));

        it = a.insert(a.begin() + 1, { 2, 4, 8 });
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 4, 8, 2 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 4, 8, 2 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));

        a.resize(2);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());

        a.resize(4, 5);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a[2] == 5);
        REQUIRE(a[3] == 5);
    }

    SECTION("Element access errors") {
        vector<int, 5> a = { 1, 2, 3 };
        try {
            a.at(4);
        }
        catch (std::exception &e) {
            REQUIRE(
                e.what()
                == std::string_view(
                    "at: cannot access element after vector::size()"));
        }
    }

    SECTION("Relational Operators") {
        vector<int, 5> a = { 1, 2, 3 };
        vector<int, 5> b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }

    SECTION("From raw vector") {
        auto a = to_vector({ 1, 2, 3 });
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 3);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { 1, 2, 3 }));

        auto b = to_vector<int, 3, 5>({ 1, 2, 3 });
        REQUIRE(b.size() == 3);
        REQUIRE(b.max_size() > 5);
        REQUIRE(b.capacity() == 5);
        REQUIRE_FALSE(b.empty());
        REQUIRE(equal_il(b, { 1, 2, 3 }));

        int cr[] = { 1, 2, 3 };
        auto c = to_vector(cr);
        REQUIRE(c.size() == 3);
        REQUIRE(c.max_size() > 3);
        REQUIRE(c.is_inline());
        REQUIRE(c.capacity() == small::default_inline_storage_v<int>);
        REQUIRE(c.capacity() == 5);
        REQUIRE_FALSE(c.empty());
        REQUIRE(equal_il(b, { 1, 2, 3 }));
    }
}

TEST_CASE("POD Max size vector") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_array, std::initializer_list<int> il) -> bool {
        return std::
            equal(sm_array.begin(), sm_array.end(), il.begin(), il.end());
    };

    auto full = [](const auto &sm_array) -> bool {
        return sm_array.is_inline() && sm_array.size() == sm_array.capacity();
    };

    SECTION("Constructor") {
        SECTION("Asserts") {
            REQUIRE(std::is_copy_constructible_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_copy_assignable_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_move_constructible_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_move_assignable_v<max_size_vector<int, 5>>);

            REQUIRE(std::is_copy_constructible_v<
                    max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_copy_assignable_v<
                    max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_move_constructible_v<
                    max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_move_assignable_v<
                    max_size_vector<std::pair<int, int>, 5>>);
        }

        SECTION("Default") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("With size") {
            max_size_vector<int, 5> b(3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
        }

        SECTION("From value") {
            max_size_vector<int, 5> c(3, 1);
            REQUIRE(c.size() == 3);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(c, { 1, 1, 1 }));
        }

        SECTION("From Iterators") {
            std::vector<int> dv = { 6, 5, 4 };
            max_size_vector<int, 5> d(dv.begin(), dv.end());
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { 6, 5, 4 }));
        }

        SECTION("From initializer list") {
            max_size_vector<int, 5> e = { 1, 2 };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { 1, 2 }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From another small array") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a = { 6, 5, 4 };

            max_size_vector<int, 5> b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
        }

        SECTION("From iterators") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            std::vector<int> v = { 6, 5, 4 };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("From size and value") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign(3, 1);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 1, 1, 1 }));
        }

        SECTION("From initializer list") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign({ 6, 5, 4 });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 6, 5, 4 }));
        }

        SECTION("Fill") {
            max_size_vector<int, 5> a(3, 1);
            REQUIRE_FALSE(a.empty());
            a.fill(2);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { 2, 2, 2 }));
        }

        SECTION("Swap") {
            max_size_vector<int, 5> a(4, 1);
            max_size_vector<int, 5> b(3, 2);

            std::initializer_list<int> ar = { 1, 1, 1, 1 };
            std::initializer_list<int> br = { 2, 2, 2 };

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
        max_size_vector<int, 5> a = { 1, 2, 3 };

        REQUIRE(a.begin() == a.data());
        REQUIRE(a.end() == a.data() + a.size());
        REQUIRE_FALSE(a.end() == a.data() + a.capacity());
        REQUIRE(*a.begin() == 1);
        REQUIRE(*std::prev(a.end()) == 3);

        REQUIRE(a.cbegin() == a.data());
        REQUIRE(a.cend() == a.data() + a.size());
        REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
        REQUIRE(*a.cbegin() == 1);
        REQUIRE(*std::prev(a.cend()) == 3);

        REQUIRE(*a.rbegin() == 3);
        REQUIRE(*std::prev(a.rend()) == 1);

        REQUIRE(*a.crbegin() == 3);
        REQUIRE(*std::prev(a.crend()) == 1);
    }

    SECTION("Capacity") {
        max_size_vector<int, 5> a = { 1, 2, 3 };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(full(a));
    }

    SECTION("Element access") {
        max_size_vector<int, 5> a = { 1, 2, 3 };
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(3) == 4);
        REQUIRE_THROWS(a.at(4) == 5);
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
        max_size_vector<int, 5> a = { 1, 2, 3 };
        a.push_back(4);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        a.push_back(std::move(5));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        a.emplace_back(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 3, 4, 5 }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 3, 4 }));

        auto it = a.emplace(a.begin() + 2, 10);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 10, 3, 4 }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 10 }));

        it = a.insert(a.begin() + 1, 20);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 20, 2, 10 }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        it = a.insert(a.begin() + 2, std::move(30));
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 20, 30, 2, 10 }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 20 }));

        it = a.insert(a.begin() + 1, 2, 10);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 10, 10, 20 }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 10 }));

        std::initializer_list<int> src = { 2, 4, 8 };
        it = a.insert(a.begin() + 1, src.begin(), src.end());
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 4, 8, 10 }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2 }));

        it = a.insert(a.begin() + 1, { 2, 4, 8 });
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, { 1, 2, 4, 8, 2 }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 4, 8, 2 }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, { 1, 2 }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {}));

        a.resize(2);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));

        a.resize(4, 5);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(a[2] == 5);
        REQUIRE(a[3] == 5);
    }

    SECTION("Element access errors") {
        max_size_vector<int, 5> a = { 1, 2, 3 };
        try {
            a.at(4);
        }
        catch (std::exception &e) {
            REQUIRE(
                e.what()
                == std::string_view(
                    "at: cannot access element after vector::size()"));
        }
    }

    SECTION("Relational Operators") {
        max_size_vector<int, 5> a = { 1, 2, 3 };
        max_size_vector<int, 5> b = { 2, 4, 5 };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}