//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_IS_RANGE_HPP
#define SMALL_DETAIL_TRAITS_IS_RANGE_HPP

namespace small {
    namespace detail {
        /// Check if type is a range (has begin() and end() functions)
        template <typename T, typename = void>
        struct is_range : std::false_type
        {};

        template <typename T>
        struct is_range<
            T,
            std::void_t<
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end()),
                typename T::value_type>> : std::true_type
        {};

        /// True if type is a range (has begin() and end() functions)
        template <typename T>
        constexpr bool is_range_v = is_range<std::decay_t<T>>::value;
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_IS_RANGE_HPP
