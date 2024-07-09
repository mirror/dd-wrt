//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_HPP
#define SMALL_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_HPP

namespace small {
    /// The default number of inline elements for a type
    namespace detail {
        /// We rarely want less than 5 elements in a small vector
        /// That's why they are vectors
        constexpr size_t expected_min_reasonable_inline_vector = 5;

        /// The number of values we can fit instead of a pointer and the size
        /// value, if the it's size_t We almost never want less inline elements
        /// than that
        template <class T>
        constexpr size_t expected_inline_values_per_heap_pointers
            = (sizeof(T *) + sizeof(size_t)) / sizeof(T);
    } // namespace detail

    template <typename T>
    struct default_inline_storage
        : std::integral_constant<
              size_t,
              std::
                  max(detail::expected_min_reasonable_inline_vector,
                      detail::expected_inline_values_per_heap_pointers<T>)>
    {};

    template <class T>
    constexpr size_t default_inline_storage_v = default_inline_storage<T>::value;

} // namespace small

#endif // SMALL_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_HPP
