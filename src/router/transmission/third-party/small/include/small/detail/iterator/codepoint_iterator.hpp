//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ITERATOR_CODEPOINT_ITERATOR_HPP
#define SMALL_DETAIL_ITERATOR_CODEPOINT_ITERATOR_HPP

#include <small/detail/algorithm/console_unicode_guard.hpp>
#include <small/detail/algorithm/utf.hpp>
#include <small/detail/container/lookup_table_view.hpp>
#include <small/detail/exception/throw.hpp>
#include <ostream>
#include <string_view>

namespace small {
    namespace detail {
        /// \brief This iterator class represents an iterator for utf8
        /// codepoints The codepoints are represented by a span from 1 to 4
        /// bytes The iterator needs to have access to:
        /// - its underlying string, which allows it to identify the size of a
        /// codepoint
        /// - a lookup table, which makes it random access
        namespace detail {
            /// \brief Type to indirectly represent a codepoint reference in a
            /// string container
            ///
            /// The codepoint reference only needs to know where the codepoint
            /// starts (its byte index), so that we know how many bytes we need
            /// to consider to lazily dereference it as a wide char.
            ///
            /// However, to be able to change the codepoint value, we need to
            /// have access to the underlying container so that we can call a
            /// function to update this reference value. This is what makes this
            /// complicated. So, in practice, this will only work efficiently
            /// with any string class with support for the our
            /// lookup_table_view.
            ///
            /// Like with iterators, we use templates to define the mutable and
            /// constant versions at once.
            ///
            template <bool IS_CONST, class String>
            struct codepoint_reference_impl
            {
            public:
                /// \brief The string type on which we are working
                using string_type = std::
                    conditional_t<IS_CONST, const String, String>;

                /// \brief The type being used represent the UTF8 codepoints
                /// Although we make this class work correctly for wide chars,
                /// this is really meant to be 1-byte chars
                using value_type = typename string_type::value_type;

                /// \brief The char type to which the codepoints get converted
                /// when we dereference them
                using wide_value_type = typename string_type::wide_value_type;

                /// \brief The type our container uses for indexes
                using size_type = typename string_type::size_type;

                /// \brief Constant to make code clearer
                /// Everything that's 1-byte is considered to be utf8
                constexpr static bool is_utf8 = sizeof(value_type) == 1;

            public:
                /// \brief The constructor containers iterators should use to
                /// create a reference
                codepoint_reference_impl(
                    string_type *container,
                    size_type codepoint_index,
                    size_type byte_index_hint) noexcept
                    : container_(container), codepoint_index_(codepoint_index),
                      byte_index_(byte_index_hint) {}

                /// \brief Get the codepoint size of this reference
                constexpr size_type
                size() const {
                    if constexpr (sizeof(value_type) == 1) {
                        return utf8_size(
                            *(container_->begin() + byte_index_),
                            container_->size() - byte_index_);
                    } else {
                        return utf8_size(container_->operator[](byte_index_));
                    }
                }

                /// \brief Cast to the wide char type if we need to access the
                /// value This should work in O(1) time because we have a hint
                /// to the byte index
                operator typename String::wide_value_type() const noexcept {
                    if constexpr (not is_utf32_v<value_type>) {
                        // Get reference to element and convert from utf8 to
                        // wide type
                        wide_value_type v;
                        to_utf32(
                            container_->begin() + byte_index_,
                            size(),
                            &v,
                            1);
                        return v;
                    } else {
                        // Just cast at that position
                        return static_cast<wide_value_type>(
                            container_->operator[](byte_index_));
                    }
                }

                /// \brief Streaming a codepoint to the console
                /// In POSIX systems, the console usually expects a span of UTF8
                /// chars. Sending a wide value to the console will usually show
                /// its integer value. In Windows, as usual, things are a little
                /// more complicated. For now, we convert the value to a wide
                /// value, which might work with wcout. A more consistent
                /// solution would be calling something like
                /// `SetConsoleOutputCP(CP_UTF8);` or `_setmode(...,
                /// _O_U16TEXT)` if we identify this codepoint has more than 1
                /// byte and the console hasn't been set to unicode yet. We
                /// would also need to ensure `os` is cout or wcout. It seems
                /// like fmt implements some similar solution. Once we find a
                /// consistent solution, we can apply that to the small::string
                /// operator<<.
                friend std::ostream &
                operator<<(
                    std::ostream &os,
                    const codepoint_reference_impl &impl) {
                    if constexpr (not is_utf32_v<value_type>) {
                        if (impl.size() == 1) {
                            os << static_cast<char>(
                                impl.container_->operator[](impl.byte_index_));
                        } else {
                            console_unicode_guard g(os, impl.size(), 1);
                            os << std::string_view(
                                reinterpret_cast<const char *>(
                                    impl.container_->data())
                                    + impl.byte_index_,
                                impl.size());
                        }
                    } else {
                        // If the underlying container is utf32, just cast the
                        // code unit at that position
                        os << static_cast<wide_value_type>(
                            impl.container_->operator[](impl.byte_index_));
                    }
                    return os;
                }

