//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/map.hpp>
#include <small/queue.hpp>
#include <small/set.hpp>
#include <small/stack.hpp>
#include <small/string.hpp>
#include <small/vector.hpp>
#include <iostream>

template <class R>
void
print(R&& v);

void
print_codepoints(const small::string& v);

int
main() {
    // Vector as usual
    small::vector<int> v1 = { 1, 2, 3, 4, 5 };
    print(v1); // 1 2 3 4 5

    // Vector with inline storage for at least 10 elements
    small::vector<int, 10> v2 = { 1, 2, 3, 4 };
    v2.push_back(5);
    print(v2); // 1 2 3 4 5

    // Vector with inline storage only
    small::max_size_vector<int, 5> v3 = { 1, 2, 3, 4 };
    v3.push_back(5);
    print(v3); // 1 2 3 4 5

    // String
    small::string s1 = "Hello world!";
    print(s1); // H e l l o   w o r l d !

    // String with custom inline storage
    small::basic_string<char, 40> s2 = "Hello world!";
    print(s2); // H e l l o   w o r l d !

    // UTF8 String from larger UTF-32 string
    small::string s3 = U"This works too! 😀  é!";
    std::cout << s3 << '\n'; // T h i s   w o r k s   t o o !   😀
    print_codepoints(s3);    // T|h|i|s| |w|o|r|k|s| |t|o|o|!| |😀| | |é|!|

    // Associative containers
    small::set<int> a1 = { 2, 1, 5, 4, 3 };
    print(a1); // 1 2 3 4 5

    small::map<int, int> a2 = {
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40},
        {5, 50}
    };
    print(a2); // <1,10> <2,20> <3,30> <4,40> <5,50>

    small::multimap<int, int> a3 = {
        {1, 10},
        {1, 20},
        {1, 30},
        {1, 40},
        {1, 50}
    };
    print(a3); // <1,10> <1,20> <1,30> <1,40> <1,50>

    small::unordered_set<int> a4 = { 2, 1, 5, 4, 3 };
    print(a4); // 2 1 5 4 3

    // Container adaptors
    small::stack<int> c1;
    c1.push(1);
    c1.push(2);
    c1.push(3);
    c1.push(4);
    c1.push(5);
    std::cout << c1.top() << '\n'; // 5

    small::queue<int> c2;
    c2.push(1);
    c2.push(2);
    c2.push(3);
    c2.push(4);
    c2.push(5);
    std::cout << c2.front() << '\n'; // 1
    std::cout << c2.back() << '\n';  // 5

    small::priority_queue<int> c3;
    c3.push(1);
    c3.push(2);
    c3.push(3);
    c3.push(4);
    c3.push(5);
    std::cout << c3.top() << '\n'; // 5

    return 0;
}

template <class R>
void
print(R&& v) {
    for (const auto& x: v) {
        constexpr bool x_is_pair = small::detail::is_pair_v<
            std::decay_t<decltype(x)>>;
        if constexpr (not x_is_pair) {
            std::cout << x << ' ';
        } else {
            std::cout << '<' << x.first << ',' << x.second << '>' << ' ';
        }
    }
    std::cout << "\n";
}

void
print_codepoints(const small::string& v) {
    for (auto it = v.begin_codepoint(); it != v.end_codepoint(); ++it) {
        std::cout << *it << '|';
    }
    std::cout << "\n";
}