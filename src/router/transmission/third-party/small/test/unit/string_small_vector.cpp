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

namespace small {
    // Make strings relocatable since we are here anyway
    // template <> struct is_relocatable<std::string> : std::true_type {};
} // namespace small

TEST_CASE("String Vector") {
    using namespace small;

    // std::string is not always relocatable
    STATIC_REQUIRE(!is_relocatable_v<std::string>);
    auto equal_il = [](const auto &sm_vector,
                       std::initializer_list<std::string> il) -> bool {
        return std::
            equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
    };

    SECTION("Constructor") {
        SECTION("Asserts") {
            REQUIRE(std::is_copy_constructible_v<vector<std::string, 5>>);
            REQUIRE(std::is_copy_assignable_v<vector<std::string, 5>>);
            REQUIRE(std::is_move_constructible_v<vector<std::string, 5>>);
            REQUIRE(std::is_move_assignable_v<vector<std::string, 5>>);

            REQUIRE(std::is_copy_constructible_v<
                    vector<std::pair<std::string, std::string>, 5>>);
            REQUIRE(std::is_copy_assignable_v<
                    vector<std::pair<std::string, std::string>, 5>>);
            REQUIRE(std::is_move_constructible_v<
                    vector<std::pair<std::string, std::string>, 5>>);
            REQUIRE(std::is_move_assignable_v<
                    vector<std::pair<std::string, std::string>, 5>>);
        }

        SECTION("Default") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<std::string> alloc;
            vector<std::string, 5, std::allocator<std::string>> a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("With size") {
            std::allocator<std::string> alloc;
            vector<std::string, 5> b(3, alloc);
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(b.get_allocator() == alloc);
        }

        SECTION("From value") {
            std::allocator<std::string> alloc;
            vector<std::string, 5> c(3, "one", alloc);
            REQUIRE(c.size() == 3);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(c, { "one", "one", "one" }));
            REQUIRE(c.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::string> alloc;
            std::vector<std::string> dv = { "six", "five", "four" };
            vector<std::string, 5> d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { "six", "five", "four" }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            vector<std::string, 5> e = { "one", "two" };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { "one", "two" }));
        }

        SECTION("From ranges") {
            std::vector<std::string> v = { "one", "two", "three" };
            vector<std::string, 5> e(v);
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { "one", "two", "three" }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            a = { "six", "five", "four" };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("From another small vector") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            a = { "six", "five", "four" };

            vector<std::string, 5> b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
        }

        SECTION("From iterators") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            std::vector<std::string> v = { "six", "five", "four" };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("From size and value") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            a.assign(3, "one");
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "one", "one", "one" }));
        }

        SECTION("From initializer list") {
            vector<std::string, 5> a;
            REQUIRE(a.empty());
            a.assign({ "six", "five", "four" });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("Fill") {
            vector<std::string, 5> a(3, "one");
            REQUIRE_FALSE(a.empty());
            a.fill("two");
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "two", "two", "two" }));
        }

        SECTION("Swap") {
            vector<std::string, 5> a(4, "one");
            vector<std::string, 5> b(3, "two");

            std::initializer_list<std::string>
                ar = { "one", "one", "one", "one" };
            std::initializer_list<std::string> br = { "two", "two", "two" };

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
        vector<std::string, 5> a = { "one", "two", "three" };
        auto iter = a.begin();

        // LegacyContiguousIterator
        REQUIRE(*(a.begin() + 2) == *(std::addressof(*a.begin()) + 2));

        // LegacyRandomAccessIterator
        REQUIRE((iter += 2) == a.begin() + 2);
        REQUIRE(a.begin() + 1 == 1 + a.begin());
        REQUIRE((iter -= 2) == a.begin());
        REQUIRE(a.end() - 2 == a.begin() + 1);
        REQUIRE(a.end() - a.begin() == 3);
        REQUIRE(a.begin()[1] == "two");
        REQUIRE(a.end()[-1] == "three");

        REQUIRE(a.begin() == a.begin());
        REQUIRE(a.begin() != a.begin() + 2);
        REQUIRE(a.begin() < a.begin() + 2);
        REQUIRE(a.begin() + 2 > a.begin());

        // LegacyBidirectionalIterator + LegacyForwardIterator
        iter = a.begin() + 1;
        REQUIRE(--iter == a.begin());
        REQUIRE(iter++ == a.begin());
        REQUIRE(iter-- == a.begin() + 1);
        iter = a.begin() + 1;
        REQUIRE(*iter-- == "two");

        // LegacyInputIterator/LegacyIterator
        iter = a.begin();
        REQUIRE(++iter == a.begin() + 1);

        REQUIRE(a.begin() == a.data());
        REQUIRE(a.end() == a.data() + a.size());
        REQUIRE_FALSE(a.end() == a.data() + a.capacity());
        REQUIRE(*a.begin() == "one");
        REQUIRE(*std::prev(a.end()) == "three");

        REQUIRE(a.cbegin() == a.data());
        REQUIRE(a.cend() == a.data() + a.size());
        REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
        REQUIRE(*a.cbegin() == "one");
        REQUIRE(*std::prev(a.cend()) == "three");

        REQUIRE(*a.rbegin() == "three");
        REQUIRE(*std::prev(a.rend()) == "one");

        REQUIRE(*a.crbegin() == "three");
        REQUIRE(*std::prev(a.crend()) == "one");

        // Custom comparison operators
        REQUIRE(a.begin() == a.data());
        REQUIRE(a.begin() != a.data() + 2);
        REQUIRE(a.begin() < a.data() + 2);
        REQUIRE(a.begin() + 2 > a.data());

        REQUIRE(a.data() == a.begin());
        REQUIRE(a.data() != a.begin() + 2);
        REQUIRE(a.data() < a.begin() + 2);
        REQUIRE(a.data() + 2 > a.begin());
    }

    SECTION("Capacity") {
        vector<std::string, 5> a = { "one", "two", "three" };
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
        vector<std::string, 5> a = { "one", "two", "three" };
        REQUIRE(a[0] == "one");
        REQUIRE(a[1] == "two");
        REQUIRE(a[2] == "three");
        REQUIRE(a.at(0) == "one");
        REQUIRE(a.at(1) == "two");
        REQUIRE(a.at(2) == "three");
        REQUIRE_THROWS(a.at(3) == "four");
        REQUIRE_THROWS(a.at(4) == "five");
        REQUIRE(a.front() == "one");
        REQUIRE(a.back() == "three");
        REQUIRE(*a.data() == "one");
        REQUIRE(*(a.data() + 1) == "two");
        REQUIRE(*(a.data() + 2) == "three");
        REQUIRE(*(a.data() + a.size() - 1) == "three");
        REQUIRE(*(a.data() + a.size() - 2) == "two");
        REQUIRE(*(a.data() + a.size() - 3) == "one");
    }

    SECTION("Modifiers") {
        vector<std::string, 5> a = { "one", "two", "three" };
        std::string tmp = "four";
        a.push_back(tmp);
        REQUIRE(a.back() == "four");
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four" }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        std::string tmp2 = "five";
        a.push_back(std::move(tmp2));
        REQUIRE(a.back() == "five");
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four", "five" }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four" }));

        a.emplace_back("five");
        REQUIRE(a.back() == "five");
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four", "five" }));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four" }));

        auto it = a.emplace(a.begin() + 2, "ten");
        REQUIRE(it == a.begin() + 2);
        auto &b = a.back();
        REQUIRE(b == "four");
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "ten", "three", "four" }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "ten" }));

        it = a.insert(a.begin() + 1, "twenty");
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "twenty", "two", "ten" }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        it = a.insert(a.begin() + 2, std::move("thirty"));
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "twenty", "thirty", "two", "ten" }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "twenty" }));

        it = a.insert(a.begin() + 1, 2, "ten");
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "ten", "ten", "twenty" }));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "ten" }));

        std::initializer_list<std::string> src = { "two", "four", "eight" };
        it = a.insert(a.begin() + 1, src.begin(), src.end());
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "four", "eight", "ten" }));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two" }));

        it = a.insert(a.begin() + 1, { "two", "four", "eight" });
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "four", "eight", "two" }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "four", "eight", "two" }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two" }));

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

        a.resize(4, "five");
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a[2] == "five");
        REQUIRE(a[3] == "five");
    }

    SECTION("Element access errors") {
        vector<std::string, 5> a = { "one", "two", "three" };
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
        vector<std::string, 5> a = { "one", "two", "three" };
        vector<std::string, 5> b = { "two", "four", "five" };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }

    SECTION("From raw vector") {
        auto a = to_vector({ "one", "two", "three" });
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 3);
        REQUIRE(a.capacity() == default_inline_storage_v<std::string>);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three" }));

        auto b = to_vector<std::string, 3, 5>({ "one", "two", "three" });
        REQUIRE(b.size() == 3);
        REQUIRE(b.max_size() > 5);
        REQUIRE(b.capacity() == 5);
        REQUIRE_FALSE(b.empty());
        REQUIRE(equal_il(b, { "one", "two", "three" }));

        std::string cr[] = { "one", "two", "three" };
        auto c = to_vector(cr);
        REQUIRE(c.size() == 3);
        REQUIRE(c.max_size() > 3);
        REQUIRE(c.capacity() == 5);
        REQUIRE(c.capacity() == default_inline_storage_v<std::string>);
        REQUIRE_FALSE(c.empty());
        REQUIRE(equal_il(b, { "one", "two", "three" }));
    }
}
