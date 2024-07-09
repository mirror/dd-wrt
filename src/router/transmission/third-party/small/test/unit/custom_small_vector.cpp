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
#include <small/string.hpp>
#include <small/vector.hpp>

// A relocatable custom type
struct custom_type
{
    enum class custom_enum
    {
        even,
        odd
    };
    custom_type() {
    } // NOLINT(modernize-use-equals-default,cppcoreguidelines-pro-type-member-init)
    custom_type(const std::string &v) // NOLINT(google-explicit-constructor)
        : type_(v.size() & 1 ? custom_enum::even : custom_enum::odd), name_(v),
          url_("https://" + v),
          version_(
              v.size() < 4 ? std::optional<int>(std::nullopt) :
                             std::optional<int>(static_cast<int>(v.size()))),
          tag_(v.substr(2)), system_(v.substr(0, 2)), raw_(v) {}
    custom_type(const char *v)
        : custom_type(std::string(v)) {} // NOLINT(google-explicit-constructor)
    custom_enum type_;
    // use relocatable internal string
    small::string name_;
    small::string url_;
    std::optional<int> version_{ std::nullopt };
    std::optional<small::string> tag_{ std::nullopt };
    std::optional<small::string> system_{ std::nullopt };
    std::optional<small::string> raw_{ std::nullopt };
};

// Allow comparing with std::string to make tests easier
bool
operator==(const custom_type &lhs, const custom_type &rhs) {
    return lhs.raw_ == rhs.raw_;
}
bool
operator!=(const custom_type &lhs, const custom_type &rhs) {
    return !(rhs == lhs);
}

bool
operator==(const custom_type &lhs, const std::string &rhs) {
    return lhs.raw_ == rhs;
}
bool
operator!=(const custom_type &lhs, const std::string &rhs) {
    return !(lhs == rhs);
}
bool
operator==(const std::string &lhs, const custom_type &rhs) {
    return rhs.raw_ == lhs;
}
bool
operator!=(const std::string &lhs, const custom_type &rhs) {
    return !(lhs == rhs);
}

bool
operator==(const custom_type &lhs, const char *rhs) {
    return lhs.raw_ == std::string(rhs);
}
bool
operator!=(const custom_type &lhs, const char *rhs) {
    return !(lhs == rhs);
}
bool
operator==(const char *lhs, const custom_type &rhs) {
    return rhs.raw_ == std::string(lhs);
}
bool
operator!=(const char *lhs, const custom_type &rhs) {
    return !(lhs == rhs);
}

bool
operator<(const custom_type &lhs, const custom_type &rhs) {
    return lhs.raw_ < rhs.raw_;
}
bool
operator>(const custom_type &lhs, const custom_type &rhs) {
    return rhs < lhs;
}
bool
operator<=(const custom_type &lhs, const custom_type &rhs) {
    return !(rhs < lhs);
}
bool
operator>=(const custom_type &lhs, const custom_type &rhs) {
    return !(lhs < rhs);
}

bool
operator<(const custom_type &lhs, const std::string &rhs) {
    return lhs.raw_ < rhs;
}
bool
operator>(const custom_type &lhs, const std::string &rhs) {
    return rhs < lhs;
}
bool
operator<=(const custom_type &lhs, const std::string &rhs) {
    return !(rhs < lhs);
}
bool
operator>=(const custom_type &lhs, const std::string &rhs) {
    return !(lhs < rhs);
}

namespace small {
    // The custom type has no internal pointers, so we can relocate it faster
    // Most types might be relocatable, but we have to conservatively assume
    // they are not
    template <>
    struct is_relocatable<custom_type> : std::true_type
    {};

    // Vectors of custom type should have 10 inlined values by default for some
    // reason, so we don't need to specify this default value for every small
    // vector
    template <>
    struct default_inline_storage<custom_type>
        : std::integral_constant<size_t, 10>
    {};
} // namespace small

