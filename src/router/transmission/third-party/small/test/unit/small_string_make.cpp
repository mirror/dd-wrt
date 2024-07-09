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

TEST_CASE("String Rule of 5") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_string, std::initializer_list<char> il) -> bool {
        return std::
            equal(sm_string.begin(), sm_string.end(), il.begin(), il.end());
    };

    SECTION("Constructor") {
        SECTION("Default") {
            string a;
            REQUIRE(a.empty());
            REQUIRE(a.size() == 0); // NOLINT(readability-container-size-empty)
            REQUIRE(a.size_codepoints() == 0);
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            string a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From char value") {
            SECTION("Zero values") {
                std::allocator<int> alloc;
                string c(0, 'x', alloc); // NOLINT(bugprone-string-constructor)
                REQUIRE(c.empty());
                REQUIRE(
                    c.size() == 0); // NOLINT(readability-container-size-empty)
                REQUIRE(c.size_codepoints() == 0);
                REQUIRE(c == "");  // NOLINT(readability-container-size-empty)
                REQUIRE(c == U""); // NOLINT(readability-container-size-empty)
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("Constant values") {
                std::allocator<int> alloc;
                string c(3, 'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == U"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From UTF16 value") {
                std::allocator<int> alloc;
                string c(3, u'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == u"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From multibyte UTF16 value") {
                // We don't provide full support/conversions for multi-code-unit
                // UTF16 for now, but other uni-code-unit UTF16 strings should
                // be fine. In any case, UTF8 and UTF32 should be able to
                // represent anything. You can use UTF32 as an intermediary if
                // you need a case that is not supported.
                std::allocator<int> alloc;
                string c(3, u'á', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c.size() == 6);
                REQUIRE(c == "ááá");
                REQUIRE(c == u"ááá");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From utf32 value") {
                std::allocator<int> alloc;
                string c(3, U'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == U"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From multibyte utf32 value") {
                std::allocator<int> alloc;
                string c(3, U'😀', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c.size() == 12);
                REQUIRE(c == "😀😀😀");
                REQUIRE(c == U"😀😀😀");
                REQUIRE(c.get_allocator() == alloc);
            }
        }

        SECTION("From char iterators") {
            SECTION("Empty range") {
                std::allocator<int> alloc;
                std::string dv;
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE(d.empty());
                REQUIRE(d.size_codepoints() == 0);
                REQUIRE(
                    d.size() == 0); // NOLINT(readability-container-size-empty)
                REQUIRE(d == "");   // NOLINT(readability-container-size-empty)
                REQUIRE(d == U"");  // NOLINT(readability-container-size-empty)
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("No unicode") {
                std::allocator<int> alloc;
                std::string dv = "654";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                std::string dv = "😀6😀5😀4";
                REQUIRE(dv.size() == 15);
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                std::string dv = "😀😀😀😀😀😀";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From wide char iterators") {
            SECTION("No unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"654";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"😀6😀5😀4";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"😀😀😀😀😀😀";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From codepoint iterators") {
            SECTION("No unicode") {
                std::allocator<int> alloc;
                string dv = U"654";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                string dv = U"😀6😀5😀4";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                string dv = U"😀😀😀😀😀😀";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From substring") {
            SECTION("From begin") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 3, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "456");
                REQUIRE(d == U"456");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From range") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 2, 2, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 2);
                REQUIRE(d.size() == 2);
                REQUIRE(d == "34");
                REQUIRE(d == U"34");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From npos range") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 2, string::npos, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 4);
                REQUIRE(d.size() == 4);
                REQUIRE(d == "3456");
                REQUIRE(d == U"3456");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("Literal string count") {
                SECTION("Char") {
                    std::allocator<int> alloc;
                    string d("123456", 2, alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 2);
                    REQUIRE(d.size() == 2);
                    REQUIRE(d == "12");
                    REQUIRE(d == U"12");
                    REQUIRE(d.get_allocator() == alloc);
                }

                SECTION("Wide char") {
                    std::allocator<int> alloc;
                    string d(U"123456", 2, alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 2);
                    REQUIRE(d.size() == 2);
                    REQUIRE(d == "12");
                    REQUIRE(d == U"12");
                    REQUIRE(d.get_allocator() == alloc);
                }
            }
        }

        SECTION("From literal") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d("123456", alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(U"123456", alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From initializer list") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d({ '1', '2', '3', '4', '5', '6' }, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d({ U'1', U'2', U'3', U'4', U'5', U'6' }, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From string view") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d(std::string_view("123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(std::u32string_view(U"123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From std string") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d(std::string("123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(std::u32string(U"123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("Rule of five") {
            SECTION("Copy") {
                string dv = "1😀2😀3😀";
                string d(dv);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == "1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
            }

            SECTION("Copy and set alloc") {
                std::allocator<int> alloc;
                string dv = "1😀2😀3😀";
                string d(dv, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == "1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("Move") {
                string dv = "1😀2😀3😀";
                string d(std::move(dv));
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(
                    dv.size_codepoints()
                    == 0); // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(
                    dv.size()
                    == 0); // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(
                    dv
                    == ""); // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(
                    dv
                    == U""); // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(dv.empty());
                // is null terminated
                REQUIRE(dv[0] == '\0');
            }

            if constexpr (not is_windows()) {
                SECTION("Move and set alloc") {
                    std::allocator<int> alloc;
                    // There's no safe way to do that on MSVC :O
                    string dv = u8"1😀2😀3😀";
                    string d(std::move(dv), alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 6);
                    REQUIRE(d.size() == 15);
                    REQUIRE(d == "1😀2😀3😀");
                    REQUIRE(d == U"1😀2😀3😀");
                    REQUIRE(
                        dv.size_codepoints()
                        == 0); // NOLINT(bugprone-use-after-move)
                    REQUIRE(
                        dv.size()
                        == 0); // NOLINT(readability-container-size-empty)
                    REQUIRE(
                        dv == ""); // NOLINT(readability-container-size-empty)
                    REQUIRE(
                        dv == U""); // NOLINT(readability-container-size-empty)
                    REQUIRE(dv.empty());
                    // is null terminated
                    REQUIRE(dv[0] == '\0');
                    REQUIRE(d.get_allocator() == alloc);
                }
            }
        }
    }

    SECTION("Assignment Operator") {
        if constexpr (not is_windows()) {
            SECTION("String") {
                string dv = u8"1😀2😀3😀";
                string d;
                d = dv;
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == u8"1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == u8"1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
            }
        }

        SECTION("Move String") {
            string dv = "1😀2😀3😀";
            string d;
            d = std::move(dv);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
            REQUIRE(
                dv.size_codepoints() == 0); // NOLINT(bugprone-use-after-move)
            REQUIRE(
                dv.size()
                == 0); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(
                dv
                == ""); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(
                dv
                == U""); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE_FALSE(d == dv);
            REQUIRE(
                dv.size()
                == 0); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv.front() == '\0');
        }

        SECTION("Literal") {
            string d;
            d = "1😀2😀3😀";
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
        }

        SECTION("Wide Literal") {
            string d;
            d = U"1😀2😀3😀";
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
        }

        SECTION("Char") {
            string d;
            d = '1';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 1);
            REQUIRE(d == "1");
            REQUIRE(d == U"1");
        }

        SECTION("Wide Char") {
            string d;
            d = U'1';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 1);
            REQUIRE(d == "1");
            REQUIRE(d == U"1");
        }

        SECTION("Unicode Wide Char") {
            string d;
            d = U'😀';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 4);
            REQUIRE(d == "😀");
            REQUIRE(d == U"😀");
        }
    }

    SECTION("Assign") {
        SECTION("Char") {
            string d;
            d.assign(3, '1');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "111");
            REQUIRE(d == U"111");
        }

        SECTION("Wide Char") {
            string d;
            d.assign(3, U'1');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "111");
            REQUIRE(d == U"111");
        }

        SECTION("Unicode Wide Char") {
            string d;
            d.assign(3, U'😀');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 12);
            REQUIRE(d == "😀😀😀");
            REQUIRE(d == U"😀😀😀");
        }

        SECTION("Substring") {
            string dv = "123456";
            string d;
            d.assign(dv, 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
            REQUIRE(dv.size_codepoints() == 6);
            REQUIRE(dv.size() == 6);
            REQUIRE(dv == "123456");
            REQUIRE(dv == U"123456");
            REQUIRE_FALSE(d == dv);
        }

        SECTION("String") {
            string dv = "123456";
            string d;
            d.assign(dv);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
            REQUIRE(dv.size_codepoints() == 6);
            REQUIRE(dv.size() == 6);
            REQUIRE(dv == "123456");
            REQUIRE(dv == U"123456");
            REQUIRE(d == dv);
        }

        SECTION("String Move") {
            string dv = "123456";
            string d;
            d.assign(std::move(dv));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
            REQUIRE(
                dv.size_codepoints() == 0); // NOLINT(bugprone-use-after-move)
            REQUIRE(
                dv.size()
                == 0); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(
                dv
                == ""); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(
                dv
                == U""); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv.front() == '\0');
            REQUIRE_FALSE(d == dv);
        }

        SECTION("Literal") {
            string d;
            d.assign("123456", 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "123");
            REQUIRE(d == U"123");
        }

        SECTION("Wide Literal") {
            string d;
            d.assign(U"123456", 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "123");
            REQUIRE(d == U"123");
        }

        SECTION("Initializer list") {
            string d;
            d.assign({ '1', '2', '3', '4', '5', '6' });
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("String view") {
            string d;
            d.assign(std::string_view("123456"));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("Wide string view") {
            string d;
            d.assign(std::u32string_view(U"123456"));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("Substring view") {
            string d;
            d.assign(std::string_view("123456"), 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
        }

        SECTION("Wide string view") {
            string d;
            d.assign(std::u32string_view(U"123456"), 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
        }
    }
}