                /// \brief Assignment operator from a char
                /// The char might be of the same type or another type. It might
                /// even be an array with a list of code units. In any case,
                /// this is just something for which we can replace in the
                /// original string. If required, the string is responsible for
                /// shifting items and doing all the hard work.
                ///
                /// This function should not be available for const references.
                template <typename Char>
                codepoint_reference_impl &
                operator=(Char codepoint) {
                    static_assert(
                        !IS_CONST,
                        "Cannot assign to a constant reference");
                    // Replace using the codepoint index as a hint for the
                    // replace operation. This hint makes it faster for the
                    // string to find the elements in the lookup table that
                    // might require an update.
                    container_
                        ->replace(byte_index_, codepoint, codepoint_index_);
                    return *this;
                }

                /// \brief Assignment operator from another codepoint reference
                template <bool RHS_IS_CONST, class RHS_String>
                codepoint_reference_impl &
                operator=(
                    const codepoint_reference_impl<RHS_IS_CONST, RHS_String>
                        &rhs) {
                    static_assert(
                        !IS_CONST,
                        "Cannot assign to a constant reference");
                    // Replace using the codepoint index as a hint for the
                    // replace operation. This hint makes it faster for the
                    // string to find the elements in the lookup table that
                    // might require an update.
                    container_->replace(
                        byte_index_,
                        rhs.operator wide_value_type(),
                        codepoint_index_);
                    return *this;
                }

                /// \brief Compare this reference to another reference
                template <bool RHS_IS_CONST, class RHS_String>
                bool
                operator==(
                    const codepoint_reference_impl<RHS_IS_CONST, RHS_String>
                        &rhs) const {
                    return this->operator wide_value_type()
                           == rhs.operator wide_value_type();
                }

                /// \brief Compare this reference to another char of any type
                template <
                    typename WChar,
                    std::enable_if_t<
                        std::is_convertible_v<WChar, wide_value_type>,
                        int> = 0>
                friend bool
                operator==(
                    const codepoint_reference_impl &lhs,
                    const WChar &rhs) {
                    return lhs.operator wide_value_type()
                           == static_cast<wide_value_type>(rhs);
                }

                /// \brief Compare this reference to a sequence of chars
                /// representing another codepoint
                template <typename WChar>
                friend bool
                operator==(
                    const codepoint_reference_impl &lhs,
                    const WChar *rhs) {
                    constexpr bool rhs_is_utf8 = sizeof(WChar) == 1;
                    if constexpr (rhs_is_utf8) {
                        // RHS is code units, something like: `if (ref == "😀")`
                        // Find its size, but don't look for more than we need
                        size_t rhs_min_size = strlen(rhs, 10);
                        if (rhs_min_size == 0) {
                            return false;
                        }
                        // Find its codepoint size
                        size_t rhs_utf8_size = utf8_size(*rhs, rhs_min_size);
                        if (lhs.size() != rhs_utf8_size) {
                            return false;
                        }
                        // Compare codepoints now
                        for (size_type i = 0; i < rhs_utf8_size; ++i) {
                            if (lhs.container_->operator[](lhs.byte_index_ + i)
                                != static_cast<
                                    typename codepoint_reference_impl::
                                        string_type::value_type>(rhs[i]))
                            {
                                return false;
                            }
                        }
                        return true;
                    } else {
                        // Rhs is not code units, something like: `if (ref ==
                        // U"😀")` In this case, we compare the codepoint with
                        // the string iff this is a string with a single
                        // codepoint
                        size_t rhs_min_size = strlen(rhs, 2);
                        if (rhs_min_size != 1) {
                            return false;
                        }
                        return lhs.operator wide_value_type() != *rhs;
                    }
                }

