//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/string.hpp>
#include <small/vector.hpp>
#include <algorithm>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <catch2/catch.hpp>
#include <string_view>
#include <unordered_set>

// UTF8 string literals are really not safe in MSVC.
// u8"" doesn't work properly even with escape sequences.
// More recent versions might improve on that a little, but we will still
// need a more robust solution to support older versions in any case.
// Until then, it seems like the most robust solution is to initialize
// small::strings with U"...", even if that requires converting the
// codepoints.
constexpr bool
is_windows() {
#if defined(_WIN32)
    return true;
#else
    return false;
#endif
}

TEST_CASE("String") {
    using namespace small;

    SECTION("Element access") {
        SECTION("At") {
            string s = "123456";
            REQUIRE(s.at(0) == '1');
            REQUIRE(s.at(1) == '2');
            REQUIRE(s.at(2) == '3');
            REQUIRE(s.at(3) == '4');
            REQUIRE(s.at(4) == '5');
            REQUIRE(s.at(5) == '6');
        }

        using cp_index = string::codepoint_index;

        SECTION("At codepoint (through references)") {
            SECTION("No unicode") {
                string s = "123456";
                REQUIRE(s.at(cp_index(0)) == '1');
                REQUIRE(s.at(cp_index(1)) == '2');
                REQUIRE(s.at(cp_index(2)) == '3');
                REQUIRE(s.at(cp_index(3)) == '4');
                REQUIRE(s.at(cp_index(4)) == '5');
                REQUIRE(s.at(cp_index(5)) == '6');
            }
            SECTION("Half unicode") {
                string s = "1😀2😀3😀";
                REQUIRE(s.at(cp_index(0)) == '1');
                REQUIRE(s.at(cp_index(1)) == U'😀');
                REQUIRE(s.at(cp_index(2)) == '2');
                REQUIRE(s.at(cp_index(3)) == U'😀');
                REQUIRE(s.at(cp_index(4)) == '3');
                REQUIRE(s.at(cp_index(5)) == U'😀');
                REQUIRE(s.at(cp_index(0)) == "1");
                REQUIRE(s.at(cp_index(1)) == "😀");
                REQUIRE(s.at(cp_index(2)) == "2");
                REQUIRE(s.at(cp_index(3)) == "😀");
                REQUIRE(s.at(cp_index(4)) == "3");
                REQUIRE(s.at(cp_index(5)) == "😀");
            }
            SECTION("Full unicode") {
                string s = "🙂😀🙂😀🙂😀";
                REQUIRE(s.at(cp_index(0)) == U'🙂');
                REQUIRE(s.at(cp_index(1)) == U'😀');
                REQUIRE(s.at(cp_index(2)) == U'🙂');
                REQUIRE(s.at(cp_index(3)) == U'😀');
                REQUIRE(s.at(cp_index(4)) == U'🙂');
                REQUIRE(s.at(cp_index(5)) == U'😀');
                REQUIRE(s.at(cp_index(0)) == "🙂");
                REQUIRE(s.at(cp_index(1)) == "😀");
                REQUIRE(s.at(cp_index(2)) == "🙂");
                REQUIRE(s.at(cp_index(3)) == "😀");
                REQUIRE(s.at(cp_index(4)) == "🙂");
                REQUIRE(s.at(cp_index(5)) == "😀");
            }
        }

        SECTION("Subscript") {
            string s = "123456";
            REQUIRE(s[0] == '1');
            REQUIRE(s[1] == '2');
            REQUIRE(s[2] == '3');
            REQUIRE(s[3] == '4');
            REQUIRE(s[4] == '5');
            REQUIRE(s[5] == '6');
        }

        SECTION("Subscript codepoint") {
            string s = "123456";
            REQUIRE(s[cp_index(0)] == '1');
            REQUIRE(s[cp_index(1)] == '2');
            REQUIRE(s[cp_index(2)] == '3');
            REQUIRE(s[cp_index(3)] == '4');
            REQUIRE(s[cp_index(4)] == '5');
            REQUIRE(s[cp_index(5)] == '6');
        }

        SECTION("Subscript codepoint (direct values)") {
            string s = "1😀2😀3😀";
            REQUIRE(s(cp_index(0)) == '1');
            REQUIRE(s(cp_index(1)) == U'😀');
            REQUIRE(s(cp_index(2)) == '2');
            REQUIRE(s(cp_index(3)) == U'😀');
            REQUIRE(s(cp_index(4)) == '3');
            REQUIRE(s(cp_index(5)) == U'😀');
        }

        SECTION("Subscript codepoint (through references)") {
            string s = "1😀2😀3😀";
            REQUIRE(s[cp_index(0)] == '1');
            REQUIRE(s[cp_index(1)] == U'😀');
            REQUIRE(s[cp_index(2)] == '2');
            REQUIRE(s[cp_index(3)] == U'😀');
            REQUIRE(s[cp_index(4)] == '3');
            REQUIRE(s[cp_index(5)] == U'😀');
        }

        SECTION("Front/Back") {
            string s = "1😀2😀3😀5";
            REQUIRE(s.front() == '1');
            REQUIRE(s.back() == '5');
        }

        SECTION("Front/Back Codepoints") {
            string s = "😀1😀2😀3😀5😀";
            REQUIRE(s.front_codepoint() == U'😀');
            REQUIRE(s.back_codepoint() == U'😀');
            REQUIRE(s.front_codepoint() == "😀");
            REQUIRE(s.back_codepoint() == "😀");
        }

        SECTION("Data") {
            string s = "1😀2😀3😀5";
            string_view sv(s.data(), s.size());
            REQUIRE(s == sv);
            REQUIRE(s.data() == s.c_str());
            REQUIRE(s.operator string_view() == sv);
        }
    }

    SECTION("Iterators") {
        SECTION("Byte Iterators") {
            string a = "123";
            REQUIRE(a.begin() == a.data());
            REQUIRE(a.end() == a.data() + a.size());

            REQUIRE(*a.begin() == '1');
            REQUIRE(*std::next(a.begin()) == '2');
            REQUIRE(*std::prev(a.end()) == '3');

            REQUIRE(a.cbegin() == a.data());
            REQUIRE(a.cend() == a.data() + a.size());

            REQUIRE(*a.cbegin() == '1');
            REQUIRE(*std::next(a.cbegin()) == '2');
            REQUIRE(*std::prev(a.cend()) == '3');

            REQUIRE(*a.rbegin() == '3');
            REQUIRE(*std::next(a.rbegin()) == '2');
            REQUIRE(*std::prev(a.rend()) == '1');

            REQUIRE(*a.crbegin() == '3');
            REQUIRE(*std::next(a.crbegin()) == '2');
            REQUIRE(*std::prev(a.crend()) == '1');
        }

        SECTION("Codepoint Iterators") {
            string a = "😐🙂😀";
            REQUIRE(
                static_cast<size_t>(a.end_codepoint() - a.begin_codepoint())
                == a.size_codepoints());

            REQUIRE(*a.begin_codepoint() == U'😐');
            REQUIRE(*std::next(a.begin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.end_codepoint()) == U'😀');

            REQUIRE(*a.cbegin_codepoint() == a.front_codepoint());
            REQUIRE(*std::prev(a.cend_codepoint()) == a.back_codepoint());

            REQUIRE(*a.cbegin_codepoint() == U'😐');
            REQUIRE(*std::next(a.cbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.cend_codepoint()) == U'😀');

            REQUIRE(*a.rbegin_codepoint() == U'😀');
            REQUIRE(*std::next(a.rbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.rend_codepoint()) == U'😐');

            REQUIRE(*a.crbegin_codepoint() == U'😀');
            REQUIRE(*std::next(a.crbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.crend_codepoint()) == U'😐');
        }
    }

    SECTION("Capacity") {
        string a = U"1😀3";

        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 13);
        REQUIRE(a.capacity() <= 15);
        size_t old_cap = a.capacity();

        a.reserve(10);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        size_t new_cap = a.capacity();
        REQUIRE(new_cap >= old_cap);

        a.reserve(20);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        new_cap = a.capacity();
        REQUIRE(new_cap > old_cap);

        a.shrink_to_fit();
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        new_cap = a.capacity();
        REQUIRE(
            new_cap
            >= 6); // larger than initial size but might not be inline anymore

        a = U"1😀3";
        a.shrink_to_fit();
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= a.size());
        REQUIRE_FALSE(is_malformed(a));
    }
}