//
// Copyright (c) 2024 Yat Ho (lagoho7@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_PTR_TO_CONST_HPP
#define SMALL_DETAIL_TRAITS_PTR_TO_CONST_HPP

#include <type_traits>

namespace small {
    namespace detail {
        /// Convert a pointer to pointer-to-const
        template <typename T, typename = std::enable_if_t<std::is_pointer_v<T>>>
        using ptr_to_const = std::add_pointer<
            std::add_const_t<std::remove_pointer_t<T>>>;

        template <typename T>
        using ptr_to_const_t = typename ptr_to_const<T>::type;
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_PTR_TO_CONST_HPP
