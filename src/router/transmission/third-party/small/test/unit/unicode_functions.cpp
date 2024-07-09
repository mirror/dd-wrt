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

TEST_CASE("Unicode") {
    using namespace small;
    using namespace small::detail;

    SECTION("UTF8") {
        utf8_char_type a = 'g';
        // utf8_char_type b = 'á'; // <- can't fit in 8 bits
        // utf8_char_type c = '😀';
        std::basic_string<utf8_char_type> d = u8"g";
        std::basic_string<utf8_char_type> e = u8"á";
        std::basic_string<utf8_char_type> f = u8"😀";
        SECTION("Check") {
            // Check container sizes
            REQUIRE(d.size() == 1);
            REQUIRE(e.size() == 2);
            REQUIRE(f.size() == 4);

            // Identify continuation bytes
            REQUIRE_FALSE(is_utf8_continuation(d[0]));
            REQUIRE_FALSE(is_utf8_continuation(e[0]));
            REQUIRE(is_utf8_continuation(e[1]));
            REQUIRE_FALSE(is_utf8_continuation(f[0]));
            REQUIRE(is_utf8_continuation(f[1]));
            REQUIRE(is_utf8_continuation(f[2]));
            REQUIRE(is_utf8_continuation(f[3]));

            // Identify utf size from first char
            REQUIRE(utf8_size(a) == 1);
            REQUIRE(utf8_size(d[0]) == 1);
            REQUIRE(utf8_size(e[0]) == 2);
            REQUIRE(utf8_size(e[1]) == 1);
            REQUIRE(utf8_size(f[0]) == 4);
            REQUIRE(utf8_size(f[1]) == 1);
            REQUIRE(utf8_size(f[2]) == 1);
            REQUIRE(utf8_size(f[3]) == 1);

            // Identify continuation bytes (inferring input type)
            REQUIRE_FALSE(is_utf_continuation(d[0]));
            REQUIRE_FALSE(is_utf_continuation(e[0]));
            REQUIRE(is_utf_continuation(e[1]));
            REQUIRE_FALSE(is_utf_continuation(f[0]));
            REQUIRE(is_utf_continuation(f[1]));
            REQUIRE(is_utf_continuation(f[2]));
            REQUIRE(is_utf_continuation(f[3]));

            // Identify utf size from first char (inferring input type)
            REQUIRE(utf_size(a, 1) == 1);
            REQUIRE(utf_size(d[0], 1) == 1);
            REQUIRE(utf_size(e[0], 2) == 2);
            REQUIRE(utf_size(e[1], 1) == 1);
            REQUIRE(utf_size(f[0], 4) == 4);
            REQUIRE(utf_size(f[1], 4) == 1);
            REQUIRE(utf_size(f[2], 4) == 1);
            REQUIRE(utf_size(f[3], 4) == 1);
        }
        SECTION("To UTF16") {
            utf16_char_type buf[2];

            REQUIRE(from_utf8_to_utf16(&a, 1, buf, 2) == 1);
            utf32_char_type r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf8_to_utf16(d.begin(), d.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf8_to_utf16(e.begin(), e.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf8_to_utf16(f.begin(), f.size(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf16(&a, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf16(d.begin(), d.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf16(e.begin(), e.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf16(f.begin(), f.size(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(d.begin(), d.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');
        }

        SECTION("To UTF32") {
            utf32_char_type r;

            r = from_utf8_to_utf32(&a, 1);
            REQUIRE(r == U'g');

            r = from_utf8_to_utf32(d.begin(), d.size());
            REQUIRE(r == U'g');

            r = from_utf8_to_utf32(e.begin(), e.size());
            REQUIRE(r == U'á');

            r = from_utf8_to_utf32(f.begin(), f.size());
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf32(&a, 1, &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf32(d.begin(), d.size(), &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf32(e.begin(), e.size(), &r, 1) == 1);
            REQUIRE(r == U'á');

            REQUIRE(to_utf32(f.begin(), f.size(), &r, 1) == 1);
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf(d.begin(), d.size(), &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), &r, 1) == 1);
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), &r, 1) == 1);
            REQUIRE(r == U'😀');
        }
    }
    SECTION("UTF16") {
        utf16_char_type a = u'g';
        utf16_char_type b = u'á';
        // utf16_char_type c = u'😀'; // <- can't fit in a char
        std::basic_string<utf16_char_type> d = u"g";
        std::basic_string<utf16_char_type> e = u"á";
        std::basic_string<utf16_char_type> f = u"😀";

        SECTION("Check") {
            // Check container sizes
            REQUIRE(d.size() == 1);
            REQUIRE(e.size() == 1);
            REQUIRE(f.size() == 2);

            // Identify surrogate code units
            REQUIRE_FALSE(is_utf16_surrogate(a));
            REQUIRE_FALSE(is_utf16_surrogate(b));
            REQUIRE_FALSE(is_utf16_surrogate(d[0]));
            REQUIRE_FALSE(is_utf16_surrogate(e[0]));
            REQUIRE(is_utf16_surrogate(f[0]));
            REQUIRE(is_utf16_surrogate(f[1]));

            // Identify high and low surrogate code units
            REQUIRE_FALSE(is_utf16_high_surrogate(a));
            REQUIRE_FALSE(is_utf16_low_surrogate(a));
            REQUIRE_FALSE(is_utf16_high_surrogate(b));
            REQUIRE_FALSE(is_utf16_low_surrogate(b));
            REQUIRE_FALSE(is_utf16_high_surrogate(d[0]));
            REQUIRE_FALSE(is_utf16_low_surrogate(d[0]));
            REQUIRE_FALSE(is_utf16_high_surrogate(e[0]));
            REQUIRE_FALSE(is_utf16_low_surrogate(e[0]));
            REQUIRE(is_utf16_high_surrogate(f[0]));
            REQUIRE_FALSE(is_utf16_low_surrogate(f[0]));
            REQUIRE_FALSE(is_utf16_high_surrogate(f[1]));
            REQUIRE(is_utf16_low_surrogate(f[1]));

            // Identify continuation code units (alias for low surrogates)
            REQUIRE_FALSE(is_utf16_continuation(a));
            REQUIRE_FALSE(is_utf16_continuation(b));
            REQUIRE_FALSE(is_utf16_continuation(d[0]));
            REQUIRE_FALSE(is_utf16_continuation(e[0]));
            REQUIRE_FALSE(is_utf16_continuation(f[0]));
            REQUIRE(is_utf16_continuation(f[1]));

            // Identify utf size from first char
            REQUIRE(utf16_size(a) == 1);
            REQUIRE(utf16_size(b) == 1);
            REQUIRE(utf16_size(d[0]) == 1);
            REQUIRE(utf16_size(e[0]) == 1);
            REQUIRE(utf16_size(f[0]) == 2);
            REQUIRE(utf16_size(f[1]) == 1);

            // Identify continuation code units identifying input type
            REQUIRE_FALSE(is_utf_continuation(a));
            REQUIRE_FALSE(is_utf_continuation(b));
            REQUIRE_FALSE(is_utf_continuation(d[0]));
            REQUIRE_FALSE(is_utf_continuation(e[0]));
            REQUIRE_FALSE(is_utf_continuation(f[0]));
            REQUIRE(is_utf_continuation(f[1]));

            // Identify utf size from first char identifying input type
            REQUIRE(utf_size(a, 1) == 1);
            REQUIRE(utf_size(b, 1) == 1);
            REQUIRE(utf_size(d[0], 1) == 1);
            REQUIRE(utf_size(e[0], 1) == 1);
            REQUIRE(utf_size(f[0], 2) == 2);
            REQUIRE(utf_size(f[1], 2) == 1);
        }
        SECTION("To UTF8") {
            utf8_char_type buf[8];

            REQUIRE(from_utf16_to_utf8(&a, 1, buf, 8) == 1);
            utf32_char_type r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf16_to_utf8(&b, 1, buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf16_to_utf8(d.begin(), d.size(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf16_to_utf8(e.begin(), e.size(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf16_to_utf8(f.begin(), f.size(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf8(&a, 1, buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf8(&b, 1, buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf8(d.begin(), d.size(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf8(e.begin(), e.size(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf8(f.begin(), f.size(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(d.begin(), d.size(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');
        }
        SECTION("To UTF32") {
            REQUIRE(utf16_surrogates_to_utf32(f[0], f[1]) == U'😀');

            utf32_char_type r;

            r = from_utf16_to_utf32(&a, 1);
            REQUIRE(r == U'g');

            r = from_utf16_to_utf32(&b, 1);
            REQUIRE(r == U'á');

            r = from_utf16_to_utf32(d.begin(), d.size());
            REQUIRE(r == U'g');

            r = from_utf16_to_utf32(e.begin(), e.size());
            REQUIRE(r == U'á');

            r = from_utf16_to_utf32(f.begin(), f.size());
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf32(&a, 1, &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf32(&b, 1, &r, 1) == 1);
            REQUIRE(r == U'á');

            REQUIRE(to_utf32(d.begin(), d.size(), &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf32(e.begin(), e.size(), &r, 1) == 1);
            REQUIRE(r == U'á');

            REQUIRE(to_utf32(f.begin(), f.size(), &r, 1) == 1);
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf(d.begin(), d.size(), &r, 1) == 1);
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), &r, 1) == 1);
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), &r, 1) == 1);
            REQUIRE(r == U'😀');
        }
    }
    SECTION("UTF32") {
        utf32_char_type a = U'g';
        utf32_char_type b = U'á';
        utf32_char_type c = U'😀';
        std::basic_string<utf32_char_type> d = U"g";
        std::basic_string<utf32_char_type> e = U"á";
        std::basic_string<utf32_char_type> f = U"😀";
        SECTION("Check") {
            // Check container sizes
            REQUIRE(d.size() == 1);
            REQUIRE(e.size() == 1);
            REQUIRE(f.size() == 1);

            // Identify continuation code units (always false for utf32)
            REQUIRE_FALSE(is_utf32_continuation(a));
            REQUIRE_FALSE(is_utf32_continuation(b));
            REQUIRE_FALSE(is_utf32_continuation(d[0]));
            REQUIRE_FALSE(is_utf32_continuation(e[0]));
            REQUIRE_FALSE(is_utf32_continuation(f[0]));

            // Identify utf size from first char (always 1 for utf32)
            REQUIRE(utf32_size(a) == 1);
            REQUIRE(utf32_size(b) == 1);
            REQUIRE(utf32_size(c) == 1);
            REQUIRE(utf32_size(d[0]) == 1);
            REQUIRE(utf32_size(e[0]) == 1);
            REQUIRE(utf32_size(f[0]) == 1);

            // Identify continuation code units identifying input type
            REQUIRE_FALSE(is_utf_continuation(a));
            REQUIRE_FALSE(is_utf_continuation(b));
            REQUIRE_FALSE(is_utf_continuation(d[0]));
            REQUIRE_FALSE(is_utf_continuation(e[0]));
            REQUIRE_FALSE(is_utf_continuation(f[0]));

            // Identify utf size from first char identifying input type
            REQUIRE(utf_size(a, 1) == 1);
            REQUIRE(utf_size(b, 1) == 1);
            REQUIRE(utf_size(c, 1) == 1);
            REQUIRE(utf_size(d[0], 1) == 1);
            REQUIRE(utf_size(e[0], 1) == 1);
            REQUIRE(utf_size(f[0], 1) == 1);
        }
        SECTION("To UTF8") {
            utf8_char_type buf[8];

            REQUIRE(from_utf32_to_utf8(a, buf, 8) == 1);
            utf32_char_type r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf32_to_utf8(b, buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf32_to_utf8(c, buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(from_utf32_to_utf8(d.front(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf32_to_utf8(e.front(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf32_to_utf8(f.front(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf8(&a, 1, buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf8(&b, 1, buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf8(&c, 1, buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(to_utf8(d.begin(), d.size(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf8(e.begin(), e.size(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf8(f.begin(), f.size(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(&b, 1, buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(&c, 1, buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(to_utf(d.begin(), d.size(), buf, 8) == 1);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), buf, 8) == 2);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), buf, 8) == 4);
            r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
            REQUIRE(r == U'😀');
        }

        SECTION("To UTF16") {
            utf16_char_type buf[2];

            REQUIRE(from_utf32_to_utf16(a, buf, 2) == 1);
            utf32_char_type r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf32_to_utf16(b, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf32_to_utf16(c, buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(from_utf32_to_utf16(d.front(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(from_utf32_to_utf16(e.front(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(from_utf32_to_utf16(f.front(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input
            REQUIRE(to_utf16(&a, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf16(&b, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf16(&c, 1, buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(to_utf16(d.begin(), d.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf16(e.begin(), e.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf16(f.begin(), f.size(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            // Inferring type from input and output
            REQUIRE(to_utf(&a, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(&b, 1, buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(&c, 1, buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');

            REQUIRE(to_utf(d.begin(), d.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'g');

            REQUIRE(to_utf(e.begin(), e.size(), buf, 2) == 1);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'á');

            REQUIRE(to_utf(f.begin(), f.size(), buf, 2) == 2);
            r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
            REQUIRE(r == U'😀');
        }
    }
}