            private:
                /// \brief A pointer to the container so we can update its values
                string_type *container_;

                /// \brief A hint of the codepoint index
                /// This helps us with the assignment operation
                size_type codepoint_index_{ 0 };

                /// \brief A hint of the byte index
                size_type byte_index_{ 0 };
            };

        } // namespace detail

        /// \section Final aliases for references
        template <class String>
        using external_codepoint_reference = detail::
            codepoint_reference_impl<false, String>;
        template <class String>
        using const_external_codepoint_reference = detail::
            codepoint_reference_impl<true, String>;

        namespace detail {
            /// \brief An iterator for accessing codepoints in a string
            ///
            /// The basic functions for the codepoint iterator and const
            /// codepoint iterator.
            ///
            /// Although we could implement a _bidirectional_ codepoint iterator
            /// with no reference to the underlying container, this class needs
            /// this reference so that we can use the look up table to find
            /// elements more efficiently and make _random access_ codepoint
            /// iterators possible.
            ///
            /// Unlike other utf8 iterators with O(m) random access, the lookup
            /// table allows us to find these elements in constant time through
            /// codepoint index hints spread through the lookup table. A
            /// detailed description is in the lookup table container header
            /// file.
            ///
            template <bool IS_CONST, class String>
            struct codepoint_iterator_impl
            {
            public:
                using string_type = std::
                    conditional_t<IS_CONST, const String, String>;

                friend codepoint_iterator_impl<!IS_CONST, String>;

                using string_view_type = std::basic_string_view<
                    typename string_type::value_type,
                    typename string_type::traits_type>;

                using lookup_table_type = lookup_table_view<
                    typename string_type::value_type,
                    typename string_type::traits_type,
                    string_type::codepoint_hint_step_size,
                    typename string_type::size_type>;

                using const_lookup_table_type = const_lookup_table_view<
                    typename string_type::value_type,
                    typename string_type::traits_type,
                    string_type::codepoint_hint_step_size,
                    typename string_type::size_type>;

            public:
                /// \section The common types come from the underlying string
                /// container
                typedef std::random_access_iterator_tag iterator_category;

                /// \brief The iterator type dereferences to the wide from the
                /// string
                typedef typename string_type::wide_value_type value_type;

                /// \brief The difference types is the same as the string uses
                /// for its indexes
                typedef typename string_type::difference_type difference_type;

                /// \brief Its address is the address of the char in the string
                /// It might be also a wide type if this is a wstring
                typedef typename string_type::pointer pointer;

                /// \brief Our reference type is the proxy object that
                /// represents a reference to the char as wide char
                typedef codepoint_reference_impl<IS_CONST, String> reference;

                /// \brief Our reference type to code units
                typedef std::conditional_t<
                    IS_CONST,
                    typename string_type::const_reference,
                    typename string_type::reference>
                    codeunit_reference;

                /// \brief The size type comes from the string
                typedef typename string_type::size_type size_type;

            public:
                /// \brief Construct default iterator
                constexpr codepoint_iterator_impl() noexcept
                    : container_(nullptr), codepoint_index_(0), byte_index_(0) {
                }

                /// \brief Constructor
                /// An iterator, like the reference, needs to have access to the
                /// string (with its lookup table), and its byte/codepoint
                /// indexes, which provide hints to allow random access in the
                /// string.
                constexpr codepoint_iterator_impl(
                    string_type *container,
                    size_type codepoint_index,
                    size_type byte_index_hint) noexcept
                    : container_(container), codepoint_index_(codepoint_index),
                      byte_index_(byte_index_hint) {}

                /// \brief Copy constructor
                /// Const iterators can be created from mutable iterators but
                /// not the other way around
                template <
                    bool RHS_IS_CONST,
                    std::enable_if_t<
                        IS_CONST == RHS_IS_CONST || !RHS_IS_CONST,
                        int> = 0>
                // NOLINTNEXTLINE(google-explicit-constructor)
                constexpr codepoint_iterator_impl(
                    const codepoint_iterator_impl<RHS_IS_CONST, String>
                        &rhs) noexcept
                    : container_(rhs.container_),
                      codepoint_index_(rhs.codepoint_index_),
                      byte_index_(rhs.byte_index_) {}

                /// \brief Default constructors
                constexpr codepoint_iterator_impl(
                    const codepoint_iterator_impl &) noexcept = default;
                constexpr codepoint_iterator_impl &
                operator=(const codepoint_iterator_impl &) noexcept = default;

