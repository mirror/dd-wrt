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

TEST_CASE("String Const Algorithms") {
    using namespace small;

    SECTION("Substr") {
        SECTION("Code unit") {
            string a = "abcdefghij";

            SECTION("Start") {
                string b = a.substr(0, 3);
                REQUIRE(b == "abc");
            }

            SECTION("Middle") {
                string b = a.substr(3, 3);
                REQUIRE(b == "def");
            }

            SECTION("End") {
                string b = a.substr(6, 4);
                REQUIRE(b == "ghij");
            }
        }

        SECTION("Code point") {
            string a = "😐🙂😀🙂😀😐";
            SECTION("Start") {
                string b = a.substr(
                    string::codepoint_index(0),
                    string::codepoint_index(2));
                REQUIRE(b == "😐🙂");
            }

            SECTION("Middle") {
                string b = a.substr(
                    string::codepoint_index(2),
                    string::codepoint_index(2));
                REQUIRE(b == "😀🙂");
            }

            SECTION("End") {
                string b = a.substr(
                    string::codepoint_index(4),
                    string::codepoint_index(2));
                REQUIRE(b == "😀😐");
            }
        }
    }

    SECTION("Copy") {
        SECTION("To UTF8") {
            SECTION("Copy count") {
                string a("abcdefghij");
                char b[7]{};
                a.copy(b, sizeof b);
                REQUIRE(std::string_view(b, 7) == "abcdefg");
            }

            SECTION("From pos") {
                string a("abcdefghij");
                char b[7]{};
                a.copy(b, sizeof b, 3);
                REQUIRE(std::string_view(b, 7) == "defghij");
            }
        }

        SECTION("To UTF32") {
            SECTION("Copy count") {
                string a("😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
                detail::utf32_char_type b[7]{};
                a.copy(b, string::codepoint_index(7));
                REQUIRE(
                    std::u32string_view(b, 7)
                    == U"😐🙂😀🙂😀😐😐");
            }

            SECTION("From pos") {
                string a("😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
                detail::utf32_char_type b[7]{};
                a.copy(
                    b,
                    string::codepoint_index(7),
                    string::codepoint_index(3));
                REQUIRE(
                    std::u32string_view(b, 7)
                    == U"🙂😀😐😐🙂😀🙂");
            }
        }
    }

    SECTION("Swap") {
        string a = "abc";
        string b = "def";
        SECTION("Member") {
            a.swap(b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }

        SECTION("Non-member") {
            swap(a, b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }

        SECTION("Std swap") {
            std::swap(a, b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }
    }

    SECTION("Search") {
        SECTION("Codeunit / Codepoint convert") {
            string a = "😐a😀b😀c😐d😀e😀f";
            auto cu_it = a.find_codeunit(string::codepoint_index(5));
            REQUIRE(*cu_it == 'c');
            auto cp_it = a.find_codepoint(10);
            REQUIRE(*cp_it == U'😀');
        }

        SECTION("Find first substring") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "def";
                    REQUIRE(a.find(b) == 3);
                    REQUIRE(a.find(b, 3) == 3);
                    REQUIRE(a.find(b, 4) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find("def") == 3);
                    REQUIRE(a.find("def", 3) == 3);
                    REQUIRE(a.find("def", 4) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find("defzxc", 0, 3) == 3);
                    REQUIRE(a.find("defzxc", 3, 3) == 3);
                    REQUIRE(a.find("defzxc", 4, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find('d', 0) == 3);
                    REQUIRE(a.find('d', 3) == 3);
                    REQUIRE(a.find('d', 4) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find(std::string_view("def"), 0) == 3);
                    REQUIRE(a.find(std::string_view("def"), 3) == 3);
                    REQUIRE(a.find(std::string_view("def"), 4) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    string b = "😀c😐";
                    REQUIRE(a.find(b) == 10);
                    REQUIRE(a.find(b, 10) == 10);
                    REQUIRE(a.find(b, 14) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find(U"😀c😐") == 10);
                    REQUIRE(a.find(U"😀c😐", 10) == 10);
                    REQUIRE(a.find(U"😀c😐", 14) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find(U"😀c😐zxc", 0, 3) == 10);
                    REQUIRE(a.find(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find(U"😀c😐zxc", 14, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find(U'😀', 0) == 5);
                    REQUIRE(a.find(U'😀', 10) == 10);
                    REQUIRE(
                        a.find(U'😐', 19)
                        == string::npos); // <- idx 19 / value d / cp_idx 7
                }

                SECTION("String view") {
                    REQUIRE(a.find(std::u32string_view(U"😀c😐"), 0) == 10);
                    REQUIRE(a.find(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(
                        a.find(std::u32string_view(U"😀c😐"), 14)
                        == string::npos);
                }
            }
        }

        SECTION("Find last substring") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "def";
                    REQUIRE(a.rfind(b) == 3);
                    REQUIRE(a.rfind(b, 3) == 3);
                    REQUIRE(a.rfind(b, 4) == 3);
                    REQUIRE(a.rfind(b, 2) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.rfind("def") == 3);
                    REQUIRE(a.rfind("def", 3) == 3);
                    REQUIRE(a.rfind("def", 4) == 3);
                    REQUIRE(a.rfind("def", 2) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.rfind("defzxc", 0, 3) == string::npos);
                    REQUIRE(a.rfind("defzxc", 3, 3) == 3);
                    REQUIRE(a.rfind("defzxc", 4, 3) == 3);
                    REQUIRE(a.rfind("defzxc", 2, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.rfind('d', 0) == string::npos);
                    REQUIRE(a.rfind('d', 3) == 3);
                    REQUIRE(a.rfind('d', 4) == 3);
                    REQUIRE(a.rfind('d', 2) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.rfind(std::string_view("def"), 0) == string::npos);
                    REQUIRE(a.rfind(std::string_view("def"), 3) == 3);
                    REQUIRE(a.rfind(std::string_view("def"), 4) == 3);
                    REQUIRE(
                        a.rfind(std::string_view("def"), 2) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    string b = "😀c😐";
                    REQUIRE(a.rfind(b) == 10);
                    REQUIRE(a.rfind(b, 10) == 10);
                    REQUIRE(a.rfind(b, 9) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.rfind(U"😀c😐") == 10);
                    REQUIRE(a.rfind(U"😀c😐", 10) == 10);
                    REQUIRE(a.rfind(U"😀c😐", 9) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.rfind(U"😀c😐zxc", 14, 3) == 10);
                    REQUIRE(a.rfind(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.rfind(U"😀c😐zxc", 9, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.rfind(U'😀', 19) == 10);
                    REQUIRE(a.rfind(U'😀', 10) == 10);
                    REQUIRE(a.rfind(U'😀', 9) == 5);
                    REQUIRE(a.rfind(U'😀', 0) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.rfind(std::u32string_view(U"😀c😐"), 14) == 10);
                    REQUIRE(a.rfind(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(
                        a.rfind(std::u32string_view(U"😀c😐"), 0)
                        == string::npos);
                }
            }
        }

        SECTION("Find first of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_first_of(b) == 3);
                    REQUIRE(a.find_first_of(b, 3) == 3);
                    REQUIRE(a.find_first_of(b, 6) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_of("fed") == 3);
                    REQUIRE(a.find_first_of("fed", 3) == 3);
                    REQUIRE(a.find_first_of("fed", 6) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_of("fedzxc", 0, 3) == 3);
                    REQUIRE(a.find_first_of("fedzxc", 3, 3) == 3);
                    REQUIRE(a.find_first_of("fedzxc", 6, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_of('e', 0) == 4);
                    REQUIRE(a.find_first_of('e', 3) == 4);
                    REQUIRE(a.find_first_of('e', 6) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_first_of(std::string_view("fed"), 0) == 3);
                    REQUIRE(a.find_first_of(std::string_view("fed"), 3) == 3);
                    REQUIRE(
                        a.find_first_of(std::string_view("fed"), 6)
                        == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_first_of(b) == 0);
                    REQUIRE(a.find_first_of(b, 10) == 10);
                    REQUIRE(a.find_first_of(b, 26) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_of(U"😀c😐") == 0);
                    REQUIRE(a.find_first_of(U"😀c😐", 10) == 10);
                    REQUIRE(a.find_first_of(U"😀c😐", 26) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 0, 3) == 0);
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 26, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_of(U'😀', 0) == 5);
                    REQUIRE(a.find_first_of(U'😀', 10) == 10);
                    REQUIRE(a.find_first_of(U'😐', 26) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_first_of(std::u32string_view(U"😀c😐"), 0) == 0);
                    REQUIRE(
                        a.find_first_of(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(
                        a.find_first_of(std::u32string_view(U"😀c😐"), 26)
                        == string::npos);
                }
            }
        }

        SECTION("Find first not of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_first_not_of(b) == 0);
                    REQUIRE(a.find_first_not_of(b, 3) == 6);
                    REQUIRE(a.find_first_not_of(b, 11) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_not_of("fed") == 0);
                    REQUIRE(a.find_first_not_of("fed", 3) == 6);
                    REQUIRE(a.find_first_not_of("fed", 11) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_not_of("fedzxc", 0, 3) == 0);
                    REQUIRE(a.find_first_not_of("fedzxc", 3, 3) == 6);
                    REQUIRE(
                        a.find_first_not_of("fedzxc", 11, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_not_of('e', 0) == 0);
                    REQUIRE(a.find_first_not_of('e', 3) == 3);
                    REQUIRE(a.find_first_not_of('e', 4) == 5);
                    REQUIRE(a.find_first_not_of('e', 11) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_first_not_of(std::string_view("fed"), 0) == 0);
                    REQUIRE(
                        a.find_first_not_of(std::string_view("fed"), 3) == 6);
                    REQUIRE(
                        a.find_first_not_of(std::string_view("fed"), 11)
                        == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_first_not_of(b) == 4);
                    REQUIRE(a.find_first_not_of(b, 10) == 19);
                    REQUIRE(a.find_first_not_of(b, 31) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_not_of(U"😀c😐") == 4);
                    REQUIRE(a.find_first_not_of(U"😀c😐", 10) == 19);
                    REQUIRE(a.find_first_not_of(U"😀c😐", 31) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_not_of(U"😀c😐zxc", 0, 3) == 4);
                    REQUIRE(a.find_first_not_of(U"😀c😐zxc", 10, 3) == 19);
                    REQUIRE(
                        a.find_first_not_of(U"😀c😐zxc", 31, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_not_of(U'😀', 0) == 0);
                    REQUIRE(a.find_first_not_of(U'😀', 10) == 14);
                    REQUIRE(a.find_first_not_of(U'😐', 31) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_first_not_of(std::u32string_view(U"😀c😐"), 0)
                        == 4);
                    REQUIRE(
                        a.find_first_not_of(std::u32string_view(U"😀c😐"), 10)
                        == 19);
                    REQUIRE(
                        a.find_first_not_of(std::u32string_view(U"😀c😐"), 31)
                        == string::npos);
                }
            }
        }

        SECTION("Find last of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_last_of(b) == 5);
                    REQUIRE(a.find_last_of(b, 3) == 3);
                    REQUIRE(a.find_last_of(b, 2) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_of("fed") == 5);
                    REQUIRE(a.find_last_of("fed", 3) == 3);
                    REQUIRE(a.find_last_of("fed", 2) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_of("fedzxc", 6, 3) == 5);
                    REQUIRE(a.find_last_of("fedzxc", 3, 3) == 3);
                    REQUIRE(a.find_last_of("fedzxc", 2, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_of('e', 6) == 4);
                    REQUIRE(a.find_last_of('e', 4) == 4);
                    REQUIRE(a.find_last_of('e', 3) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_last_of(std::string_view("fed"), 6) == 5);
                    REQUIRE(a.find_last_of(std::string_view("fed"), 3) == 3);
                    REQUIRE(
                        a.find_last_of(std::string_view("fed"), 2)
                        == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_last_of(b) == 25);
                    REQUIRE(a.find_last_of(b, 10) == 10);
                    REQUIRE(a.find_last_of(b, 0) == 0);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_of(U"😀c😐") == 25);
                    REQUIRE(a.find_last_of(U"😀c😐", 10) == 10);
                    REQUIRE(a.find_last_of(U"😀c😐", 0) == 0);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 0, 3) == 0);
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 26, 3) == 25);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_of(U'😀', 0) == string::npos);
                    REQUIRE(a.find_last_of(U'😀', 10) == 10);
                    REQUIRE(a.find_last_of(U'😐', 26) == 15);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_last_of(std::u32string_view(U"😀c😐"), 26) == 25);
                    REQUIRE(
                        a.find_last_of(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(
                        a.find_last_of(std::u32string_view(U"😀c😐"), 0) == 0);
                }
            }
        }

        SECTION("Find last not of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_last_not_of(b) == 9);
                    REQUIRE(a.find_last_not_of(b, 3) == 2);
                    REQUIRE(a.find_last_not_of(b, 11) == 9);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_not_of("fed") == 9);
                    REQUIRE(a.find_last_not_of("fed", 3) == 2);
                    REQUIRE(a.find_last_not_of("fed", 11) == 9);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_not_of("fedzxc", 0, 3) == 0);
                    REQUIRE(a.find_last_not_of("fedzxc", 3, 3) == 2);
                    REQUIRE(a.find_last_not_of("fedzxc", 11, 3) == 9);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_not_of('e', 0) == 0);
                    REQUIRE(a.find_last_not_of('e', 3) == 3);
                    REQUIRE(a.find_last_not_of('e', 4) == 3);
                    REQUIRE(a.find_last_not_of('e', 11) == 9);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_last_not_of(std::string_view("fed"), 0) == 0);
                    REQUIRE(
                        a.find_last_not_of(std::string_view("fed"), 3) == 2);
                    REQUIRE(
                        a.find_last_not_of(std::string_view("fed"), 11) == 9);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_last_not_of(b) == 29);
                    REQUIRE(a.find_last_not_of(b, 10) == 9);
                    REQUIRE(a.find_last_not_of(b, 31) == 29);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_not_of(U"😀c😐") == 29);
                    REQUIRE(a.find_last_not_of(U"😀c😐", 10) == 9);
                    REQUIRE(a.find_last_not_of(U"😀c😐", 31) == 29);
                }

                SECTION("Literal substring") {
                    REQUIRE(
                        a.find_last_not_of(U"😀c😐zxc", 0, 3) == string::npos);
                    REQUIRE(a.find_last_not_of(U"😀c😐zxc", 10, 3) == 9);
                    REQUIRE(a.find_last_not_of(U"😀c😐zxc", 31, 3) == 29);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_not_of(U'😀', 0) == 0);
                    REQUIRE(a.find_last_not_of(U'😀', 10) == 9);
                    REQUIRE(a.find_last_not_of(U'😐', 31) == 29);
                }

                SECTION("String view") {
                    REQUIRE(
                        a.find_last_not_of(std::u32string_view(U"😀c😐"), 0)
                        == string::npos);
                    REQUIRE(
                        a.find_last_not_of(std::u32string_view(U"😀c😐"), 10)
                        == 9);
                    REQUIRE(
                        a.find_last_not_of(std::u32string_view(U"😀c😐"), 31)
                        == 29);
                }
            }
        }
    }
}