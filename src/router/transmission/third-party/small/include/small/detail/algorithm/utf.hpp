//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_UTF_HPP
#define SMALL_DETAIL_ALGORITHM_UTF_HPP

#include <small/detail/algorithm/intcmp.hpp>
#include <small/detail/algorithm/leading_zeros.hpp>
#include <small/detail/algorithm/strlen.hpp>
#include <small/detail/traits/cpp_version.hpp>

/// \headerfile Function to convert strings and chars from and to utf8
/// Although our string data structure layout is quite different, many of these
/// functions are adapted from tiny-utf8

namespace small {
    namespace detail {
        /// Set the types for UTF in C++
#ifdef cpp_char8_t
        // there's no operator<<(stream, char8_t) in C++20
        using utf8_char_type = char8_t;
#else
        using utf8_char_type = char;
#endif

#ifdef cpp_unicode_characters
        using utf16_char_type = char16_t;
        using utf32_char_type = char32_t;
#else
        using utf16_char_type = uint16_t;
        using utf32_char_type = uint32_t;
#endif

        /// \struct Identify the utf8 type of a char type
        /// Integral types represent themselves, but wide char might sometimes
        /// be a proxy, like a codepoint_reference which is only implicitly
        /// convertible to a utf32 char. If it's integral, we use its size to
        /// decide. If it's proxy, we attempt to convert from low to high. This
        /// second heuristic might fail in intention.
        namespace detail {
            template <typename From, typename To>
            using is_utf_convertible = std::disjunction<
                std::is_same<From, To>,
                std::conjunction<
                    std::is_integral<From>,
                    std::conditional_t<
                        sizeof(From) == sizeof(To),
                        std::true_type,
                        std::false_type>>,
                std::conjunction<
                    std::negation<std::is_integral<From>>,
                    std::conditional_t<
                        std::is_convertible_v<From, To>,
                        std::true_type,
                        std::false_type>>>;
        } // namespace detail

        template <typename CharT>
        using is_utf8 = detail::is_utf_convertible<CharT, utf8_char_type>;

        template <typename CharT>
        constexpr bool is_utf8_v = is_utf8<CharT>::value;

        template <typename CharT>
        using is_utf16 = detail::is_utf_convertible<CharT, utf16_char_type>;

        template <typename CharT>
        constexpr bool is_utf16_v = is_utf16<CharT>::value;

        template <typename CharT>
        using is_utf32 = detail::is_utf_convertible<CharT, utf32_char_type>;

        template <typename CharT>
        constexpr bool is_utf32_v = is_utf32<CharT>::value;

        /// \brief Trait to identify if two types have the same UTF encoding
        template <typename CharT1, typename CharT2>
        using is_same_utf_encoding = std::disjunction<
            std::conjunction<is_utf8<CharT1>, is_utf8<CharT2>>,
            std::conjunction<is_utf16<CharT1>, is_utf16<CharT2>>,
            std::conjunction<is_utf32<CharT1>, is_utf32<CharT2>>>;

        template <typename CharT1, typename CharT2>
        constexpr bool is_same_utf_encoding_v
            = is_same_utf_encoding<CharT1, CharT2>::value;

        /// \brief Check if a char is a UTF8 continuation char/byte
        /// If a string is well formed, a continuation char/byte is a char/byte
        /// that comes after the first char/byte in a utf8 multibyte code point.
        /// \note This function also accepts other char types containing utf8
        /// information \tparam Char Char type \param ch The char that might be
        /// a continuation \return True if that char is a utf8 continuation char
        template <class Char>
        constexpr bool
        is_utf8_continuation(Char ch) noexcept {
            if (ch) {
                // Char represented as unsigned integer
                const uint32_t char_as_uint = (uint32_t) ch;
                // Number of bits we need to shift left to put the char on the
                // leftmost bytes
                constexpr size_t shift_left_size = (sizeof(uint32_t) - 1) * 8;
                // Shift char
                const uint32_t char_leading = char_as_uint << shift_left_size;
                // Count leading ones
                auto codepoint_bytes = leading_zeros(~char_leading);
                // Continuation bytes have one leading 1
                return cmp_equal(codepoint_bytes, 1);
            }
            return false;
        }