            public:
                /// \brief Getter for the iterator index
                [[nodiscard]] constexpr size_type
                index() const noexcept {
                    return codepoint_index_;
                }

                /// \brief Get the index of the codepoint the iterator points to
                [[nodiscard]] constexpr size_type
                byte_index() const noexcept {
                    return byte_index_;
                }

                /// \brief Get the wide char for the codepoint at this position
                [[nodiscard]] constexpr value_type
                wide_value() const noexcept {
                    // Create a reference and dereference it
                    return static_cast<value_type>(this->operator*());
                }

                /// \brief Get the char value that the iterator points to
                [[nodiscard]] constexpr typename string_type::value_type
                byte_value() const noexcept {
                    // Dereference the byte value in the string
                    return container_->operator[](byte_index_);
                }

                /// \brief Get the wide char for the codepoint at this position
                [[nodiscard]] constexpr reference
                wide_reference() const noexcept {
                    return reference(container_, codepoint_index_, byte_index_);
                }

                /// \brief Get the char value that the iterator points to
                [[nodiscard]] constexpr codeunit_reference
                byte_reference() const noexcept {
                    return container_->operator[](byte_index_);
                }

            public:
                /// \section Common iterator functions

                /// \brief Prefix ++iter
                codepoint_iterator_impl &
                operator++() noexcept {
                    this->increment();
                    return *this;
                }

                /// \brief Postfix iter++
                codepoint_iterator_impl
                operator++(int) noexcept {
                    codepoint_iterator_impl tmp{ *this };
                    this->increment();
                    return tmp;
                }

                /// \brief Prefix --iter
                /// Decrease the codepoint_iterator by one with
                codepoint_iterator_impl &
                operator--() noexcept {
                    this->decrement();
                    return *this;
                }

                /// \brief Postfix iter--
                /// Decrease the codepoint_iterator by one with
                codepoint_iterator_impl
                operator--(int) noexcept {
                    codepoint_iterator_impl tmp{ *this };
                    this->decrement();
                    return tmp;
                }

                /// \brief Advance the by n positions in place
                codepoint_iterator_impl &
                operator+=(typename codepoint_iterator_impl::difference_type
                               n) noexcept {
                    this->advance(n);
                    return *this;
                }

                /// \brief Decrease the Iterator n times in place
                codepoint_iterator_impl &
                operator-=(typename codepoint_iterator_impl::difference_type
                               n) noexcept {
                    this->advance(-n);
                    return *this;
                }

                /// \brief Returns the value of the codepoint behind the
                /// codepoint_iterator
                reference
                operator*() const noexcept {
                    return this->wide_reference();
                }

            public:
                /// \section Relational operators declared inline
                /// These operators consider the codepoint indexes although the
                /// byte indexes would have the same results

                friend constexpr bool
                operator==(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ == b.codepoint_index_;
                }

                friend constexpr bool
                operator!=(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ != b.codepoint_index_;
                }

                friend constexpr bool
                operator<(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ < b.codepoint_index_;
                }

                friend constexpr bool
                operator<=(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ <= b.codepoint_index_;
                }

                friend constexpr bool
                operator>(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ > b.codepoint_index_;
                }

                friend constexpr bool
                operator>=(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ >= b.codepoint_index_;
                }

                /// \section Relational operators with size_type and symmetry
                /// for convenience

                friend constexpr bool
                operator==(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ == b;
                }

                friend constexpr bool
                operator==(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a == b.codepoint_index_;
                }

                friend constexpr bool
                operator!=(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ != b;
                }

                friend constexpr bool
                operator!=(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a != b.codepoint_index_;
                }

                friend constexpr bool
                operator<(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ < b;
                }

                friend constexpr bool
                operator<(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a < b.codepoint_index_;
                }

                friend constexpr bool
                operator<=(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ <= b;
                }

                friend constexpr bool
                operator<=(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a <= b.codepoint_index_;
                }

                friend constexpr bool
                operator>(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ > b;
                }

                friend constexpr bool
                operator>(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a > b.codepoint_index_;
                }

                friend constexpr bool
                operator>=(
                    const codepoint_iterator_impl &a,
                    size_type b) noexcept {
                    return a.codepoint_index_ >= b;
                }

                friend constexpr bool
                operator>=(
                    size_type a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a >= b.codepoint_index_;
                }

            public:
                /// \section Arithmetic operations declared inline

