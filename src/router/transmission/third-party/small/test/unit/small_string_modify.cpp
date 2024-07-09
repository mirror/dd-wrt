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

TEST_CASE("String Modify") {
    using namespace small;

    SECTION("Resize") {
        SECTION("Code units") {
            string a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 13);
            REQUIRE(a.capacity() <= 15);
            REQUIRE(a.size_codepoints() == 1);
            REQUIRE(is_malformed(a));

            a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(20);
            REQUIRE(a.size() == 20);
            REQUIRE(a.max_size() > 20);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 20);
            REQUIRE(a.size_codepoints() == 17);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(14, 'x');
            REQUIRE(a.size() == 6 + 8);
            REQUIRE(a.size_codepoints() == 3 + 8);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(14, U'x');
            REQUIRE(a.size() == 6 + 8);
            REQUIRE(a.size_codepoints() == 3 + 8);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";          // <- size 6
            a.resize(14, U'😀'); // <- size 6 + 8 = 14 (two extra codepoints)
            REQUIRE(a.size_codepoints() == 5);
            REQUIRE(a.size() == 14);
            REQUIRE(a.max_size() > 40);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.shrink_to_fit();
            REQUIRE(a.size() == 6);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > a.size());
            REQUIRE_FALSE(is_malformed(a));
        }

        SECTION("Code points") {
            string a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(string::codepoint_index(4));
            REQUIRE(a.size() == 7);
            REQUIRE(a.max_size() > 7);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 13);
            REQUIRE(a.capacity() <= 17);
            REQUIRE(a.size_codepoints() == 4);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(string::codepoint_index(20));
            REQUIRE(a.size() == 23);
            REQUIRE(a.max_size() > 23);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 23);
            REQUIRE(a.size_codepoints() == 20);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), 'x');
            REQUIRE(a.size() == 6 + 14 - 3);
            REQUIRE(a.size_codepoints() == 3 + 11);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), U'x');
            REQUIRE(a.size() == 6 + 11);
            REQUIRE(a.size_codepoints() == 3 + 11);
            REQUIRE(a.max_size() > 17);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 17);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), U'😀');
            REQUIRE(a.size_codepoints() == 14);
            REQUIRE(a.size() == 12 * 4 + 2);
            REQUIRE(a.max_size() > 40);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));
        }
    }

    SECTION("Clear") {
        string a = "1😀3";
        a.clear();
        REQUIRE(a.empty());
        REQUIRE(a.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(a.size_codepoints() == 0);
        REQUIRE(a.max_size() > 10);
        REQUIRE(a.capacity() > a.size());
        REQUIRE_FALSE(is_malformed(a));
    }

    SECTION("Insert") {
        SECTION("Char") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, 1, '3');
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, 3, '.');
                    REQUIRE(a == "abc...z");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 1, '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 3, '.');
                    REQUIRE(a == "😐🙂...😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 3, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }
            }

            SECTION("At iterator") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(a.begin() + 2, 1, '3');
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 3, '.');
                    REQUIRE(a == "abc...z");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀z");
                }
            }

            SECTION("At codepoint iterator") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        1,
                        '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        3,
                        '.');
                    REQUIRE(a == "😐🙂...😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        3,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }
            }
        }

        SECTION("Literal string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, "3");
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, "defgh");
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, U"🙂😀");
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "3");
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "defgh");
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), U"🙂😀");
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Partial literal string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, "3456", 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, "defghijklmn", 5);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, U"🙂😀🙂😀🙂😀", 2);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "3456", 1);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "defghijkl", 5);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(
                        string::codepoint_index(2),
                        U"🙂😀🙂😀🙂😀",
                        2);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Other string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3");
                    a.insert(2, other);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defgh");
                    a.insert(3, other);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀");
                    a.insert(3, other);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defgh");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Other string suffix") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, 1);
                    REQUIRE(a == "124564");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, 5);
                    REQUIRE(a == "abcijklmnz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 8);
                    REQUIRE(a == "abc🙂😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, 1);
                    REQUIRE(a == "😐🙂456😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5);
                    REQUIRE(a == "😐🙂ijkl😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 8);
                    REQUIRE(a == "😐🙂🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Other string codepoint suffix") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, string::codepoint_index(1));
                    REQUIRE(a == "124564");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, string::codepoint_index(5));
                    REQUIRE(a == "abcijklmnz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, string::codepoint_index(2));
                    REQUIRE(a == "abc🙂😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(1));
                    REQUIRE(a == "😐🙂456😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(5));
                    REQUIRE(a == "😐🙂ijkl😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Other string substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, 0, 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 4, 12);
                    REQUIRE(a == "abc😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, 1, 2);
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5, 2);
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 12, 8);
                    REQUIRE(a == "😐🙂😀🙂😐");
                }
            }
        }

        SECTION("Other string codepoint substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(
                        2,
                        other,
                        string::codepoint_index(1),
                        string::codepoint_index(2));
                    REQUIRE(a == "12454");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(
                        3,
                        other,
                        string::codepoint_index(1),
                        string::codepoint_index(3));
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(
                        3,
                        other,
                        string::codepoint_index(2),
                        string::codepoint_index(3));
                    REQUIRE(a == "abc🙂😀🙂z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(1),
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(5),
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(
                        string::codepoint_index(2),
                        other,
                        string::codepoint_index(3),
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😐");
                }
            }
        }

        SECTION("Single char") {
            SECTION("At index") {
                SECTION("Unibyte") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 'd');
                    REQUIRE(a == "abcdz");
                }
                SECTION("Multibyte") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, U'🙂');
                    REQUIRE(a == "abc🙂z");
                }
            }

            SECTION("At codepoint") {
                SECTION("Unibyte") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multibyte") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        U'😀');
                    REQUIRE(a == "😐🙂😀😐");
                }
            }
        }

        SECTION("Other container iterator") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string other("3");
                    a.insert(a.begin() + 2, other.begin(), other.end());
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string other("defgh");
                    a.insert(a.begin() + 3, other.begin(), other.end());
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string other(U"🙂😀");
                    a.insert(a.begin() + 3, other.begin(), other.end());
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string other("3");
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        other.begin(),
                        other.end());
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string other("defgh");
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        other.begin(),
                        other.end());
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string other(U"🙂😀");
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        other.begin(),
                        other.end());
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Initializer list") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(a.begin() + 2, { '3' });
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, { 'd', 'e', 'f', 'g', 'h' });
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, { U'🙂', U'😀' });
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        { '3' });
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        { 'd', 'e', 'f', 'g', 'h' });
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string other(U"🙂😀");
                    a.insert(
                        a.begin_codepoint() + string::codepoint_index(2),
                        { U'🙂', U'😀' });
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("String view") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string_view other("3");
                    a.insert(2, other);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string_view other("defgh");
                    a.insert(3, other);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string_view other(U"🙂😀");
                    a.insert(3, other);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string_view other("3");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string_view other("defgh");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string_view other(U"🙂😀");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("String view substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string_view other("3456");
                    a.insert(2, other, 0, 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string_view other("defghijklmn");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string_view other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abc😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string_view other("3456");
                    a.insert(string::codepoint_index(2), other, 1, 2);
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string_view other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5, 2);
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string_view other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 1, 3);
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }
    }

    SECTION("Erase") {
        SECTION("Index suffix") {
            string a = "abcdefghij";
            a.erase(3);
            REQUIRE(a == "abc");
        }
        SECTION("Index substr") {
            string a = "abcdefghij";
            a.erase(3, 2);
            REQUIRE(a == "abcfghij");
        }
        SECTION("Codepoint suffix") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(string::codepoint_index(3));
            REQUIRE(a == "😐🙂😀");
        }
        SECTION("Codepoint substr") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(string::codepoint_index(3), string::codepoint_index(2));
            REQUIRE(a == "😐🙂😀😐");
        }
        SECTION("Single position") {
            string a = "abcdefghij";
            a.erase(a.begin() + 3);
            REQUIRE(a == "abcefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(a.begin_codepoint() + string::codepoint_index(3));
            REQUIRE(a == "😐🙂😀😀😐");
        }
        SECTION("Index range") {
            string a = "abcdefghij";
            a.erase(a.begin() + 3, a.begin() + 5);
            REQUIRE(a == "abcfghij");
        }
        SECTION("Codepoint range") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(
                a.begin_codepoint() + string::codepoint_index(3),
                a.begin_codepoint() + string::codepoint_index(5));
            REQUIRE(a == "😐🙂😀😐");
        }
    }

    SECTION("Push back") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.push_back('k');
            REQUIRE(a == "abcdefghijk");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.push_back(U'😀');
            REQUIRE(a == "😐🙂😀🙂😀😐😀");
        }
    }

    SECTION("Pop back") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.pop_back();
            REQUIRE(a == "abcdefghi");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.pop_back_codepoint();
            REQUIRE(a == "😐🙂😀🙂😀");
        }
    }

    SECTION("Push \"front\"") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.insert(a.begin(), 'k');
            REQUIRE(a == "kabcdefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.insert(a.begin(), U'😀');
            REQUIRE(a == "😀😐🙂😀🙂😀😐");
        }
    }

    SECTION("Pop \"front\"") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.erase(a.begin());
            REQUIRE(a == "bcdefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(a.begin_codepoint());
            REQUIRE(a == "🙂😀🙂😀😐");
        }
    }
}