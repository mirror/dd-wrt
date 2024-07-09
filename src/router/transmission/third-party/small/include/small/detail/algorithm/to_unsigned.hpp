//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_TO_UNSIGNED_HPP
#define SMALL_DETAIL_ALGORITHM_TO_UNSIGNED_HPP

#include <small/detail/traits/is_range.hpp>
#include <cassert>
#include <limits>

namespace small {
    namespace detail {
        /// \brief Convert a range of bytes to an unsigned number
        /// We could use *(const std::uint8_t *)key_ptr, but this doesn't
        /// generalize well specially because we have lots of cases where the
        /// output span has three bytes, and mixing the two doesn't work well.
        template <
            typename Unsigned,
            typename InputIt,
            std::enable_if_t<
                std::is_unsigned_v<
                    Unsigned> && sizeof(typename std::iterator_traits<InputIt>::value_type) == 1,
                int> = 0>
        constexpr Unsigned
        to_unsigned(InputIt first, InputIt last) {
            Unsigned result = 0;
            while (first != last) {
                result <<= 8;
                result |= static_cast<Unsigned>(*first);
                ++first;
            }
            return result;
        }

        template <
            typename Unsigned,
            typename Range,
            std::enable_if_t<
                std::is_unsigned_v<Unsigned> && is_range_v<Range>,
                int> = 0>
        constexpr Unsigned
        to_unsigned(Range &&r) {
            return to_unsigned<Unsigned>(r.begin(), r.end());
        }

        /// \brief Convert an unsigned number to a span of bytes to an unsigned
        /// number
        template <
            typename Unsigned,
            typename OutputIt,
            std::enable_if_t<
                std::is_unsigned_v<
                    Unsigned> && sizeof(typename std::iterator_traits<OutputIt>::value_type) == 1,
                int> = 0>
        constexpr void
        to_bytes(Unsigned v, OutputIt first, OutputIt last) {
            using byte_type = typename std::iterator_traits<
                OutputIt>::value_type;
            using difference_type = typename std::iterator_traits<
                OutputIt>::difference_type;
            difference_type distance = last - first;
            assert(std::abs(distance) < std::numeric_limits<uint8_t>::max());
            uint8_t bytes_to_fill = static_cast<uint8_t>(std::abs(distance));
            assert(bytes_to_fill <= sizeof(Unsigned));
            uint8_t byte_shift = bytes_to_fill - 1;
            while (first != last) {
                *first = static_cast<byte_type>((v >> (byte_shift * 8)) & 0xFF);
                ++first;
                --byte_shift;
            }
        }

        template <
            typename Unsigned,
            typename Range,
            std::enable_if_t<
                std::is_unsigned_v<Unsigned> && is_range_v<Range>,
                int> = 0>
        constexpr void
        to_bytes(Unsigned v, Range &&r) {
            return to_bytes<Unsigned>(v, r.begin(), r.end());
        }

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ALGORITHM_TO_UNSIGNED_HPP
