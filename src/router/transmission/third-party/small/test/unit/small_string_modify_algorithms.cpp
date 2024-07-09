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

TEST_CASE("String Modifying Algorithms") {
    using namespace small;

    SECTION("Append") {
        SECTION("Chars") {
            SECTION("Single position") {
                string a = "abcdefghij";
                a.append(3, 'k');
                REQUIRE(a == "abcdefghijkkk");
            }
            SECTION("Single codepoint") {
                string a = "😐🙂😀🙂😀😐";
                a.append(3, U'😀');
                REQUIRE(a == "😐🙂😀🙂😀😐😀😀😀");
            }
        }
        SECTION("Other string") {
            SECTION("Complete") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b);
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Suffix") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b, 2);
                REQUIRE(a == "abcdefghijmnop");
            }
            SECTION("Substr") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b, 2, 3);
                REQUIRE(a == "abcdefghijmno");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(b);
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Suffix") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(b, string::codepoint_index(2));
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀😐");
            }
            SECTION("Codepoint Substr") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(
                    b,
                    string::codepoint_index(2),
                    string::codepoint_index(3));
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀");
            }
        }
        SECTION("String Literal") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a.append("klmnop");
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Prefix") {
                string a = "abcdefghij";
                a.append("klmnop", 2);
                REQUIRE(a == "abcdefghijkl");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a.append("😐🙂😀🙂😀😐");
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Prefix") {
                string a = "😐🙂😀🙂😀😐";
                a.append(U"😐🙂😀🙂😀😐", 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂");
            }
        }

        SECTION("Iterator ranges") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string b = "klmnop";
                a.append(b.begin(), b.end());
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Prefix") {
                string a = "abcdefghij";
                std::string b = "klmnop";
                a.append(b.begin(), b.begin() + 2);
                REQUIRE(a == "abcdefghijkl");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string b = U"😐🙂😀🙂😀😐";
                a.append(b.begin(), b.end());
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Prefix") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string b = U"😐🙂😀🙂😀😐";
                a.append(b.begin(), b.begin() + 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂");
            }
        }

        SECTION("Initializer list") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a.append({ 'k', 'l', 'm', 'n', 'o', 'p' });
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a.append({ U'😐', U'🙂', U'😀', U'🙂', U'😀', U'😐' });
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Other string view") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b);
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Suffix") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b, 2);
                REQUIRE(a == "abcdefghijmnop");
            }
            SECTION("Substr") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b, 2, 3);
                REQUIRE(a == "abcdefghijmno");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::string_view b = "😐🙂😀🙂😀😐";
                a.append(b);
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Suffix") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string_view b = U"😐🙂😀🙂😀😐";
                a.append(b, 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀😐");
            }
            SECTION("Codepoint Substr") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string_view b = U"😐🙂😀🙂😀😐";
                a.append(b, 2, 3);
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀");
            }
        }
    }

    SECTION("Operator +=") {
        SECTION("Other string") {
            SECTION("Complete") {
                string a = "abcdefghij";
                string b = "klmnop";
                a += b;
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a += b;
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }
        SECTION("Chars") {
            SECTION("Single position") {
                string a = "abcdefghij";
                a += 'k';
                REQUIRE(a == "abcdefghijk");
            }
            SECTION("Single codepoint") {
                string a = "😐🙂😀🙂😀😐";
                a += U'😀';
                REQUIRE(a == "😐🙂😀🙂😀😐😀");
            }
        }
        SECTION("String Literal") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a += "klmnop";
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a += "😐🙂😀🙂😀😐";
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Initializer list") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a += { 'k', 'l', 'm', 'n', 'o', 'p' };
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a += { U'😐', U'🙂', U'😀', U'🙂', U'😀', U'😐' };
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Other string view") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a += b;
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::string_view b = "😐🙂😀🙂😀😐";
                a += b;
                REQUIRE(
                    a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }
    }

    SECTION("Starts with") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") {
                    std::string_view b = "abcde";
                    REQUIRE(a.starts_with(b));
                }

                SECTION("Might find but does not start with") {
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.starts_with(b));
                }

                SECTION("Always start with empty") {
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.starts_with(b));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.starts_with(b));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.starts_with(b));
                }
            }

            if constexpr (not is_windows()) {
                SECTION("UTF32 rhs") {
                    string a = u8"😐🙂😀🙂😀😐";
                    SECTION("Starts not-empty") {
                        std::u32string_view b = U"😐🙂😀";
                        REQUIRE(a.starts_with(b));
                    }

                    SECTION("Might find but does not start with") {
                        std::u32string_view b = U"🙂😀🙂";
                        REQUIRE_FALSE(a.starts_with(b));
                    }

                    SECTION("Always start with empty") {
                        std::u32string_view b
                            = U""; // NOLINT(readability-redundant-string-init)
                        REQUIRE(a.starts_with(b));
                    }

                    SECTION("Cannot start with if empty") {
                        a.clear();
                        std::u32string_view b = U"😐🙂😀";
                        REQUIRE_FALSE(a.starts_with(b));
                    }

                    SECTION("Always start if both empty") {
                        a.clear();
                        std::u32string_view b
                            = U""; // NOLINT(readability-redundant-string-init)
                        REQUIRE(a.starts_with(b));
                    }
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") {
                    REQUIRE(a.starts_with('a'));
                }

                SECTION("Might find but does not start with") {
                    REQUIRE_FALSE(a.starts_with('b'));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with('a'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Starts not-empty") {
                    REQUIRE(a.starts_with(U'😐'));
                }

                SECTION("Might find but does not start with") {
                    REQUIRE_FALSE(a.starts_with(U'🙂'));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with(U'😐'));
                }
            }
        }

        SECTION("String literal") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") {
                    REQUIRE(a.starts_with("abcde"));
                }

                SECTION("Might find but does not start with") {
                    REQUIRE_FALSE(a.starts_with("bcdef"));
                }

                SECTION("Always start with empty") {
                    REQUIRE(a.starts_with(""));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with("bcdef"));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    REQUIRE(a.starts_with(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Starts not-empty") {
                    REQUIRE(a.starts_with(U"😐🙂😀"));
                }

                SECTION("Might find but does not start with") {
                    REQUIRE_FALSE(a.starts_with(U"🙂😀🙂"));
                }

                SECTION("Always start with empty") {
                    REQUIRE(a.starts_with(U""));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with(U"😐🙂😀"));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    REQUIRE(a.starts_with(U""));
                }
            }
        }
    }

    SECTION("Ends with") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") {
                    std::string_view b = "ghij";
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Might find but does not end with") {
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end with empty") {
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    std::string_view b = "ghij";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") {
                    std::u32string_view b = U"🙂😀😐";
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Might find but does not end with") {
                    std::u32string_view b = U"🙂😀🙂";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end with empty") {
                    std::u32string_view b
                        = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    std::u32string_view b = U"🙂😀😐";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    std::u32string_view b
                        = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") {
                    REQUIRE(a.ends_with('j'));
                }

                SECTION("Might find but does not end with") {
                    REQUIRE_FALSE(a.ends_with('b'));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with('j'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") {
                    REQUIRE(a.ends_with(U'😐'));
                }

                SECTION("Might find but does not end with") {
                    REQUIRE_FALSE(a.ends_with(U'🙂'));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with(U'😐'));
                }
            }
        }

        SECTION("String literal") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") {
                    REQUIRE(a.ends_with("ghij"));
                }

                SECTION("Might find but does not end with") {
                    REQUIRE_FALSE(a.ends_with("bcdef"));
                }

                SECTION("Always end with empty") {
                    REQUIRE(a.ends_with(""));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with("bcdef"));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    REQUIRE(a.ends_with(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") {
                    REQUIRE(a.ends_with(U"🙂😀😐"));
                }

                SECTION("Might find but does not end with") {
                    REQUIRE_FALSE(a.ends_with(U"🙂😀🙂"));
                }

                SECTION("Always end with empty") {
                    REQUIRE(a.ends_with(U""));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with(U"🙂😀😐"));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    REQUIRE(a.ends_with(U""));
                }
            }
        }
    }

    SECTION("Contains") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Contains start") {
                    std::string_view b = "abc";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains middle") {
                    std::string_view b = "def";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains end") {
                    std::string_view b = "hij";
                    REQUIRE(a.contains(b));
                }

                SECTION("Does not contain") {
                    std::string_view b = "ijk";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains empty") {
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    std::string_view b = "ghij";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    std::string_view b
                        = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Contains start") {
                    std::u32string_view b = U"😐🙂";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains middle") {
                    std::u32string_view b = U"😀🙂";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains end") {
                    std::u32string_view b = U"😀😐";
                    REQUIRE(a.contains(b));
                }

                SECTION("Does not contain") {
                    std::u32string_view b = U"😐😀";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains empty") {
                    std::u32string_view b
                        = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    std::u32string_view b = U"😐🙂";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    std::u32string_view b
                        = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Start") {
                    REQUIRE(a.contains('a'));
                }

                SECTION("Middle") {
                    REQUIRE(a.contains('f'));
                }

                SECTION("End") {
                    REQUIRE(a.contains('j'));
                }

                SECTION("Cannot contains if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains('j'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Start") {
                    REQUIRE(a.contains(U'😐'));
                }

                SECTION("Middle") {
                    REQUIRE(a.contains(U'🙂'));
                }

                SECTION("End") {
                    REQUIRE(a.contains(U'😐'));
                }

                SECTION("Cannot contains if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains(U'😐'));
                }
            }
        }

        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Contains start") {
                    REQUIRE(a.contains("abc"));
                }

                SECTION("Contains middle") {
                    REQUIRE(a.contains("def"));
                }

                SECTION("Contains end") {
                    REQUIRE(a.contains("hij"));
                }

                SECTION("Does not contain") {
                    REQUIRE_FALSE(a.contains("ijk"));
                }

                SECTION("Always contains empty") {
                    REQUIRE(a.contains(""));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains("ghij"));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    REQUIRE(a.contains(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Contains start") {
                    REQUIRE(a.contains(U"😐🙂"));
                }

                SECTION("Contains middle") {
                    REQUIRE(a.contains(U"😀🙂"));
                }

                SECTION("Contains end") {
                    REQUIRE(a.contains(U"😀😐"));
                }

                SECTION("Does not contain") {
                    REQUIRE_FALSE(a.contains(U"😐😀"));
                }

                SECTION("Always contains empty") {
                    REQUIRE(a.contains(U""));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains(U"😐🙂"));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    REQUIRE(a.contains(U""));
                }
            }
        }
    }

    SECTION("Replace") {
        SECTION("Other string") {
            SECTION("Replace code units") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    string b = "xxx";
                    a.replace(a.begin(), a.begin() + 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    string b = "xxx";
                    a.replace(a.begin() + 3, a.begin() + 6, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    string b = "xxx";
                    a.replace(a.begin() + 7, a.begin() + 10, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code points") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    string b = "xxx";
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    string b = "xxx";
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    string b = "xxx";
                    a.replace(
                        a.begin_codepoint() + 4,
                        a.begin_codepoint() + 6,
                        b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code units with substr") {
                string a = "abcdefghij";

                SECTION("Replace with suffix") {
                    string b = "123";
                    a.replace(3, 3, b, 1);
                    REQUIRE(a == "abc23ghij");
                }

                SECTION("Replace with substr") {
                    string b = "123";
                    a.replace(3, 3, b, 1, 1);
                    REQUIRE(a == "abc2ghij");
                }

                SECTION("Replace with code point suffix") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(3, 3, b, string::codepoint_index(2));
                    REQUIRE(a == "abc😀🙂😀😐ghij");
                }

                SECTION("Replace with code point substr") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(
                        3,
                        3,
                        b,
                        string::codepoint_index(2),
                        string::codepoint_index(2));
                    REQUIRE(a == "abc😀🙂ghij");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace with suffix") {
                    string b = "123";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b,
                        1);
                    REQUIRE(a == "😐🙂23😀😐");
                }

                SECTION("Replace with substr") {
                    string b = "123";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b,
                        1,
                        1);
                    REQUIRE(a == "😐🙂2😀😐");
                }

                SECTION("Replace with code point suffix") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b,
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😀😐😀😐");
                }

                SECTION("Replace with code point substr") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b,
                        string::codepoint_index(2),
                        string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Iterators") {
            SECTION("Code units iterator") {
                string a = "abcdefghij";
                SECTION("Rhs is smaller") {
                    string b = "123";
                    a.replace(
                        a.begin() + 3,
                        a.begin() + 5,
                        b.begin() + 1,
                        b.begin() + 2);
                    REQUIRE(a == "abc2fghij");
                }

                SECTION("Rhs is same size") {
                    string b = "123";
                    a.replace(
                        a.begin() + 3,
                        a.begin() + 5,
                        b.begin() + 1,
                        b.begin() + 3);
                    REQUIRE(a == "abc23fghij");
                }

                SECTION("Rhs is larger") {
                    string b = "123";
                    a.replace(
                        a.begin() + 3,
                        a.begin() + 5,
                        b.begin() + 0,
                        b.begin() + 3);
                    REQUIRE(a == "abc123fghij");
                }
            }

            SECTION("Code point iterator") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Rhs is smaller") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        b.begin_codepoint() + 1,
                        b.begin_codepoint() + 2);
                    REQUIRE(a == "😐🙂🙃😀😐");
                }

                SECTION("Rhs is same size") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        b.begin_codepoint() + 1,
                        b.begin_codepoint() + 3);
                    REQUIRE(a == "😐🙂🙃🙃😀😐");
                }

                SECTION("Rhs is larger") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        b.begin_codepoint() + 1,
                        b.begin_codepoint() + 4);
                    REQUIRE(a == "😐🙂🙃🙃🙃😀😐");
                }
            }
        }

        SECTION("String literal") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(0, 0 + 3, "xxx");
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(3, 3, "xxx");
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(7, 3, "xxx");
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code unit indexes with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(0, 0 + 3, "xxx", 2);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(3, 3, "xxx", 2);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    a.replace(7, 3, "xxx", 2);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(
                        string::codepoint_index(0),
                        string::codepoint_index(3),
                        U"🙃🙃🙃🙃");
                    REQUIRE(a == "🙃🙃🙃🙃🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        U"🙃🙃🙃🙃");
                    REQUIRE(a == "😐🙂🙃🙃🙃🙃😀😐");
                }

                SECTION("Replace end") {
                    a.replace(
                        string::codepoint_index(4),
                        string::codepoint_index(2),
                        U"🙃🙃🙃🙃");
                    REQUIRE(a == "😐🙂😀🙂🙃🙃🙃🙃");
                }
            }

            SECTION("Replace code point indexes with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(
                        string::codepoint_index(0),
                        string::codepoint_index(2),
                        U"🙃🙃🙃🙃",
                        2);
                    REQUIRE(a == "🙃🙃😀🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        U"🙃🙃🙃🙃",
                        2);
                    REQUIRE(a == "😐🙂🙃🙃😀😐");
                }

                SECTION("Replace end") {
                    a.replace(
                        string::codepoint_index(4),
                        string::codepoint_index(2),
                        U"🙃🙃🙃🙃",
                        2);
                    REQUIRE(a == "😐🙂😀🙂🙃🙃");
                }
            }

            SECTION("Replace code units iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, "xxx");
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, "xxx");
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, "xxx");
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code units iterators with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, "xxx", 2);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, "xxx", 2);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, "xxx", 2);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code points") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(
                        a.begin_codepoint(),
                        a.begin_codepoint() + 3,
                        "xxx");
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        "xxx");
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(
                        a.begin_codepoint() + 4,
                        a.begin_codepoint() + 6,
                        "xxx");
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(
                        a.begin_codepoint(),
                        a.begin_codepoint() + 3,
                        "xxx",
                        2);
                    REQUIRE(a == "xx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        "xxx",
                        2);
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(
                        a.begin_codepoint() + 4,
                        a.begin_codepoint() + 6,
                        "xxx",
                        2);
                    REQUIRE(a == "😐🙂😀🙂xx");
                }
            }

            SECTION("Replace code units with substr") {
                string a = "abcdefghij";

                SECTION("Replace with suffix") {
                    a.replace(3, 3, "123", 1);
                    REQUIRE(a == "abc1ghij");
                }

                SECTION("Replace with substr") {
                    a.replace(3, 3, "123", 1, 1);
                    REQUIRE(a == "abc2ghij");
                }

                SECTION("Replace with code point suffix") {
                    a.replace(3, 3, "😐🙂😀🙂😀😐", 8);
                    REQUIRE(a == "abc😐🙂ghij");
                }

                SECTION("Replace with code point substr") {
                    a.replace(3, 3, "😐🙂😀🙂😀😐", 8, 8);
                    REQUIRE(a == "abc😀🙂ghij");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace with suffix") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        "123",
                        1);
                    REQUIRE(a == "😐🙂1😀😐");
                }

                SECTION("Replace with substr") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        "123",
                        1,
                        1);
                    REQUIRE(a == "😐🙂2😀😐");
                }

                SECTION("Replace with code point suffix") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        "😐🙂😀🙂😀😐",
                        8);
                    REQUIRE(a == "😐🙂😐🙂😀😐");
                }

                SECTION("Replace with code point substr") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        "😐🙂😀🙂😀😐",
                        8,
                        8);
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Char") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Smaller") {
                    a.replace(3, 2, 1, 'x');
                    REQUIRE(a == "abcxfghij");
                }

                SECTION("Same size") {
                    a.replace(3, 2, 2, 'x');
                    REQUIRE(a == "abcxxfghij");
                }

                SECTION("Larger") {
                    a.replace(3, 2, 3, 'x');
                    REQUIRE(a == "abcxxxfghij");
                }

                SECTION("Wide char") {
                    a.replace(3, 2, 1, U'😀');
                    REQUIRE(a == "abc😀fghij");
                }

                SECTION("Wide char twice") {
                    a.replace(3, 2, 2, U'😀');
                    REQUIRE(a == "abc😀😀fghij");
                }

                SECTION("Wide char 3x") {
                    a.replace(3, 2, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀fghij");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Smaller") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        1,
                        'x');
                    REQUIRE(a == "😐🙂x😀😐");
                }

                SECTION("Same size") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        2,
                        'x');
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Larger") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        3,
                        'x');
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Wide char") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        1,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😐");
                }

                SECTION("Wide char twice") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        2,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }

                SECTION("Wide char 3x") {
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        3,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😀😐");
                }
            }

            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Smaller") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 1, 'x');
                    REQUIRE(a == "abcxfghij");
                }

                SECTION("Same size") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 2, 'x');
                    REQUIRE(a == "abcxxfghij");
                }

                SECTION("Larger") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 3, 'x');
                    REQUIRE(a == "abcxxxfghij");
                }

                SECTION("Wide char") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 1, U'😀');
                    REQUIRE(a == "abc😀fghij");
                }

                SECTION("Wide char twice") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 2, U'😀');
                    REQUIRE(a == "abc😀😀fghij");
                }

                SECTION("Wide char 3x") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀fghij");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Smaller") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        1,
                        'x');
                    REQUIRE(a == "😐🙂x😀😐");
                }

                SECTION("Same size") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        2,
                        'x');
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Larger") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        3,
                        'x');
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Wide char") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        1,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😐");
                }

                SECTION("Wide char twice") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        2,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }

                SECTION("Wide char 3x") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 2 + 2,
                        3,
                        U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😀😐");
                }
            }
        }

        SECTION("Initializer list") {
            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, { 'x', 'x', 'x' });
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, { 'x', 'x', 'x' });
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, { 'x', 'x', 'x' });
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(
                        a.begin_codepoint(),
                        a.begin_codepoint() + 3,
                        { 'x', 'x', 'x' });
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        { 'x', 'x', 'x' });
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(
                        a.begin_codepoint() + 4,
                        a.begin_codepoint() + 6,
                        { 'x', 'x', 'x' });
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }
        }

        SECTION("String view") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(0, 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(3, 3, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(7, 3, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(0),
                        string::codepoint_index(3),
                        b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(4),
                        string::codepoint_index(2),
                        b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(a.begin(), a.begin() + 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(a.begin() + 3, a.begin() + 6, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(a.begin() + 7, a.begin() + 10, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(
                        a.begin_codepoint() + 2,
                        a.begin_codepoint() + 4,
                        b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(
                        a.begin_codepoint() + 4,
                        a.begin_codepoint() + 6,
                        b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code unit indexes with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(0, 3, b, 1);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(3, 3, b, 1);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(7, 3, b, 1);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code point indexes with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(0),
                        string::codepoint_index(3),
                        b,
                        1);
                    REQUIRE(a == "xx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(2),
                        string::codepoint_index(2),
                        b,
                        1);
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(
                        string::codepoint_index(4),
                        string::codepoint_index(2),
                        b,
                        1);
                    REQUIRE(a == "😐🙂😀🙂xx");
                }
            }
        }
    }
}