                friend constexpr codepoint_iterator_impl
                operator+(
                    codepoint_iterator_impl a,
                    const codepoint_iterator_impl &b) noexcept {
                    a -= b.codepoint_index_;
                    return a;
                }

                friend constexpr codepoint_iterator_impl
                operator+(codepoint_iterator_impl a, size_type b) noexcept {
                    a += b;
                    return a;
                }

                /// \section Arithmetic operations with size_type and symmetry
                /// for convenience
                friend constexpr codepoint_iterator_impl
                operator-(codepoint_iterator_impl a, size_type b) noexcept {
                    a -= b;
                    return a;
                }

                friend constexpr difference_type
                operator-(
                    const codepoint_iterator_impl &a,
                    const codepoint_iterator_impl &b) noexcept {
                    return a.codepoint_index_ - b.codepoint_index_;
                }

            protected:
                /// \section Members

                /// \brief Reference to the container to allow random access
                string_type *container_ = nullptr;

                /// \brief Codepoint counter
                size_type codepoint_index_;

                /// \brief Codepoint byte, as a hint for random access
                size_type byte_index_;

            protected:
                /// \section Moving iterators
                /// This is where the complex operations come in because we need
                /// to check the lookup table correctly to update the byte_index
                /// hint in constant time.
                ///

                /// \brief Move the iterator one codepoint ahead
                /// Incrementing is the easiest operation, as it doesn't depend
                /// on the lookup table for constant time
                void
                increment() {
                    constexpr bool string_is_utf8
                        = sizeof(typename string_type::value_type) == 1;
                    if constexpr (string_is_utf8) {
                        if (container_->empty()) {
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator: cannot increment on an "
                                "empty string");
                            return;
                        }
                        if (byte_index_ == container_->size()) {
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator: cannot increment the "
                                "end() iterator");
                            return;
                        }
                        const codeunit_reference &first_code_unit
                            = byte_reference();
                        const size_type bytes_left = container_->size()
                                                     - byte_index_;
                        const uint8_t codepoint_size
                            = utf8_size(first_code_unit, bytes_left);
                        byte_index_ += codepoint_size;
                    } else {
                        ++byte_index_;
                    }
                    ++codepoint_index_;
                }

                /// \brief Move the iterator one codepoint backwards
                /// Incrementing and decrementing are easier operations, as they
                /// don't depend on the lookup table for constant time
                void
                decrement() {
                    // We have 4 candidates for the previous bytecode, depending
                    // on the size of the previous codepoint
                    constexpr bool string_is_utf8
                        = sizeof(typename string_type::value_type) == 1;
                    if constexpr (string_is_utf8) {
                        if (container_->empty()) {
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator::decrement: cannot "
                                "decrement on an empty string");
                            return;
                        }
                        if (byte_index_ == 0) {
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator::decrement: cannot "
                                "decrement the begin() iterator");
                            return;
                        }