        /// \brief Get number of utf8 bytes in a utf8 codepoint that starts with
        /// this char
        ///
        /// If the utf8 code point is well formed, it will have this number of
        /// chars.
        ///
        /// UTF8 size = 1:
        /// - 0...    last byte
        /// UTF8 size = n:
        /// - 111<n times>0... first byte of n code units
        /// - 10...            continuation byte
        /// - 0...             last byte
        /// \param first_char First char in the codepoint
        /// \param available_code_units How many code units there are for this
        /// utf8 codepoint, including first_char \note available_code_units will
        /// truncate the utf8 size if there isn't enough codepoints for the size
        /// indicated by the first char. Otherwise, only the first char is
        /// considered. \return Number of bytes in this codepoint
        template <class Char, class Size = size_t, class Result = uint8_t>
        constexpr Result
        utf8_size(Char first_char, Size available_code_units = 8) noexcept {
            if (first_char) {
                // Before counting the leading one's we need to shift the byte
                // into the most significant part of the integer
                constexpr size_t n_last_bits = (sizeof(unsigned int) - 1) * 8;
                const unsigned int char_leading = (unsigned int) first_char
                                                  << n_last_bits;
                using unsigned_size = std::make_unsigned_t<Size>;
                unsigned_size codepoint_bytes = leading_zeros(~char_leading);

                // The test below would actually be ( codepoint_bytes <=
                // available_code_units && codepoint_bytes ), but
                // codepoint_bytes is unsigned and thus wraps around zero, which
                // makes the following faster:
                if (unsigned_size(codepoint_bytes - 1)
                    < unsigned_size(available_code_units)) {
                    return (Result) codepoint_bytes;
                }
            }
            return 1;
        }

        /// \brief Check if a char is a UTF16 surrogate char
        /// If a string is well formed, a surrogate char is a char that comes in
        /// a pair of chars representing a code point \note This function also
        /// accepts other char types containing utf16 information \tparam Char
        /// Char type \param ch The char that might be a continuation \return
        /// True if that char is a utf8 continuation char
        template <class Char16 = utf16_char_type>
        constexpr bool
        is_utf16_surrogate(Char16 ch) noexcept {
            return cmp_less(ch - 0xd800u, 2048u);
        }

        /// \brief Check if a char is a UTF16 high surrogate char
        /// If a string is well formed, a surrogate char is a char that comes in
        /// a pair of chars representing a code point \note This function also
        /// accepts other char types containing utf16 information \tparam Char
        /// Char type \param ch The char that might be a continuation \return
        /// True if that char is a utf8 continuation char
        template <class Char16 = utf16_char_type>
        constexpr bool
        is_utf16_high_surrogate(Char16 ch) noexcept {
            return cmp_equal(ch & 0xfffffc00, 0xd800);
        }

        /// \brief Check if a char is a UTF16 low surrogate char
        /// If a string is well formed, a surrogate char is a char that comes in
        /// a pair of chars representing a code point \note This function also
        /// accepts other char types containing utf16 information \tparam Char
        /// Char type \param ch The char that might be a continuation \return
        /// True if that char is a utf8 continuation char
        template <class Char16 = utf16_char_type>
        constexpr bool
        is_utf16_low_surrogate(Char16 ch) noexcept {
            return cmp_equal(ch & 0xfffffc00, 0xdc00);
        }

        /// \brief Convert a pair of surrogate bytes to UTF32
        /// If a string is well formed, a surrogate char is a char that comes in
        /// a pair of chars representing a code point \note This function also
        /// accepts other char types containing utf16 information \tparam Char
        /// Char type \tparam Size Size type \param ch The char that might be a
        /// continuation \return True if that char is a utf8 continuation char
        template <class Char32 = utf32_char_type, class Char16 = utf16_char_type>
        Char32
        utf16_surrogates_to_utf32(Char16 high, Char16 low) {
            assert(is_utf16_high_surrogate(high));
            assert(is_utf16_low_surrogate(low));
            return (high << 10) + low - 0x35fdc00;
        }

        /// \brief Check if a char is a UTF16 is a continuation char
        /// In UTF16, this is equivalent to being a low surrogate
        /// \tparam Char Char type
        /// \param ch The char that might be a continuation
        /// \return True if that char is a utf8 continuation char
        template <class Char16 = utf16_char_type>
        constexpr bool
        is_utf16_continuation(Char16 ch) noexcept {
            return is_utf16_low_surrogate(ch);
        }

