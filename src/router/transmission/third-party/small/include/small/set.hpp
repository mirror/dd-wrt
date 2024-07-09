//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_SET_HPP
#define SMALL_SET_HPP

#include <small/vector.hpp>
#include <small/detail/container/associative_vector.hpp>

/// \file A set container for small sets

namespace small {
    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>,
        class Allocator = std::allocator<T>>
    using set = detail::
        associative_vector<false, false, true, vector<T, N, Allocator>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>>
    using max_size_set = small::detail::
        associative_vector<false, false, true, max_size_vector<T, N>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>,
        class Allocator = std::allocator<T>>
    using multiset = detail::
        associative_vector<false, true, true, vector<T, N, Allocator>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>>
    using max_size_multiset = small::detail::
        associative_vector<false, true, true, max_size_vector<T, N>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>,
        class Allocator = std::allocator<T>>
    using unordered_set = detail::
        associative_vector<false, false, false, vector<T, N, Allocator>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>>
    using max_size_unordered_set = small::detail::
        associative_vector<false, false, false, max_size_vector<T, N>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>,
        class Allocator = std::allocator<T>>
    using unordered_multiset = detail::
        associative_vector<false, true, false, vector<T, N, Allocator>, Compare>;

    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Compare = std::less<>>
    using max_size_unordered_multiset = small::detail::
        associative_vector<false, true, false, max_size_vector<T, N>, Compare>;

} // namespace small

#endif // SMALL_SET_HPP
