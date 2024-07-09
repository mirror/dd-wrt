//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/string.hpp>
#include <iostream>

int
main() {
    // Input is UTF32, but string stores it as UTF8
    small::string str = U"Hello 🌎!";

    // Accessing bytes
    std::cout << static_cast<char>(str[4]) << '\n'; // o
    std::cout << static_cast<char>(str[6]) << '\n'; // �

    // Accessing codepoints
    using cp_idx = small::string::codepoint_index;
    std::cout << str[cp_idx(4)] << '\n'; // o
    std::cout << str[cp_idx(6)] << '\n'; // 🌎

    // Malformed unicode strings
    assert(not small::is_malformed(str));
    str[7] = 'a';
    assert(small::is_malformed(str));

    return 0;
}