        /// \brief Get number of utf16 code units in a utf16 codepoint that
        /// starts with this char
        ///
        /// This is much simpler than UTF8, where this size can vary a lot.
        /// In UTF16, all we have to check is whether this char is a high
        /// surrogate.
        ///
        /// \param first_char First char in the codepoint
        /// \param available_code_units How many code units there are for this
        /// utf8 codepoint, including first_char \return Number of code units in
        /// this codepoint
        template <class Char, class Size = size_t, class Result = uint8_t>
        constexpr Result
        utf16_size(Char first_char, Size available_code_units = 2) noexcept {
            return is_utf16_high_surrogate(first_char)
                           && available_code_units > 1 ?
                       2 :
                       static_cast<Result>(
                           (std::min)(Size(1), available_code_units));
        }

        /// \brief Check if a char is a UTF32 is a continuation char
        /// In UTF32, there are no continuations. This function is only defined
        /// for symmetry. \tparam Char Char type \param ch The char that might
        /// be a continuation \return True if that char is a utf32 continuation
        /// char
        template <class Char32 = utf32_char_type>
        constexpr bool
        is_utf32_continuation(Char32) noexcept {
            return false;
        }

        /// \brief Get number of utf32 code units in a utf32 codepoint that
        /// starts with this char
        ///
        /// In UTF32, all code points fit in a single code unit.
        /// This function is only defined for symmetry.
        ///
        /// \param first_char First char in the codepoint
        /// \param available_code_units How many code units there are for this
        /// utf8 codepoint, including first_char \return Number of code units in
        /// this codepoint
        template <class Char32, class Size = size_t, class Result = uint8_t>
        constexpr Result
        utf32_size(Char32, Size available_code_units = 1) noexcept {
            return static_cast<Result>(
                (std::min)(Size(1), available_code_units));
        }

        /// \brief Get size a utf32 char would have when/if converted to utf8
        /// This function is useful to use before actually converting the
        /// from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf8.
        /// \param first_char Wide char with the codepoint \return Number of
        /// bytes in potential utf8 codepoint
        template <typename Char32, typename Result = uint8_t>
        constexpr Result
        utf32_size_as_utf8(Char32 wide_char) noexcept {
            const utf32_char_type integral_wide_char = wide_char;
            if constexpr (system_has_leading_zeros_v) {
                if (!integral_wide_char) {
                    return 1;
                }
                constexpr uint8_t lut[32] = {
                    1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3,
                    4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7
                };
                return lut[31 - leading_zeros(integral_wide_char)];
            } else {
                if (integral_wide_char <= 0x7F) {
                    return 1;
                } else if (integral_wide_char <= 0x7FF) {
                    return 2;
                } else if (integral_wide_char <= 0xFFFF) {
                    return 3;
                } else if (integral_wide_char <= 0x1FFFFF) {
                    return 4;
                } else if (integral_wide_char <= 0x3FFFFFF) {
                    return 5;
                } else if (integral_wide_char <= 0x7FFFFFFF) {
                    return 6;
                }
                return 7;
            }
        }

        /// \brief Get size a utf32 char would have when/if converted to utf16
        /// This function is useful to use before actually converting the
        /// from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf8.
        /// \param first_char Wide char with the codepoint \return Number of
        /// code units in potential utf16 codepoint
        template <typename Char32, typename Result = uint8_t>
        constexpr Result
        utf32_size_as_utf16(Char32 ch) noexcept {
            return (ch <= 0x0000FFFF) || (ch > 0x0010FFFF) ? 1 : 2;
        }

        /// \brief Convert a sequence of UTF16 code points to a single UTF32 char
        template <
            typename Char32 = utf32_char_type,
            class InputIt,
            typename Size = uint8_t,
            typename std::enable_if_t<sizeof(Char32) != 1, int> = 0>
        constexpr Char32
        from_utf16_to_utf32(InputIt first, Size count) noexcept {
            Char32 cp;
            if (count == 0) {
                return 0;
            }
            if (!is_utf16_surrogate(first[0])) {
                cp = first[0];
            } else {
                if (is_utf16_high_surrogate(first[0]) && count > 1
                    && is_utf16_low_surrogate(first[1]))
                {
                    cp = utf16_surrogates_to_utf32(first[0], first[1]);
                } else {
                    return 0;
                }
            }
            return cp;
        }

