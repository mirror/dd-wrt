//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/map.hpp>
#include <small/set.hpp>
#include <iostream>

// A relocatable custom type whose default inline storage should be at least 10
// elements
struct my_type
{
    int a;
    double b;
    friend bool
    operator<(const my_type &lhs, const my_type &rhs) {
        return lhs.a < rhs.a;
    }
};

namespace small {
    template <>
    struct is_relocatable<my_type> : std::true_type
    {};
    template <>
    struct default_inline_storage<my_type> : std::integral_constant<size_t, 10>
    {};
} // namespace small

int
main() {
    // Inline storage for at least 5 elements
    small::max_size_map<int, my_type, 5> v1 = {
        {1, { 1, 1.1 }},
        {2, { 2, 2.2 }},
        {3, { 3, 3.3 }},
        {4, { 4, 4.4 }},
        {5, { 5, 5.5 }}
    };
    for (const auto &x: v1) {
        std::cout << '<' << x.first << ',' << '<' << x.second.a << ','
                  << x.second.b << '>' << '>' << ' ';
    }
    std::cout << "\n";

    // Default inline storage for at least 10 elements
    small::unordered_multiset<my_type> v2 = {
        {1, 1.1},
        {2, 2.2},
        {3, 3.3},
        {4, 4.4},
        {5, 5.5}
    };
    for (const auto &x: v2) {
        std::cout << '<' << x.a << ',' << x.b << '>' << ' ';
    }
    std::cout << "\n";

    return 0;
}