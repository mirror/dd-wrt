//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_STRING_HPP
#define SMALL_STRING_HPP

#include <small/vector.hpp>
#include <small/detail/algorithm/utf.hpp>
#include <small/detail/container/lookup_table_view.hpp>
#include <small/detail/iterator/codepoint_iterator.hpp>
#include <small/detail/traits/extract_value_type.hpp>
#include <small/detail/traits/strong_type.hpp>
#include <sstream>

namespace small {
    /// \class Small string
    ///
    /// This class has a few goals:
    /// - allow the same small/inline/cache optimization for strings (like we do
    /// for vectors) on all platforms
    ///     - many compilers don't implement small string optimization
    /// - allowing for custom max expected string size, like we do for vectors
    /// - supporting utf8 by default (since it's a new container anyway) while
    /// still being compatible
    ///   with the string API (iterators to bytes rather than code points)
    ///     - supporting utf8 is also what makes it small because it spends up
    ///     to 4x less memory compared to some
    ///       common alternatives to represent general strings
    /// - providing some basic operations to convert to wide chars and
    /// manipulate utf8 by default when these are needed
    ///     - attempt to stream to cout/cerr/... buffers without having to worry
    ///     too much about converting to wide chars
    ///     - create a UTF8 string from literal strings without worrying too
    ///     much about literal char type
    ///     - avoid using libraries such as boost::nowide unless we really have
    ///     to
    ///
    /// The bits of this implementation that handle utf8 are adapted from the
    /// tiny-utf8 string functions. If you are used to tiny-utf8, the main
    /// differences from a user perspective are:
    /// - the custom max expected size
    /// - changes in the API to make it compatible with std::string so it can be
    /// a drop-in replacement in most cases:
    ///     - bytes from operator[] rather than code points by default
    ///     (operator() is reserved for code-points)
    ///     - byte iterators from begin()/end()/insert/... rather than code
    ///     points by default
    ///         - functions whose return types handle code points are prefixed
    ///         with cp_* (cp_begin()/cp_end()/...)
    ///     - we attempt to provide the complete std::string interface
    ///     (reserve/max_size/variations of insert/...)
    /// - Assumes C++17
    /// From an implementation perspective:
    /// - the optimizations are more like our small vectors
    /// - better growth factors
    /// - adjustments to reuse the lookup table more efficiently with these
    /// growth factors
    /// - using our own inline vectors for storage to reduce code bloat,
    /// simplify the API, make it safer, inherit
    ///   optimizations and remove some unneeded fields in the tiny-utf8 data
    ///   structure for inline storage
    ///
    /// Although we attempt to minimize unneeded differences, note that this
    /// string is not an exact replacement for std::string in this case. In
    /// particular, this is not a drop-in replacement if you depend on
    /// std::string to represent bytes with your string. The main reason is that
    /// small::string will convert to wchar (usually on windows) if it has to.
    /// Although this is relatively conservative (only to std::cout), this might
    /// break things. In other words, this small string is intended to represent
    /// words/sentences/etc... rather than bytes. For representing bytes you can
    /// always use std::vector<uint8_t> or std::span instead.
    ///
    /// Reference:
    /// - https://github.com/DuffsDevice/tiny-utf8
    ///   - already has SSO, so we only we only need to allow setting the size
    ///   - in our applications, we need to make it compatible with the string
    ///   API though
    ///      - So the iterators still point to bytes rather than code points
    ///      - cp_begin()/cp_end() point to chars / wcp_begin()/wchar() point to
    ///      code points
    ///   - the release function allows us to move to a string
    ///
    /// \see UTF8 string implementation:
    /// https://github.com/DuffsDevice/tiny-utf8 \see UTF8 functions:
    /// https://github.com/sheredom/utf8.h \note fmt::print from the fmt library
    /// supports printing as utf8 on windows so we can reuse that
    namespace detail {
        constexpr size_t default_codepoint_hint_step = 10;
    } // namespace detail

    template <
        typename CharT = char,
        size_t N = default_inline_storage_v<CharT>,
        class Traits = std::char_traits<CharT>,
        typename WCharT = char32_t,
        class Allocator = std::allocator<CharT>,
        size_t CP_HINT_STEP = detail::default_codepoint_hint_step,
        class SizeType = size_t>
    class basic_string
    {
    public:
        /// \section public types common with the std::string API
        typedef Traits traits_type;
        typedef CharT value_type;
        typedef Allocator allocator_type;
        typedef SizeType size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef typename std::allocator_traits<Allocator>::pointer pointer;
        typedef typename std::allocator_traits<Allocator>::const_pointer
            const_pointer;
        typedef detail::pointer_wrapper<pointer> iterator;
        typedef detail::pointer_wrapper<const_pointer> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        enum : size_type
        {
            npos = (size_type) -1
        };

        /// \section public types relative to the utf8 functionality / code
        /// points

        /// \section Type we use for represent larger codepoints
        typedef WCharT wide_value_type;

        /// \brief Data type capable of holding the number of code units in a
        /// codepoint
        typedef std::uint_fast8_t width_type;

        /// \brief Step sizes for codepoint hints in the lookup table
        static constexpr size_type codepoint_hint_step_size = CP_HINT_STEP;

        /// \section Type we use for the lookup table, if it exists
        /// The type of codepoint map view for accessing the byte and codepoint
        /// indexes of multibyte chars The table will use the codepoint hint
        /// defined above
        using lookup_table_type = detail::lookup_table_view<
            value_type,
            traits_type,
            codepoint_hint_step_size,
            size_type>;
        using const_lookup_table_type = detail::const_lookup_table_view<
            value_type,
            traits_type,
            codepoint_hint_step_size,
            size_type>;

        /// \brief We string view type for this string type
        using string_view_type = std::basic_string_view<value_type, traits_type>;

        /// \section Self type alias for convenience in the next types
        using self_type = basic_string<
            CharT,
            N,
            Traits,
            WCharT,
            Allocator,
            CP_HINT_STEP,
            SizeType>;

        /// \section Types for iterating codepoints
        using codepoint_reference = detail::external_codepoint_reference<
            self_type>;
        using const_codepoint_reference = detail::
            const_external_codepoint_reference<self_type>;
        using codepoint_iterator = detail::external_codepoint_iterator<
            self_type>;
        using const_codepoint_iterator = detail::
            const_external_codepoint_iterator<self_type>;
        using reverse_codepoint_iterator = detail::
            reverse_external_codepoint_iterator<self_type>;
        using const_reverse_codepoint_iterator = detail::
            const_reverse_external_codepoint_iterator<self_type>;

        /// \brief A strong integer type for representing code point indexes
        /// Although it's a strong type, it still includes most operations to
        /// work with size_type
    private:
        struct codepoint_size_type_tag
        {};

    public:
        /// \brief A strong type to allow overloads looking for codepoint
        /// indexes rather than byte indexes
        using codepoint_size_type = strong::type<
            size_type,
            codepoint_size_type_tag,
            strong::implicitly_convertible_to<size_type>>;

        using codepoint_index = codepoint_size_type;

    private:
        /// \brief The std::string functions that work with string views use
        /// this predicate for overload resolution This predicate means
        /// something can become a string view but not a c-string pointer, for
        /// which the API has other overloads. The difference here is that we
        /// accept string views of any char type
        template <class T>
        using is_api_string_view = std::conjunction<
            std::negation<std::is_same<detail::extract_value_type_t<T>, void>>,
            std::is_convertible<
                const T &,
                std::basic_string_view<
                    detail::extract_value_type_t<T>,
                    std::char_traits<detail::extract_value_type_t<T>>>>,
            std::negation<std::is_convertible<
                const T &,
                const detail::extract_value_type_t<T> *>>>;

        template <class T>
        static constexpr bool is_api_string_view_v = is_api_string_view<
            T>::value;

    public:
        /// \brief Default contructor
        constexpr basic_string() noexcept(
            noexcept(std::is_nothrow_default_constructible<allocator_type>()))
            : basic_string(allocator_type()) {}

        /// \brief Allocator_type constructor
        /// Construct the initial buffer with "\0"
        explicit constexpr basic_string(const allocator_type &alloc) noexcept
            : buffer_(has_lookup_table ? 2 : 1, '\0', alloc) {
            buffer_.resize(buffer_.capacity());
            if constexpr (has_lookup_table) {
                // Set up the lookup table
                lookup_table_type t = lookup_table();
                t.clear();
            }
        }

        /// \brief Construct with count initial chars of value cp
        /// This is a somewhat special case because we already know how many
        /// bytes we need for the whole string. We are not usually that lucky.
        template <typename InputChar>
        constexpr basic_string(
            size_type count,
            InputChar cp,
            const allocator_type &alloc = allocator_type())
            : buffer_(alloc) {
            // Handle the most trivial case first
            if (count == 0) {
                buffer_.resize(has_lookup_table ? 2 : 1);
                buffer_.resize(buffer_.capacity());
                buffer_.front() = '\0';
                if constexpr (has_lookup_table) {
                    lookup_table_type t = lookup_table();
                    t.clear();
                }
                return;
            }

            // Initialize size
            constexpr bool input_is_wide = sizeof(InputChar) > 1;
            uint8_t codeunits_per_codepoint = 1;
            if constexpr (input_is_wide) {
                codeunits_per_codepoint = detail::utf_size_as<
                    value_type>(&cp, 1);
            }
            size_.codeunit_size() = codeunits_per_codepoint * count;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() = count;
            }

            // Initialize buffer
            const size_type n_multibyte_codepoints
                = codeunits_per_codepoint != 1 ? count : 0;
            const size_type lookup_buffer_size
                = has_lookup_table ? lookup_table_type::size_for(
                      size_.codeunit_size(),
                      n_multibyte_codepoints) :
                                     0;
            buffer_.resize(size_.codeunit_size() + 1 + lookup_buffer_size);
            buffer_.resize(buffer_.capacity());

            // Fill the buffer
            if constexpr (!detail::is_same_utf_encoding_v<InputChar, value_type>)
            {
                if (codeunits_per_codepoint == 1) {
                    std::fill(
                        buffer_.begin(),
                        buffer_.begin() + count,
                        static_cast<value_type>(cp));
                } else {
                    detail::
                        to_utf(&cp, 1, buffer_.data(), codeunits_per_codepoint);
                    size_type n_to_copy = count;
                    for (value_type *it = buffer_.data(); --n_to_copy > 0;) {
                        std::memcpy(
                            it += codeunits_per_codepoint,
                            buffer_.data(),
                            codeunits_per_codepoint);
                    }
                }
            } else {
                std::memset(buffer_.data(), cp, count);
            }
            buffer_[size_.codeunit_size()] = '\0';

            // Fill the lookup table
            if constexpr (has_lookup_table) {
                lookup_table_type t = lookup_table();
                t.clear();
                if (n_multibyte_codepoints != 0) {
                    t.resize(n_multibyte_codepoints);
                    for (size_type i = 0; i < count; ++i) {
                        t.insert_or_assign(i, i * codeunits_per_codepoint);
                    }
                }
            }
        }

        /// \brief Constructs the string with the contents of the range [first,
        /// last) of wide chars
        template <class InputIt>
        constexpr basic_string(
            InputIt first,
            InputIt last,
            const allocator_type &alloc = allocator_type())
            : buffer_(alloc) {
            // Handle the most trivial case first
            if (first == last) {
                buffer_.resize(has_lookup_table ? 2 : 1);
                buffer_.resize(buffer_.capacity());
                buffer_.front() = '\0';
                if constexpr (has_lookup_table) {
                    lookup_table_type t = lookup_table();
                    t.clear();
                }
                return;
            }

            // Handle input iterators the hard way
            constexpr bool is_input_iterator = std::is_same_v<
                typename std::iterator_traits<InputIt>::iterator_category,
                std::input_iterator_tag>;
            if (is_input_iterator) {
                buffer_.resize(has_lookup_table ? 2 : 1);
                buffer_.resize(buffer_.capacity());
                buffer_.front() = '\0';
                if constexpr (has_lookup_table) {
                    lookup_table_type t = lookup_table();
                    t.clear();
                }
                while (first != last) {
                    push_back(*first++);
                }
                return;
            }

            // Initialize size
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            const size_type input_code_units = std::distance(first, last);
            size_type code_points = 0;
            size_type output_code_units = 0;
            size_type output_multibyte_codepoints = 0;
            for (InputIt input_code_unit_it = first;
                 input_code_unit_it != last && *input_code_unit_it;)
            {
                // Get code point sizes
                const size_type input_cu_left = last - input_code_unit_it;
                uint8_t it_input_code_units = detail::
                    utf_size(*input_code_unit_it, input_cu_left);
                uint8_t it_output_code_units = detail::utf_size_as<
                    value_type>(input_code_unit_it, input_cu_left);

                // Count code points we will have in the output
                if (it_output_code_units > 1) {
                    ++output_multibyte_codepoints;
                }
                output_code_units += it_output_code_units;

                // Next
                ++code_points;
                std::advance(input_code_unit_it, it_input_code_units);
            }
            size_.codeunit_size() = output_code_units;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() = code_points;
            }

            // Initialize buffer
            const size_type lookup_buffer_size
                = has_lookup_table ? lookup_table_type::size_for(
                      size_.codeunit_size(),
                      output_multibyte_codepoints) :
                                     0;
            buffer_.resize(size_.codeunit_size() + 1 + lookup_buffer_size);
            buffer_.resize(buffer_.capacity());

            // Fill the buffer
            if constexpr (detail::is_same_utf_encoding_v<
                              input_value_type,
                              value_type>) {
                if constexpr (std::is_pointer_v<InputIt>) {
                    std::memcpy(buffer_.data(), first, input_code_units);
                } else /* if (is iterator) */ {
                    std::copy(first, last, buffer_.begin());
                }
            } else {
                size_type buffer_idx = 0;
                InputIt input_code_unit_it = first;
                while (input_code_unit_it != last && *input_code_unit_it) {
                    // Get sizes
                    const size_type input_cu_left = last - input_code_unit_it;
                    const size_type output_cu_left = output_code_units
                                                     - buffer_idx;
                    const size_type it_input_code_units = detail::
                        utf_size(*input_code_unit_it, input_cu_left);
                    const size_type it_output_code_units = detail::utf_size_as<
                        value_type>(input_code_unit_it, input_cu_left);
                    // Convert
                    detail::to_utf(
                        input_code_unit_it,
                        input_cu_left,
                        buffer_.data() + buffer_idx,
                        output_cu_left);
                    // Next
                    input_code_unit_it += it_input_code_units;
                    buffer_idx += it_output_code_units;
                }
            }
            buffer_[size_.codeunit_size()] = '\0';

