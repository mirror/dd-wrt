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

TEST_CASE("String Non-Member") {
    using namespace small;

    SECTION("Non-member") {
        SECTION("Concatenate strings") {
            SECTION("const string & + const string &") {
                string lhs = "abc";
                string rhs = "def";
                string c = lhs + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("const string & + const char *") {
                string lhs = "abc";
                string c = lhs + "def";
                REQUIRE(c == "abcdef");
            }
            SECTION("const string & + char ") {
                string lhs = "abc";
                char rhs = 'd';
                string c = lhs + rhs;
                REQUIRE(c == "abcd");
            }
            SECTION("const char * + const string &") {
                string rhs = "def";
                string c = "abc" + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("char  + const string &") {
                char lhs = 'a';
                string rhs = "def";
                string c = lhs + rhs;
                REQUIRE(c == "adef");
            }
            SECTION("string && + string &&") {
                string lhs = "abc";
                string rhs = "def";
                string c = std::move(lhs) + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + const string &") {
                string lhs = "abc";
                string rhs = "def";
                string c = std::move(lhs) + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + const char *") {
                string lhs = "abc";
                string c = std::move(lhs) + "def";
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + char ") {
                string lhs = "abc";
                char rhs = 'd';
                string c = std::move(lhs) + rhs;
                REQUIRE(c == "abcd");
            }
            SECTION("const string & + string &&") {
                string lhs = "abc";
                string rhs = "def";
                string c = lhs + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("const char * + string &&") {
                string rhs = "def";
                string c = "abc" + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("char  + string &&") {
                char lhs = 'a';
                string rhs = "def";
                string c = lhs + std::move(rhs);
                REQUIRE(c == "adef");
            }
        }

        SECTION("Erase") {
            string cnt(10, ' ');
            std::iota(cnt.begin(), cnt.end(), '0');

            SECTION("Values") {
                erase(cnt, '3');
                REQUIRE(cnt == "012456789");
            }

            SECTION("Condition") {
                size_t n_erased = erase_if(cnt, [](char x) {
                    return (x - '0') % 2 == 0;
                });
                REQUIRE(cnt == "13579");
                REQUIRE(n_erased == 5);
            }
        }

        SECTION("Streams") {
            small::string a = "123456";

            SECTION("Output") {
                std::stringstream ss;
                ss << a;
                REQUIRE(ss.str() == "123456");
            }

            SECTION("Input") {
                std::stringstream ss;
                ss << "123";
                REQUIRE(ss.str() == "123");
                ss >> a;
                REQUIRE(a == "123");
            }

            SECTION("Getline") {
                std::stringstream ss;
                ss << "123 456\n789\n";
                getline(ss, a);
                REQUIRE(a == "123 456");
            }
        }

        SECTION("String to number") {
            SECTION("Integer") {
                small::string i = "123";
                std::unique_ptr<size_t> size = std::make_unique<size_t>(0);
                SECTION("stoi") {
                    int n = stoi(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stol") {
                    long n = stol(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoll") {
                    long long n = stoll(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoul") {
                    unsigned long n = stoul(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoull") {
                    unsigned long long n = stoull(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
            }

            SECTION("Floating") {
                small::string d = "123.456";
                std::unique_ptr<size_t> size = std::make_unique<size_t>(0);
                SECTION("stof") {
                    float n = stof(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
                SECTION("stod") {
                    double n = stod(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
                SECTION("stold") {
                    long double n = stold(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
            }
        }

        SECTION("Number to string") {
            REQUIRE(small::to_string(static_cast<int>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<unsigned>(123)) == "123");
            REQUIRE(small::to_string(static_cast<unsigned long>(123)) == "123");
            REQUIRE(
                small::to_string(static_cast<unsigned long long>(123))
                == "123");
            REQUIRE(small::to_string(static_cast<float>(123)) == "123");
            REQUIRE(small::to_string(static_cast<double>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long double>(123)) == "123");
        }

        SECTION("Hash") {
            SECTION("Isolated") {
                std::hash<small::string> hasher;
                string a = "abc";
                REQUIRE_FALSE(hasher(a) == 0);
            }

            SECTION("Hash table") {
                std::unordered_set<small::string> s;
                s.insert("abc");
                s.insert("def");
                s.insert("ghi");
                REQUIRE(s.size() == 3);
            }
        }

        SECTION("Relocatable in inline vector") {
            small::vector<small::string> v(5);
            v.emplace_back("new str");
            v.emplace(v.begin() + 3, "middle str");
            REQUIRE(v.size() == 7);
        }
    }
}