TEST_CASE("Custom Vector") {
    using namespace small;

    STATIC_REQUIRE(is_relocatable_v<custom_type>);
    auto equal_il = [](const auto &sm_vector,
                       std::initializer_list<custom_type> il) -> bool {
        return std::
            equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
    };

    SECTION("Constructor") {
        SECTION("Asserts") {
            REQUIRE(std::is_copy_constructible_v<vector<custom_type, 5>>);
            REQUIRE(std::is_copy_assignable_v<vector<custom_type, 5>>);
            REQUIRE(std::is_move_constructible_v<vector<custom_type, 5>>);
            REQUIRE(std::is_move_assignable_v<vector<custom_type, 5>>);

            REQUIRE(std::is_copy_constructible_v<
                    vector<std::pair<custom_type, custom_type>, 5>>);
            REQUIRE(std::is_copy_assignable_v<
                    vector<std::pair<custom_type, custom_type>, 5>>);
            REQUIRE(std::is_move_constructible_v<
                    vector<std::pair<custom_type, custom_type>, 5>>);
            REQUIRE(std::is_move_assignable_v<
                    vector<std::pair<custom_type, custom_type>, 5>>);
        }

        SECTION("Default") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<custom_type> alloc;
            vector<custom_type, 5, std::allocator<custom_type>> a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("With size") {
            std::allocator<custom_type> alloc;
            vector<custom_type, 5> b(3, alloc);
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(b.get_allocator() == alloc);
        }

        SECTION("From value") {
            std::allocator<custom_type> alloc;
            vector<custom_type, 5> c(3, "one", alloc);
            REQUIRE(c.size() == 3);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(c, { "one", "one", "one" }));
            REQUIRE(c.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<custom_type> alloc;
            std::vector<custom_type> dv = { "six", "five", "four" };
            vector<custom_type, 5> d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, { "six", "five", "four" }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            vector<custom_type, 5> e = { "one", "two" };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { "one", "two" }));
        }

        SECTION("From ranges") {
            std::vector<custom_type> v = { "one", "two", "three" };
            vector<custom_type, 5> e(v);
            REQUIRE(e.size() == 3);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, { "one", "two", "three" }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            a = { "six", "five", "four" };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("From another small vector") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            a = { "six", "five", "four" };

            vector<custom_type, 5> b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
        }

        SECTION("From iterators") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            std::vector<custom_type> v = { "six", "five", "four" };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("From size and value") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            a.assign(3, "one");
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "one", "one", "one" }));
        }

        SECTION("From initializer list") {
            vector<custom_type, 5> a;
            REQUIRE(a.empty());
            a.assign({ "six", "five", "four" });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "six", "five", "four" }));
        }

        SECTION("Fill") {
            vector<custom_type, 5> a(3, "one");
            REQUIRE_FALSE(a.empty());
            a.fill("two");
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, { "two", "two", "two" }));
        }

        SECTION("Swap") {
            vector<custom_type, 5> a(4, "one");
            vector<custom_type, 5> b(3, "two");

            std::initializer_list<custom_type>
                ar = { "one", "one", "one", "one" };
            std::initializer_list<custom_type> br = { "two", "two", "two" };

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
        vector<custom_type, 5> a = { "one", "two", "three" };

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
    }

    SECTION("Capacity") {
        /*
         * The inline capacity depends on the platform, but these values don't
         * vary a lot
         */
        vector<custom_type, 5> a = { "one", "two", "three" };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.is_inline());
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
        vector<custom_type, 5> a = { "one", "two", "three" };
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
        vector<custom_type, 5> a = { "one", "two", "three" };
        a.push_back("four");
        REQUIRE(a.back() == "four");
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three", "four" }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        a.push_back(std::move("five"));
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

        std::initializer_list<custom_type> src = { "two", "four", "eight" };
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
        vector<custom_type, 5> a = { "one", "two", "three" };
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
        vector<custom_type, 5> a = { "one", "two", "three" };
        vector<custom_type, 5> b = { "two", "four", "five" };

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
        REQUIRE(a.capacity() == 5);
        REQUIRE(a.capacity() == default_inline_storage_v<std::string>);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(a, { "one", "two", "three" }));

        auto b = to_vector<custom_type, 3, 5>({ "one", "two", "three" });
        REQUIRE(b.size() == 3);
        REQUIRE(b.max_size() > 5);
        REQUIRE(b.capacity() == 5);
        REQUIRE_FALSE(b.empty());
        REQUIRE(equal_il(b, { "one", "two", "three" }));

        custom_type cr[] = { "one", "two", "three" };
        auto c = to_vector(cr);
        REQUIRE(c.size() == 3);
        REQUIRE(c.max_size() > 3);
        REQUIRE(c.capacity() == 10);
        REQUIRE(c.capacity() == default_inline_storage_v<custom_type>);
        REQUIRE_FALSE(c.empty());
        REQUIRE(equal_il(b, { "one", "two", "three" }));
    }
}
