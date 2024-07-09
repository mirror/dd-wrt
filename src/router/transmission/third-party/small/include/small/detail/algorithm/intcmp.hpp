//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_INTCMP_HPP
#define SMALL_DETAIL_ALGORITHM_INTCMP_HPP

#include <small/detail/traits/cpp_version.hpp>
#include <cstddef>
#include <type_traits>

/// \headerfile Compare numbers of different types

namespace small {
    namespace detail {

        template <class T, class U>
        constexpr bool
        cmp_equal(T t, U u) noexcept {
            using UT = std::make_unsigned_t<T>;
            using UU = std::make_unsigned_t<U>;
            if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
                return t == u;
            else if constexpr (std::is_signed_v<T>)
                return t < 0 ? false : UT(t) == u;
            else
                return u < 0 ? false : t == UU(u);
        }

        template <class T, class U>
        constexpr bool
        cmp_not_equal(T t, U u) noexcept {
            return !cmp_equal(t, u);
        }

        template <class T, class U>
        constexpr bool
        cmp_less(T t, U u) noexcept {
            using UT = std::make_unsigned_t<T>;
            using UU = std::make_unsigned_t<U>;
            if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
                return t < u;
            else if constexpr (std::is_signed_v<T>)
                return t < 0 ? true : UT(t) < u;
            else
                return u < 0 ? false : t < UU(u);
        }

        template <class T, class U>
        constexpr bool
        cmp_greater(T t, U u) noexcept {
            return cmp_less(u, t);
        }

        template <class T, class U>
        constexpr bool
        cmp_less_equal(T t, U u) noexcept {
            return !cmp_greater(t, u);
        }

        template <class T, class U>
        constexpr bool
        cmp_greater_equal(T t, U u) noexcept {
            return !cmp_less(t, u);
        }

        namespace detail {
            /// \section Traits to promote integer types

            /// \struct Get the integer type for the given parameters
            template <size_t N, bool IsSigned>
            struct custom_int;

            template <>
            struct custom_int<8, true>
            {
                typedef int8_t type;
            };

            template <>
            struct custom_int<16, true>
            {
                typedef int16_t type;
            };

            template <>
            struct custom_int<32, true>
            {
                typedef int32_t type;
            };

            template <>
            struct custom_int<64, true>
            {
                typedef int64_t type;
            };

            template <>
            struct custom_int<8, false>
            {
                typedef uint8_t type;
            };

            template <>
            struct custom_int<16, false>
            {
                typedef uint16_t type;
            };

            template <>
            struct custom_int<32, false>
            {
                typedef uint32_t type;
            };

            template <>
            struct custom_int<64, false>
            {
                typedef uint64_t type;
            };

            template <size_t N, bool IsSigned>
            using custom_int_t = typename custom_int<N, IsSigned>::type;

            template <class T, class U>
            struct promoted_size
            {
                static constexpr std::size_t first_size = sizeof(T);
                static constexpr std::size_t second_size = sizeof(U);
                static constexpr bool first_is_signed = std::is_signed_v<T>;
                static constexpr bool second_is_signed = std::is_signed_v<U>;
                static constexpr std::size_t largest_size = std::
                    max(first_size, second_size);
                static constexpr std::size_t smallest_size = std::
                    min(first_size, second_size);
                static constexpr bool largest_is_signed
                    = first_size < second_size ?
                          second_is_signed :
                          first_is_signed;
                static constexpr bool smallest_is_signed
                    = first_size < second_size ?
                          first_is_signed :
                          second_is_signed;
                static constexpr bool largest_needs_double
                    = largest_size == smallest_size && largest_size != 64 / 8
                      && not largest_is_signed && smallest_is_signed;
                static constexpr size_t value = largest_size
                                                * (largest_needs_double ? 2 : 1);
            };

            template <class T, class U>
            constexpr std::size_t promoted_size_v = promoted_size<T, U>::value;

            /// \struct Promote an integer to the proper type capable of
            /// representing both integers
            template <class T, class U>
            using promoted = custom_int<
                promoted_size_v<T, U> * 8,
                std::is_signed_v<T> || std::is_signed_v<U>>;

            template <class T, class U>
            using promoted_t = typename promoted<T, U>::type;
        } // namespace detail

        /// \brief Minimum number of two types. This returns a value rather than
        template <class T, class U>
        constexpr detail::promoted_t<T, U>
        min_value(T t, U u) noexcept {
            if (cmp_less(t, u)) {
                return static_cast<detail::promoted_t<T, U>>(t);
            } else {
                return static_cast<detail::promoted_t<T, U>>(u);
            }
        }

        template <class T, class U>
        constexpr detail::promoted_t<T, U>
        max_value(T t, U u) noexcept {
            if (cmp_less(t, u)) {
                return static_cast<detail::promoted_t<T, U>>(u);
            } else {
                return static_cast<detail::promoted_t<T, U>>(t);
            }
        }

        inline long long int
        div_ceil(long long int numerator, long long int denominator) {
            lldiv_t res = std::div(numerator, denominator);
            return res.rem ? (res.quot + 1) : res.quot;
        }

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ALGORITHM_INTCMP_HPP
