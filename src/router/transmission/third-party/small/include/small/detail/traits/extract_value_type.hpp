//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_EXTRACT_VALUE_TYPE_HPP
#define SMALL_DETAIL_TRAITS_EXTRACT_VALUE_TYPE_HPP

namespace small {
    namespace detail {
        /// \struct Get value type from T, if present
        template <class T, class = void>
        struct extract_value_type
        {
            using type = void;
        };

        template <class T>
        struct extract_value_type<T, std::void_t<typename T::value_type>>
        {
            using type = typename T::value_type;
        };

        template <class T>
        using extract_value_type_t = typename extract_value_type<T>::type;

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_EXTRACT_VALUE_TYPE_HPP
