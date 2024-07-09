//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_LITTLE_ENDIAN_HPP
#define SMALL_DETAIL_TRAITS_LITTLE_ENDIAN_HPP

namespace small {
    namespace detail {
        /// \brief Helper to detect little endian
        class is_little_endian
        {
            constexpr static std::uint32_t u4 = 1;
            constexpr static std::uint8_t u1 = (const std::uint8_t &) u4;

        public:
            constexpr static bool value = u1;
        };

        /// \brief Helper to modify the last (address-wise) byte of a little
        /// endian value of type 'T'
        template <typename T, std::size_t = sizeof(T)>
        union last_byte
        {
            T number;
            struct
            {
                char dummy[sizeof(T) - 1];
                char last;
            } bytes;
        };
        template <typename T>
        union last_byte<T, 1>
        {
            T number;
            struct
            {
                char last;
            } bytes;
        };
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_TRAITS_LITTLE_ENDIAN_HPP