            // Fill the lookup table
            if constexpr (has_lookup_table) {
                lookup_table_type t = lookup_table();
                t.clear();
                if (output_multibyte_codepoints != 0) {
                    t.resize(output_multibyte_codepoints);
                    size_type codeunit_idx = 0;
                    size_type codepoint_idx = 0;
                    size_type multibyte_idx = 0;
                    while (codeunit_idx < size_.codeunit_size()) {
                        const size_type n_code_units = detail::utf_size(
                            buffer_[codeunit_idx],
                            size_.codeunit_size() - codeunit_idx);
                        if (n_code_units > 1) {
                            t.insert_or_assign(
                                multibyte_idx,
                                codeunit_idx,
                                codepoint_idx);
                            ++multibyte_idx;
                        }
                        codeunit_idx += n_code_units;
                        ++codepoint_idx;
                    }
                }
            }
        }

        /// \brief Constructs the string with a substring [pos, end) of other
        /// string
        constexpr basic_string(
            const basic_string &other,
            size_type pos,
            const allocator_type &alloc = allocator_type())
            : basic_string(other.begin() + pos, other.end(), alloc) {}

        /// \brief Constructs the string with a substring [pos, pos+count) of
        /// other
        constexpr basic_string(
            const basic_string &other,
            size_type pos,
            size_type count,
            const allocator_type &alloc = allocator_type())
            : basic_string(
                other.begin() + pos,
                count != npos ? (other.begin() + pos + count) : other.end(),
                alloc) {}

        /// \brief Constructs the string with the first count characters of
        /// character string pointed to by s.
        template <typename InputChar>
        constexpr basic_string(
            const InputChar *s,
            size_type count,
            const allocator_type &alloc = allocator_type())
            : basic_string(
                s,
                count != npos ? (s + count) : s + detail::strlen(s),
                alloc) {}

        /// \brief Constructs the string with the contents initialized with a
        /// copy of the null-terminated character string pointed to by s.
        template <typename InputChar>
        /// NOLINTNEXTLINE(google-explicit-constructor): replicating the
        /// std::string constructor
        constexpr basic_string(
            const InputChar *s,
            const allocator_type &alloc = allocator_type())
            : basic_string(s, detail::strlen(s), alloc) {}

        /// \brief Copy constructor. Constructs the string with a copy of the
        /// contents of other.
        constexpr basic_string(const basic_string &other)
            : buffer_(other.buffer_), size_(other.size_) {}

        /// \brief Copy constructor. Constructs the string with a copy of the
        /// contents of other.
        constexpr basic_string(
            const basic_string &other,
            const allocator_type &alloc)
            : buffer_(other.buffer_, alloc), size_(other.size_) {}

        /// \brief Move constructor. Constructs the string with the contents of
        /// other using move semantics. other is left in valid, but unspecified
        /// state
        constexpr basic_string(basic_string &&other) noexcept
            : buffer_(std::move(other.buffer_)), size_(std::move(other.size_)) {
            other.clear();
        }

        /// \brief Move constructor. Constructs the string with the contents of
        /// other using move semantics. other is left in valid, but unspecified
        /// state
        constexpr basic_string(basic_string &&other, const allocator_type &alloc)
            : buffer_(std::move(other.buffer_), alloc),
              size_(std::move(other.size_)) {
            other.clear();
        }

        /// \brief Constructs the string with the contents of the initializer
        /// list ilist
        template <typename CharType>
        constexpr basic_string(
            std::initializer_list<CharType> ilist,
            const allocator_type &alloc = allocator_type())
            : basic_string(ilist.begin(), ilist.end(), alloc) {}

        /// \brief Construct from string view
        template <typename CharType, class TraitsType>
        explicit constexpr basic_string(
            const std::basic_string_view<CharType, TraitsType> &sv,
            const allocator_type &alloc = allocator_type())
            : basic_string(sv.begin(), sv.end(), alloc) {}

        /// \brief Construct from something convertible to a string view
        /// Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<value_type, traits_type> sv = t;, then
        /// initializes the string with the contents of sv, as if by
        /// basic_string(sv.data(), sv.size(), alloc). This overload
        /// participates in overload resolution only if
        /// std::is_convertible_v<const T&, std::basic_string_view<value_type,
        /// traits_type>> is true and std::is_convertible_v<const T&, const
        /// value_type*> is false.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        explicit constexpr basic_string(
            const T &t,
            const allocator_type &alloc = allocator_type())
            : basic_string(
                std::basic_string_view<
                    typename T::value_type,
                    std::char_traits<typename T::value_type>>(t),
                alloc) {}

        /// \brief Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<value_type, traits_type> sv = t;, then
        /// initializes the string with the subrange [pos, pos + n) of sv as if
        /// by basic_string(sv.substr(pos, n), alloc).
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        explicit constexpr basic_string(
            const T &t,
            size_type pos,
            size_type n,
            const allocator_type &alloc = allocator_type())
            : basic_string(
                std::basic_string_view<
                    typename T::value_type,
                    std::char_traits<typename T::value_type>>(t)
                    .substr(pos, n),
                alloc) {}

        /// \brief Basic_string cannot be constructed from nullptr
        constexpr basic_string(std::nullptr_t) = delete;

        /// \brief Replaces the contents with a copy of str
        constexpr basic_string &
        operator=(const basic_string &str) {
            this->buffer_ = str.buffer_;
            this->size_ = str.size_;
            return *this;
        }

        /// \brief Replaces the contents with those of str using move semantics
        constexpr basic_string &
        operator=(basic_string &&other) noexcept(
            std::allocator_traits<
                Allocator>::propagate_on_container_move_assignment::value) {
            this->buffer_ = std::move(other.buffer_);
            this->size_ = std::move(other.size_);
            other.clear();
            return *this;
        }

        /// \brief Replaces the contents with those of null-terminated character
        /// string pointed to by s
        template <typename Char>
        constexpr basic_string &
        operator=(const Char *s) {
            *this = basic_string(s, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with character ch as if by
        /// assign(std::addressof(ch), 1)
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        operator=(Char ch) {
            assign(1, ch);
            return *this;
        }

        /// \brief Replaces the contents with those of the initializer list
        /// ilist as if by assign(ilist.begin(), ilist.size())
        template <typename Char>
        constexpr basic_string &
        operator=(std::initializer_list<Char> ilist) {
            assign(ilist.begin(), ilist.size());
            return *this;
        }

        /// \brief Implicitly converts t to a string view sv and assign
        /// As if by std::basic_string_view<CharT, Traits> sv = t;, then
        /// replaces the contents with those of the sv as if by assign(sv)
        template <
            class T,
            std::enable_if_t<
                std::is_convertible_v<
                    const T &,
                    std::basic_string_view<
                        typename T::value_type,
                        std::char_traits<
                            typename T::
                                value_type>>> && !std::is_convertible_v<const T &, const value_type *>,
                int> = 0>
        constexpr basic_string &
        operator=(const T &t) {
            assign(std::basic_string_view<
                   typename T::value_type,
                   std::char_traits<typename T::value_type>>(t));
            return *this;
        }

        /// \brief basic_string cannot be assigned from nullptr
        constexpr basic_string &operator=(std::nullptr_t) = delete;

        /// \brief Replaces the contents with count copies of character ch.
        template <typename Char>
        constexpr basic_string &
        assign(size_type count, Char ch) {
            *this = basic_string(count, ch, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with a copy of str
        constexpr basic_string &
        assign(const basic_string &str) {
            *this = str;
            return *this;
        }

        /// \brief Replaces the contents with a substring [pos, pos+count) of str
        constexpr basic_string &
        assign(
            const basic_string &str,
            size_type pos,
            size_type count = basic_string::npos) {
            auto view = std::basic_string_view<value_type>(str);
            *this = basic_string(view, pos, count, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with those of str using move semantics
        constexpr basic_string &
        assign(basic_string &&str) {
            *this = std::move(str);
            return *this;
        }

        /// \brief Replaces the contents with copies of the characters in the
        /// range [s, s+count)
        template <typename Char>
        constexpr basic_string &
        assign(const Char *s, size_type count) {
            auto view = std::basic_string_view<Char>(s, count);
            *this = basic_string(view, 0, count, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with those of null-terminated character
        /// string pointed to by s.
        template <typename Char>
        constexpr basic_string &
        assign(const Char *s) {
            auto view = std::basic_string_view<Char>(s);
            *this = basic_string(view, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with copies of the characters in the
        /// range [first, last)
        template <class InputIt>
        constexpr basic_string &
        assign(InputIt first, InputIt last) {
            *this = basic_string(first, last, get_allocator());
            return *this;
        }

        /// \brief Replaces the contents with those of the initializer list ilist
        template <typename Char>
        constexpr basic_string &
        assign(std::initializer_list<Char> ilist) {
            return assign(ilist.begin(), ilist.end());
        }

        /// \brief Implicitly converts t to a string view sv and assign
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        assign(const T &t) {
            *this = basic_string(t, get_allocator());
            return *this;
        }

        /// \brief Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<CharT, Traits> sv = t;, then replaces the
        /// contents with the characters from the subview [pos, pos+count) of sv
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        assign(const T &t, size_type pos, size_type count = basic_string::npos) {
            *this = basic_string(t, pos, count, get_allocator());
            return *this;
        }

        /// \brief Returns the allocator associated with the string
        [[nodiscard]] constexpr allocator_type
        get_allocator() const noexcept {
            return buffer_.get_allocator();
        }

    public:
        /// \section Element access

        /// \brief Returns a reference to the character at specified location
        /// pos Bounds checking is performed, exception of type
        /// std::out_of_range will be thrown on invalid access
        constexpr reference
        at(size_type byte_index) {
            const size_type size = this->size();
            if (byte_index >= size) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::at: index out of bounds");
            }
            return operator[](byte_index);
        }

        /// \brief Returns a reference to the character at specified location
        /// pos Bounds checking is performed, exception of type
        /// std::out_of_range will be thrown on invalid access
        [[nodiscard]] constexpr const_reference
        at(size_type byte_index) const {
            return (const_cast<basic_string *>(this))->at(byte_index);
        }

        /// \brief Returns a wide char with the i-th code point in the string
        /// Bounds checking is performed, exception of type std::out_of_range
        /// will be thrown on invalid access
        constexpr codepoint_reference
        at(codepoint_index index) {
            const size_type size = this->size_.codepoint_size();
            if (index >= size) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::at: index out of bounds");
            }
            return operator[](index);
        }

        /// \brief Returns a wide char with the i-th code point in the string
        /// Bounds checking is performed, exception of type std::out_of_range
        /// will be thrown on invalid access
        [[nodiscard]] constexpr const_codepoint_reference
        at(codepoint_index index) const {
            return (const_cast<basic_string *>(this))->at(index);
        }

        /// \brief Returns a reference to the character at specified location
        /// pos. No bounds checking is performed. If pos > size(), the behavior
        /// is undefined If pos == size(), a reference to the character with
        /// value CharT() (the null character) is returned.
        constexpr reference
        operator[](size_type byte_index) {
            assert(
                byte_index <= size()
                && "string::operator[]: index out of bounds");
            return *(data() + byte_index);
        }

        /// \brief Returns a reference to the character at specified location
        /// pos. No bounds checking is performed. If pos > size(), the behavior
        /// is undefined If pos == size(), a reference to the character with
        /// value CharT() (the null character) is returned.
        constexpr const_reference
        operator[](size_type byte_index) const {
            assert(
                byte_index <= size()
                && "string::operator[]: index out of bounds");
            return *(data() + byte_index);
        }

        /// \brief Returns a wide char with the i-th code point in the string
        /// No bounds checking is performed. If pos > size(), the behavior is
        /// undefined
        constexpr codepoint_reference
        operator[](codepoint_index index) {
            codepoint_iterator it = begin_codepoint() + index.value_of();
            return it.operator*();
        }

        /// \brief Returns a wide char with the i-th code point in the string
        /// No bounds checking is performed. If pos > size(), the behavior is
        /// undefined
        constexpr const_codepoint_reference
        operator[](codepoint_index index) const {
            return const_codepoint_reference(index, this);
        }

        /// \brief Returns a wide char with the i-th code point in the string
        /// This is like the unchecked counterpart of at_value
        /// No bounds checking is performed. If pos > size(), the behavior is
        /// undefined
        constexpr wide_value_type
        operator()(codepoint_index index) const {
            assert(
                index <= size_codepoints()
                && "string::operator(): codepoint index out of bounds");
            const_codepoint_iterator it = cbegin_codepoint() + index;
            return it.wide_value();
        }

        /// \brief Returns a byte index of a codepoint
        [[nodiscard]] constexpr wide_value_type
        byte_index(codepoint_index index) const {
            assert(
                index <= size_codepoints()
                && "string::operator(): codepoint index out of bounds");
            return get_num_bytes_from_start(index);
        }

        /// \brief Returns reference to the first character in the string
        /// The behavior is undefined if empty() == true
        constexpr reference
        front() {
            return operator[](0);
        }

        /// \brief Returns reference to the first character in the string
        /// The behavior is undefined if empty() == true
        [[nodiscard]] constexpr const_reference
        front() const {
            return operator[](0);
        }

        /// \brief Returns value of the first codepoint in the string
        /// The behavior is undefined if empty() == true
        constexpr codepoint_reference
        front_codepoint() {
            return operator[](codepoint_index(0));
        }

        /// \brief Returns value of the first codepoint in the string
        /// The behavior is undefined if empty() == true
        [[nodiscard]] constexpr const_codepoint_reference
        front_codepoint() const {
            return operator[](codepoint_index(0));
        }

        /// \brief Returns reference to the last character in the string
        /// The behavior is undefined if empty() == true
        constexpr reference
        back() {
            return operator[](size() - 1);
        }

        /// \brief Returns reference to the last character in the string
        /// The behavior is undefined if empty() == true
        [[nodiscard]] constexpr const_reference
        back() const {
            return operator[](size() - 1);
        }

        /// \brief Returns reference to the last code point in the string
        /// The behavior is undefined if empty() == true
        constexpr codepoint_reference
        back_codepoint() {
            return operator[](codepoint_index(size_codepoints() - 1));
        }

        /// \brief Returns reference to the last code point in the string
        /// The behavior is undefined if empty() == true
        [[nodiscard]] constexpr const_codepoint_reference
        back_codepoint() const {
            return operator[](codepoint_index(size_codepoints() - 1));
        }

        /// \brief Returns a pointer to the underlying array serving as
        /// character storage The pointer is such that the range [data(); data()
        /// + size()] is valid and the values in it correspond to the values
        /// stored in the string. The returned array is null-terminated, that
        /// is, data() and c_str() perform the same function If empty() returns
        /// true, the pointer points to a single null character.
        constexpr pointer
        data() noexcept {
            return this->buffer_.data();
        }

        /// \brief Returns a pointer to the underlying array serving as
        /// character storage The pointer is such that the range [data(); data()
        /// + size()] is valid and the values in it correspond to the values
        /// stored in the string. The returned array is null-terminated, that
        /// is, data() and c_str() perform the same function If empty() returns
        /// true, the pointer points to a single null character.
        [[nodiscard]] constexpr const_pointer
        data() const noexcept {
            return this->buffer_.data();
        }

        /// \brief Returns a pointer to a null-terminated character array with
        /// data equivalent to those stored in the string
        constexpr pointer
        c_str() noexcept {
            return data();
        }

        [[nodiscard]] constexpr const_pointer
        c_str() const noexcept {
            return data();
        }

        /// \brief Returns a standard C++ string with data equivalent to this
        /// basic string stored in the string
        [[nodiscard]] constexpr std::basic_string<CharT>
        cpp_str() const {
            return std::basic_string<CharT>(begin(), end());
        }

        /// \brief Returns a std::basic_string_view from this string
        /// NOLINTNEXLINE(google-explicit-constructor): Replicating the STL API
        constexpr operator string_view_type() const noexcept {
            return string_view_type(data(), size());
        }

    public:
        /// \section Bytes iterators

        /// \brief Returns an iterator to the first character of the string
        constexpr iterator
        begin() noexcept {
            return buffer_.begin();
        }

        /// \brief Returns an iterator to the first character of the string
        [[nodiscard]] constexpr const_iterator
        begin() const noexcept {
            return buffer_.cbegin();
        }

        /// \brief Returns an iterator to the first character of the string
        [[nodiscard]] constexpr const_iterator
        cbegin() const noexcept {
            return buffer_.cbegin();
        }

        /// \brief Returns an iterator to the character following the last
        /// character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        constexpr iterator
        end() noexcept {
            return buffer_.begin() + size();
        }

        /// \brief Returns an iterator to the character following the last
        /// character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        [[nodiscard]] constexpr const_iterator
        end() const noexcept {
            return buffer_.cbegin() + size();
        }

        /// \brief Returns an iterator to the character following the last
        /// character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        [[nodiscard]] constexpr const_iterator
        cend() const noexcept {
            return buffer_.cbegin() + size();
        }

        /// \brief Returns a reverse iterator to the first character of the
        /// reversed string
        constexpr reverse_iterator
        rbegin() noexcept {
            return std::reverse_iterator<iterator>(end());
        }

        /// \brief Returns a reverse iterator to the first character of the
        /// reversed string
        [[nodiscard]] constexpr const_reverse_iterator
        rbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(end());
        }

        /// \brief Returns a reverse iterator to the first character of the
        /// reversed string
        [[nodiscard]] constexpr const_reverse_iterator
        crbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(end());
        }

        /// \brief Returns a reverse iterator to the character following the
        /// last character of the reversed string
        constexpr reverse_iterator
        rend() noexcept {
            return std::reverse_iterator<iterator>(begin());
        }

        /// \brief Returns a reverse iterator to the character following the
        /// last character of the reversed string
        [[nodiscard]] constexpr const_reverse_iterator
        rend() const noexcept {
            return std::reverse_iterator<const_iterator>(begin());
        }

        /// \brief Returns a reverse iterator to the character following the
        /// last character of the reversed string
        [[nodiscard]] constexpr const_reverse_iterator
        crend() const noexcept {
            return std::reverse_iterator<const_iterator>(begin());
        }

    public:
        /// \section Codepoint iterators

        /// \brief Returns an codepoint_iterator to the first character of the
        /// string
        constexpr codepoint_iterator
        begin_codepoint() noexcept {
            return codepoint_iterator{ this, 0, 0 };
        }

        /// \brief Returns an codepoint_iterator to the first character of the
        /// string
        [[nodiscard]] constexpr const_codepoint_iterator
        begin_codepoint() const noexcept {
            return cbegin_codepoint();
        }

        /// \brief Returns an codepoint_iterator to the first character of the
        /// string
        [[nodiscard]] constexpr const_codepoint_iterator
        cbegin_codepoint() const noexcept {
            return const_codepoint_iterator{ this, 0, 0 };
        }

        /// \brief Returns an codepoint_iterator to the character following the
        /// last character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        constexpr codepoint_iterator
        end_codepoint() noexcept {
            return codepoint_iterator{ this, size_codepoints(), size() };
        }

        /// \brief Returns an codepoint_iterator to the character following the
        /// last character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        [[nodiscard]] constexpr const_codepoint_iterator
        end_codepoint() const noexcept {
            return const_codepoint_iterator{ this, size_codepoints(), size() };
        }

        /// \brief Returns an codepoint_iterator to the character following the
        /// last character of the string This character acts as a placeholder,
        /// attempting to access it results in undefined behavior
        [[nodiscard]] constexpr const_codepoint_iterator
        cend_codepoint() const noexcept {
            return const_codepoint_iterator{ this, size_codepoints(), size() };
        }

        /// \brief Returns a reverse codepoint_iterator to the first character
        /// of the reversed string
        constexpr reverse_codepoint_iterator
        rbegin_codepoint() noexcept {
            return std::reverse_iterator<codepoint_iterator>(end_codepoint());
        }

        /// \brief Returns a reverse codepoint_iterator to the first character
        /// of the reversed string
        [[nodiscard]] constexpr const_reverse_codepoint_iterator
        rbegin_codepoint() const noexcept {
            return std::reverse_iterator<const_codepoint_iterator>(
                end_codepoint());
        }

        /// \brief Returns a reverse codepoint_iterator to the first character
        /// of the reversed string
        [[nodiscard]] constexpr const_reverse_codepoint_iterator
        crbegin_codepoint() const noexcept {
            return std::reverse_iterator<const_codepoint_iterator>(
                end_codepoint());
        }

        /// \brief Returns a reverse codepoint_iterator to the character
        /// following the last character of the reversed string
        constexpr reverse_codepoint_iterator
        rend_codepoint() noexcept {
            return std::reverse_iterator<codepoint_iterator>(begin_codepoint());
        }

        /// \brief Returns a reverse codepoint_iterator to the character
        /// following the last character of the reversed string
        [[nodiscard]] constexpr const_reverse_codepoint_iterator
        rend_codepoint() const noexcept {
            return std::reverse_iterator<const_codepoint_iterator>(
                begin_codepoint());
        }

        /// \brief Returns a reverse codepoint_iterator to the character
        /// following the last character of the reversed string
        [[nodiscard]] constexpr const_reverse_codepoint_iterator
        crend_codepoint() const noexcept {
            return std::reverse_iterator<const_codepoint_iterator>(
                begin_codepoint());
        }

    public:
        /// \section Capacity
        /// All functions whose name are the same as the basic_string API refer
        /// to code units A number of extra functions are provided deal with
        /// code point capacity These branch into many functions as this can
        /// vary according to our expectations about the size of our future
        /// codepoints.

        /// \brief Checks if the string has no code units or code points
        [[nodiscard]] constexpr bool
        empty() const noexcept {
            return !size_.codeunit_size();
        }

        /// \brief Returns the number of code units in the string
        [[nodiscard]] constexpr size_type
        size() const noexcept {
            return size_.codeunit_size();
        }

        /// \brief Returns the number of code units in the string
        [[nodiscard]] constexpr size_type
        length() const noexcept {
            return size();
        }

        /// \brief Returns the number of codepoints in the string
        [[nodiscard]] constexpr size_type
        size_codepoints() const noexcept {
            return size_.codepoint_size();
        }

        /// \brief Returns the number of codepoints in the string
        [[nodiscard]] constexpr size_type
        length_codepoints() const noexcept {
            return size_codepoints();
        }

        /// \brief Returns the maximum number of elements the string is able to
        /// hold
        [[nodiscard]] constexpr size_type
        max_size() const noexcept {
            return buffer_.max_size() - 1;
        }

        /// \brief Returns the number of code units the string has allocated
        /// space for This assumes the one code unit per code point in the
        /// elements to come In terms of code points, this is an upper bound
        /// capacity
        [[nodiscard]] constexpr size_type
        capacity() const noexcept {
            if constexpr (has_lookup_table) {
                const_lookup_table_type t = const_lookup_table();
                const size_type look_size_in_bytes = lookup_table_type::
                    size_for(buffer_.capacity(), t.size());
                return buffer_.capacity() - null_char_size
                       - static_cast<size_type>(detail::div_ceil(
                           look_size_in_bytes,
                           sizeof(value_type)));
            } else {
                return buffer_.capacity() - null_char_size;
            }
        }

        /// \brief Returns the buffer size, which is different from the capacity
        /// for strings This size is important to safely create string views
        /// that include the lookup table
        [[nodiscard]] constexpr size_type
        buffer_capacity() const {
            return buffer_.size();
        }

        /// \brief Informs a std::basic_string object of a planned change in
        /// size Note that this adjusts for a new expected number of bytes /
        /// code units, not for a new expected number of codepoints or for a new
        /// expected buffer size. \param new_cap Number of bytes we plan to have
        /// in the string \param new_cap Number of multibyte codepoints we plan
        /// to have in the string
        constexpr void
        reserve(size_type new_cap) {
            const_lookup_table_type t = const_lookup_table();
            const size_type cur_n_codepoint = size();
            const size_type cur_n_multibyte = t.size();
            const size_type expected_new_multibyte_size
                = cur_n_codepoint != 0 ?
                      (new_cap * cur_n_multibyte) / cur_n_codepoint :
                      0;
            reserve(new_cap, expected_new_multibyte_size);
        }

        /// \brief Informs a std::basic_string object of a planned change in
        /// size Note that this adjusts for a new expected number of bytes /
        /// code units, not for a new expected number of codepoints or for a new
        /// expected buffer size. \param new_cap Number of code units we plan to
        /// have in the string \param new_multibytes Number of multi-codeunit
        /// codepoints we plan to have in the string
        constexpr void
        reserve(size_type new_cap, size_type new_multibyte_entries_cap) {
            // Trivial case
            if constexpr (has_lookup_table) {
                if (new_cap < capacity()) {
                    const_lookup_table_type t = const_lookup_table();
                    const size_type lookup_table_size = t.size();
                    if (new_multibyte_entries_cap < lookup_table_size) {
                        return;
                    }
                }
            } else {
                if (new_cap < capacity()) {
                    return;
                }
            }

            if constexpr (has_lookup_table) {
                // Get lookup table size (in bytes)
                const_lookup_table_type t = const_lookup_table();
                const size_type lookup_table_size = t.size_for();

                // Estimate a new cap for the lookup table
                const size_type expected_multibyte_size = std::
                    max(t.size(), new_multibyte_entries_cap);
                const size_type expected_lookup_size = const_lookup_table_type::
                    size_for(new_cap, expected_multibyte_size);

                // Reserve memory for these targets
                const size_type old_buffer_size = buffer_.size();
                buffer_.resize(new_cap + 1 + expected_lookup_size);
                buffer_.resize(buffer_.capacity());

                // Move the lookup table to the new begin position
                value_type *new_lookup_begin = buffer_.data() + buffer_.size()
                                               - lookup_table_size;
                value_type *old_lookup_begin = buffer_.data() + old_buffer_size
                                               - lookup_table_size;
                std::memmove(
                    new_lookup_begin,
                    old_lookup_begin,
                    lookup_table_size);
            } else {
                buffer_.resize(new_cap + 1);
                buffer_.resize(buffer_.capacity());
            }
        }

        /// \brief Requests the removal of unused capacity.
        /// Don't use this function if you are planning to insert more elements
        /// to this string
        constexpr void
        shrink_to_fit() {
            if constexpr (has_lookup_table) {
                // Get buffer and lookup table sizes
                const_lookup_table_type t = const_lookup_table();
                const size_type lookup_table_size = t.size_for();

                // Plan the new buffer size
                const size_type new_buffer_size = size_.codeunit_size() + 1
                                                  + lookup_table_size;
                if (new_buffer_size == buffer_.size()) {
                    return;
                }

                // Move the lookup table right after the string buffer
                value_type *old_lookup_begin = buffer_.data() + buffer_.size()
                                               - lookup_table_size;
                value_type *new_lookup_begin = buffer_.data() + size() + 1;
                std::memmove(
                    new_lookup_begin,
                    old_lookup_begin,
                    lookup_table_size);

                // Shrink the buffer size
                buffer_.resize(new_buffer_size);
                buffer_.shrink_to_fit();
            } else {
                buffer_.resize(size_.codeunit_size() + 1);
                buffer_.shrink_to_fit();
            }
        }

    public:
        /// \section Operations

        /// \brief Clear the string
        constexpr void
        clear() {
            buffer_.clear();
            buffer_.resize(2);
            buffer_.resize(buffer_.capacity());
            size_.codeunit_size() = 0;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() = 0;
            }
            buffer_[0] = '\0';
            if constexpr (has_lookup_table) {
                lookup_table_type t = lookup_table();
                t.clear();
            }
        }

        /// \brief Inserts count copies of character ch at the position index
        /// \throws std::out_of_range if index > size()
        template <typename Char>
        constexpr basic_string &
        insert(size_type index, size_type count, Char ch) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            insert(begin() + index, count, ch);
            return *this;
        }

        template <typename Char>
        constexpr basic_string &
        insert(codepoint_index index, size_type count, Char ch) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            codepoint_iterator it = begin_codepoint() + index;
            return insert(it.byte_index(), count, ch);
        }

        /// \brief Inserts null-terminated character string pointed to by s at
        /// the position index. \throws std::out_of_range if index > size() The
        /// length of the string is determined by the first null character using
        /// Traits::length(s)
        template <typename Char>
        constexpr basic_string &
        insert(size_type index, const Char *s) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            insert(begin() + index, s, s + std::char_traits<Char>::length(s));
            return *this;
        }

        template <typename Char>
        constexpr basic_string &
        insert(codepoint_index index, const Char *s) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            insert(
                begin_codepoint() + index,
                s,
                s + std::char_traits<Char>::length(s));
            return *this;
        }

        /// \brief Inserts the characters in the range [s, s+count) at the
        /// position index. The range can contain null characters \throws
        /// std::out_of_range if index > size()
        template <typename Char>
        constexpr basic_string &
        insert(size_type index, const Char *s, size_type count) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            insert(begin() + index, s, s + count);
            return *this;
        }

        template <typename Char>
        constexpr basic_string &
        insert(codepoint_index index, const Char *s, size_type count) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            insert(begin_codepoint() + index, s, s + count);
            return *this;
        }

        /// \brief Inserts string str at the position index
        /// \throws std::out_of_range if index > size()
        constexpr basic_string &
        insert(size_type index, const basic_string &str) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            insert(begin() + index, str.begin(), str.end());
            return *this;
        }

        constexpr basic_string &
        insert(codepoint_index index, const basic_string &str) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            insert(begin_codepoint() + index, str.begin(), str.end());
            return *this;
        }

        /// \brief Inserts a string, obtained by str.substr(index_str, count) at
        /// the position index \throws Throws std::out_of_range if index >
        /// size() or if index_str > str.size()
        constexpr basic_string &
        insert(
            size_type index,
            const basic_string &str,
            size_type index_str,
            size_type count = npos) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            if (index_str > str.size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size()");
            }
            const_iterator str_first = str.begin() + index_str;
            const_iterator str_last = count != npos ? str_first + count :
                                                      str.end();
            insert(begin() + index, str_first, str_last);
            return *this;
        }

        constexpr basic_string &
        insert(
            size_type index,
            const basic_string &str,
            codepoint_index index_str,
            codepoint_index count = codepoint_index(npos)) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            if (index_str > str.size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size_codepoints()");
            }
            const_codepoint_iterator str_codepoint_first = str.begin_codepoint()
                                                           + index_str;
            const_codepoint_iterator str_codepoint_last
                = count != npos ? str_codepoint_first + count :
                                  str.end_codepoint();
            const_iterator str_first = str.begin()
                                       + str_codepoint_first.byte_index();
            const_iterator str_last = str.begin()
                                      + str_codepoint_last.byte_index();
            insert(begin() + index, str_first, str_last);
            return *this;
        }

        constexpr basic_string &
        insert(
            codepoint_index index,
            const basic_string &str,
            size_type index_str,
            size_type count = npos) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            if (index_str > str.size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size()");
            }
            codepoint_iterator this_cp_pos = begin_codepoint() + index;
            const_iterator str_first = str.begin() + index_str;
            const_iterator str_last = count != npos ? str_first + count :
                                                      str.end();
            insert(begin() + this_cp_pos.byte_index(), str_first, str_last);
            return *this;
        }

        constexpr basic_string &
        insert(
            codepoint_index index,
            const basic_string &str,
            codepoint_index index_str,
            codepoint_index count = codepoint_index(npos)) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            if (index_str > str.size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size_codepoints()");
            }
            codepoint_iterator this_cp_pos = begin_codepoint() + index;
            const_codepoint_iterator str_codepoint_first = str.begin_codepoint()
                                                           + index_str;
            const_codepoint_iterator str_codepoint_last
                = count != npos ? str_codepoint_first + count :
                                  str.end_codepoint();
            const_iterator str_first = str.begin()
                                       + str_codepoint_first.byte_index();
            const_iterator str_last = str.begin()
                                      + str_codepoint_last.byte_index();
            insert(begin() + this_cp_pos.byte_index(), str_first, str_last);
            return *this;
        }

        /// \brief Inserts character ch before the character pointed by pos
        /// \return An iterator which refers to the copy of the first inserted
        /// character or pos if no characters were inserted (count==0 or
        /// first==last or ilist.size()==0)
        template <typename Char>
        constexpr iterator
        insert(const_iterator pos, Char ch) {
            return insert(pos, 1, ch);
        }

        template <typename Char>
        constexpr codepoint_iterator
        insert(const_codepoint_iterator pos, Char ch) {
            return insert(pos, 1, ch);
        }

        /// \brief Inserts count copies of character ch before the element (if
        /// any) pointed by pos \return An iterator which refers to the copy of
        /// the first inserted character or pos if no characters were inserted
        /// (count==0 or first==last or ilist.size()==0)
        template <typename Char>
        constexpr iterator
        insert(const_iterator pos, size_type count, Char ch) {
            // Take the pos index as pos itself is about to be invalidated
            const size_type pos_idx = pos - begin();

            // Handle the most trivial case first
            if (count == 0) {
                return begin() + pos_idx;
            }

            // Input properties
            uint8_t codeunits_per_codepoint = detail::utf_size_as<
                value_type>(&ch, 1);
            const size_type count_code_units = count * codeunits_per_codepoint;
            const size_type count_code_points = count;

            // Update buffer size for the new elements
            size_type new_cap = size() + count_code_units;
            lookup_table_type t = lookup_table();
            size_type new_multibyte_cap
                = t.size() + (codeunits_per_codepoint == 1 ? 0 : count);
            reserve(new_cap, new_multibyte_cap);
            buffer_[new_cap] = '\0';

            // Shift existing substring [pos, end] to the right
            iterator new_pos = begin() + pos_idx + count_code_units;
            std::memmove(
                new_pos.base(),
                buffer_.data() + pos_idx,
                size_.codeunit_size() - pos_idx);

            // Update internal size
            size_.codeunit_size() += codeunits_per_codepoint * count;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() += count;
            }

            // Fill the buffer with new elements
            if constexpr (!detail::is_same_utf_encoding_v<value_type, Char>) {
                if (codeunits_per_codepoint == 1) {
                    std::fill(
                        begin() + pos_idx,
                        new_pos,
                        static_cast<value_type>(ch));
                } else {
                    detail::to_utf(
                        &ch,
                        1,
                        buffer_.data() + pos_idx,
                        codeunits_per_codepoint);
                    size_type n_to_copy = count_code_points;
                    for (value_type *it = buffer_.data() + pos_idx;
                         --n_to_copy > 0;) {
                        std::memcpy(
                            it += codeunits_per_codepoint,
                            buffer_.data() + pos_idx,
                            codeunits_per_codepoint);
                    }
                }
            } else {
                std::memset(buffer_.data() + pos_idx, ch, count);
            }

            // Update the lookup table for the range [pos,end()]
            if constexpr (has_lookup_table) {
                index_multibyte_codepoints(begin() + pos_idx, end());
            }

            // Return position where elements where inserted
            return begin() + pos_idx;
        }

        template <typename Char>
        constexpr codepoint_iterator
        insert(const_codepoint_iterator pos, size_type count, Char ch) {
            size_type pos_offset = pos.index();
            if (pos.index() > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: pos.index() > size_codepoints()");
            }
            insert(begin() + pos.byte_index(), count, ch);
            return begin_codepoint() + pos_offset;
        }

        /// \brief Inserts characters from the range [first, last) before the
        /// element (if any) pointed by pos. This overload does not participate
        /// in overload resolution if InputIt does not satisfy
        /// LegacyInputIterator \return An iterator which refers to the copy of
        /// the first inserted character or pos if no characters were inserted
        /// (count==0 or first==last or ilist.size()==0)
        template <class InputIt>
        constexpr iterator
        insert(const_iterator pos, InputIt first, InputIt last) {
            // Take the pos index as pos itself is about to be invalidated
            const size_type pos_idx = pos - begin();

            // Handle the most trivial case first
            if (first == last) {
                return begin() + pos_idx;
            }

            // Handle input iterators
            using category = typename std::iterator_traits<
                InputIt>::iterator_category;
            if constexpr (std::is_same_v<category, std::input_iterator_tag>) {
                while (first != last) {
                    pos = insert(pos, 1, *first++);
                    ++pos;
                }
                return begin() + pos_idx;
            }

            // Input properties
            using input_value_type = typename std::iterator_traits<
                InputIt>::value_type;
            size_type code_points = 0;
            size_type output_code_units = 0;
            size_type output_multibytes = 0;
            auto it = first;
            while (it != last) {
                const size_type bytes_left = last - it;
                const uint8_t it_input_code_units = detail::
                    utf_size(*it, bytes_left);
                const uint8_t it_output_code_units = detail::utf_size_as<
                    value_type>(it, bytes_left);
                output_code_units += it_output_code_units;
                it += it_input_code_units;
                if (it_output_code_units > 1) {
                    ++output_multibytes;
                }
                ++code_points;
            }

            // Update buffer size for the new elements
            size_type new_cap = size() + output_code_units;
            lookup_table_type t = lookup_table();
            size_type new_multibyte_cap = t.size() + output_multibytes;
            reserve(new_cap, new_multibyte_cap);
            buffer_[new_cap] = '\0';

            // Shift existing substring [pos, end] to the right
            const size_type new_pos_idx = pos_idx + output_code_units;
            iterator new_pos = begin() + new_pos_idx;
            std::memmove(
                new_pos.base(),
                buffer_.data() + pos_idx,
                size_.codeunit_size() - pos_idx);

            // Update internal size
            size_.codeunit_size() += output_code_units;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() += code_points;
            }

            // Fill the buffer with new elements
            if constexpr (!detail::is_same_utf_encoding_v<
                              value_type,
                              input_value_type>) {
                auto from_it = first;
                auto to_it = begin() + pos_idx;
                while (from_it != last) {
                    uint8_t input_codepoint_size = detail::
                        utf_size(*from_it, last - from_it);
                    uint8_t output_codepoint_size = detail::utf_size_as<
                        value_type>(from_it, last - from_it);
                    detail::to_utf(
                        from_it,
                        input_codepoint_size,
                        to_it.base(),
                        output_codepoint_size);
                    from_it += input_codepoint_size;
                    to_it += output_codepoint_size;
                }
            } else {
                std::copy(first, last, buffer_.data() + pos_idx);
            }

            // Update the lookup table for the range [pos,end()]
            if constexpr (has_lookup_table) {
                index_multibyte_codepoints(begin() + pos_idx, end());
            }

            // Return position where elements where inserted
            return begin() + pos_idx;
        }

        template <class InputIt>
        constexpr codepoint_iterator
        insert(const_codepoint_iterator pos, InputIt first, InputIt last) {
            const size_type pos_offset = pos.index();
            if (pos > end_codepoint()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: pos > end_codepoint()");
            }
            insert(begin() + pos.byte_index(), first, last);
            return begin_codepoint() + pos_offset;
        }

        /// \brief Inserts elements from initializer list ilist before the
        /// element (if any) pointed by pos \return An iterator which refers to
        /// the copy of the first inserted character or pos if no characters
        /// were inserted (count==0 or first==last or ilist.size()==0)
        template <typename Char>
        constexpr iterator
        insert(const_iterator pos, std::initializer_list<Char> ilist) {
            return insert(pos, ilist.begin(), ilist.end());
        }

        template <typename Char>
        constexpr codepoint_iterator
        insert(const_codepoint_iterator pos, std::initializer_list<Char> ilist) {
            const size_type pos_offset = pos.index();
            insert(begin() + pos.byte_index(), ilist.begin(), ilist.end());
            return begin_codepoint() + pos_offset;
        }

        /// \brief Convert to string view and insert
        /// Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<CharT, Traits> sv = t;, then inserts the
        /// elements from sv before the element (if any) pointed by pos, as if
        /// by insert(pos, sv.data(), sv.size()). This overload participates in
        /// overload resolution only if std::is_convertible_v<const T&,
        /// std::basic_string_view<CharT, Traits>> is true and
        /// std::is_convertible_v<const T&, const CharT*> is false \throws
        /// std::out_of_range if index > size()
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        insert(size_type pos, const T &t) {
            std::basic_string_view<
                typename T::value_type,
                std::char_traits<typename T::value_type>>
                sv(t);
            insert(begin() + pos, sv.begin(), sv.end());
            return *this;
        }

        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        insert(codepoint_index pos, const T &t) {
            std::basic_string_view<
                typename T::value_type,
                std::char_traits<typename T::value_type>>
                sv(t);
            insert(begin_codepoint() + pos, sv.begin(), sv.end());
            return *this;
        }

        /// \brief Convert to string view and insert
        /// Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<CharT, Traits> sv = t;, then inserts, before
        /// the element (if any) pointed by pos, the characters from the subview
        /// [index_str, index_str+count) of sv. If the requested subview lasts
        /// past the end of sv, or if count == npos, the resulting subview is
        /// [index_str, sv.size()). If index_str > sv.size(), or if index >
        /// size(), std::out_of_range is thrown. This overload participates in
        /// overload resolution only if std::is_convertible_v<const T&,
        /// std::basic_string_view<CharT, Traits>> is true and
        /// std::is_convertible_v<const T&, const CharT*> is false
        /// \throws std::out_of_range if index > size() or if index_str >
        /// sv.size()
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        insert(
            size_type index,
            const T &t,
            size_type index_str,
            size_type count = npos) {
            std::basic_string_view<
                typename T::value_type,
                std::char_traits<typename T::value_type>>
                sv(t);
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size()");
            }
            if (index_str > sv.size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size()");
            }
            auto substr = sv.substr(index_str, count);
            insert(begin() + index, substr.begin(), substr.end());
            return *this;
        }

        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        insert(
            codepoint_index index,
            const T &t,
            size_type index_str,
            size_type count = npos) {
            std::basic_string_view<
                typename T::value_type,
                std::char_traits<typename T::value_type>>
                sv(t);
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index > size_codepoints()");
            }
            if (index_str > sv.size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::insert: index_str > str.size()");
            }
            auto substr = sv.substr(index_str, count);
            insert(begin_codepoint() + index, substr.begin(), substr.end());
            return *this;
        }

        /// \brief Removes min(count, size() - index) characters starting at
        /// index. \param index first character to remove \param count number of
        /// characters to remove \return *this \throws std::out_of_range if
        /// index > size()
        constexpr basic_string &
        erase(size_type index = 0, size_type count = npos) {
            if (index > size()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::erase: index > size()");
            }
            iterator first = begin() + index;
            iterator last = first + (std::min)(count, size() - index);
            erase(first, last);
            return *this;
        }

        constexpr basic_string &
        erase(
            codepoint_index index = 0,
            codepoint_index count = codepoint_index(npos)) {
            if (index > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "basic_string::erase: index > size_codepoints()");
            }
            codepoint_iterator first_codepoint = begin_codepoint() + index;
            size_type count_codepoint = std::
                min(count.value_of(), size_codepoints() - index);
            codepoint_iterator last_codepoint = first_codepoint
                                                + count_codepoint;
            iterator first = begin() + first_codepoint.byte_index();
            iterator last = begin() + last_codepoint.byte_index();
            erase(first, last);
            return *this;
        }

        /// \brief Removes the character at position.
        /// \param position iterator to the character to remove
        /// \return iterator pointing to the character immediately following the
        /// character erased, or end() if no such character exists
        constexpr iterator
        erase(const_iterator position) {
            return erase(position, std::next(position));
        }

        constexpr codepoint_iterator
        erase(const_codepoint_iterator position) {
            return erase(position, std::next(position));
        }

        /// \brief Removes the characters in the range [first, last).
        /// \param first range of the characters to remove
        /// \param last range of the characters to remove
        /// \return iterator pointing to the character last pointed to before
        /// the erase, or end() if no such character exists
        constexpr iterator
        erase(const_iterator first, const_iterator last) {
            // Get offsets
            const size_type first_offset = first - cbegin();
            const size_type last_offset = last - cbegin();

            // Handle most trivial case
            if (first_offset >= last_offset) {
                return begin() + first_offset;
            }

            // Shift buffer elements to the left
            const size_type count_codeunit = last_offset - first_offset;
            size_type count_codepoint = count_codeunit;
            if constexpr (store_codepoint_size) {
                count_codepoint = size_codepoints(first, last);
            }
            value_type *new_code_unit_begin = buffer_.data() + first_offset;
            value_type *code_unit_end_pos = buffer_.data() + size();
            detail::shift::shift_left(
                new_code_unit_begin,
                code_unit_end_pos,
                count_codeunit);

            // Set null char
            value_type *new_code_unit_end_pos = code_unit_end_pos
                                                - count_codeunit;
            *new_code_unit_end_pos = '\0';

            // Update internal size
            size_.codeunit_size() = size() - count_codeunit;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() = size() - count_codepoint;
            }

            // Update lookup table
            if constexpr (has_lookup_table) {
                // Reindex everything in the [first, end()) range
                // as all multibytes in that range will now point to a
                // difference string index
                index_multibyte_codepoints(first, end());
            }

            return begin() + first_offset;
        }

        constexpr codepoint_iterator
        erase(const_codepoint_iterator first, const_codepoint_iterator last) {
            const size_type first_offset = first - begin_codepoint();
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            erase(first_code_unit, last_code_unit);
            return begin_codepoint() + first_offset;
        }

        /// \brief Appends the given character ch to the end of the string.
        /// \param ch 	the character to append
        template <typename Char>
        constexpr void
        push_back(Char ch) {
            insert(end(), 1, ch);
        }

        /// \brief Removes the last character from the string.
        /// Equivalent to erase(end()-1). The behavior is undefined if the
        /// string is empty.
        constexpr void
        pop_back() {
            erase(end() - 1);
        }

        constexpr void
        pop_back_codepoint() {
            erase(end_codepoint() - 1);
        }

        /// \brief Appends count copies of character ch
        template <typename Char>
        constexpr basic_string &
        append(size_type count, Char ch) {
            insert(end(), count, ch);
            return *this;
        }

        /// \brief Appends string str
        constexpr basic_string &
        append(const basic_string &str) {
            insert(end(), str.begin(), str.end());
            return *this;
        }

        /// \brief Appends a substring [pos, pos+count) of str
        /// If the requested substring lasts past the end of the string, or if
        /// count == npos, the appended substring is [pos, size()). \throws If
        /// pos > str.size(), std::out_of_range is thrown.
        constexpr basic_string &
        append(const basic_string &str, size_type pos, size_type count = npos) {
            insert(
                end(),
                str.begin() + pos,
                str.begin() + pos + (std::min)(count, str.size() - pos));
            return *this;
        }

        constexpr basic_string &
        append(
            const basic_string &str,
            codepoint_index pos,
            codepoint_index count = codepoint_index(npos)) {
            const_codepoint_iterator first_codepoint = str.cbegin_codepoint()
                                                       + pos;
            const_codepoint_iterator last_codepoint
                = first_codepoint
                  + (std::min)(count.value_of(), str.size_codepoints() - pos);
            insert(
                end(),
                str.cbegin() + first_codepoint.byte_index(),
                str.cbegin() + last_codepoint.byte_index());
            return *this;
        }

        /// \brief Appends characters in the range [s, s + count).
        /// This range can contain null characters
        template <typename Char>
        constexpr basic_string &
        append(const Char *s, size_type count) {
            insert(end(), s, s + count);
            return *this;
        }

        /// \brief Appends the null-terminated character string pointed to by s
        /// The length of the string is determined by the first null character
        /// using Traits::length(s)
        template <typename Char>
        constexpr basic_string &
        append(const Char *s) {
            insert(end(), s, s + std::char_traits<Char>::length(s));
            return *this;
        }

        /// \brief Appends characters in the range [first, last).
        /// This overload has the same effect as overload (1) if InputIt is an
        /// integral type This overload only participates in overload resolution
        /// if InputIt qualifies as an LegacyInputIterator
        template <class InputIt>
        constexpr basic_string &
        append(InputIt first, InputIt last) {
            insert(end(), first, last);
            return *this;
        }

        /// \brief Appends characters from the initializer list ilist
        template <typename Char>
        constexpr basic_string &
        append(std::initializer_list<Char> ilist) {
            insert(end(), ilist);
            return *this;
        }

        /// \brief Appends characters from the a string view
        /// Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<CharT, Traits> sv = t;, then appends all
        /// characters from sv as if by append(sv.data(), sv.size()). This
        /// overload participates in overload resolution only if
        /// std::is_convertible_v<const T&, std::basic_string_view<CharT,
        /// Traits>> is true and std::is_convertible_v<const T&, const CharT*>
        /// is false.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        append(const T &t) {
            insert(size(), t);
            return *this;
        }

        /// \brief Appends characters from the a string view
        /// Implicitly converts t to a string view sv as if by
        /// std::basic_string_view<CharT, Traits> sv = t;, then appends the
        /// characters from the subview [pos, pos+count) of sv. If the requested
        /// subview extends past the end of sv, or if count == npos, the
        /// appended subview is [pos, sv.size()). If pos >= sv.size(),
        /// std::out_of_range is thrown. This overload participates in overload
        /// resolution only if std::is_convertible_v<const T&,
        /// std::basic_string_view<CharT, Traits>> is true and
        /// std::is_convertible_v<const T&, const CharT*> is false
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        append(const T &t, size_type pos, size_type count = npos) {
            insert(size(), t, pos, count);
            return *this;
        }

        /// \brief Appends string str
        constexpr basic_string &
        operator+=(const basic_string &str) {
            return append(str);
        }

        /// \brief Appends character ch
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        operator+=(Char ch) {
            return append(1, ch);
        }

        /// \brief Appends the null-terminated character string pointed to by s.
        template <typename Char>
        constexpr basic_string &
        operator+=(const Char *s) {
            return append(s);
        }

        /// \brief Appends characters in the initializer list ilist.
        template <typename Char>
        constexpr basic_string &
        operator+=(std::initializer_list<Char> ilist) {
            return append(ilist);
        }

        /// \brief Appends characters in the (convertible to) string view t.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        operator+=(const T &t) {
            return append(t);
        }

        /// \brief Checks if the string begins with a string view sv
        /// This functions cannot be implemented as string_view::starts_with
        /// because it might require conversions from code points to code units
        template <typename RhsChar, typename RhsTraits>
        constexpr bool
        starts_with(
            std::basic_string_view<RhsChar, RhsTraits> sv) const noexcept {
            if (empty()) {
                return sv.empty();
            }
            if (sv.empty()) {
                return true;
            }
            if constexpr (!detail::is_same_utf_encoding_v<value_type, RhsChar>)
            {
                auto this_first = begin();
                auto this_last = end();
                auto other_first = sv.begin();
                auto other_last = sv.end();
                while (this_first != this_last && other_first != other_last) {
                    const uint8_t buf_size = detail::utf_size_as<
                        value_type>(other_first, other_last - other_first);
                    value_type buf[8];
                    detail::to_utf(
                        other_first,
                        other_last - other_first,
                        buf,
                        buf_size);
                    const size_type this_code_units_left = this_last
                                                           - this_first;
                    if (this_code_units_left < buf_size) {
                        return false;
                    }
                    if (!std::equal(
                            this_first,
                            this_first + buf_size,
                            buf,
                            buf + buf_size)) {
                        return false;
                    }
                    other_first += detail::
                        utf_size(*other_first, other_last - other_first);
                    this_first += buf_size;
                }
                return !*other_first;
            } else {
                if (size() < sv.size()) {
                    return false;
                }
                return std::
                    equal(begin(), begin() + sv.size(), sv.begin(), sv.end());
            }
        }

        /// \brief Checks if the string begins with a single character c.
        /// This functions cannot be implemented as string_view::starts_with
        /// because it might require conversions from code points to code units
        template <typename Char>
        constexpr bool
        starts_with(Char c) const noexcept {
            if (empty()) {
                return false;
            }
            if constexpr (!detail::is_same_utf_encoding_v<Char, value_type>) {
                const uint8_t s = detail::utf_size_as<value_type>(&c, 1);
                value_type buf[8];
                detail::to_utf(&c, 1, buf, s);
                return size() >= s
                       && std::equal(begin(), begin() + s, buf, buf + s);
            } else {
                return front() == c;
            }
        }

        /// \brief Checks if the string begins with a null-terminated character
        /// string s. This functions cannot be implemented as
        /// string_view::starts_with because it might require conversions from
        /// code points to code units
        template <typename Char>
        constexpr bool
        starts_with(const Char *s) const {
            if (empty()) {
                return !*s;
            }
            if (!*s) {
                return true;
            }
            if constexpr (!detail::is_same_utf_encoding_v<value_type, Char>) {
                auto this_first = begin();
                auto this_last = end();
                auto other_first = s;
                auto other_last = s + std::char_traits<Char>::length(s);
                while (this_first != this_last && *other_first) {
                    const size_type other_values_left = other_last - other_first;
                    const uint8_t buf_size = detail::utf_size_as<
                        value_type>(other_first, other_values_left);
                    value_type buf[8];
                    detail::
                        to_utf(other_first, other_values_left, buf, buf_size);
                    const size_type this_bytes_left = this_last - this_first;
                    if (this_bytes_left < buf_size) {
                        return false;
                    }
                    if (!std::equal(
                            this_first,
                            this_first + buf_size,
                            buf,
                            buf + buf_size)) {
                        return false;
                    }
                    other_first += detail::
                        utf_size(*other_first, other_values_left);
                    this_first += buf_size;
                }
                return !*other_first;
            } else {
                const size_type other_size = std::char_traits<Char>::length(s);
                if (size() < other_size) {
                    return false;
                }
                return std::
                    equal(begin(), begin() + other_size, s, s + other_size);
            }
        }

        /// \brief Checks if the string end with a string view sv
        /// This functions cannot be implemented as string_view::ends_with
        /// because it might require conversions from code points to code units
        template <typename RhsChar, typename RhsTraits>
        constexpr bool
        ends_with(
            std::basic_string_view<RhsChar, RhsTraits> sv) const noexcept {
            if (empty()) {
                return sv.empty();
            }
            if (sv.empty()) {
                return true;
            }
            if constexpr (!detail::is_same_utf_encoding_v<value_type, RhsChar>)
            {
                // Count code points in other
                auto it_other = sv.begin();
                size_type other_code_points = 0;
                while (it_other != sv.end()) {
                    auto it_cp_size = detail::
                        utf_size(*it_other, sv.end() - it_other);
                    other_code_points += it_cp_size;
                    it_other += it_cp_size;
                }
                // Compare with last code points in this
                if (size_codepoints() < other_code_points) {
                    return false;
                }
                auto this_first = end_codepoint() - other_code_points;
                auto this_last = end_codepoint();
                auto other_first = sv.begin();
                auto other_last = sv.end();
                while (this_first != this_last && other_first != other_last) {
                    // Find other code point
                    auto other_entries_left = sv.end() - other_first;
                    auto other_cp_size = detail::
                        utf_size(*other_first, other_entries_left);
                    // Convert other codepoint to utf32
                    detail::utf32_char_type r{};
                    auto ok = detail::
                        to_utf32(other_first, other_cp_size, &r, 1);
                    if (!ok) {
                        break;
                    }
                    // Compare code points
                    if (*this_first != r) {
                        return false;
                    }
                    // Advance iterators
                    ++this_first;
                    other_first += other_cp_size;
                }
                return other_first == other_last;
            } else {
                if (size() < sv.size()) {
                    return false;
                }
                return std::equal(
                    rbegin(),
                    rbegin() + sv.size(),
                    sv.rbegin(),
                    sv.rend());
            }
        }

        /// \brief Checks if the string end with a single character c.
        /// This functions cannot be implemented as string_view::ends_with
        /// because it might require conversions from code points to code units
        template <typename Char>
        constexpr bool
        ends_with(Char c) const noexcept {
            if (empty()) {
                return false;
            }
            if constexpr (!detail::is_same_utf_encoding_v<Char, value_type>) {
                return *rbegin_codepoint() == c;
            } else {
                return back() == c;
            }
        }

        /// \brief Checks if the string end with a null-terminated character
        /// string s. This functions cannot be implemented as
        /// string_view::ends_with because it might require conversions from
        /// code points to code units
        template <typename Char>
        constexpr bool
        ends_with(const Char *s) const {
            std::basic_string_view<Char>
                sv(s, std::char_traits<Char>::length(s));
            return ends_with(sv);
        }

        /// \brief Checks if the string contains with a string view s as
        /// substring This functions cannot be implemented as
        /// string_view::contains because it might require conversions from code
        /// points to code units
        template <typename RhsChar, typename RhsTraits>
        constexpr bool
        contains(std::basic_string_view<RhsChar, RhsTraits> sv) const noexcept {
            return find(sv) != npos;
        }

        /// \brief Checks if the string contains with a single character c as
        /// substring This functions cannot be implemented as
        /// string_view::contains because it might require conversions from code
        /// points to code units
        template <typename Char>
        constexpr bool
        contains(Char c) const noexcept {
            return find(c) != npos;
        }

        /// \brief Checks if the string contains with a null-terminated
        /// character string s as substring This functions cannot be implemented
        /// as string_view::contains because it might require conversions from
        /// code points to code units
        template <typename Char>
        constexpr bool
        contains(const Char *s) const {
            return find(s) != npos;
        }

        /// \brief Replaces the part of the string with str
        constexpr basic_string &
        replace(
            const_iterator first,
            const_iterator last,
            const basic_string &str) {
            return replace(first, last, str.begin(), str.end());
        }

        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            const basic_string &str) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                str.begin(),
                str.end());
        }

        /// \brief Replaces the part of the string with a substr of str
        constexpr basic_string &
        replace(
            size_type pos,
            size_type count,
            const basic_string &str,
            size_type pos2,
            size_type count2 = npos) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            if (pos2 > str.size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos2 > str.size()");
            }
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                str.begin() + pos2,
                str.begin() + pos2 + (std::min)(count2, str.size() - pos2));
        }

        constexpr basic_string &
        replace(
            size_type pos,
            size_type count,
            const basic_string &str,
            codepoint_index pos2,
            codepoint_index count2 = codepoint_index(npos)) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            if (pos2 > str.size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos2 > str.size_codepoints()");
            }
            const_codepoint_iterator other_first_code_point
                = str.begin_codepoint() + pos2;
            const_codepoint_iterator other_last_code_point
                = other_first_code_point
                  + (std::min)(count2.value_of(), str.size_codepoints() - pos2);
            const_iterator other_first_code_unit
                = str.begin() + other_first_code_point.byte_index();
            const_iterator other_last_code_unit
                = str.begin() + other_last_code_point.byte_index();
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                other_first_code_unit,
                other_last_code_unit);
        }

        constexpr basic_string &
        replace(
            codepoint_index pos,
            codepoint_index count,
            const basic_string &str,
            size_type pos2,
            size_type count2 = npos) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            if (pos2 > str.size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos2 > str.size()");
            }
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + (std::min)(count.value_of(), size_codepoints() - pos);
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                str.begin() + pos2,
                str.begin() + pos2 + (std::min)(count2, str.size() - pos2));
        }

        constexpr basic_string &
        replace(
            codepoint_index pos,
            codepoint_index count,
            const basic_string &str,
            codepoint_index pos2,
            codepoint_index count2 = codepoint_index(npos)) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            if (pos2 > str.size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos2 > str.size_codepoints()");
            }
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + (std::min)(count.value_of(), size_codepoints() - pos);
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            const_codepoint_iterator other_first_code_point
                = str.begin_codepoint() + pos;
            const_codepoint_iterator other_last_code_point
                = str.begin_codepoint() + pos
                  + (std::min)(count2.value_of(), str.size_codepoints() - pos2);
            const_iterator other_first_code_unit
                = str.begin() + other_first_code_point.byte_index();
            const_iterator other_last_code_unit
                = str.begin() + other_last_code_point.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                other_first_code_unit,
                other_last_code_unit);
        }

        /// \brief Replaces the part of the string with the range [first2,last2)
        template <class InputIt>
        constexpr basic_string &
        replace(
            const_iterator first,
            const_iterator last,
            InputIt first2,
            InputIt last2) {
            // The general case involves erasing and inserting
            // However, we still need to implement the more specific case where
            // the ranges have the same size If that's the case, we can avoid
            // allocating/deallocating space for elements However, we still need
            // to check if they have the same multibyte pattern because,
            // otherwise, we still need to update the lookup table.
            constexpr bool is_input_iterator = std::is_same_v<
                typename std::iterator_traits<InputIt>::iterator_category,
                std::input_iterator_tag>;
            using input_value_type = typename std::iterator_traits<
                InputIt>::iterator_category;
            constexpr bool input_is_same_size = sizeof(value_type)
                                                == sizeof(input_value_type);
            if constexpr (is_input_iterator || !input_is_same_size) {
                iterator new_first = erase(first, last);
                insert(new_first, first2, last2);
                return *this;
            }

            if constexpr (input_is_same_size) {
                // Distance might be the same
                auto d1 = std::distance(first, last);
                auto d2 = std::distance(first2, last2);
                if (d1 == d2) {
                    const size_type prev_codepoint
                        = size_codepoints(first, last);
                    size_type first_offset = first - begin();
                    std::transform(
                        first2,
                        last2,
                        begin() + first_offset,
                        [](auto ch) { return static_cast<value_type>(ch); });
                    const size_type new_codepoint = size_codepoints(first, last);
                    if constexpr (has_lookup_table) {
                        const bool prev_had_multibyte = detail::
                            cmp_not_equal(prev_codepoint, d1);
                        const bool new_has_multibyte = detail::
                            cmp_not_equal(new_codepoint, d1);
                        if (prev_had_multibyte || new_has_multibyte) {
                            index_multibyte_codepoints(first, end());
                        }
                    }
                    return *this;
                }
            }

            // Last option
            iterator new_first = erase(first, last);
            insert(new_first, first2, last2);
            return *this;
        }

        template <class InputIt>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            InputIt first2,
            InputIt last2) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(first_code_unit, last_code_unit, first2, last2);
        }

        /// \brief Replaces the part of the string with the range [cstr,
        /// cstr+count2)
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        replace(
            size_type pos,
            size_type count,
            const Char *cstr,
            size_type count2) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                cstr,
                cstr + count2);
        }

        template <typename Char>
        constexpr basic_string &
        replace(
            codepoint_index pos,
            codepoint_index count,
            const Char *cstr,
            size_type count2) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(first_code_unit, last_code_unit, cstr, cstr + count2);
        }

        /// \brief Replaces the part of the string with the range [cstr,
        /// cstr+count2)
        template <typename Char>
        constexpr basic_string &
        replace(
            const_iterator first,
            const_iterator last,
            const Char *cstr,
            size_type count2) {
            return replace(first, last, cstr, cstr + count2);
        }

        template <typename Char>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            const Char *cstr,
            size_type count2) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(first_code_unit, last_code_unit, cstr, cstr + count2);
        }

        /// \brief Replaces the part of the string with cstr
        template <typename Char>
        constexpr basic_string &
        replace(size_type pos, size_type count, const Char *cstr) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - count),
                cstr,
                std::char_traits<Char>::length(cstr));
        }

        template <typename Char>
        constexpr basic_string &
        replace(codepoint_index pos, codepoint_index count, const Char *cstr) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                cstr,
                std::char_traits<Char>::length(cstr));
        }

        /// \brief Replaces the part of the string with cstr
        template <typename Char>
        constexpr basic_string &
        replace(const_iterator first, const_iterator last, const Char *cstr) {
            return replace(
                first,
                last,
                cstr,
                std::char_traits<Char>::length(cstr));
        }

        template <typename Char>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            const Char *cstr) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                cstr,
                std::char_traits<Char>::length(cstr));
        }

        /// \brief Replaces the part of the string with count2 copies of ch
        template <typename Char>
        constexpr basic_string &
        replace(size_type pos, size_type count, size_type count2, Char ch) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                count2,
                ch);
        }

        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        replace(
            codepoint_index pos,
            codepoint_index count,
            size_type count2,
            Char ch) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(first_code_unit, last_code_unit, count2, ch);
        }

        /// \brief Replaces the part of the string with count2 copies of ch
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        replace(
            const_iterator first,
            const_iterator last,
            size_type count2,
            Char ch) {
            iterator new_first = erase(first, last);
            insert(new_first, count2, ch);
            return *this;
        }

        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            size_type count2,
            Char ch) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(first_code_unit, last_code_unit, count2, ch);
        }

        /// \brief Replaces the part of the string with characters in ilist
        template <typename Char>
        constexpr basic_string &
        replace(
            const_iterator first,
            const_iterator last,
            std::initializer_list<Char> ilist) {
            return replace(first, last, ilist.begin(), ilist.end());
        }

        template <typename Char>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            std::initializer_list<Char> ilist) {
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(
                first_code_unit,
                last_code_unit,
                ilist.begin(),
                ilist.end());
        }

        /// \brief Replaces the part of the string with (convertible to) string
        /// view t
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(size_type pos, size_type count, const T &t) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            std::basic_string_view sv(t);
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                sv.begin(),
                sv.end());
        }

        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(codepoint_index pos, codepoint_index count, const T &t) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            std::basic_string_view sv(t);
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(first_code_unit, last_code_unit, sv.begin(), sv.end());
        }

        /// \brief Replaces the part of the string with (convertible to) string
        /// view t
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(const_iterator first, const_iterator last, const T &t) {
            std::basic_string_view sv(t);
            return replace(first, last, sv.begin(), sv.end());
        }

        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(
            const_codepoint_iterator first,
            const_codepoint_iterator last,
            const T &t) {
            std::basic_string_view sv(t);
            iterator first_code_unit = begin() + first.byte_index();
            iterator last_code_unit = begin() + last.byte_index();
            return replace(first_code_unit, last_code_unit, sv.begin(), sv.end());
        }

        /// \brief Replaces the part of the string with substr of (convertible
        /// to) string view t
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(
            size_type pos,
            size_type count,
            const T &t,
            size_type pos2,
            size_type count2 = npos) {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size()");
            }
            std::basic_string_view sv(t);
            sv = sv.substr(pos2, count2);
            return replace(
                begin() + pos,
                begin() + pos + (std::min)(count, size() - pos),
                sv.begin(),
                sv.end());
        }

        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr basic_string &
        replace(
            codepoint_index pos,
            codepoint_index count,
            const T &t,
            size_type pos2,
            size_type count2 = npos) {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::replace: pos > size_codepoints()");
            }
            std::basic_string_view sv(t);
            sv = sv.substr(pos2, count2);
            codepoint_iterator first_code_point = begin_codepoint() + pos;
            codepoint_iterator last_code_point
                = begin_codepoint() + pos
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            iterator first_code_unit = begin() + first_code_point.byte_index();
            iterator last_code_unit = begin() + last_code_point.byte_index();
            return replace(first_code_unit, last_code_unit, sv.begin(), sv.end());
        }

        /// \brief Returns a substring [pos, pos+count)
        /// If the requested substring extends past the end of the string, i.e.
        /// the count is greater than size() - pos (e.g. if count == npos), the
        /// returned substring is [pos, size()) The returned string is
        /// constructed as if by basic_string(data()+pos, count), which implies
        /// that the returned string's allocator will be default-constructed —
        /// the new allocator might not be a copy of this->get_allocator()
        [[nodiscard]] constexpr basic_string
        substr(size_type pos = 0, size_type count = npos) const {
            return basic_string(data() + pos, count);
        }

        [[nodiscard]] constexpr basic_string
        substr(
            codepoint_index pos = 0,
            codepoint_index count = codepoint_index(npos)) const {
            const_codepoint_iterator first_code_point = begin_codepoint() + pos;
            const_codepoint_iterator last_code_point
                = first_code_point
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            return basic_string(
                data() + first_code_point.byte_index(),
                data() + last_code_point.byte_index());
        }

        /// \brief Copies a substring [pos, pos+count) to character string
        /// pointed to by dest. If the requested substring lasts past the end of
        /// the string, or if count == npos, the copied substring is [pos,
        /// size()). The resulting character string is not null-terminated.
        /// \throws If pos > size(), std::out_of_range is thrown
        template <typename Char>
        constexpr size_type
        copy(Char *dest, size_type count, size_type pos = 0) const {
            if (pos > size()) {
                detail::throw_exception<std::out_of_range>(
                    "string::copy: pos > size()");
            }
            const_iterator first = begin() + pos;
            const_iterator last = first + (std::min)(count, size() - pos);
            if constexpr (sizeof(Char) == sizeof(value_type)) {
                auto last_copied = std::copy(first, last, dest);
                return last_copied - dest;
            } else {
                const_codepoint_iterator first_cp = find_codepoint(
                    first - begin());
                const_codepoint_iterator last_cp = find_codepoint(
                    last - begin());
                auto last_copied = std::copy(first_cp, last_cp, dest);
                return last_copied - dest;
            }
        }

        template <typename Char>
        constexpr size_type
        copy(
            Char *dest,
            codepoint_index count,
            codepoint_index pos = codepoint_index(0)) const {
            if (pos > size_codepoints()) {
                detail::throw_exception<std::out_of_range>(
                    "string::copy: pos > size_codepoints()");
            }
            const const_codepoint_iterator first_cp = begin_codepoint() + pos;
            const const_codepoint_iterator last_cp
                = first_cp
                  + std::
                      min(count.value_of(), size_codepoints() - pos.value_of());
            const const_iterator first = find_codeunit(
                codepoint_index(first_cp - begin_codepoint()));
            const const_iterator last = find_codeunit(
                codepoint_index(last_cp - begin_codepoint()));
            const size_type first_offset = first - begin();
            const size_type last_offset = last - begin();
            const size_type code_unit_count = last_offset - first_offset;
            return copy(dest, code_unit_count, first_offset);
        }

        /// \brief Resizes the string to contain count code units
        constexpr void
        resize(size_type count) {
            resize(count, '\0');
        }

        constexpr void
        resize(codepoint_index count) {
            resize(count, '\0');
        }

        /// \brief Resizes the string to contain count characters
        /// \note This can lead to malformed utf strings.
        template <typename Char>
        constexpr void
        resize(size_type count, Char ch) {
            if (count < size()) {
                downsize(count);
            } else if (count > size()) {
                upsize(count, ch);
            }
        }

        /// \brief Resizes the string to contain count code points
        template <typename Char>
        constexpr void
        resize(codepoint_index count, Char ch) {
            if (count < size_codepoints()) {
                codepoint_iterator it = begin_codepoint() + count;
                size_type codeunit_count = it.byte_index();
                resize(codeunit_count, ch);
                downsize(count);
            } else if (count > size_codepoints()) {
                size_type codepoint_increase = count - size_codepoints();
                uint8_t code_units_per_code_point = detail::utf_size_as<
                    value_type>(&ch, 1);
                size_type code_unit_increase = codepoint_increase
                                               * code_units_per_code_point;
                upsize(size() + code_unit_increase, ch);
            }
        }

        /// \brief Exchanges the contents of the string with those of other
        /// The behavior is undefined if Allocator does not propagate on swap
        /// and the allocators of *this and other are unequal
        constexpr void
        swap(basic_string &other) {
            std::swap(buffer_, other.buffer_);
            std::swap(size_, other.size_);
        }

    public:
        /// \section Search

        /// \brief Find a code unit iterator for a given codepoint
        [[nodiscard]] iterator
        find_codeunit(codepoint_index index) {
            // Find codepoint
            const_codepoint_iterator index_it = begin_codepoint() + index;
            // Add its index offset to begin()
            return begin() + index_it.byte_index();
        }

        [[nodiscard]] const_iterator
        find_codeunit(codepoint_index index) const {
            return (const_cast<basic_string *>(this))->find_codeunit(index);
        }

        /// \brief Find a code point iterator for a given code unit
        /// If this is a continuation code unit, this function might return a
        /// codepoint that begins before code_unit_index.
        [[nodiscard]] codepoint_iterator
        find_codepoint(size_type code_unit_index) {
            // Decrement to a codepoint start
            while (detail::is_utf8_continuation(buffer_[code_unit_index])
                   && code_unit_index > 0)
            {
                --code_unit_index;
            }
            // Find first multibyte >= this code unit and get its codepoint
            const_lookup_table_type t = const_lookup_table();
            auto first_multibyte_after = t.lower_bound_codeunit(
                code_unit_index);
            const size_type first_multibyte_after_codepoint
                = first_multibyte_after.codepoint_index();
            // Calculate their distance
            const size_type multibyte_code_unit_index = first_multibyte_after
                                                            .byte_index();
            const size_type byte_distance = multibyte_code_unit_index
                                            - code_unit_index;
            if (byte_distance == 0) {
                return begin_codepoint() + first_multibyte_after_codepoint;
            } else {
                const size_type this_size = detail::utf8_size(
                    buffer_[code_unit_index],
                    size() - code_unit_index);
                const size_type codepoint_distance = byte_distance - this_size
                                                     + 1;
                const size_type this_codepoint_index
                    = first_multibyte_after_codepoint - codepoint_distance;
                return begin_codepoint() + this_codepoint_index;
            }
        }

        [[nodiscard]] const_codepoint_iterator
        find_codepoint(size_type code_unit_index) const {
            return (const_cast<basic_string *>(this))
                ->find_codepoint(code_unit_index);
        }

        /// \brief Finds the first substring equal to str.
        /// - a substring can be found only if pos <= size() - str.size()
        /// - an empty substring is found at pos if and only if pos <= size()
        /// - for a non-empty substring, if pos >= size(), the function always
        /// returns npos \return Position of the first character of the found
        /// substring or npos if no such substring is found
        [[nodiscard]] constexpr size_type
        find(const basic_string &str, size_type pos = 0) const noexcept {
            return find(str.data(), pos, str.size());
        }

        /// \brief Finds the first substring equal to the range [s, s+count).
        /// - a substring can be found only if pos <= size() - str.size()
        /// - an empty substring is found at pos if and only if pos <= size()
        /// - for a non-empty substring, if pos >= size(), the function always
        /// returns npos \return Position of the first character of the found
        /// substring or npos if no such substring is found
        template <typename Char>
        constexpr size_type
        find(const Char *s, size_type pos, size_type count) const {
            if (count == 0) {
                // return pos
                return pos;
            }
            if (not(pos <= size() - count)) {
                return npos;
            }
            if (pos >= size()) {
                return npos;
            }
            constexpr bool same_encoding = detail::
                is_same_utf_encoding_v<value_type, Char>;
            if constexpr (same_encoding) {
                // Sequential search
                const_iterator it = std::
                    search(begin() + pos, end(), s, s + count);
                if (it != end()) {
                    return it - begin();
                } else {
                    return npos;
                }
            } else {
                const_codepoint_iterator this_cp_first = find_codepoint(pos);
                if (this_cp_first.byte_index() < pos) {
                    ++this_cp_first;
                }
                const_codepoint_iterator this_cp_last = find_codepoint(size());
                const_codepoint_iterator it = std::
                    search(this_cp_first, this_cp_last, s, s + count);
                if (it != end_codepoint()) {
                    return it.byte_index();
                } else {
                    return npos;
                }
            }
        }

        /// \brief Finds the first substring equal to the character string
        /// pointed to by s
        /// - a substring can be found only if pos <= size() - str.size()
        /// - an empty substring is found at pos if and only if pos <= size()
        /// - for a non-empty substring, if pos >= size(), the function always
        /// returns npos \return Position of the first character of the found
        /// substring or npos if no such substring is found
        template <typename Char>
        constexpr size_type
        find(const Char *s, size_type pos = 0) const {
            return find(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the first character ch (treated as a single-character
        /// substring by the formal rules below)
        /// - a substring can be found only if pos <= size() - str.size()
        /// - an empty substring is found at pos if and only if pos <= size()
        /// - for a non-empty substring, if pos >= size(), the function always
        /// returns npos \return Position of the first character of the found
        /// substring or npos if no such substring is found
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        find(Char ch, size_type pos = 0) const noexcept {
            return find(&ch, pos, 1);
        }

        /// \brief Implicitly converts t to a string view sv and find it
        /// - a substring can be found only if pos <= size() - str.size()
        /// - an empty substring is found at pos if and only if pos <= size()
        /// - for a non-empty substring, if pos >= size(), the function always
        /// returns npos \return Position of the first character of the found
        /// substring or npos if no such substring is found
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        find(const T &t, size_type pos = 0) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return find(sv.data(), pos, sv.size());
        }

        /// \brief Finds the last substring equal to the given character
        /// sequence Search begins at pos, i.e. the found substring must not
        /// begin in a position following pos. If npos or any value not smaller
        /// than size()-1 is passed as pos, whole string will be searched
        /// \return Position of the first character of the found substring or
        /// npos if no such substring is found.
        ///         - Note that this is an offset from the start of the string,
        ///         not the end
        ///         - If searching for an empty string, returns pos unless pos >
        ///         size(), in which case returns size()
        ///         - if size() is zero, npos is always returned.
        [[nodiscard]] constexpr size_type
        rfind(const basic_string &str, size_type pos = npos) const noexcept {
            return rfind(str.data(), pos, str.size());
        }

        /// \brief Finds the last substring equal to the given character
        /// sequence Search begins at pos, i.e. the found substring must not
        /// begin in a position following pos. If npos or any value not smaller
        /// than size()-1 is passed as pos, whole string will be searched
        /// \return Position of the first character of the found substring or
        /// npos if no such substring is found.
        ///         - Note that this is an offset from the start of the string,
        ///         not the end
        ///         - If searching for an empty string, returns pos unless pos >
        ///         size(), in which case returns size()
        ///         - if size() is zero, npos is always returned.
        template <typename Char>
        constexpr size_type
        rfind(const Char *s, size_type pos, size_type count) const {
            if (count == 0) {
                // return pos
                return pos;
            }
            if (pos == npos) {
                pos = size() - 1;
            }
            if (pos + count > size()) {
                pos = size() - count;
            }
            if (pos >= size()) {
                return npos;
            }
            constexpr bool same_encoding = detail::
                is_same_utf_encoding_v<value_type, Char>;
            if constexpr (same_encoding) {
                const_iterator it = begin() + pos;
                if (std::equal(it, it + count, s, s + count)) {
                    return it - begin();
                }
                while (it != begin()) {
                    --it;
                    if (std::equal(it, it + count, s, s + count)) {
                        return it - begin();
                    }
                }
                return npos;
            } else {
                if (count > size_codepoints()) {
                    return npos;
                }
                const_codepoint_iterator this_cp_first = find_codepoint(pos);
                const_codepoint_iterator end = end_codepoint();
                auto d_to_end = end - this_cp_first;
                if (detail::cmp_less(d_to_end, count)) {
                    this_cp_first -= (count - d_to_end);
                }
                const_codepoint_iterator this_cp_last = this_cp_first + count;
                if (std::equal(this_cp_first, this_cp_last, s, s + count)) {
                    return this_cp_first.byte_index();
                }
                while (this_cp_first != begin_codepoint()) {
                    --this_cp_first;
                    --this_cp_last;
                    if (std::equal(this_cp_first, this_cp_last, s, s + count)) {
                        return this_cp_first.byte_index();
                    }
                }
                return npos;
            }
        }

        /// \brief Finds the last substring equal to the given character
        /// sequence Search begins at pos, i.e. the found substring must not
        /// begin in a position following pos. If npos or any value not smaller
        /// than size()-1 is passed as pos, whole string will be searched
        /// \return Position of the first character of the found substring or
        /// npos if no such substring is found.
        ///         - Note that this is an offset from the start of the string,
        ///         not the end
        ///         - If searching for an empty string, returns pos unless pos >
        ///         size(), in which case returns size()
        ///         - if size() is zero, npos is always returned.
        template <typename Char>
        constexpr size_type
        rfind(const Char *s, size_type pos = npos) const {
            return rfind(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the last substring equal to the given character
        /// sequence Search begins at pos, i.e. the found substring must not
        /// begin in a position following pos. If npos or any value not smaller
        /// than size()-1 is passed as pos, whole string will be searched
        /// \return Position of the first character of the found substring or
        /// npos if no such substring is found.
        ///         - Note that this is an offset from the start of the string,
        ///         not the end
        ///         - If searching for an empty string, returns pos unless pos >
        ///         size(), in which case returns size()
        ///         - if size() is zero, npos is always returned.
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        rfind(Char ch, size_type pos = npos) const noexcept {
            return rfind(&ch, pos, 1);
        }

        /// \brief Finds the last substring equal to the given character
        /// sequence Search begins at pos, i.e. the found substring must not
        /// begin in a position following pos. If npos or any value not smaller
        /// than size()-1 is passed as pos, whole string will be searched
        /// \return Position of the first character of the found substring or
        /// npos if no such substring is found.
        ///         - Note that this is an offset from the start of the string,
        ///         not the end
        ///         - If searching for an empty string, returns pos unless pos >
        ///         size(), in which case returns size()
        ///         - if size() is zero, npos is always returned.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        rfind(const T &t, size_type pos = npos) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return rfind(sv.data(), pos, sv.size());
        }

        /// \brief Finds the first character equal to one of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned \return Position of the found character or npos if
        /// no such character is found
        [[nodiscard]] constexpr size_type
        find_first_of(const basic_string &str, size_type pos = 0)
            const noexcept {
            return find_first_of(str.data(), pos, str.size());
        }

        /// \brief Finds the first character equal to one of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned \return Position of the found character or npos if
        /// no such character is found
        template <typename Char>
        constexpr size_type
        find_first_of(const Char *s, size_type pos, size_type count) const {
            return find_first_of_common<true>(s, pos, count);
        }

        /// \brief Finds the first character equal to one of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned \return Position of the found character or npos if
        /// no such character is found
        template <typename Char>
        constexpr size_type
        find_first_of(const Char *s, size_type pos = 0) const {
            return find_first_of(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the first character equal to one of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned \return Position of the found character or npos if
        /// no such character is found
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        find_first_of(Char ch, size_type pos = 0) const noexcept {
            return find_first_of(&ch, pos, 1);
        }

        /// \brief Finds the first character equal to one of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned \return Position of the found character or npos if
        /// no such character is found
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        find_first_of(const T &t, size_type pos = 0) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return find_first_of(sv.data(), pos, sv.size());
        }

        /// \brief Finds the first character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned. \return Position of the found character or npos if
        /// no such character is found
        [[nodiscard]] constexpr size_type
        find_first_not_of(const basic_string &str, size_type pos = 0)
            const noexcept {
            return find_first_not_of(str.data(), pos, str.size());
        }

        /// \brief Finds the first character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned. \return Position of the found character or npos if
        /// no such character is found
        template <typename Char>
        constexpr size_type
        find_first_not_of(const Char *s, size_type pos, size_type count) const {
            return find_first_of_common<false>(s, pos, count);
        }

        /// \brief Finds the first character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned. \return Position of the found character or npos if
        /// no such character is found
        template <typename Char>
        constexpr size_type
        find_first_not_of(const Char *s, size_type pos = 0) const {
            return find_first_not_of(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the first character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned. \return Position of the found character or npos if
        /// no such character is found
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        find_first_not_of(Char ch, size_type pos = 0) const noexcept {
            return find_first_not_of(&ch, pos, 1);
        }

        /// \brief Finds the first character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [pos, size()). If the character is not present in the interval, npos
        /// will be returned. \return Position of the found character or npos if
        /// no such character is found
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        find_first_not_of(const T &t, size_type pos = 0) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return find_first_not_of(sv.data(), pos, sv.size());
        }

        /// \brief Finds the last character equal to one of characters in the
        /// given character sequence The search considers only the interval [0,
        /// pos] If the character is not present in the interval, npos will be
        /// returned \return Position of the found character or npos if no such
        /// character is found.
        [[nodiscard]] constexpr size_type
        find_last_of(const basic_string &str, size_type pos = npos)
            const noexcept {
            return find_last_of(str.data(), pos, str.size());
        }

        /// \brief Finds the last character equal to one of characters in the
        /// given character sequence The search considers only the interval [0,
        /// pos] If the character is not present in the interval, npos will be
        /// returned \return Position of the found character or npos if no such
        /// character is found.
        template <typename Char>
        constexpr size_type
        find_last_of(const Char *s, size_type pos, size_type count) const {
            return find_last_of_common<true>(s, pos, count);
        }

        /// \brief Finds the last character equal to one of characters in the
        /// given character sequence The search considers only the interval [0,
        /// pos] If the character is not present in the interval, npos will be
        /// returned \return Position of the found character or npos if no such
        /// character is found.
        template <typename Char>
        constexpr size_type
        find_last_of(const Char *s, size_type pos = npos) const {
            return find_last_of(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the last character equal to one of characters in the
        /// given character sequence The search considers only the interval [0,
        /// pos] If the character is not present in the interval, npos will be
        /// returned \return Position of the found character or npos if no such
        /// character is found.
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        find_last_of(Char ch, size_type pos = npos) const noexcept {
            return find_last_of(&ch, pos, 1);
        }

        /// \brief Finds the last character equal to one of characters in the
        /// given character sequence The search considers only the interval [0,
        /// pos] If the character is not present in the interval, npos will be
        /// returned \return Position of the found character or npos if no such
        /// character is found.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        find_last_of(const T &t, size_type pos = npos) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return find_last_of(sv.data(), pos, sv.size());
        }

        /// \brief Finds the last character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [0, pos] If the character is not present in the interval, npos will
        /// be returned \return Position of the found character or npos if no
        /// such character is found.
        [[nodiscard]] constexpr size_type
        find_last_not_of(const basic_string &str, size_type pos = npos)
            const noexcept {
            return find_last_not_of(str.data(), pos, str.size());
        }

        /// \brief Finds the last character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [0, pos] If the character is not present in the interval, npos will
        /// be returned \return Position of the found character or npos if no
        /// such character is found.
        template <typename Char>
        constexpr size_type
        find_last_not_of(const Char *s, size_type pos, size_type count) const {
            return find_last_of_common<false>(s, pos, count);
        }

        /// \brief Finds the last character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [0, pos] If the character is not present in the interval, npos will
        /// be returned \return Position of the found character or npos if no
        /// such character is found.
        template <typename Char>
        constexpr size_type
        find_last_not_of(const Char *s, size_type pos = npos) const {
            return find_last_not_of(s, pos, std::char_traits<Char>::length(s));
        }

        /// \brief Finds the last character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [0, pos] If the character is not present in the interval, npos will
        /// be returned \return Position of the found character or npos if no
        /// such character is found.
        template <
            typename Char,
            std::enable_if_t<!is_api_string_view_v<Char>, int> = 0>
        constexpr size_type
        find_last_not_of(Char ch, size_type pos = npos) const noexcept {
            return find_last_not_of(&ch, pos, 1);
        }

        /// \brief Finds the last character equal to none of the characters in
        /// the given character sequence The search considers only the interval
        /// [0, pos] If the character is not present in the interval, npos will
        /// be returned \return Position of the found character or npos if no
        /// such character is found.
        template <class T, std::enable_if_t<is_api_string_view_v<T>, int> = 0>
        constexpr size_type
        find_last_not_of(const T &t, size_type pos = npos) const {
            std::basic_string_view<typename T::value_type> sv(t);
            return find_last_not_of(sv.data(), pos, sv.size());
        }

    public:
        /// \brief Spaceship operator to compare with another range of bytes
        template <class InputIt>
        constexpr int
        compare(InputIt first, InputIt last) const noexcept {
            using input_value_type = std::decay_t<
                typename std::iterator_traits<InputIt>::value_type>;
            const value_type *this_it = data();
            auto *this_end = this_it + size();
            while (this_it != this_end && first != last && *first) {
                if constexpr (detail::is_same_utf_encoding_v<
                                  value_type,
                                  input_value_type>) {
                    if (*this_it != static_cast<value_type>(*first)) {
                        return *this_it < *first ? -1 : 1;
                    }
                    ++this_it;
                    ++first;
                } else {
                    // Encode to "this" utf type
                    value_type buf[8];
                    std::memset(buf, '\0', 8);
                    width_type other_code_units = detail::
                        utf_size(*first, last - first);
                    width_type this_code_units = detail::utf_size_as<
                        value_type>(first, other_code_units);
                    detail::
                        to_utf(first, other_code_units, buf, this_code_units);
                    // Compare the encoded arrays
                    value_type *encoded_first = buf;
                    value_type *encoded_last = buf + this_code_units;
                    while (this_it != this_end && encoded_first != encoded_last
                           && *encoded_first) {
                        if (*this_it != *encoded_first) {
                            return *this_it < *encoded_first ? -1 : 1;
                        }
                        ++this_it;
                        ++encoded_first;
                    }
                    ++first;
                }
            }
            const bool rhs_is_larger = first != last && *first;
            return rhs_is_larger ? -1 : this_it == this_end ? 0 : 1;
        }

        /// \brief Compare this string with a range of chars
        template <
            class Range,
            std::enable_if_t<detail::is_range_v<Range>, int> = 0>
        constexpr int
        compare(Range &&r) const noexcept {
            return compare(r.begin(), r.end());
        }

        /// \brief Compare this string with a null terminated char array
        template <typename Char>
        constexpr int
        compare(const Char *s) const {
            return compare(s, s + detail::strlen(s));
        }

        /// \brief Compare this string with a null terminated std::string
        template <typename RhsCharT, typename RhsTraits, typename RhsAllocator>
        constexpr int
        compare(
            std::basic_string<RhsCharT, RhsTraits, RhsAllocator> &&r) noexcept {
            using rhs_string_view_type = std::
                basic_string_view<RhsCharT, RhsTraits>;
            return compare(rhs_string_view_type(r.c_str(), r.size()));
        }

        /// \section Relational operators: delegate everything to the spaceship
        /// operator
        constexpr friend bool
        operator==(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) == 0;
        }

        constexpr friend bool
        operator!=(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) != 0;
        }

        constexpr friend bool
        operator>(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) > 0;
        }

        constexpr friend bool
        operator>=(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) >= 0;
        }

        constexpr friend bool
        operator<(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) < 0;
        }

        constexpr friend bool
        operator<=(const basic_string &lhs, const basic_string &rhs) noexcept {
            return lhs.compare(rhs) <= 0;
        }

    private:
        template <typename>
        struct is_small_basic_string : std::false_type
        {};
        template <typename... Args>
        struct is_small_basic_string<basic_string<Args...>> : std::true_type
        {};

        template <typename>
        struct is_std_basic_string : std::false_type
        {};
        template <typename RhsCharT, typename RhsTraits, typename RhsAllocator>
        struct is_std_basic_string<
            std::basic_string<RhsCharT, RhsTraits, RhsAllocator>>
            : std::true_type
        {};

        /// \brief Types to which we can compare this basic_string
        template <typename T>
        using is_valid_comparison_rhs = std::conjunction<
            // not the same small::basic_string
            std::negation<std::is_same<T, basic_string>>,
            std::disjunction<
                // other small basic string type
                is_small_basic_string<T>,
                // range of chars
                detail::is_range<T>,
                // std::string
                is_std_basic_string<T>,
                // CharT*
                std::conjunction<
                    std::is_pointer<T>,
                    std::is_trivial<std::remove_pointer_t<T>>>>>;

        template <typename T>
        static constexpr bool is_valid_comparison_rhs_v
            = is_valid_comparison_rhs<T>::value;

    public:
        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator==(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) == 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator!=(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) != 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator>(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) > 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator>=(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) >= 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator<(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) < 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator<=(const basic_string &lhs, T &&rhs) noexcept {
            return lhs.compare(rhs) <= 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator==(T &&lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) == 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator!=(T &&lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) != 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator>(T &&lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) < 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator>=(T &&lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) <= 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator<(T && lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) > 0;
        }

        template <
            typename T,
            std::enable_if_t<is_valid_comparison_rhs_v<std::decay_t<T>>, int> = 0>
        constexpr friend bool
        operator<=(T &&lhs, const basic_string &rhs) noexcept {
            return rhs.compare(lhs) >= 0;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// std::allocator_traits<Alloc>::select_on_container_copy_construction(lhs.get_allocator())
        /// \return A string containing characters from lhs followed by the
        /// characters from rhs
        friend constexpr basic_string
        operator+(const basic_string &lhs, const basic_string &rhs) {
            basic_string res(lhs);
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// std::allocator_traits<Alloc>::select_on_container_copy_construction(lhs.get_allocator())
        /// \return A string containing characters from lhs followed by the
        /// characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(const basic_string &lhs, const Char *rhs) {
            basic_string res(lhs);
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// std::allocator_traits<Alloc>::select_on_container_copy_construction(lhs.get_allocator())
        /// \return A string containing characters from lhs followed by the
        /// characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(const basic_string &lhs, Char rhs) {
            basic_string res(lhs);
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// std::allocator_traits<Alloc>::select_on_container_copy_construction(rhs.get_allocator())
        /// \return A string containing characters from lhs followed by the
        /// characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(const Char *lhs, const basic_string &rhs) {
            basic_string res(lhs);
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// std::allocator_traits<Alloc>::select_on_container_copy_construction(rhs.get_allocator())
        /// \return A string containing characters from lhs followed by the
        /// characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(Char lhs, const basic_string &rhs) {
            basic_string res(1, lhs);
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// lhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        friend constexpr basic_string
        operator+(basic_string &&lhs, basic_string &&rhs) {
            basic_string res(std::move(lhs));
            res += std::move(rhs);
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// lhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        friend constexpr basic_string
        operator+(basic_string &&lhs, const basic_string &rhs) {
            basic_string res(std::move(lhs));
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// lhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(basic_string &&lhs, const Char *rhs) {
            basic_string res(std::move(lhs));
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// lhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(basic_string &&lhs, Char rhs) {
            basic_string res(std::move(lhs));
            res += rhs;
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// rhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        friend constexpr basic_string
        operator+(const basic_string &lhs, basic_string &&rhs) {
            basic_string res(lhs);
            res += std::move(rhs);
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// rhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(const Char *lhs, basic_string &&rhs) {
            basic_string res(lhs);
            res += std::move(rhs);
            return res;
        }

        /// \brief Returns a string containing characters from lhs followed by
        /// the characters from rhs The allocator used for the result is
        /// rhs.get_allocator() \return A string containing characters from lhs
        /// followed by the characters from rhs
        template <typename Char>
        friend constexpr basic_string
        operator+(Char lhs, basic_string &&rhs) {
            basic_string res(1, lhs);
            res += std::move(rhs);
            return res;
        }

        /// \brief Specializes the std::swap algorithm for std::basic_string
        /// Swaps the contents of lhs and rhs.
        /// Equivalent to lhs.swap(rhs)
        friend constexpr void
        swap(basic_string &lhs, basic_string &rhs) noexcept(
            noexcept(lhs.swap(rhs))) {
            lhs.swap(rhs);
        }

        /// \brief Erases all elements that compare equal to value from the
        /// container \return The number of erased elements
        template <class U>
        friend constexpr typename basic_string::size_type
        erase(basic_string &c, const U &value) {
            auto it = std::remove(c.begin(), c.end(), value);
            auto r = std::distance(it, c.end());
            c.erase(it, c.end());
            return r;
        }

        /// \brief Erases all elements that satisfy the predicate pred from the
        /// container \return The number of erased elements
        template <class Pred>
        friend constexpr typename basic_string::size_type
        erase_if(basic_string &c, Pred pred) {
            auto it = std::remove_if(c.begin(), c.end(), pred);
            auto r = std::distance(it, c.end());
            c.erase(it, c.end());
            return r;
        }

        /// \brief Output stream operation
        /// Behaves as a FormattedOutputFunction
        /// \see
        /// https://en.cppreference.com/w/cpp/named_req/FormattedOutputFunction
        /// \throws may throw std::ios_base::failure if an exception is thrown
        /// during output
        friend std::ostream &
        operator<<(std::ostream &os, const basic_string &str) {
            // After constructing and checking the sentry object, determines the
            // output format padding as follows: 1) Behaves as a
            // FormattedOutputFunction
            //   a) If str.size() is not less than os.width(), uses the range
            //   [str.begin(), str.end()) as-is b) Otherwise, if (os.flags() &
            //   ios_base::adjustfield) == ios_base::left, places
            //   os.width()-str.size() copies of the os.fill() character after
            //   the character sequence c) Otherwise, places
            //   os.width()-str.size() copies of the os.fill() character before
            //   the character sequence
            // 2) Then stores each character from the resulting sequence (the
            // contents of str plus padding) to the output stream os as if by
            // calling os.rdbuf()->sputn(seq, n), where n=(std::max)(os.width(),
            // str.size()) 3) Finally, calls os.width(0) to cancel the effects
            // of std::setw, if any
            detail::console_unicode_guard
                g(os,
                  str.size(),
                  str.size_codepoints(),
                  (str.size() - str.size_codepoints()) > 2);
            os << str.c_str();
            return os;
        }

        /// \brief Input stream operation
        /// Behaves as a FormattedInputFunction
        /// \see
        /// https://en.cppreference.com/w/cpp/named_req/FormattedInputFunction
        /// \throws may throw std::ios_base::failure if no characters are
        /// extracted from is (e.g the stream is at end of file, or consists of
        /// whitespace only), or if an exception is thrown during input.
        friend std::istream &
        operator>>(std::istream &is, basic_string &str) {
            // 1) After constructing and checking the sentry object, which may
            // skip leading whitespace,
            //   a) first clears str with str.erase()
            //   b) then reads characters from is and appends them to str as if
            //   by str.append(1, c), until one of the following conditions
            //   becomes true:
            //      (i) N characters are read, where N is is.width() if
            //      is.width() > 0, otherwise N is str.max_size() (ii) the
            //      end-of-file condition occurs in the stream is (iii)
            //      std::isspace(c,is.getloc()) is true for the next character c
            //      in is (this whitespace character remains in the input
            //      stream).
            // 2) If no characters are extracted then std::ios::failbit is set
            // on is, which may throw std::ios_base::failure 3) Finally, calls
            // is.width(0) to cancel the effects of std::setw, if any
            std::ios_base::iostate state = std::ios_base::goodbit;
            using sentry_char_type = std::conditional_t<
                std::is_same_v<detail::utf8_char_type, CharT>,
                char,
                CharT>;
            using sentry_traits_type = std::conditional_t<
                std::is_same_v<detail::utf8_char_type, CharT>,
                std::char_traits<char>,
                Traits>;
            typename std::basic_istream<sentry_char_type, sentry_traits_type>::
                sentry sen(is);
            if (sen) {
#ifndef SMALL_DISABLE_EXCEPTIONS
                try {
#endif
                    str.clear();
                    std::streamsize n = is.width();
                    if (n <= 0)
                        n = str.max_size();
                    if (n <= 0)
                        n = std::numeric_limits<std::streamsize>::max();
                    std::streamsize c = 0;
                    const std::ctype<sentry_char_type>
                        &ct = std::use_facet<std::ctype<sentry_char_type>>(is.getloc());
                    while (c < n) {
                        typename Traits::int_type i = is.rdbuf()->sgetc();
                        if (Traits::eq_int_type(i, Traits::eof())) {
                            state |= std::ios_base::eofbit;
                            break;
                        }
                        sentry_char_type ch = Traits::to_char_type(i);
                        if (ct.is(ct.space, ch))
                            break;
                        str.push_back(ch);
                        ++c;
                        is.rdbuf()->sbumpc();
                    }
                    is.width(0);
                    if (c == 0)
                        state |= std::ios_base::failbit;
#ifndef SMALL_DISABLE_EXCEPTIONS
                }
                catch (...) {
                    state |= std::ios_base::badbit;
                    is.setstate(state);
                    if (is.exceptions() & std::ios_base::badbit) {
                        throw;
                    }
                }
#endif
                is.setstate(state);
            }
            return is;
        }

        /// \brief Reads characters from an input stream and places them into a
        /// string Behaves as UnformattedInputFunction, except that
        /// input.gcount() is not affected. \see
        /// https://en.cppreference.com/w/cpp/named_req/UnformattedInputFunction
        /// When consuming whitespace-delimited input (e.g. int n; std::cin >>
        /// n;) any whitespace that follows, including a newline character, will
        /// be left on the input stream. Then when switching to line-oriented
        /// input, the first line retrieved with getline will be just that
        /// whitespace. In the likely case that this is unwanted behaviour,
        /// possible solutions include:
        /// - An explicit extraneous initial call to getline
        /// - Removing consecutive whitespace with std::cin >> std::ws
        /// - Ignoring all leftover characters on the line of input with
        /// cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        friend std::istream &
        getline(std::istream &is, basic_string &str, CharT delim) {
            // After constructing and checking the sentry object, performs the
            // following 1) Calls str.erase() 2) Extracts characters from input
            // and appends them to str until one of the following occurs
            // (checked in the order listed)
            //   a) end-of-file condition on input, in which case, getline sets
            //   eofbit b) the next available input character is delim, as
            //   tested by Traits::eq(c, delim), in which case the delimiter
            //   character is extracted from input, but is not appended to str
            //   c) str.max_size() characters have been stored, in which case
            //   getline sets failbit and returns
            // 3) If no characters were extracted for whatever reason (not even
            // the discarded delimiter), getline sets failbit and returns
            std::ios_base::iostate state = std::ios_base::goodbit;
            using sentry_char_type = std::conditional_t<
                std::is_same_v<detail::utf8_char_type, CharT>,
                char,
                CharT>;
            using sentry_traits_type = std::conditional_t<
                std::is_same_v<detail::utf8_char_type, CharT>,
                std::char_traits<char>,
                Traits>;
            typename std::basic_istream<sentry_char_type, sentry_traits_type>::
                sentry sen(is, true);
            if (sen) {
#ifndef SMALL_DISABLE_EXCEPTIONS
                try {
#endif
                    str.clear();
                    std::streamsize extra = 0;
                    while (true) {
                        typename Traits::int_type i = is.rdbuf()->sbumpc();
                        if (Traits::eq_int_type(i, Traits::eof())) {
                            state |= std::ios_base::eofbit;
                            break;
                        }
                        ++extra;
                        CharT ch = Traits::to_char_type(i);
                        if (Traits::eq(ch, delim))
                            break;
                        str.push_back(ch);
                        if (str.size() == str.max_size()) {
                            state |= std::ios_base::failbit;
                            break;
                        }
                    }
                    if (extra == 0)
                        state |= std::ios_base::failbit;
#ifndef SMALL_DISABLE_EXCEPTIONS
                }
                catch (...) {
                    state |= std::ios_base::badbit;
                    is.setstate(state);
                    if (is.exceptions() & std::ios_base::badbit) {
                        throw;
                    }
                }
#endif
                is.setstate(state);
            }
            return is;
        }

        friend std::istream &
        getline(std::istream &&is, basic_string &str, CharT delim) {
            return getline(is, str, delim);
        }

        /// \brief Reads characters from an input stream and places them into a
        /// string Behaves as UnformattedInputFunction, except that
        /// input.gcount() is not affected. \see
        /// https://en.cppreference.com/w/cpp/named_req/UnformattedInputFunction
        friend std::istream &
        getline(std::istream &input, basic_string &str) {
            return getline(input, str, input.widen('\n'));
        }

        friend std::istream &
        getline(std::istream &&input, basic_string &str) {
            return getline(input, str, input.widen('\n'));
        }

    private:
        /// \section Private useful types used for the storage type

        /// \brief Type we use for the lookup size of size indicator
        typedef std::uint8_t size_of_size_type;

        /// \brief The vector type we use to store the raw chars
        /// Using our inline vector brings the cache optimizations to the string
        /// type
        using char_vector_type = small::vector<value_type, N + 1>;

        /// \brief Size it takes to store the null '\0' char
        /// Of course we could just write 1 in the code, but this gives us
        /// better semantics
        static constexpr size_type null_char_size = '\0';

        /// \brief True if this string type supports utf8
        /// We use utf8 for the string whenever the char size is 1.
        static constexpr bool is_utf8_string = detail::is_utf8_v<value_type>;
        static constexpr bool is_utf16_string = detail::is_utf16_v<value_type>;
        static constexpr bool is_utf32_string = detail::is_utf32_v<value_type>;

        static_assert(
            is_utf8_string || is_utf16_string || is_utf32_string,
            "string: A unicode char type is required");

        /// \brief True if this string type has a lookup table
        /// All utf8/utf16 strings have a lookup table because they have more
        /// than one code unit per code point. If the string ends up having no
        /// multiple code units per codepoint, the lookup table takes only one
        /// byte.
        static constexpr bool store_codepoint_size = is_utf8_string
                                                     || is_utf16_string;
        static constexpr bool has_lookup_table = store_codepoint_size;

    private:
        /// \section Storage helpers

        /// \brief Returns a map representing the index of codepoints
        constexpr lookup_table_type
        lookup_table() noexcept {
            return lookup_table_type(
                buffer_.data() + size_.codeunit_size() + 1,
                buffer_.size() - size_.codeunit_size() - 1,
                this->operator string_view_type());
        }

        [[nodiscard]] constexpr const_lookup_table_type
        const_lookup_table() const noexcept {
            return const_lookup_table_type(
                buffer_.data() + size_.codeunit_size() + 1,
                buffer_.size() - size_.codeunit_size() - 1,
                this->operator string_view_type());
        }

        /// \brief Increase the string size to contain count characters
        /// If the input is wide with more than one code unit per code points,
        /// then count will refer to code points, as to avoid malformed strings
        template <typename Char>
        constexpr void
        upsize(size_type count, Char ch) {
            assert(count > size());

            // Calculate increments
            uint8_t code_units_per_char = detail::utf_size_as<
                value_type>(&ch, 1);
            size_type code_unit_increment = count - size();
            size_type code_point_increment = code_unit_increment
                                             / code_units_per_char;
            const_lookup_table_type ct = const_lookup_table();

            // Allocate memory in the buffer for the extra chars
            reserve(
                size() + code_unit_increment,
                ct.size()
                    + (code_units_per_char > 1 ? code_point_increment : 0));

            // Fill the buffer
            value_type *fill_begin = (buffer_.begin() + size()).base();
            value_type *fill_end = fill_begin + code_unit_increment;
            if constexpr (!detail::is_same_utf_encoding_v<value_type, Char>) {
                if (code_units_per_char == 1) {
                    std::fill(fill_begin, fill_end, static_cast<value_type>(ch));
                } else {
                    // Fill buffer
                    detail::to_utf(&ch, 1, fill_begin, code_unit_increment);
                    if (code_units_per_char < code_unit_increment) {
                        for (size_type i = 1; i < code_point_increment; ++i) {
                            std::memcpy(
                                fill_begin + i * code_units_per_char,
                                fill_begin,
                                code_units_per_char);
                        }
                        // Include a malformed code point if we need to complete
                        // the buffer
                        value_type *fill_end_codepoint
                            = fill_begin
                              + code_point_increment * code_units_per_char;
                        const size_type extra_code_units = fill_end
                                                           - fill_end_codepoint;
                        if (extra_code_units > 0) {
                            std::memcpy(
                                fill_end_codepoint,
                                fill_begin,
                                extra_code_units);
                        }
                    }
                    // Fill the lookup table
                    if constexpr (has_lookup_table) {
                        lookup_table_type t = lookup_table();
                        size_type multibyte_index = t.size();
                        size_type multibyte_code_unit_index = size();
                        size_type multibyte_codepoint_index = size_codepoints();
                        for (size_type i = 0; i < code_point_increment; ++i) {
                            t.insert_or_assign(
                                multibyte_index,
                                multibyte_code_unit_index,
                                multibyte_codepoint_index);
                            ++multibyte_index;
                            multibyte_code_unit_index += code_units_per_char;
                            ++multibyte_codepoint_index;
                        }
                    }
                }
            } else {
                // Increase buffer
                reserve(count);
                // Fill buffer
                std::fill(fill_begin, fill_end, ch);
            }
            // Set null char
            *fill_end = '\0';
            // Update internal size
            size_.codeunit_size() += code_unit_increment;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() += code_point_increment;
            }
        }

        /// \brief Count the number of code points in a range of code units
        /// [first, last) \param first First code unit \param last  Last+1 code
        /// unit \return Number of code points between these two code units
        constexpr size_type
        size_codepoints(const_iterator first, const_iterator last) {
            if (first >= last) {
                return 0;
            }
            if constexpr (has_lookup_table) {
                // Convert to indexes
                size_type first_idx = first - begin();
                size_type last_idx = last - begin();

                // Find first multibyte index after first
                lookup_table_type t = lookup_table();
                auto multibyte_first_it = t.lower_bound_codeunit(first_idx);
                const size_type mb_first_idx = multibyte_first_it
                                                   .codepoint_index();

                const bool range_contains_multibyte_code_units
                    = multibyte_first_it.byte_index() < last_idx;
                if (!range_contains_multibyte_code_units) {
                    // If there are no multibytes, we just have one code point
                    // per code unit
                    return last_idx - first_idx;
                } else {
                    // Get last multibyte after last
                    auto multibyte_last_it
                        = multibyte_first_it == t.end() ?
                              multibyte_first_it :
                              t.lower_bound_codeunit(last_idx);
                    // Get distance in codepoints between them
                    const size_type mb_last_idx
                        = multibyte_first_it == multibyte_last_it ?
                              mb_first_idx :
                              multibyte_last_it.codepoint_index();
                    const size_type multibyte_codepoint_distance
                        = mb_last_idx - mb_first_idx;
                    // Adjust the distance to:
                    // 1) include [first,first_multibyte] range
                    // 2) remove [last,last_multibyte] range
                    const size_type range_first = multibyte_first_it.byte_index()
                                                  - first_idx;
                    const size_type range_last = multibyte_last_it.byte_index()
                                                 - last_idx;
                    return multibyte_codepoint_distance + range_first
                           - range_last;
                }
            } else {
                return static_cast<size_type>(last - first);
            }
        }

        /// \brief Decrease the string size to contain count characters
        constexpr void
        downsize(size_type count) {
            assert(count < size());
            const size_type cur_size = size();
            const size_type code_unit_decrement = cur_size - count;
            const size_type code_point_decrement
                = size_codepoints(begin() + count, end());

            // Remove elements from the lookup table
            if constexpr (has_lookup_table) {
                lookup_table_type t = lookup_table();
                // Find first entry indexing an index >= count
                typename lookup_table_type::iterator it = t.lower_bound_codeunit(
                    count);
                size_type lookup_count = it.multibyte_index();
                t.resize(lookup_count);
            }

            // Set the null char
            buffer_[count] = '\0';

            // Update size counter
            size_.codeunit_size() -= code_unit_decrement;
            if constexpr (store_codepoint_size) {
                size_.codepoint_size() -= code_point_decrement;
            }
        }

        /// \brief Index the multibyte codepoints in the range [first,last]
        ///
        /// We use this function to update the multibyte indexes relative to
        /// this range after any operation that affects its elements.
        ///
        /// Very often, there's nothing to do here, so we want doing any work
        /// we don't have to.
        ///
        /// In the general case, when there is work to do, resizing the entries
        /// is very expensive, so we (i) count the number of multibyte entries
        /// first, (ii) resize the table once, and (iii) iterate again to set
        /// the values. \param first First element that has been updated \param
        /// last Last + 1 element that has been updated
        void
        index_multibyte_codepoints(const_iterator first, const_iterator last) {
            // Get indexes for first and last
            const size_type first_idx = first - cbegin();
            const size_type last_idx = last - cbegin();

            // Count how many multibyte codepoints are indexed in this range
            lookup_table_type t = lookup_table();
            using lut_iterator = typename lookup_table_type::iterator;
            const lut_iterator first_multibyte_after_first
                = t.lower_bound_codeunit(first_idx);
            const size_type first_multibyte_after_first_idx
                = first_multibyte_after_first.byte_index();
            const bool lut_has_multibytes_in_range
                = first_multibyte_after_first_idx < last_idx;
            const lut_iterator first_multibyte_after_last
                = lut_has_multibytes_in_range ?
                      t.lower_bound_codeunit(last_idx) :
                      first_multibyte_after_first;
            const size_type lut_multibytes_in_range
                = first_multibyte_after_last - first_multibyte_after_first;

            // Count how many multibyte codepoints we have to index in the new
            // input range
            size_type input_multibytes_in_range = 0;
            auto it = first;
            while (it < last) {
                uint8_t s = detail::utf8_size(*it, end() - it);
                if (s > 1) {
                    ++input_multibytes_in_range;
                }
                it += s;
            }

            // Check if there's any work to do
            if (lut_multibytes_in_range == 0 && input_multibytes_in_range == 0)
            {
                return;
            }

            // Find properties of first multibyte entry before resizing the
            // table We need the end() iterator to refer to the old table at
            // this point
            size_type codeunit_idx = first_idx;
            size_type codepoint_idx
                = first_multibyte_after_first.codepoint_index()
                  - (first_multibyte_after_first_idx - first_idx);
            size_type multibyte_idx = first_multibyte_after_first
                                          .multibyte_index();

            // Resize the lookup table
            const size_type table_old_size = t.size();
            if (input_multibytes_in_range > lut_multibytes_in_range) {
                const size_type table_increment = input_multibytes_in_range
                                                  - lut_multibytes_in_range;
                const size_type table_new_size = table_old_size
                                                 + table_increment;
                t.resize(table_new_size);
            } else if (input_multibytes_in_range < lut_multibytes_in_range) {
                const size_type table_decrement = lut_multibytes_in_range
                                                  - input_multibytes_in_range;
                const size_type table_new_size = table_old_size
                                                 - table_decrement;
                t.resize(table_new_size);
            }

            // Iterate and reindex codepoints from first to end()
            while (codeunit_idx < size_.codeunit_size()) {
                const size_type utf8_code_units = detail::utf8_size(
                    buffer_[codeunit_idx],
                    size_.codeunit_size() - codeunit_idx);
                if (utf8_code_units > 1) {
                    t.insert_or_assign(
                        multibyte_idx,
                        codeunit_idx,
                        codepoint_idx);
                    ++multibyte_idx;
                }
                codeunit_idx += utf8_code_units;
                ++codepoint_idx;
            }
        }

        template <bool look_for_existence, typename Char>
        constexpr size_type
        find_first_of_common(const Char *s, size_type pos, size_type count)
            const {
            constexpr bool same_encoding = sizeof(Char) == sizeof(value_type);
            if (pos >= size()) {
                return npos;
            }
            if constexpr (same_encoding) {
                auto it = std::find_if(begin() + pos, end(), [&](value_type ch) {
                    if constexpr (look_for_existence) {
                        return std::find(s, s + count, ch) != s + count;
                    } else {
                        return std::find(s, s + count, ch) == s + count;
                    }
                });
                if (it != end()) {
                    return it - begin();
                } else {
                    return npos;
                }
            } else {
                const_codepoint_iterator this_cp_first = find_codepoint(pos);
                if (this_cp_first.byte_index() < pos) {
                    ++this_cp_first;
                }
                const_codepoint_iterator this_cp_last = end_codepoint();
                const_codepoint_iterator it = std::find_if(
                    this_cp_first,
                    this_cp_last,
                    [&](auto wide_ch) {
                    if constexpr (look_for_existence) {
                        return std::find(s, s + count, wide_ch) != s + count;
                    } else {
                        return std::find(s, s + count, wide_ch) == s + count;
                    }
                    });
                if (it != end_codepoint()) {
                    return it.byte_index();
                } else {
                    return npos;
                }
            }
        }

        template <bool look_for_existence, typename Char>
        constexpr size_type
        find_last_of_common(const Char *s, size_type pos, size_type count)
            const {
            constexpr bool same_encoding = sizeof(Char) == sizeof(value_type);
            if (pos >= size()) {
                pos = size() - 1;
            }
            if constexpr (same_encoding) {
                auto it = begin() + pos;
                bool it_matches = look_for_existence ?
                                      std::find(s, s + count, *it)
                                          != s + count :
                                      std::find(s, s + count, *it) == s + count;
                if (it_matches) {
                    return it - begin();
                }
                while (it != begin()) {
                    --it;
                    it_matches = look_for_existence ?
                                     std::find(s, s + count, *it) != s + count :
                                     std::find(s, s + count, *it) == s + count;
                    if (it_matches) {
                        return it - begin();
                    }
                }
                return npos;
            } else {
                const_codepoint_iterator it = find_codepoint(pos);
                if (it.byte_index() < pos) {
                    ++it;
                }
                const_codepoint_iterator cp_begin = begin_codepoint();
                bool cp_match = look_for_existence ?
                                    std::find(s, s + count, *it) != s + count :
                                    std::find(s, s + count, *it) == s + count;
                if (cp_match) {
                    return it.byte_index();
                }
                while (it != cp_begin) {
                    --it;
                    cp_match = look_for_existence ?
                                   std::find(s, s + count, *it) != s + count :
                                   std::find(s, s + count, *it) == s + count;
                    if (cp_match) {
                        return it.byte_index();
                    }
                }
                return npos;
            }
        }

    private:
        /// \section Members
        /// \brief An inlined vector with space for:
        /// (i) all our string bytes,
        /// (ii) '\0',
        /// (iii) memory reserved for more chars to come,
        /// (iv) lookup table, including
        ///     (v.i) one entry per multibyte codepoint,
        ///     (v.ii) the table size
        ///     (v.iii) the size of size
        /// This acts as a buffer, because this vector is always larger than the
        /// string, to allocate the '\0' char and the lookup table
        char_vector_type buffer_;

        /// \brief Size representation for utf8 strings
        struct internal_size_multibyte
        {
        public:
            internal_size_multibyte() : codeunit_size_(0), codepoint_size_(0) {}

            internal_size_multibyte(const internal_size_multibyte &other)
                : codeunit_size_(other.codeunit_size_),
                  codepoint_size_(other.codepoint_size_) {}

            internal_size_multibyte(internal_size_multibyte &&other)
                : codeunit_size_(other.codeunit_size_),
                  codepoint_size_(other.codepoint_size_) {
                other.codeunit_size_ = 0;
                other.codepoint_size_ = 0;
            }

            internal_size_multibyte &
            operator=(const internal_size_multibyte &other) {
                codeunit_size_ = other.codeunit_size_;
                codepoint_size_ = other.codepoint_size_;
                return *this;
            }

            internal_size_multibyte &
            operator=(internal_size_multibyte &&other) {
                codeunit_size_ = other.codeunit_size_;
                codepoint_size_ = other.codepoint_size_;
                other.codeunit_size_ = 0;
                other.codepoint_size_ = 0;
                return *this;
            }

            size_type &
            codeunit_size() {
                return codeunit_size_;
            }

            size_type &
            codepoint_size() {
                return codepoint_size_;
            }

            [[nodiscard]] const size_type &
            codeunit_size() const {
                return codeunit_size_;
            }

            [[nodiscard]] const size_type &
            codepoint_size() const {
                return codepoint_size_;
            }

            [[nodiscard]] size_type
            byte_size() const {
                return codeunit_size() * sizeof(value_type);
            }

        private:
            /// \brief Buffer size allocated for the string without the \0 and
            /// or the lookup table (see (i) above) i.e.: Where the string ends
            /// in the buffer i.e.: What basic_string::size() returns
            size_type codeunit_size_{ 0 };

            /// \brief The number of codepoints in this string
            /// If the string has no multibyte codepoints, this is the same as
            /// codeunit_size_
            size_type codepoint_size_{ 0 };
        };

        /// \brief Size representation for other strings (assuming one codepoint
        /// per char)
        struct internal_size_codeunits
        {
        public:
            internal_size_codeunits() : codeunit_size_(0) {}

            internal_size_codeunits(const internal_size_codeunits &other)
                : codeunit_size_(other.codeunit_size_) {}

            internal_size_codeunits(internal_size_codeunits &&other)
                : codeunit_size_(other.codeunit_size_) {
                other.codeunit_size_ = 0;
            }

            internal_size_codeunits &
            operator=(const internal_size_codeunits &other) {
                codeunit_size_ = other.codeunit_size_;
                return *this;
            }

            internal_size_codeunits &
            operator=(internal_size_codeunits &&other) {
                codeunit_size_ = other.codeunit_size_;
                other.codeunit_size_ = 0;
                return *this;
            }

            size_type &
            codeunit_size() {
                return codeunit_size_;
            }

            size_type &
            codepoint_size() {
                return codeunit_size_;
            }

            const size_type &
            codeunit_size() const {
                return codeunit_size_;
            }

            const size_type &
            codepoint_size() const {
                return codeunit_size_;
            }

            size_type
            byte_size() const {
                return codeunit_size() * sizeof(value_type);
            }

        private:
            /// \brief Buffer size allocated for the string without the \0 and
            /// or the lookup table (see (i) above) i.e.: Where the string ends
            /// in the buffer i.e.: What basic_string::size() returns
            size_type codeunit_size_{ 0 };
        };

        using internal_size_type = std::conditional_t<
            is_utf8_string,
            internal_size_multibyte,
            internal_size_codeunits>;

        /// \brief Size variable, abstracting the difference between
        /// utf8/utf16/utf32 strings
        internal_size_type size_;
    };

    /// \brief Typedef of string (data type: char)
    using string = basic_string<detail::utf8_char_type>;

    /// \brief Typedef of u8string (data type char8_t)
    using u8string = basic_string<detail::utf8_char_type>;

    /// \brief Typedef of string (data type: char)
    using string_view = std::basic_string_view<
        detail::utf8_char_type,
        std::char_traits<detail::utf8_char_type>>;

    /// \brief Typedef of string (data type: char8_t)
    using u8_string_view = std::basic_string_view<
        detail::utf8_char_type,
        std::char_traits<detail::utf8_char_type>>;

    /// \brief Forms a string literal of the desired type
    /// returns std::string{str, len}
    inline small::string operator""_ss(const char *str, std::size_t len) {
        return small::string{ str, len };
    }

    /// \brief Interprets a signed integer value in the string str
    /// calls std::strtol(str.c_str(), &ptr, base) or std::wcstol(str.c_str(),
    /// &ptr, base) \throws std::invalid_argument if no conversion could be
    /// performed \throws std::out_of_range if the converted value would fall
    /// out of the range of the result type or if the underlying function
    /// (std::strtol or std::strtoll) sets errno to ERANGE.
    inline int
    stoi(const small::string &str, std::size_t *pos = nullptr, int base = 10) {
        errno = 0;
        char *p_end;
        // this is OK because numbers are in the same value range
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const long i = std::strtol(s_begin, &p_end, base);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stoi(small::string): no conversion could be performed");
        }
        if (errno == ERANGE
            || detail::cmp_greater(i, std::numeric_limits<int>::max())
            || detail::cmp_less(i, std::numeric_limits<int>::min()))
        {
            detail::throw_exception<std::out_of_range>(
                "stoi(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return static_cast<int>(i);
    }

    /// \brief Interprets a signed integer value in the string str
    /// calls std::strtol(str.c_str(), &ptr, base) or std::wcstol(str.c_str(),
    /// &ptr, base) \throws std::invalid_argument if no conversion could be
    /// performed \throws std::out_of_range if the converted value would fall
    /// out of the range of the result type or if the underlying function
    /// (std::strtol or std::strtoll) sets errno to ERANGE.
    inline long
    stol(const small::string &str, std::size_t *pos = nullptr, int base = 10) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const long i = std::strtol(s_begin, &p_end, base);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stol(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stol(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets a signed integer value in the string str
    /// calls std::strtoll(str.c_str(), &ptr, base) or std::wcstoll(str.c_str(),
    /// &ptr, base) \throws std::invalid_argument if no conversion could be
    /// performed \throws std::out_of_range if the converted value would fall
    /// out of the range of the result type or if the underlying function
    /// (std::strtol or std::strtoll) sets errno to ERANGE.
    inline long long
    stoll(const small::string &str, std::size_t *pos = nullptr, int base = 10) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const long long i = std::strtoll(s_begin, &p_end, base);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stoll(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stoll(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets an unsigned integer value in the string str
    /// calls std::strtoul(str.c_str(), &ptr, base) or std::wcstoul(str.c_str(),
    /// &ptr, base) \throws std::invalid_argument if no conversion could be
    /// performed \throws std::out_of_range if the converted value would fall
    /// out of the range of the result type or if the underlying function
    /// (std::strtoul or std::strtoull) sets errno to ERANGE.
    inline unsigned long
    stoul(const small::string &str, std::size_t *pos = nullptr, int base = 10) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const unsigned long i = std::strtoul(s_begin, &p_end, base);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stoul(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stoul(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets an unsigned integer value in the string str
    /// calls std::strtoull(str.c_str(), &ptr, base) or
    /// std::wcstoull(str.c_str(), &ptr, base) \throws std::invalid_argument if
    /// no conversion could be performed \throws std::out_of_range if the
    /// converted value would fall out of the range of the result type or if the
    /// underlying function (std::strtoul or std::strtoull) sets errno to ERANGE.
    inline unsigned long long
    stoull(const small::string &str, std::size_t *pos = nullptr, int base = 10) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const unsigned long long i = std::strtoull(s_begin, &p_end, base);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stoull(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stoull(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets a floating point value in a string str
    /// calls std::strtof(str.c_str(), &ptr) or std::wcstof(str.c_str(), &ptr)
    /// \throws std::invalid_argument if no conversion could be performed
    /// \throws std::out_of_range if the converted value would fall out of the
    /// range of the result type or if the underlying function (std::strtoul or
    /// std::strtoull) sets errno to ERANGE.
    inline float
    stof(const small::string &str, std::size_t *pos = nullptr) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const float i = std::strtof(s_begin, &p_end);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stof(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stof(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets a floating point value in a string str
    /// calls std::strtod(str.c_str(), &ptr) or std::wcstod(str.c_str(), &ptr)
    /// \throws std::invalid_argument if no conversion could be performed
    /// \throws std::out_of_range if the converted value would fall out of the
    /// range of the result type or if the underlying function (std::strtoul or
    /// std::strtoull) sets errno to ERANGE.
    inline double
    stod(const small::string &str, std::size_t *pos = nullptr) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const double i = std::strtod(s_begin, &p_end);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stod(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stod(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Interprets a floating point value in a string str
    /// calls std::strtold(str.c_str(), &ptr) or std::wcstold(str.c_str(), &ptr)
    /// \throws std::invalid_argument if no conversion could be performed
    /// \throws std::out_of_range if the converted value would fall out of the
    /// range of the result type or if the underlying function (std::strtoul or
    /// std::strtoull) sets errno to ERANGE.
    inline long double
    stold(const small::string &str, std::size_t *pos = nullptr) {
        errno = 0;
        char *p_end;
        auto s_begin = reinterpret_cast<const char *>(str.c_str());
        const long double i = std::strtold(s_begin, &p_end);
        if (s_begin == p_end) {
            detail::throw_exception<std::invalid_argument>(
                "stold(small::string): no conversion could be performed");
        }
        if (errno == ERANGE) {
            detail::throw_exception<std::out_of_range>(
                "stold(small::string): converted value falls out of the range "
                "of the result type");
        }
        if (pos) {
            *pos = p_end - s_begin;
        }
        return i;
    }

    /// \brief Converts a numeric value to small::string
    /// We use a string stream rather than the usual std::sprintf for this
    /// operation because it's safer. In any case, we should prefer from_chars /
    /// to_chars.
    template <
        typename T,
        std::enable_if_t<
            std::is_integral_v<T> || std::is_floating_point_v<T>,
            int> = 0>
    small::string
    to_string(T value) {
        std::ostringstream ss;
        ss << value;
        return small::string(ss.str());
    }

    /// \brief Checks if the string has malformed utf code units
    /// This string class allows loose and malformed sequences of code units in
    /// order to be compatible with std::string and provide byte access. We
    /// assume only utf8 strings can be malformed at this point and that
    /// utf16/utf32 can only have one code point per code unit, which is
    /// strictly false for utf16.
    template <
        typename CharT,
        size_t N,
        class Traits,
        typename WCharT,
        class Allocator,
        size_t CP_HINT_STEP,
        class SizeType>
    [[nodiscard]] constexpr bool
    is_malformed(
        const small::basic_string<
            CharT,
            N,
            Traits,
            WCharT,
            Allocator,
            CP_HINT_STEP,
            SizeType> &string) noexcept {
        using string_type = small::basic_string<
            CharT,
            N,
            Traits,
            WCharT,
            Allocator,
            CP_HINT_STEP,
            SizeType>;
        if constexpr (!detail::is_utf32_v<typename string_type::value_type>) {
            auto first = string.begin();
            auto last = string.end();
            while (first != last) {
                if (::small::detail::is_utf_continuation(*first)) {
                    return true;
                } else {
                    uint8_t codepoint_size = detail::utf_size(*first, 8);
                    ++first;
                    for (uint8_t i = 1; i < codepoint_size; ++i) {
                        if (!detail::is_utf_continuation(*first)) {
                            return true;
                        }
                        ++first;
                    }
                }
            }
            return false;
        } else {
            return false;
        }
    }

    /// \brief Make small strings relocatable when they are inside a vector
    /// This allows us to create an inline vector of inline strings that we can
    /// memcpy
    template <
        typename CharT,
        size_t N,
        class Traits,
        typename WCharT,
        class Allocator,
        size_t CP_HINT_STEP,
        class SizeType>
    struct is_relocatable<small::basic_string<
        CharT,
        N,
        Traits,
        WCharT,
        Allocator,
        CP_HINT_STEP,
        SizeType>> : std::true_type
    {};

} // namespace small

///// \brief std::hash specialization
namespace std {

    template <
        typename CharT,
        size_t N,
        class Traits,
        typename WCharT,
        class Allocator,
        size_t CP_HINT_STEP,
        class SizeType>
    struct hash<small::basic_string<
        CharT,
        N,
        Traits,
        WCharT,
        Allocator,
        CP_HINT_STEP,
        SizeType>>
    {
        std::size_t
        operator()(const small::basic_string<
                   CharT,
                   N,
                   Traits,
                   WCharT,
                   Allocator,
                   CP_HINT_STEP,
                   SizeType> &string) const noexcept {
            using value_type = typename small::basic_string<
                CharT,
                N,
                Traits,
                WCharT,
                Allocator,
                CP_HINT_STEP,
                SizeType>::value_type;
            std::hash<value_type> hasher;
            std::size_t size = string.size();
            std::size_t result = 0;
            const value_type *buffer = string.data();
            for (std::size_t iterator = 0; iterator < size; ++iterator)
                result = result * 31u + hasher(buffer[iterator]);
            return result;
        }
    };
} // namespace std

#endif // SMALL_STRING_HPP