        /// \brief Get size a utf16 sequence would have when/if converted to
        /// utf8 This function is useful to use before actually converting the
        /// from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf8.
        /// \param source Iterator to first element in the UTF16 sequence \param
        /// source_count Number of elements in the UTF16 code point \return
        /// Number of bytes in potential utf8 codepoint
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf16_size_as_utf8(InputIt source, Size source_count) noexcept {
            utf32_char_type wider_char
                = from_utf16_to_utf32(source, source_count);
            return utf32_size_as_utf8(wider_char);
        }

        /// \brief Get size a utf8 sequence would have when/if converted to
        /// utf16 This function is useful to use before actually converting the
        /// from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf8.
        /// \param source Iterator to first element in the UTF16 sequence \param
        /// source_count Number of elements in the UTF16 code point \return
        /// Number of code units in potential utf16 codepoint
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf8_size_as_utf16(InputIt source, Size source_count) noexcept {
            utf32_char_type wider_char
                = from_utf8_to_utf32(source, source_count);
            return utf32_size_as_utf16(wider_char);
        }

        /// \brief Get size a utf16 char would have when/if converted to utf32
        /// This is always 1 for utf32, but we keep this function for symmetry
        /// This function is useful to use before actually converting the
        /// from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf32.
        /// \param first_char Wide char with the codepoint
        /// \return Number of code units in potential utf32 codepoint
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf16_size_as_utf32(InputIt, Size count) noexcept {
            return (std::min)(1, count);
        }

        /// \brief Get size a utf8 sequence would have when/if converted to
        /// utf32 This is always 1 for utf32, but we keep this function for
        /// symmetry This function is useful to use before actually converting
        /// the from/to another utf encoding If the char is a malformed, i.e. a
        /// continuation code unit, we consider the malformed size in utf8.
        /// \param source Iterator to first element in the UTF8 sequence \param
        /// source_count Number of elements in the UTF8 code point \return
        /// Number of code units in potential utf32 codepoint
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf8_size_as_utf32(InputIt, Size count) noexcept {
            return (std::min)(1, count);
        }

        /// \brief Get size a utf sequence would have when/if converted to utf8
        /// The input encoding is inferred from the input type
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf_size_as_utf8(InputIt source, Size count) noexcept {
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            if constexpr (is_utf32_v<input_value_type>) {
                return utf32_size_as_utf8(*source);
            } else if constexpr (is_utf16_v<input_value_type>) {
                return utf16_size_as_utf8(source, count);
            } else {
                return utf8_size(*source, count);
            }
        }

        /// \brief Get size a utf sequence would have when/if converted to utf16
        /// The input encoding is inferred from the input type
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf_size_as_utf16(InputIt source, Size count) noexcept {
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            if constexpr (is_utf32_v<input_value_type>) {
                return utf32_size_as_utf16(*source);
            } else if constexpr (is_utf16_v<input_value_type>) {
                return utf16_size(*source, count);
            } else {
                return utf8_size_as_utf16(source, count);
            }
        }

        /// \brief Get size a utf sequence would have when/if converted to utf32
        /// The input encoding is inferred from the input type
        template <typename InputIt, typename Size, typename Result = uint8_t>
        constexpr Result
        utf_size_as_utf32(InputIt, Size count) noexcept {
            return (std::min)(1, count);
        }

        /// \brief Get size a utf sequence would have when/if converted to
        /// utf8/utf16/utf32 The input encoding is inferred from the input type
        /// The output encoding is inferred from the output type
        template <
            typename OutputCharType,
            typename InputIt,
            typename Size,
            typename Result = uint8_t>
        constexpr Result
        utf_size_as(InputIt source, Size count) noexcept {
            if constexpr (is_utf32_v<OutputCharType>) {
                return utf_size_as_utf32(source, count);
            } else if constexpr (is_utf16_v<OutputCharType>) {
                return utf_size_as_utf16(source, count);
            } else {
                return utf_size_as_utf8(source, count);
            }
        }

