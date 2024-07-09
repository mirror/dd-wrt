//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/vector.hpp>

int
main() {
    small::vector<int> v(5);
    return (!v.empty()) ? 0 : 1;
}