                        // Check which previous byte is the multibyte first char
                        size_type code_units_before = 1;
                        typename string_type::value_type current_byte
                            = container_->operator[](
                                byte_index_ - code_units_before);
                        while (is_utf8_continuation(current_byte)
                               && code_units_before < byte_index_) {
                            ++code_units_before;
                            current_byte = container_->operator[](
                                byte_index_ - code_units_before);
                        }
                        if (code_units_before > byte_index_) {
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator::decrement: only "
                                "continuation bytes were found");
                            return;
                        }
                        byte_index_ -= code_units_before;
                    } else {
                        --byte_index_;
                    }
                    --codepoint_index_;
                }

                /// \brief Advance the iterator n times
                void
                advance(difference_type n) noexcept {
                    // Handle the trivial cases
                    if (n == 0) {
                        return;
                    }
                    if (n == 1) {
                        increment();
                        return;
                    }
                    if (n == -1) {
                        decrement();
                        return;
                    }
                    // Advance the current codepoint index. This is what really
                    // requires the lookup table.
                    constexpr bool string_is_utf8
                        = sizeof(typename string_type::value_type) == 1;
                    if constexpr (string_is_utf8) {
                        // Trivial case, empty string
                        if (container_->empty()) {
                            // Impossible to advance
                            throw_exception<std::out_of_range>(
                                "codepoint_iterator::advance: cannot advance "
                                "on an empty container");
                            return;
                        }

                        // Our final destination
                        const size_type new_codepoint_index = codepoint_index_
                                                              + n;
                        assert(
                            new_codepoint_index
                            <= container_->size_codepoints());

                        // Create a lookup table view for this string
                        // note: don't include container_->size()+1 in the sv!
                        //       - the lookup table depends on sv.size()
                        //       matching string.size()
                        typename string_type::const_pointer first_ptr
                            = container_->data();
                        string_view_type sv(first_ptr, container_->size());

                        const size_type full_size = container_
                                                        ->buffer_capacity();
                        const size_type main_str_size = container_->size() + 1;
                        const size_type lookup_table_capacity = full_size
                                                                - main_str_size;
                        const_lookup_table_type table(
                            container_->data() + main_str_size,
                            lookup_table_capacity,
                            sv);

                        // Trivial case, no multibyte codepoints
                        if (table.empty()) {
                            byte_index_ += n;
                        } else {
                            // Increment or decrement the byte index with the
                            // lookup table using lookup_iterator = typename
                            // const_lookup_table_type::iterator;
                            auto [it, codepoint_idx]
                                = table.lower_bound_codepoint(
                                    new_codepoint_index);
                            if (new_codepoint_index == codepoint_idx) {
                                // The destination codepoint is a multibyte
                                // codepoint whose codepoint index is already
                                // what we looking for
                                byte_index_ = it.byte_index();
                            } else if (it == table.begin()) {
                                // The first multibyte codepoint comes after
                                // what we are looking for This means we are in
                                // that initial region where the codepoints
                                // match the bytes
                                byte_index_ = new_codepoint_index;
                            } else {
                                // We found the closest multibyte codepoint, but
                                // this was not what we are looking for yet.
                                // Because of how lower bound functions work,
                                // this codepoint_idx we found must be greater
                                // than what we are looking for. So our target
                                // codepoint must be between the multibyte
                                // codepoint we found and the multibyte
                                // codepoint before.

                                // Get the codepoint of the multibyte before
                                // without using its expensive codepoint_index()
                                // function.
                                const size_type multibyte_bytes_idx
                                    = it != table.end() ?
                                          it.byte_index() :
                                          container_->size();
                                const size_type multibyte_codepoint_idx
                                    = it != table.end() ?
                                          codepoint_idx :
                                          container_->size_codepoints();

                                // Find the multibyte codepoint and byte indexes
                                // before that
                                auto multibyte_before = std::prev(it);
                                const size_type multibyte_before_bytes_idx
                                    = multibyte_before.byte_index();
                                const size_type byte_distance
                                    = multibyte_bytes_idx
                                      - multibyte_before_bytes_idx;
                                const uint8_t multibyte_before_utf8_size
                                    = multibyte_before.utf8_size();
                                const size_type codepoint_distance
                                    = byte_distance - multibyte_before_utf8_size
                                      + 1;
                                const size_type multibyte_before_codepoint_idx
                                    = multibyte_codepoint_idx
                                      - codepoint_distance;

                                // Find the codepoint and byte index of the next
                                // codepoint now
                                const size_type next_unibyte_byte_idx
                                    = multibyte_before_bytes_idx
                                      + multibyte_before_utf8_size;
                                const size_type next_unibyte_codepoint_idx
                                    = multibyte_before_codepoint_idx + 1;

                                // The range between next_unibyte and the
                                // unibyte we are searching is now composed of
                                // only unibyte codepoints so their distance is
                                // finally trivial
                                const size_type unibyte_codepoint_distance
                                    = new_codepoint_index
                                      - next_unibyte_codepoint_idx;
                                byte_index_ = next_unibyte_byte_idx
                                              + unibyte_codepoint_distance;
                            }
                        }
                        codepoint_index_ = new_codepoint_index;
                    } else {
                        byte_index_ += n;
                        codepoint_index_ += n;
                    }
                }
            };
        } // namespace detail

        template <class String>
        using external_codepoint_iterator = detail::
            codepoint_iterator_impl<false, String>;
        template <class String>
        using const_external_codepoint_iterator = detail::
            codepoint_iterator_impl<true, String>;
        template <class String>
        using reverse_external_codepoint_iterator = std::reverse_iterator<
            external_codepoint_iterator<String>>;
        template <class String>
        using const_reverse_external_codepoint_iterator = std::reverse_iterator<
            const_external_codepoint_iterator<String>>;

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ITERATOR_CODEPOINT_ITERATOR_HPP