        /// \brief Check if a char is a UTF continuation char
        /// The encoding is inferred from the input type.
        /// \tparam Char Char type
        /// \param ch The char that might be a continuation
        /// \return True if that char is a utf32 continuation char
        template <class Char>
        constexpr bool
        is_utf_continuation(Char ch) noexcept {
            if constexpr (is_utf32_v<Char>) {
                return is_utf32_continuation(ch);
            } else if constexpr (is_utf16_v<Char>) {
                return is_utf16_continuation(ch);
            } else {
                return is_utf8_continuation(ch);
            }
        }

        /// \brief Get number of utf code units in a utf codepoint that starts
        /// with this char The encoding is inferred from the input type. \param
        /// first_char First char in the codepoint \param available_code_units
        /// How many code units there are for this utf8 codepoint, including
        /// first_char \return Number of code units in this codepoint
        template <class Char, class Size = size_t, class Result = uint8_t>
        constexpr Result
        utf_size(Char first_char, Size available_code_units) noexcept {
            if constexpr (is_utf32_v<Char>) {
                return utf32_size(first_char, available_code_units);
            } else if constexpr (is_utf16_v<Char>) {
                return utf16_size(first_char, available_code_units);
            } else {
                return utf8_size(first_char, available_code_units);
            }
        }

        /// \brief Convert a sequence of utf8 byte / code units into an utf16
        /// array of code units
        template <
            class InputIt,
            class OutputIt,
            class Size1 = uint8_t,
            class Size2 = uint8_t>
        Size2
        from_utf8_to_utf16(
            InputIt src,
            Size1 utf8_size,
            OutputIt dest,
            Size2 utf16_size) noexcept {
            // Convert to unicode bits
            size_t source_idx = 0;
            unsigned long uni;
            size_t todo;
            unsigned char ch = src[source_idx++];
            if (ch <= 0x7F) {
                uni = ch;
                todo = 0;
            } else if (ch <= 0xBF) {
                // Invalid input
                return 0;
            } else if (ch <= 0xDF) {
                uni = ch & 0x1F;
                todo = 1;
            } else if (ch <= 0xEF) {
                uni = ch & 0x0F;
                todo = 2;
            } else if (ch <= 0xF7) {
                uni = ch & 0x07;
                todo = 3;
            } else {
                // Invalid input
                return 0;
            }
            for (size_t j = 0; j < todo; ++j) {
                if (cmp_equal(source_idx, utf8_size)) {
                    // Invalid input
                    return 0;
                }
                unsigned char ch2 = src[source_idx++];
                if (ch2 < 0x80 || ch2 > 0xBF)
                    // Invalid input
                    return 0;
                uni <<= 6;
                uni += ch2 & 0x3F;
            }
            if (uni >= 0xD800 && uni <= 0xDFFF)
                // Invalid input
                return 0;
            if (uni > 0x10FFFF)
                // Invalid input
                return 0;

            // Convert unicode to UTF16 output
            if (uni <= 0xFFFF) {
                if (utf16_size > 0) {
                    *dest = (wchar_t) uni;
                    return 1;
                } else {
                    return 0;
                }
            } else {
                uni -= 0x10000;
                if (utf16_size > 0) {
                    *dest = (wchar_t) ((uni >> 10) + 0xD800);
                    ++dest;
                } else {
                    return 0;
                }
                if (utf16_size > 1) {
                    *dest = (wchar_t) ((uni & 0x3FF) + 0xDC00);
                    return 2;
                } else {
                    return 1;
                }
            }
        }

        /// \brief Convert a sequence of utf8 bytes to a single UTF32 char
        template <
            typename Char32 = utf32_char_type,
            class InputIt,
            typename Size = uint8_t,
            typename std::enable_if_t<sizeof(Char32) != 1, int> = 0>
        constexpr Char32
        from_utf8_to_utf32(InputIt first, Size count) noexcept {
            Char32 cp = (unsigned char) *first;
            if (count > 1) {
                // Mask out the header bits
                cp &= 0x7F >> count;
                for (uint8_t i = 1; i < count; i++) {
                    cp = (cp << 6) | ((unsigned char) first[i] & 0x3F);
                }
            }
            return cp;
        }

