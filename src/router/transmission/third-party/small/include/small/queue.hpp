//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_QUEUE_HPP
#define SMALL_QUEUE_HPP

#include <small/vector.hpp>
#include <queue>

namespace small {
    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Container = small::vector<T, N>>
    using queue = std::queue<T, Container>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Container = small::vector<T, N>,
        class Compare = std::less<>>
    using priority_queue = std::priority_queue<T, Container, Compare>;
} // namespace small

#endif // SMALL_QUEUE_HPP
