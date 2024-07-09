//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_STACK_HPP
#define SMALL_STACK_HPP

#include <small/vector.hpp>
#include <stack>

namespace small {
    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Container = small::vector<T, N>>
    using stack = std::stack<T, Container>;
} // namespace small

#endif // SMALL_STACK_HPP