        /// \brief Convert a utf32 char/codepoint into an utf8 array of bytes
        /// The destination iterator should be able to hold utf8 size bytes
        /// \param wide_char A wide char
        /// \param dest Destination of the utf8 code units
        /// \param utf8_size Size of the utf8 array
        /// \return utf8 size inserted in dest
        template <class OutputIt, class Char32, class Size = uint8_t>
        Size
        from_utf32_to_utf8(Char32 ch, OutputIt dest, Size utf8_size) noexcept {
            uint8_t size_in_utf8 = utf32_size_as_utf8(ch);
            switch (
                min_value(min_value(size_in_utf8, utf8_size), sizeof(Char32))) {
            case 7:
                *std::next(dest, size_in_utf8 - 6) = 0x80 | ((ch >> 30) & 0x3F);
                [[fallthrough]];
            case 6:
                *std::next(dest, size_in_utf8 - 5) = 0x80 | ((ch >> 24) & 0x3F);
                [[fallthrough]];
            case 5:
                *std::next(dest, size_in_utf8 - 4) = 0x80 | ((ch >> 18) & 0x3F);
                [[fallthrough]];
            case 4:
                *std::next(dest, size_in_utf8 - 3) = 0x80 | ((ch >> 12) & 0x3F);
                [[fallthrough]];
            case 3:
                *std::next(dest, size_in_utf8 - 2) = 0x80 | ((ch >> 6) & 0x3F);
                [[fallthrough]];
            case 2:
                *std::next(dest, size_in_utf8 - 1) = 0x80 | ((ch >> 0) & 0x3F);
                *std::next(dest, 0)
                    = (unsigned char) ((std::uint_least16_t(0xFF00uL) >> size_in_utf8) | (ch >> (6 * size_in_utf8 - 6)));
                break;
            case 1:
                *std::next(dest, 0) = (unsigned char) ch;
                break;
            }
            return size_in_utf8;
        }

        /// \brief Convert a utf32 code point to a sequence of utf16 code units
        template <class OutputIt, class Char32, class Size = uint8_t>
        Size
        from_utf32_to_utf16(Char32 ch, OutputIt dest, Size utf16_size) noexcept {
            using dest_value_type = typename std::iterator_traits<
                OutputIt>::value_type;
            if (utf16_size != 0) {
                if (ch <= 0x0000FFFF) {
                    /* UTF-16 surrogate values are illegal in UTF-32
                       0xFFFF or 0xFFFE are both reserved values */
                    if (ch >= 0xD800 && ch <= 0xDFFF) {
                        *dest++ = 0x0000FFFD;
                    } else {
                        /* source is a BMP Character */
                        *dest = static_cast<dest_value_type>(ch);
                        ++dest;
                    }
                    return 1;
                } else if (ch > 0x0010FFFF) {
                    /* U+10FFFF is the largest code point of Unicode Character
                     * Set */
                    *dest = 0x0000FFFD;
                    ++dest;
                    return 1;
                } else {
                    /* source is a character in range 0xFFFF - 0x10FFFF */
                    ch -= 0x0010000UL;
                    *dest++ = static_cast<dest_value_type>((ch >> 10) + 0xD800);
                    *dest++ = static_cast<dest_value_type>(
                        (ch & 0x3FFUL) + 0xDC00);
                    return 2;
                }
            }
            return 0;
        }

        /// \brief Convert a sequence of utf16 code units into an utf8 array of
        /// bytes
        template <
            class InputIt,
            class OutputIt,
            class InputSize = uint8_t,
            class OutputSize = uint8_t>
        OutputSize
        from_utf16_to_utf8(
            InputIt src,
            InputSize utf16_size,
            OutputIt dest,
            OutputSize utf8_size) noexcept {
            auto utf32_cp = from_utf16_to_utf32(src, utf16_size);
            return from_utf32_to_utf8(utf32_cp, dest, utf8_size);
        }

        /// \brief Convert a range of utf8/utf16/utf32 code units to a range of
        /// utf8 code units The source UTF type is inferred from the input type
        /// The destination iterator should be able to hold the required number
        /// of code units \return Number of code units inserted in the
        /// destination
        template <class InputIt, class InputSize, class OutputIt, class OutputSize>
        uint8_t
        to_utf8(
            InputIt source,
            InputSize source_count,
            OutputIt dest,
            OutputSize dest_count) noexcept {
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            if constexpr (is_utf32_v<input_value_type>) {
                return static_cast<uint8_t>(
                    from_utf32_to_utf8(*source, dest, dest_count));
            } else if constexpr (is_utf16_v<input_value_type>) {
                return static_cast<uint8_t>(
                    from_utf16_to_utf8(source, source_count, dest, dest_count));
            } else {
                using output_value_type = typename std::iterator_traits<
                    OutputIt>::value_type;
                std::transform(
                    source,
                    source + min_value(source_count, dest_count),
                    dest,
                    [](auto in) { return static_cast<output_value_type>(in); });
                return static_cast<uint8_t>(
                    min_value(source_count, dest_count));
            }
        }

