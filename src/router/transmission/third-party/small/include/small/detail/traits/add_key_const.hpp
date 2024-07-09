//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_ADD_KEY_CONST_HPP
#define SMALL_DETAIL_TRAITS_ADD_KEY_CONST_HPP

#include <utility>

namespace small {
    namespace detail {
        /// Make pair key constant
        template <typename T>
        struct add_key_const : std::add_const<T>
        {};

        template <class T, class U>
        struct add_key_const<std::pair<T, U>>
        {
            typedef std::pair<std::add_const_t<T>, U> type;
        };

        /// Make pair key constant from pointer to pair
        template <class T>
        struct add_key_const<T *>
        {
            typedef typename add_key_const<T>::type *type;
        };

        /// Make pair key constant from reference to pair
        template <class T>
        struct add_key_const<T &>
        {
            typedef typename add_key_const<T>::type &type;
        };

        /// Check if pair has a const key
        template <typename>
        struct is_key_const : std::false_type
        {};

        template <typename T, typename U>
        struct is_key_const<std::pair<T, U>> : std::is_const<T>
        {};

        template <class T>
        constexpr bool is_key_const_v = is_key_const<T>::value;
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_ADD_KEY_CONST_HPP
