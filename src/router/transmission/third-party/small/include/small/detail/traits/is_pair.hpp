//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_IS_PAIR_HPP
#define SMALL_DETAIL_TRAITS_IS_PAIR_HPP

#include <type_traits>

namespace small {
    namespace detail {
        /// Check if type is a pair
        template <typename>
        struct is_pair : std::false_type
        {};

        template <typename T, typename U>
        struct is_pair<std::pair<T, U>> : std::true_type
        {};

        template <class T>
        constexpr bool is_pair_v = is_pair<T>::value;
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_IS_PAIR_HPP