        /// \brief Convert a range of utf8/utf16/utf32 code units to a range of
        /// utf16 code units The source UTF type is inferred from the input type
        /// The destination iterator should be able to hold the required number
        /// of code units \return Number of code units inserted in the
        /// destination
        template <class InputIt, class InputSize, class OutputIt, class OutputSize>
        uint8_t
        to_utf16(
            InputIt source,
            InputSize source_count,
            OutputIt dest,
            OutputSize dest_count) noexcept {
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            if constexpr (is_utf32_v<input_value_type>) {
                return static_cast<uint8_t>(
                    from_utf32_to_utf16(*source, dest, dest_count));
            } else if constexpr (is_utf16_v<input_value_type>) {
                using output_value_type = typename std::iterator_traits<
                    OutputIt>::value_type;
                std::transform(
                    source,
                    source + (std::min)(source_count, dest_count),
                    dest,
                    [](auto in) { return static_cast<output_value_type>(in); });
                return static_cast<uint8_t>(
                    (std::min)(source_count, dest_count));
            } else {
                return static_cast<uint8_t>(
                    from_utf8_to_utf16(source, source_count, dest, dest_count));
            }
        }

        /// \brief Convert a range of utf8/utf16/utf32 code units to a range of
        /// utf32 code units The source UTF type is inferred from the input type
        /// The destination iterator should be able to hold the required number
        /// of code units We are only going to spend one code unit for the UTF32
        /// codepoint, but we still expect an iterator as input in conformity
        /// with the other conversion functions. \return Number of code units
        /// inserted in the destination
        template <class InputIt, class InputSize, class OutputIt, class OutputSize>
        uint8_t
        to_utf32(
            InputIt source,
            InputSize source_count,
            OutputIt dest,
            OutputSize dest_count) noexcept {
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            if constexpr (is_utf32_v<input_value_type>) {
                using output_value_type = typename std::iterator_traits<
                    OutputIt>::value_type;
                std::transform(
                    source,
                    source + min_value(source_count, dest_count),
                    dest,
                    [](auto in) { return static_cast<output_value_type>(in); });
                return static_cast<uint8_t>(
                    min_value(source_count, dest_count));
            } else if constexpr (is_utf16_v<input_value_type>) {
                *dest = from_utf16_to_utf32(source, source_count);
                return 1;
            } else {
                *dest = from_utf8_to_utf32(source, source_count);
                return 1;
            }
        }

        /// \brief Convert a range of utf8/utf16/or utf32 code units to a range
        /// of utf8/utf16/or utf32 code units The source UTF type is inferred
        /// from the input type. The output UTF type is inferred from the output
        /// type. The destination iterator should be able to hold the required
        /// number of code units. \return Number of code units inserted in the
        /// destination
        template <class InputIt, class InputSize, class OutputIt, class OutputSize>
        uint8_t
        to_utf(
            InputIt source,
            InputSize source_count,
            OutputIt dest,
            OutputSize dest_count) noexcept {
            using output_value_type = typename std::iterator_traits<
                OutputIt>::value_type;
            if constexpr (is_utf32_v<output_value_type>) {
                return to_utf32(source, source_count, dest, dest_count);
            } else if constexpr (is_utf16_v<output_value_type>) {
                return to_utf16(source, source_count, dest, dest_count);
            } else {
                return to_utf8(source, source_count, dest, dest_count);
            }
        }

    } // namespace detail
} // namespace small

#ifdef cpp_char8_t
inline
std::ostream&
operator<<(std::ostream& os, char8_t c) {
    return os << static_cast<char>(c);
}

inline
std::ostream&
operator<<(std::ostream& os, const char8_t* c) {
    return os << reinterpret_cast<const char*>(c);
}
#endif

#endif // SMALL_DETAIL_ALGORITHM_UTF_HPP
