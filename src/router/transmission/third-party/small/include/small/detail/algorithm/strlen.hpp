//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_STRLEN_HPP
#define SMALL_DETAIL_ALGORITHM_STRLEN_HPP

#include <cstring>

namespace small {
    namespace detail {
        /// \brief strlen for different character types
        template <typename T>
        inline std::size_t
        strlen(const T *str) {
            std::size_t len = 0u;
            while (*str++) {
                ++len;
            }
            return len;
        }

        /// \brief Usual strlen function
        template <>
        inline std::size_t
        strlen<char>(const char *str) {
            return std::strlen(str);
        }

        /// \brief strlen for different character types with a size limit
        template <typename T>
        inline std::size_t
        strlen(const T *str, std::size_t limit) {
            std::size_t len = 0u;
            while (*str++ && len < limit) {
                ++len;
            }
            return len;
        }
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ALGORITHM_STRLEN_HPP
