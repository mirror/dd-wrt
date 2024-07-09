//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_MAP_HPP
#define SMALL_MAP_HPP

#include <small/vector.hpp>
#include <small/detail/container/associative_vector.hpp>

/// \file A map container for small maps

namespace small {
    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>,
        class Allocator = std::allocator<std::pair<K, T>>>
    using map = detail::associative_vector<
        true,
        false,
        true,
        vector<std::pair<K, T>, N, Allocator>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>>
    using max_size_map = small::detail::associative_vector<
        true,
        false,
        true,
        max_size_vector<std::pair<K, T>, N>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>,
        class Allocator = std::allocator<std::pair<K, T>>>
    using multimap = detail::associative_vector<
        true,
        true,
        true,
        vector<std::pair<K, T>, N, Allocator>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>>
    using max_size_multimap = small::detail::associative_vector<
        true,
        true,
        true,
        max_size_vector<std::pair<K, T>, N>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>,
        class Allocator = std::allocator<std::pair<K, T>>>
    using unordered_map = detail::associative_vector<
        true,
        false,
        false,
        vector<std::pair<K, T>, N, Allocator>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>>
    using max_size_unordered_map = small::detail::associative_vector<
        true,
        false,
        false,
        max_size_vector<std::pair<K, T>, N>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>,
        class Allocator = std::allocator<std::pair<K, T>>>
    using unordered_multimap = detail::associative_vector<
        true,
        true,
        false,
        vector<std::pair<K, T>, N, Allocator>,
        Compare>;

    template <
        class K,
        class T,
        size_t N = default_inline_storage_v<std::pair<K, T>>,
        class Compare = std::less<>>
    using max_size_unordered_multimap = small::detail::associative_vector<
        true,
        true,
        false,
        max_size_vector<std::pair<K, T>, N>,
        Compare>;

} // namespace small

#endif // SMALL_MAP_HPP
