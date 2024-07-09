//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_CONTAINER_LOOKUP_TABLE_VIEW_HPP
#define SMALL_DETAIL_CONTAINER_LOOKUP_TABLE_VIEW_HPP

#include <small/detail/algorithm/shift.hpp>
#include <small/detail/algorithm/to_unsigned.hpp>
#include <small/detail/algorithm/utf.hpp>
#include <small/detail/container/span-lite.hpp>
#include <small/detail/exception/throw.hpp>
#include <cstring>
#include <optional>


namespace small {
    namespace detail {
        /// \class small::string lookup table
        ///
        /// This container represents a span of bytes from a small::string
        /// representing a lookup table for codepoints. Because of how these
        /// tables works, the API is a little weird as it resembles the vector
        /// API when inserting elements and the map API when querying elements.
        ///
        /// We can access and manipulate this lookup table from an API that
        /// resembles a map so that it's easy and efficient to manipulate the
        /// lookup table from the a small::string.
        ///
        /// Lookup tables are fundamental to allow random access to utf8
        /// strings, since these string may contain multibyte codepoints.
        ///
        /// With this lookup table, we can access any codepoint in O(1) time!
        ///
        /// A codepoint iterator is a random access iterator to (logical) tuples
        /// generated lazily.
        ///
        /// An iterator that points to the i-th multibyte codepoint,
        /// lazily/logically dereferences to a std::tuple<size_type, size_type,
        /// size_type> with the following elements:
        /// - <0>: index of the i-th multibyte codepoint among other multibyte
        /// codepoints
        /// - <1>: index of the i-th multibyte codepoint in the byte string (in
        /// which byte it starts)
        /// - <2>: index of the i-th multibyte codepoint among all codepoints
        ///
        /// This values are calculated are returned by the iterator but not
        /// stored anywhere. So to make this easier to remember and to avoid
        /// calculating something we don't need, each iterator provides the
        /// following functions to calculate only the relevante elements:
        /// - <0>: iterator::multibyte_index();
        /// - <1>: iterator::byte_index();
        /// - <2>: iterator::codepoint_index();
        ///
        /// Example:
        /// - If we create a string "a😀b😆c🤣"
        /// - The lookup table will contain information about the "😀", "😆",
        /// "🤣" codepoints
        /// - Iterating these codepoints from begin() to end() and returning all
        /// its values we would get:
        /// - pre condition:  it = table.begin();
        /// - step 1:         it == make_tuple(/* multibyte_index */ 0, /*
        /// byte_index /* 1,  /* codepoint_index */ 1); it++;
        /// - step 2:         it == make_tuple(/* multibyte_index */ 1, /*
        /// byte_index /* 6,  /* codepoint_index */ 3); it++;
        /// - step 3:         it == make_tuple(/* multibyte_index */ 2, /*
        /// byte_index /* 11, /* codepoint_index */ 5); it++;
        /// - post condition: it == table.end();
        ///
        /// The internal workings of this table are a little more complicated.
        ///
        /// Example:
        /// - The string "a😀b😆c🤣" has:
        ///   - 6 codepoints
        ///      - 3 1-byte codepoints
        ///      - 3 4-byte codepoints
        ///   - 3 * 1 + 3 * 4 = 15 bytes in total
        ///
        /// - The codepoint positions (0-indexed) are:
        ///   - 'a'  - codepoint_index: 0 - byte_index: 0
        ///   - '😀' - codepoint_index: 1 - byte_index: 1
        ///   - 'b'  - codepoint_index: 2 - byte_index: 5
        ///   - '😆' - codepoint_index: 3 - byte_index: 6
        ///   - 'c'  - codepoint_index: 4 - byte_index: 10
        ///   - '🤣' - codepoint_index: 5 - byte_index: 11
        ///
        /// - The lookup table indexes only multibyte codepoints, so it
        /// contains:
        ///   - '😀' - multibyte_codepoint_index: 0: byte_index: 1
        ///   (codepoint_index: 1)
        ///   - '😆' - multibyte_codepoint_index: 1: byte_index: 6
        ///   (codepoint_index: 3)
        ///   - '🤣' - multibyte_codepoint_index: 2: byte_index: 11
        ///   (codepoint_index: 5)
        ///
        /// - In C++, the codepoint iterators in the range begin()/end() would
        /// dereference to:
        ///   - '😀' - std::pair(/* multibyte_index */ 0, /* byte_index */ 1)  /*
        ///   codepoint_index 1 */
        ///   - '😆' - std::pair(/* multibyte_index */ 1, /* byte_index */ 6)  /*
        ///   codepoint_index 3 */
        ///   - '🤣' - std::pair(/* multibyte_index */ 2, /* byte_index */ 11)
        ///   /* codepoint_index 5 */
        ///
        /// This compact information allows us to find codepoints in a string by
        /// their index in O(m) time, where m is the number of multibyte
        /// indexes. Meanwhile, the common 1-byte codepoint strings are not
        /// penalized and still have random access in O(1) time.
        ///
        /// Note that:
        /// (i)  even though '😆' is in the table, if we want the codepoint index
        /// 3 of '😆', this table does
        ///      not contain this information. We need to iterate the multibyte
        ///      indexes to identify the codepoint index of '😆'. This is what
        ///      makes the operation O(m) on the number of multibyte codepoints.
        /// (ii) the table makes us spend at least one extra byte per multibyte
        /// codepoint in strings
        ///      with up to 256 bytes (two extra bytes in strings with up to
        ///      2^15=65536). If the string is mostly multibyte, we have a
        ///      25%-100% larger string (from 4 byte codepoints to 2 byte
        ///      codepoints). If we consider the usual growth factor of vectors
        ///      and assume we always use their reserved space, this would be
        ///      more like 6%-75%.
        ///
        /// So we are very efficient for strings with mostly 1-byte codepoints,
        /// but the next problem is generalizing that for strings with O(n)
        /// multibyte codepoints. So, in summary, our next problem is:
        /// - We have this high time and memory cost when generalizing for
        /// strings with O(n)
        ///   multibyte codepoints, where O(m) becomes O(n)).
        /// - we already spend about 6%-75% extra memory for codepoints, which
        /// is a reasonable cost,
        ///   but we cannot store much more stuff in the lookup table or it
        ///   would be less efficient than converting from and to UTF32.
        ///
        /// From the perspective of memory, the problem with many extra bytes is
        /// not that they are O(m), but that this constant needs to be less than
        /// 3 bytes for most cases or it would be simpler to use UTF16 and UTF32
        /// strings when we have O(n) multibyte codepoints. Although simply not
        /// supporting this operation in O(1) for this case, we want our strings
        /// to at least generalize well with O(1) time when that's the case.
        ///
        /// Given these constraints, when generalizing the lookup tables to make
        /// them O(1) for strings with O(n) multibyte codepoints, some obvious
        /// (bad) ideas come to mind:
        /// - Storing the codepoint index rather than the byte index (this is
        /// just replacing one
        ///   rarer O(m) operation with one expensive O(n) on which we depend
        ///   more)
        /// - Storing the codepoint index AND the byte index in the lookup
        /// table. This does work,
        ///   but makes us spend 12%-150% more bytes per code points, which is
        ///   not reasonable.
        ///
        /// The solution to this problem is to spread the codepoint index
        /// information in the table. If we know the codepoint index of a
        /// codepoint, we also know the codepoint index of the next codepoint
        /// with a constant operation.
        ///
        /// We do not need the codepoint index of all codepoints to keep this
        /// operation constant. All we need is the information for some
        /// codepoint such that we can ensure we can find a relative codepoint
        /// index in constant time:
        ///
        /// - Inserting codepoint indexes in the table (codepoint stride = 1)
        ///   - '😀' - multibyte_codepoint_index: 0 / byte_index: 1  /
        ///   codepoint_index: 1
        ///   - '😆' - multibyte_codepoint_index: 1 / byte_index: 6  /
        ///   codepoint_index: 3
        ///   - '🤣' - multibyte_codepoint_index: 2 / byte_index: 11 /
        ///   codepoint_index: 5
        ///
        /// - Inserting codepoint indexes in the table (codepoint stride = 2)
        ///   - '😀' - multibyte_codepoint_index: 0 / byte_index: 1
        ///   - '😆' - multibyte_codepoint_index: 1 / byte_index: 6  /
        ///   codepoint_index: 3
        ///   - '🤣' - multibyte_codepoint_index: 2 / byte_index: 11
        ///
        /// Note that we create a codepoint entry when (i % stride == stride -
        /// 1) instead of i % stride == 0 to ensure no larger entries are
        /// created unless we need to.
        ///
        /// When the stride = 1, we have 1 codepoint index reference for each
        /// codepoint.
        /// - [6% , 75%] extra memory per multibyte codepoint indexed in the
        /// lookup table
        /// - First subscript access as fast as in 1-byte codepoint string (but
        /// O(1) rather than O(m)) When the stride = 2, we have 1 codepoint
        /// index reference for every 2 codepoints.
        /// - [3% , 37%] extra memory per multibyte codepoint indexed in the
        /// lookup table
        /// - First subscript access 1.5 slower as in 1-byte codepoint string
        /// (but still O(1) rather than O(m)) When the stride = 10, we have 1
        /// codepoint index reference for every 10 codepoints.
        /// - [2.5% , 7.5%] extra memory per multibyte codepoint indexed in the
        /// lookup table
        /// - First subscript access 5 times slower as in 1-byte codepoint
        /// string (but still O(1) rather than O(m)) When the stride = 100, we
        /// have 1 codepoint index reference for every 100 codepoints.
        /// - [0.25% , 0.75%] extra memory per multibyte codepoint indexed in
        /// the lookup table
        /// - First subscript access 50 times slower as in 1-byte codepoint
        /// string (but still O(1) rather than O(m)) When the stride = n, we
        /// have 1 codepoint index reference for every n codepoints.
        /// - [25/n%, 37/n%] extra memory per multibyte codepoint indexed in the
        /// lookup table
        /// - First subscript access n/2 slower as in 1-byte codepoint string
        /// (but still O(1) rather than O(m))
        ///
        /// A minor problem is that entries in the lookup table don't have fixed
        /// size anymore, which is fine, as we are already operating the the
        /// byte level and these steps are predictable.
        ///
        /// A second problem is defining a default step size.
        ///
        /// Although any O(1) step size will keep subscript access to O(1) in
        /// any case, and bidirectional iterators can take it from there:
        /// - subscript access is still fast (O(1)) whenever m = O(1)
        /// - a small n will make multibyte subscript access faster (still O(1))
        /// at a higher memory cost when m = O(n)
        /// - a large n will make multibyte subscript access slower (still O(1))
        /// at a lower memory cost when m = O(n)
        ///
        /// Considering that:
        /// - this an edge case for which we still want O(1) subscript access,
        /// - there's no extra access cost after the first access if user uses
        /// iterators from the first codepoint found,
        /// - the proportions involved in memory cost when changing to
        /// UTF16-UTF32: [100%,400%] more for all codepoints
        /// - this step size can be adjusted through an extra template parameter
        /// if this happens to be a problem
        ///
        /// We settle on a default step size = 10:
        /// - an extra [2.5% , 7.5%] memory cost is much less than the
        /// [100%,400%] memory cost of UTF16/UTF32
        /// - an extra 10 times subscript access cost for the first access can
        /// be easily amortized by subsequent
        ///   accesses with iterators or by not using subscript access at all
        /// - there's no penalty at all for one-byte codepoint strings
        ///
        template <
            bool IS_CONST_LOOKUP,
            class CharT,
            class Traits,
            size_t step_size = 10,
            class SizeType = size_t>
        class lookup_table_view_impl
        {
        public:
            /// \section Custom types
            using byte_type = std::conditional_t<
                IS_CONST_LOOKUP,
                const std::byte,
                std::byte>;
#if span_USES_STD_SPAN
            static constexpr std::size_t dynamic_extent_value
                = std::dynamic_extent;
#else
            static constexpr nonstd::span_lite::extent_t dynamic_extent_value
                = nonstd::span_lite::dynamic_extent;
#endif
            using byte_span_type = nonstd::span<byte_type, dynamic_extent_value>;
            using string_view_type = std::basic_string_view<CharT, Traits>;

        public:
            /// \section Usual map types
            /// The types return SizeType because that's what they refer to
            /// A number of transformations need happen before the size_type
            /// value is obtained because the values are stored in less than
            /// sizeof(size_type) bytes, so this container doesn't return
            /// references at all, because there's no point in emulating them.

            typedef SizeType key_type;
            typedef SizeType mapped_type;
            typedef std::pair<const key_type, mapped_type> value_type;
            typedef std::less<> key_compare;
            typedef value_type reference;
            typedef const value_type const_reference;
            typedef value_type pointer;
            typedef const value_type const_pointer;
            typedef SizeType size_type;
            typedef ptrdiff_t difference_type;

        private:
            /// \section Iterator implementation

            /// \brief An iterator to access the values in the lookup table
            /// The layout of a lookup table in bytes is somewhat reversed
            /// - The last byte indicates the size of its size
            /// - The bytes before indicate its size
            /// - The bytes before represent the entries
            /// That's why the iterator is moving backwards in the span when
            /// it's moving forward in the table
            template <bool IS_CONST>
            class iterator_impl
            {
            public:
                /// \section Types
                /// The types are the same for const and mutable iterators
                /// because this container is a view that doesn't return
                /// references. Everything is calculated and returned by value.
                typedef std::random_access_iterator_tag iterator_category;
                typedef typename lookup_table_view_impl::value_type value_type;
                typedef typename lookup_table_view_impl::difference_type
                    difference_type;
                typedef typename lookup_table_view_impl::pointer pointer;
                typedef typename lookup_table_view_impl::reference reference;

            public:
                /// \section Constructors

                /// \brief Construct empty lookup table iterator which not good
                /// for much
                constexpr iterator_impl() noexcept
                    : base_table_(nullptr), multibyte_index_(0) {}

                /// \brief Construct lookup table iterator from the table and
                /// the position
                constexpr explicit iterator_impl(
                    const lookup_table_view_impl *base,
                    size_type index) noexcept
                    : base_table_(base), multibyte_index_(index) {}

                /// \brief Construct lookup table iterator from another lookup
                /// table iterator Const iterators can be created from mutable
                /// iterators but not the other way around
                template <
                    bool RHS_IS_CONST,
                    std::enable_if_t<
                        IS_CONST == RHS_IS_CONST || !RHS_IS_CONST,
                        int> = 0>
                // NOLINTNEXTLINE(google-explicit-constructor)
                constexpr iterator_impl(
                    const iterator_impl<RHS_IS_CONST> &rhs) noexcept
                    : base_table_(rhs.base_table_),
                      multibyte_index_(rhs.multibyte_index_) {}

            public:
                /// \section Codepoint information functions
                /// These are the functions we really need because they are more
                /// efficient and it's where the logic for finding codepoint
                /// indexes is implemented

                /// \brief Get the multibyte index of this codepoint among other
                /// multibyte indexes This is the easiest function because it's
                /// what the iterator refers to
                constexpr size_type
                multibyte_index() const {
                    return multibyte_index_;
                }

                /// \brief Get the span on bytes in the lookup table
                /// representing this entry A pointer to the position where the
                /// mapped type of a byte index is stored and the entry size is
                /// obtained in the resulting span \return Span to bytes in the
                /// lookup table relative to this iterator
                [[nodiscard]] constexpr byte_span_type
                span() const {
                    assert(
                        multibyte_index_ <= base_table_->size()
                        && "lookup_table_view::iterator::byte_index: "
                           "attempting to access more multibytes than there "
                           "are");
                    typename byte_span_type::pointer key_ptr
                        = base_table_->data_.data() + base_table_->data_.size();
                    // move back 1 byte that always represents the size of size
                    key_ptr--;
                    // move back the number of bytes always representing the
                    // size itself
                    const size_type s = base_table_->size_of_size();
                    key_ptr -= s;
                    // move back the number of entries + 1 (1 for this entry
                    // itself)
                    const uint8_t es = base_table_->entry_size();
                    key_ptr -= (multibyte_index_ + 1) * es;
                    // move back the number of special codepoint entries we have
                    // between usual byte_index entries we have a special
                    // codepoint entry every step_size entries (see the class
                    // comments)
                    key_ptr -= multibyte_index_ / step_size;
                    // we arrived at the position representing the byte index
                    return byte_span_type(key_ptr, es);
                }

                /// \brief If this is an entry at a step size, get the byte span
                /// representing this value
                [[nodiscard]] constexpr byte_span_type
                codepoint_span() const {
                    assert(
                        (multibyte_index_ % step_size == step_size - 1)
                        && "this_step_codepoint_index: this iterator is not at "
                           "a step size");
                    // So we do know its codepoint index. It's stored one entry
                    // ahead
                    auto bytes = span();
                    // Return previous bytes
                    return byte_span_type(
                        bytes.data() - bytes.size(),
                        bytes.size());
                }

                /// \brief Get the byte index of this codepoint in the base
                /// string We need to find the byte corresponding to this
                /// codepoint in the table as bytes and the convert the value we
                /// find to size_type. This is very dependant on the layout of
                /// the lookup table
                constexpr size_type
                byte_index() const {
                    if (base_table_->empty() || is_end()) {
                        return base_table_->str_.size();
                    }
                    // Get bytes in the lookup table that represent this
                    // iterator position
                    auto entry_bytes = span();
                    // We just need to convert this entry from its proper size
                    // type now
                    return to_unsigned<size_type>(entry_bytes);
                }

                /// \brief Set the byte index of this codepoint in the base
                /// string
                constexpr void
                byte_index(size_type index) const {
                    if (base_table_->empty()) {
                        return;
                    }
                    // Get bytes in the lookup table that represent this
                    // iterator position
                    auto entry_bytes = span();
                    // We just need to convert this entry from its proper size
                    // type now
                    return to_bytes<size_type>(index, entry_bytes);
                }

                /// \brief Get the codepoint index of this multibyte codepoint
                /// IFF this is at a step size
                [[nodiscard]] constexpr bool
                is_at_step_size() const {
                    return multibyte_index_ % step_size == step_size - 1
                           && not is_end();
                }

                /// \brief Check if this is the end iterator
                [[nodiscard]] constexpr bool
                is_end() const {
                    return base_table_->size() == multibyte_index_;
                }

                /// \brief Get the codepoint index of this multibyte codepoint
                /// IFF this is at a step size
                [[nodiscard]] constexpr bool
                contains_codepoint_information() const {
                    return is_at_step_size() || multibyte_index_ == 0;
                }

            private:
                /// \brief Get the codepoint index of this multibyte codepoint
                /// IFF this is at a step size
                constexpr size_type
                this_step_codepoint_index() const {
                    // A special case
                    if (multibyte_index_ == 0) {
                        return byte_index();
                    }
                    byte_span_type previous_bytes = codepoint_span();
                    return to_unsigned<size_type>(previous_bytes);
                }

            public:
                /// \brief The utf8 size in bytes of this multibyte codepoint
                constexpr uint8_t
                utf8_size() {
                    const auto codepoint_first_byte = base_table_
                                                          ->str_[byte_index()];
                    const auto bytes_left = base_table_->str_.size()
                                            - codepoint_first_byte;
                    return ::small::detail::
                        utf8_size(codepoint_first_byte, bytes_left);
                }

                /// \brief Get the codepoint index of this multibyte codepoint
                /// among all codepoints This is still a little more complicated
                /// than the previous operation because we only know the
                /// codepoint for codepoints at every step_size positions \param
                /// force_from_previous Force using only previous elements
                /// (rather than closest) as a reference \return The codepoint
                /// index of that multibyte codepoint
                constexpr size_type
                codepoint_index() const {
                    if (base_table_->empty()) {
                        // if this is an empty table, this is a special case of
                        // is_end() the end iterator should assume we are
                        // talking about the final null char. If the multibyte
                        // table is empty, the codepoint size is the same as the
                        // number of bytes
                        return base_table_->str_.size();
                    }
                    if (is_end()) {
                        // If this is the end iterator, we should assume this
                        // refers to the null char. If we got here, the table is
                        // not empty so this is not the same as the size()
                        // because we have multibyte codepoints. We can't use
                        // size_codepoint() either because base_table_->str_ is
                        // a string view. So we take the last entry in the
                        // multibyte table and calculate the difference from
                        // there.
                        iterator_impl last = std::prev(*this);
                        const size_type last_multibyte_codepoint_idx
                            = last.codepoint_index();
                        const size_type last_multibyte_codeunit_idx
                            = last.byte_index();
                        const size_type end_multibyte_byte_idx
                            = this->byte_index();
                        const size_type byte_distance
                            = end_multibyte_byte_idx
                              - last_multibyte_codeunit_idx;
                        const size_type codepoint_distance
                            = byte_distance - last.utf8_size() + 1;
                        const size_type end_multibyte_codepoint_idx
                            = last_multibyte_codepoint_idx + codepoint_distance;
                        return end_multibyte_codepoint_idx;
                    }
                    // Check if this is a multibyte index for which we also know
                    // the codepoint index
                    if (contains_codepoint_information()) {
                        // So we know this contains a codepoint index too
                        return this_step_codepoint_index();
                    }

                    // Otherwise, things get a little more complicated.
                    // The previous entry with a codepoint index
                    const size_type step_size_mod = multibyte_index_ % step_size;
                    const size_type positions_before_to_step_size
                        = step_size_mod + 1;
                    const size_type positions_before_to_first = multibyte_index_;
                    const size_type positions_before = std::
                        min(positions_before_to_first,
                            positions_before_to_step_size);

                    // Get iterator to its PREVIOUS codepoint for which we know
                    // the index explicitly
                    iterator_impl<true> multibyte_it(
                        base_table_,
                        multibyte_index_ - positions_before);
                    // Store its codepoint index as a starting point
                    size_type codepoint_index
                        = multibyte_it.this_step_codepoint_index();

                    // Get iterator to its NEXT multibyte codepoint
                    iterator_impl<true> adjancent_multibyte_it = std::next(
                        multibyte_it);
                    for (;;) {
                        // Compare their distances in bytes in the original
                        // string
                        const size_type multibyte_byte_idx = multibyte_it
                                                                 .byte_index();
                        const size_type adjacent_multibyte_byte_idx
                            = adjancent_multibyte_it.byte_index();
                        assert(
                            adjacent_multibyte_byte_idx > multibyte_byte_idx);
                        const size_type byte_distance
                            = adjacent_multibyte_byte_idx - multibyte_byte_idx;
                        // The number of 1-byte codepoints between them should
                        // be roughly this distance, but we still need to access
                        // the underlying string to know the exact number
                        const auto codepoint_first_byte
                            = base_table_->str_[multibyte_byte_idx];
                        const uint8_t multibyte_bytes = ::small::detail::
                            utf8_size(
                                codepoint_first_byte,
                                base_table_->str_.size() - multibyte_byte_idx);
                        // Increment the number of codepoints between these two
                        // multibyte codepoints
                        // - One codepoint for each one-byte codepoint, which is
                        // not in the table
                        // - Minus the number of bytes of this multibyte (which
                        // we can't count as more than one)
                        // - Plus 1 for the current multibyte codepoint (which
                        // we haven't counted yet)
                        codepoint_index += byte_distance - multibyte_bytes + 1;
                        // Advance iterators
                        if (adjancent_multibyte_it != *this) {
                            ++multibyte_it;
                            ++adjancent_multibyte_it;
                        } else {
                            break;
                        }
                    }
                    return codepoint_index;
                }

            public:
                /// \section Element access

                /// \brief Dereference iterator
                /// We could also return the codepoint index here but we don't
                /// expose that because this is a more expensive operation for
                /// which can also get the results with codepoint_index()
                constexpr reference
                operator*() const noexcept {
                    return std::make_pair(multibyte_index(), byte_index());
                }

                /// \brief Dereference iterator and get member
                constexpr pointer
                operator->() const noexcept {
                    return operator*();
                }

                /// \brief Dereference iterator n positions ahead
                constexpr reference
                operator[](difference_type n) const noexcept {
                    iterator_impl<IS_CONST> it(*this);
                    it += n;
                    return it.operator*();
                }

                /// \brief Get base index in the table
                constexpr size_type
                base() const noexcept {
                    return multibyte_index_;
                }

                /// \brief Get reference to the base table
                constexpr const lookup_table_view_impl &
                base_table() const noexcept {
                    return *base_table_;
                }

                public /* modifiers */:
                /// \brief Advance iterator
                constexpr iterator_impl &
                operator++() noexcept {
                    ++multibyte_index_;
                    return *this;
                }

                /// \brief Advance iterator (postfix)
                constexpr iterator_impl
                operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
                    iterator_impl tmp(*this);
                    ++(*this);
                    return tmp;
                }

                /// \brief Rewind iterator
                constexpr iterator_impl &
                operator--() noexcept {
                    --multibyte_index_;
                    return *this;
                }

                /// \brief Rewind iterator (postfix)
                constexpr iterator_impl
                operator--(int) noexcept { // NOLINT(cert-dcl21-cpp)
                    iterator_impl tmp(*this);
                    --(*this);
                    return tmp;
                }

                /// \brief Return copy of iterator advanced by n positions
                constexpr iterator_impl
                operator+(difference_type n) const noexcept {
                    iterator_impl w(*this);
                    w += n;
                    return w;
                }

                /// \brief Advance iterator by n positions
                constexpr iterator_impl &
                operator+=(difference_type n) noexcept {
                    multibyte_index_ += n;
                    return *this;
                }

                /// \brief Return copy of iterator n positions behind
                constexpr iterator_impl
                operator-(difference_type n) const noexcept {
                    return *this + (-n);
                }

                /// \brief Rewind iterator by n positions
                constexpr iterator_impl &
                operator-=(difference_type n) noexcept {
                    *this += -n;
                    return *this;
                }

                public /* relational operators */:
                /// \brief Make the other lookup table iterator type a friend
                template <bool RHS_IS_CONST>
                friend class iterator_impl;

                public /* friends */:
                /// \brief Get distance between iterators
                template <bool LHS_IS_CONST, bool RHS_IS_CONST>
                constexpr friend auto
                operator-(
                    const iterator_impl<LHS_IS_CONST> &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept
                    -> decltype(x.multibyte_index() - y.multibyte_index());

                /// \brief Sum iterators
                template <bool RHS_IS_CONST>
                constexpr friend iterator_impl<RHS_IS_CONST> operator+(
                    typename iterator_impl<RHS_IS_CONST>::difference_type,
                    iterator_impl<RHS_IS_CONST>) noexcept;

            public:
                /// \section Equality
                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator==(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return x.multibyte_index() == y.multibyte_index();
                }

                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator!=(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return !(x == y);
                }

                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator<(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return x.multibyte_index() < y.multibyte_index();
                }

                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator>(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return y < x;
                }

                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator>=(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return !(x < y);
                }

                template <bool RHS_IS_CONST>
                constexpr friend bool
                operator<=(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return !(y < x);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator==(
                    const iterator_impl &x,
                    const MultibyteIndexType &y) noexcept {
                    return x.multibyte_index() == y;
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator!=(
                    const iterator_impl &x,
                    const MultibyteIndexType &y) noexcept {
                    return !(x == y);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator>(
                    const iterator_impl &x,
                    const MultibyteIndexType &y) noexcept {
                    return y < x;
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator>=(
                    const iterator_impl &x,
                    const MultibyteIndexType &y) noexcept {
                    return !(x < y);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator<=(
                    const iterator_impl &x,
                    const MultibyteIndexType &y) noexcept {
                    return !(y < x);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator==(
                    const MultibyteIndexType &x,
                    const iterator_impl &y) noexcept {
                    return x == y.multibyte_index();
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator!=(
                    const MultibyteIndexType &x,
                    const iterator_impl &y) noexcept {
                    return !(x == y);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator>(
                    const MultibyteIndexType &x,
                    const iterator_impl &y) noexcept {
                    return y < x;
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator>=(
                    const MultibyteIndexType &x,
                    const iterator_impl &y) noexcept {
                    return !(x < y);
                }

                template <
                    class MultibyteIndexType,
                    std::enable_if_t<
                        std::is_convertible_v<MultibyteIndexType, size_type>,
                        int> = 0>
                constexpr friend bool
                operator<=(
                    const MultibyteIndexType &x,
                    const iterator_impl &y) noexcept {
                    return !(y < x);
                }

                template <bool RHS_IS_CONST>
                constexpr friend size_type
                operator-(
                    const iterator_impl &x,
                    const iterator_impl<RHS_IS_CONST> &y) noexcept {
                    return x.multibyte_index() - y.multibyte_index();
                }

                template <bool RHS_IS_CONST>
                constexpr friend iterator_impl
                operator+(
                    typename iterator_impl::difference_type n,
                    iterator_impl<RHS_IS_CONST> x) noexcept {
                    x += n;
                    return x;
                }

            private:
                /// \brief A pointer to the table
                /// We need to access its byte layout
                const lookup_table_view_impl *base_table_{ nullptr };

                /// \brief The index of the element among multibyte codepoints
                size_type multibyte_index_{ 0 };
            };

        public:
            /// \section Iterator aliases
            typedef iterator_impl<false> iterator;
            typedef iterator_impl<true> const_iterator;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

            /// \class This is just a proxy for std::less
            class value_compare
            {
                friend class lookup_table_view_impl;

            protected:
                key_compare comp;
                constexpr explicit value_compare() : comp(key_compare()) {}
                constexpr explicit value_compare(key_compare c) : comp(c) {}

            public:
                constexpr bool
                operator()(const value_type &x, const value_type &y) const {
                    return comp(x.first, y.first);
                }
            };

        private:
            /// \brief Convert to appropriate byte span type
            template <class SpanType>
            constexpr static byte_span_type
            to_byte_span(const SpanType &span) {
                if constexpr (IS_CONST_LOOKUP) {
                    return nonstd::as_bytes(span);
                } else {
                    return nonstd::as_writable_bytes(span);
                }
            }

        public:
            /// \section construct/copy/destroy
            /// The constructors API are more similar to the span constructor
            /// than map constructors We just delegate everything to the span
            /// constructors However, the codepoint map needs to know the size
            /// of the string it's indexing because it affects how many bytes we
            /// should spend per entry

            /// \brief Constructor: Empty table
            constexpr lookup_table_view_impl() noexcept : data_(), str_() {}

            /// \brief Constructor: From iterator and count
            template <class It>
            constexpr lookup_table_view_impl(
                It first,
                size_type count,
                string_view_type str)
                : data_(to_byte_span(nonstd::span(first, count))), str_(str) {}

            /// \brief Constructor: From iterator pairs
            template <class It, class End>
            constexpr lookup_table_view_impl(
                It first,
                End last,
                string_view_type str)
                : data_(to_byte_span(nonstd::span(first, last))), str_(str) {}

            /// \brief Constructor: From literal array of bytes
            template <std::size_t N>
            constexpr lookup_table_view_impl(
                byte_type (&arr)[N],
                string_view_type str) noexcept
                : data_(to_byte_span(nonstd::span(arr))), str_(str) {}

            /// \brief Constructor: From c++ array
            template <std::size_t N>
            constexpr lookup_table_view_impl(
                const std::array<byte_type, N> &arr,
                string_view_type str) noexcept
                : data_(to_byte_span(nonstd::span(arr))), str_(str) {}

            /// \brief Constructor: From ranges
            template <class R>
            constexpr lookup_table_view_impl(R &&range, string_view_type str)
                : data_(to_byte_span(nonstd::span(range))), str_(str) {}

            /// \brief Constructor: From source map/span
            template <class U>
            constexpr lookup_table_view_impl(
                const nonstd::span<U, dynamic_extent_value> &source,
                string_view_type str) noexcept
                : data_(to_byte_span(source)), str_(str) {}

            /// \brief Constructor: Copy
            constexpr lookup_table_view_impl(
                const lookup_table_view_impl &other) noexcept = default;

        public:
            /// \section Iterators to the multibyte codepoints in the string
            /// These iterators can return the multibyte_index, byte_index, and
            /// codepoint_index of each multibyte codepoint.

            /// \brief Iterator to first multibyte codepoint
            constexpr iterator
            begin() noexcept {
                return iterator(this, 0);
            }

            constexpr const_iterator
            cbegin() const noexcept {
                return const_iterator(this, 0);
            }

            constexpr const_iterator
            begin() const noexcept {
                return cbegin();
            }

            constexpr iterator
            end() noexcept {
                return iterator(this, size());
            }

            constexpr const_iterator
            cend() const noexcept {
                return const_iterator(this, size());
            }

            constexpr const_iterator
            end() const noexcept {
                return cend();
            }

            constexpr reverse_iterator
            rbegin() noexcept {
                return reverse_iterator(end());
            }

            constexpr const_reverse_iterator
            rbegin() const noexcept {
                return reverse_iterator(cend());
            }

            constexpr const_reverse_iterator
            crbegin() const noexcept {
                return reverse_iterator(cend());
            }

            constexpr reverse_iterator
            rend() noexcept {
                return reverse_iterator(begin());
            }

            constexpr const_reverse_iterator
            rend() const noexcept {
                return reverse_iterator(cbegin());
            }

            constexpr const_reverse_iterator
            crend() const noexcept {
                return reverse_iterator(cbegin());
            }

        public:
            /// \section Capacity in the lookup table
            /// We have a few kinds of sizes involved here:
            /// - The size of the indicator:   1
            /// - The size of size components: 1-4 (depends on number of
            /// multibyte codepoints)
            /// - The size of each entry:      1-4 (depends on the string size)
            ///
            /// We usually have function pairs such as resize and resize_for
            /// - The first function resets the indicator with the number of
            /// multibyte indexes in the table
            ///    - Setting the size indicator is usually enough
            /// - The second function rearranges the table entries so that they
            /// can index a new number of codepoints

            /// \brief True if table is empty
            /// We can make this cheaper than !size() by only checking the
            /// indicator
            [[nodiscard]] constexpr bool
            empty() const noexcept {
                return data_.back() == byte_type(0) || data_.empty();
            }

            /// \brief Number of multibyte codepoints in this table
            constexpr size_type
            size() const noexcept {
                const size_type ss = size_of_size();
                byte_span_type size_representation_span(
                    data_.end() - 1 - ss,
                    data_.end() - 1);
                return to_unsigned<size_type>(size_representation_span);
            }

            /// \brief Number of bytes we need for this lookup table
            constexpr size_type
            size_for() noexcept {
                return size_for(str_.size(), size());
            }

            /// \brief Calculate number of bytes we need for a lookup table with
            /// that number of entries
            constexpr static size_type
            size_for(
                size_type total_codeunits,
                size_type multibyte_codepoints) noexcept {
                const size_type indicator_size = 1;
                const size_type size_size = size_of_size_for(
                    multibyte_codepoints);
                const size_type entries_size = entry_size_for(total_codeunits)
                                               * multibyte_codepoints;
                const size_type codepoint_hints_size = entries_size / step_size;
                return indicator_size + size_size + entries_size
                       + codepoint_hints_size;
            }

            /// \brief Get the max number of entries we can store in the lookup
            /// table as it is This number usually not be much larger than the
            /// number of codepoints in the string because the reserved storage
            /// for strings is somewhat proportional to the number of bytes in
            /// the main buffer. However, this might vary a little, because
            /// string resizing takes into account an estimate on the number of
            /// multibyte codepoints that currently exist in the string.
            constexpr size_type
            max_size() const noexcept {
                // External constraints on this table
                const size_type data_span_capacity = data_.size();
                const size_type str_size = str_.size();
                // Each size component constraints on this table
                const size_type size_of_size_indicator = 1;
                const size_type size_of_size = entry_size_for(str_size);
                const size_type entry_size = entry_size_for(str_size);
                // Return number of entries we can store outside the indicators
                const size_type byte_capacity_for_entries
                    = data_span_capacity - size_of_size_indicator
                      + size_of_size;
                return byte_capacity_for_entries / entry_size;
            }

            /// \brief Reset the table size for a new number of multibyte
            /// codepoints This doesn't affect the size of an entry. Only the
            /// size indicators. If changing the size makes us have a new size
            /// of size, then this is a more expensive operation because we need
            /// to move the entries back and forth to match the new layout. This
            /// case needs to be handled but it shouldn't happen very often.
            /// \param new_size Number of multibyte codepoints in the string
            /// \return
            constexpr void
            resize(size_type max_multibyte_codepoints) noexcept {
                // Old and new indicators
                // const size_type old_size = size();
                const size_type old_size = size();
                const size_type old_size_of_size = size_of_size();
                const size_type new_size = max_multibyte_codepoints;
                const size_type new_size_of_size = size_of_size_for(new_size);
                // Set the size of size indicator
                assert(!data_.empty() && "cannot resize an empty data span");
                data_.back() = static_cast<byte_type>(new_size_of_size);

                // Check old and new size spans
                if (old_size_of_size != 0
                    && old_size_of_size != new_size_of_size) {
                    // Shift all entries to make room for the new size
                    // 1) Find range of pointer to all table entries
                    // 1.a) First byte is where the last is stored
                    iterator last_it = std::prev(end());
                    const size_type codepoint_hint_shift
                        = last_it.is_at_step_size() ?
                              entry_size_for(old_size) :
                              0;
                    typename byte_span_type::iterator byte_begin_pos
                        = last_it.span().begin() - codepoint_hint_shift;
                    // 1.b) Last byte is after where the first is stored (where
                    // size storage begins)
                    typename byte_span_type::iterator byte_end_pos
                        = data_.end() - 1 - old_size_of_size;
                    // Shift the table entries:
                    // - left by new_size_of_size - old_size_of_size bytes, or
                    // - right by old_size_of_size - new_size_of_size bytes
                    if (new_size_of_size > old_size_of_size) {
                        const size_type shift_size = old_size_of_size
                                                     - new_size_of_size;
                        typename byte_span_type::iterator new_byte_begin
                            = byte_begin_pos - shift_size;
                        shift::shift_left(
                            new_byte_begin,
                            byte_end_pos,
                            shift_size);
                    } else {
                        const size_type shift_size = old_size_of_size
                                                     - new_size_of_size;
                        typename byte_span_type::iterator new_byte_end
                            = byte_end_pos + shift_size;
                        shift::shift_right(
                            byte_begin_pos,
                            new_byte_end,
                            shift_size);
                    }
                }

                // Set the new size in the byte representation
                byte_span_type size_representation(
                    data_.end() - 1 - new_size_of_size,
                    data_.end() - 1);
                to_bytes(new_size, size_representation);
            }

            /// \brief Change the entry sizes to index a new number of 1-byte or
            /// multibyte codepoints
            constexpr void
            resize_for(size_type n_codepoints) const noexcept {
                // Check how many bytes an entry for n_codepoints takes
                constexpr size_type new_entry_size = entry_size_for(
                    n_codepoints);
                constexpr size_type old_entry_size = entry_size();
                // Resize the entries if we need to
                if (new_entry_size != old_entry_size) {
                    // Range of pointers to the old entries
                    iterator last_it = rbegin().multibyte_index();
                    typename byte_span_type::pointer old_first_address
                        = last_it.span().data();
                    if (last_it.is_at_step_size()) {
                        old_first_address -= old_entry_size;
                    }
                    iterator first_it = begin();
                    typename byte_span_type::pointer old_last_address
                        = first_it.span().data() + old_entry_size;

                    // Number of slots we need to move
                    // This includes not only the main entries but also the
                    // special codepoint index entries
                    auto slots_to_move = (old_last_address - old_first_address)
                                         / old_entry_size;

                    // Range of pointers to the new entries
                    typename byte_span_type::pointer new_last_address
                        = old_last_address;
                    typename byte_span_type::pointer new_first_address
                        = new_last_address - slots_to_move * new_entry_size;

                    // Shift the entries
                    const bool expand_left = new_entry_size > old_entry_size;
                    const bool compress_right = new_entry_size < old_entry_size;
                    if (expand_left) {
                        // Copy everything from the old range to the new range,
                        // expanding the entries
                        auto entry_size_diff = new_entry_size - old_entry_size;
                        while (old_first_address != old_last_address
                               && new_first_address != new_last_address)
                        {
                            std::memset(new_first_address, 0, entry_size_diff);
                            std::memcpy(
                                new_first_address + entry_size_diff,
                                old_first_address,
                                old_entry_size);
                            new_first_address += new_entry_size;
                            old_first_address += old_entry_size;
                        }
                    } else if (compress_right) {
                        // Copy everything from the old range to the new range,
                        // compressing the entries We need to go from last to
                        // first then
                        auto entry_size_diff = old_entry_size - new_entry_size;
                        while (old_first_address != old_last_address
                               && new_first_address != new_last_address)
                        {
                            std::memcpy(
                                new_last_address - new_entry_size,
                                old_last_address - old_entry_size
                                    + entry_size_diff,
                                old_entry_size - entry_size_diff);
                            new_last_address -= new_entry_size;
                            old_last_address -= old_entry_size;
                        }
                    }
                }
            }

            /// \brief Max. number of codepoints we can index in this table as
            /// it is The data_ span should also include the bytes we are not
            /// using for this to work
            constexpr size_type
            capacity() const noexcept {
                constexpr size_type span_size = data_.size();
                constexpr size_type indicator_size = 1;
                constexpr size_type size_size = size_of_size();
                constexpr size_type entries_size = size() * entry_size();
                constexpr size_type spare_size = span_size - indicator_size
                                                 - size_size - entries_size;
                return spare_size / entry_size();
            }

            /// \brief Minmax number of bytes for the indicator with the size of
            /// size
            constexpr static uint8_t max_size_of_size = sizeof(size_type);
            constexpr static uint8_t min_size_of_size = 0;

            /// \brief Number of bytes we need to represent the size of this
            /// lookup table
            [[nodiscard]] constexpr uint8_t
            size_of_size() const noexcept {
                return data_.empty() ? min_size_of_size :
                                       size_type(data_.back());
            }

            /// \brief Number of bytes we need to represent the size of size of
            /// a lookup table for n elements
            constexpr static size_type
            size_of_size_for(size_type n_multibytes_entries) noexcept {
                return n_multibytes_entries == 0 ?
                           min_size_of_size :
                           entry_size_for(n_multibytes_entries);
            }

            /// \brief The size, in bytes, of each entry in the codepoint lookup
            /// table This calculate how many bytes we need in a table indexing
            /// "string_size_" bytes The size of entries depend on how many
            /// entries we have in the original string
            [[nodiscard]] constexpr uint8_t
            entry_size() const noexcept {
                return entry_size_for(str_.size());
            }

            /// \brief Calculates the entry size of a table indexing n bytes
            /// \param n Number of codepoints in the string
            /// \return Size of an entry in the table
            static constexpr uint8_t
            entry_size_for(size_type n) noexcept {
                return n <= (size_type) std::numeric_limits<std::uint8_t>::max()
                                   + 1 ?
                           sizeof(std::uint8_t) :
                       n <= (size_type) std::numeric_limits<std::uint16_t>::max()
                                   + 1 ?
                           sizeof(std::uint16_t) :
                       n <= (size_type) std::numeric_limits<std::uint32_t>::max()
                                   + 1 ?
                           sizeof(std::uint32_t) :
                           sizeof(std::uint64_t);
            }

        public:
            /// \section Element access

            /// \brief Byte index of the k-th multibyte codepoint
            constexpr mapped_type
            operator[](const key_type &k) {
                assert(k < size());
                return (begin() + k).byte_index();
            }

            constexpr mapped_type
            at(const key_type &k) {
                if (k < size()) {
                    throw_exception<std::out_of_range>(
                        "lookup_table_view: out of bounds. Multibyte index not "
                        "in the table");
                }
                return (begin() + k).byte_index();
            }

        public:
            /// \section Modifiers

            /// \brief Insert a new index in the multibyte index table
            /// \param byte_index The byte index of the new last multibyte index
            constexpr void
            push_back(const mapped_type &byte_index) {
                assert(
                    max_size() >= size() + 1
                    && "Lookup table has no room for another index");
                // resize the lookup table
                size_type prev_size = size();
                size_type new_size = prev_size + 1;
                resize(new_size);
                // find the last element
                iterator it = std::prev(end());
                // get its byte span
                byte_span_type s = it.span();
                // set its value to the byte index
                to_bytes(byte_index, s);
                // check if we need to store the codepoint index
                if (it.is_at_step_size()) {
                    // span where we should store the codepoint index
                    byte_span_type codepoint_bytes = it.codepoint_span();
                    // calculate the codepoint index from the previous elements
                    if (new_size != 1) {
                        iterator prev_it = std::prev(it);
                        size_type prev_multibyte_codepoint_idx
                            = prev_it.codepoint_index(true);
                        size_type codepoint_distance
                            = byte_index - prev_it.byte_index() + 1
                              - prev_it.utf8_size();
                        to_bytes(
                            prev_multibyte_codepoint_idx + codepoint_distance,
                            codepoint_bytes);
                    } else {
                        to_bytes(byte_index, codepoint_bytes);
                    }
                }
            }

            /// \brief Set the byte index of the k-th multibyte codepoint if
            /// it's not there yet
            constexpr std::pair<iterator, bool>
            insert(
                const key_type &multibyte_index,
                const mapped_type &byte_index) {
                // Find an iterator for this multibyte index
                auto it = find(multibyte_index);
                auto end_it = end();
                if (it == end_it) {
                    // multibyte index must be >= size()
                    push_back(byte_index);
                    return std::make_pair(std::prev(end_it), true);
                } else {
                    return std::make_pair(it, false);
                }
            }

            constexpr std::pair<iterator, bool>
            insert(value_type &&v) {
                return insert(v.first, v.second);
            }

            /// \brief Set the byte index of the k-th multibyte codepoint
            constexpr std::pair<iterator, bool>
            insert_or_assign(
                const key_type &multibyte_index,
                const mapped_type &byte_index,
                const std::optional<size_type> &codepoint_index = std::nullopt) {
                // Find an iterator for this multibyte index
                assert(
                    multibyte_index <= size()
                    && "lookup_table_view::insert_or_assign: multibyte index "
                       "is larger than capacity");
                iterator it = find(multibyte_index);
                iterator end_it = end();
                if (it == end_it) {
                    resize(size() + 1);
                    iterator last_it = std::prev(end());
                    byte_span_type last_span = last_it.span();
                    to_bytes(byte_index, last_span.begin(), last_span.end());
                    if (last_it.is_at_step_size()) {
                        if (codepoint_index) {
                            byte_span_type codepoint_span
                                = last_it.codepoint_span();
                            to_bytes(*codepoint_index, codepoint_span);
                        } else {
                            if (it != begin()) {
                                iterator before_last_it = std::prev(last_it);
                                const size_type before_codepoint_idx
                                    = before_last_it.codepoint_index();
                                const size_type byte_distance
                                    = byte_index - before_last_it.byte_index();
                                const size_type codepoint_distance
                                    = byte_distance - before_last_it.utf8_size()
                                      + 1;
                                const size_type last_codepoint_idx
                                    = before_codepoint_idx + codepoint_distance;
                                byte_span_type codepoint_span
                                    = last_it.codepoint_span();
                                to_bytes(last_codepoint_idx, codepoint_span);
                            } else {
                                byte_span_type codepoint_span
                                    = last_it.codepoint_span();
                                to_bytes(byte_index, codepoint_span);
                            }
                        }
                    }
                    return std::make_pair(std::prev(end_it), true);
                } else {
                    // update span of that entry
                    byte_span_type bytes = it.span();
                    to_bytes(byte_index, bytes);
                    if (it.is_at_step_size()) {
                        if (codepoint_index) {
                            byte_span_type codepoint_span = it.codepoint_span();
                            to_bytes(*codepoint_index, codepoint_span);
                        } else {
                            if (it != begin()) {
                                iterator prev_it = std::prev(it);
                                const size_type before_codepoint_idx
                                    = prev_it.codepoint_index();
                                const size_type byte_distance
                                    = byte_index - prev_it.byte_index();
                                const size_type codepoint_distance
                                    = byte_distance - prev_it.utf8_size() + 1;
                                const size_type last_codepoint_idx
                                    = before_codepoint_idx + codepoint_distance;
                                byte_span_type codepoint_span
                                    = it.codepoint_span();
                                to_bytes(last_codepoint_idx, codepoint_span);
                            } else {
                                byte_span_type codepoint_span
                                    = it.codepoint_span();
                                to_bytes(byte_index, codepoint_span);
                            }
                        }
                    }
                    return std::make_pair(it, false);
                }
            }

            /// \brief Erase an entry from the table
            constexpr size_type
            erase(const key_type &multibyte_index) {
                auto it = find(multibyte_index);
                if (it != end()) {
                    erase(it);
                    return 1;
                }
                return 0;
            }

            constexpr iterator
            erase(iterator first, iterator last) {
                // Trivial case
                auto end_it = end();
                if (first == end_it) {
                    // go directly to resizing. the previous entry bytes are now
                    // garbage.
                    resize(size() - 1);
                    return end();
                }

                // The iterator contains a multibyte index from which we need
                // to: 1) remove that entry value by shifting back all entry
                // values after this position in the layout 2) update the
                // codepoint indexes for all step sizes that where moved
                iterator from_position = last;
                iterator to_position = first;
                while (from_position != end_it) {
                    if (to_position.is_at_step_size()) {
                        // The destination position should have a codepoint
                        // index hint We should calculate this for the source
                        // position and set it here Calculate it before setting
                        // the byte indexes because they are part of this
                        // equation
                        const size_type next_codepoint_index
                            = next_codepoint_index.codepoint_index();
                        to_position.byte_index(from_position.byte_index());
                        to_position.codepoint_index(next_codepoint_index);
                    } else {
                        to_position.byte_index(from_position.byte_index());
                    }
                    ++to_position;
                    ++from_position;
                }

                // 3) update the table size with all the memory tricks we need
                resize(size() - 1);
            }

            constexpr iterator
            erase(const_iterator first, const_iterator last) {
                return erase(
                    const_iterator(this, first.multibyte_index()),
                    const_iterator(this, last.multibyte_index()));
            }

            /// \brief Set the indicators to make this map have 0 entries
            /// We manually erase the last bytes of the lookup table span so
            /// that everything before that is considered garbage and there's
            /// nothing to shift
            constexpr void
            clear() noexcept {
                to_bytes(0U, std::prev(data_.end()), data_.end());
            }

        public:
            /// \section Observers:
            /// This is just the std::less function

            [[nodiscard]] constexpr key_compare
            key_comp() const {
                return key_compare();
            }
            [[nodiscard]] constexpr value_compare
            value_comp() const {
                return value_compare();
            }

        public:
            /// \section map operations
            /// Because this "map" is based on an array, the find functions are
            /// only returning offsets from the last elements

            /// \brief Find the element that refers to the k-th multibyte
            /// codepoint \param k Multibyte index \return
            constexpr iterator
            find(const key_type &multibyte_index) {
                if (multibyte_index < size()) {
                    return iterator(this, multibyte_index);
                } else {
                    return end();
                }
            }

            constexpr const_iterator
            find(const key_type &multibyte_index) const {
                if (multibyte_index < size()) {
                    return const_iterator(this, multibyte_index);
                } else {
                    return end();
                }
            }

            /// \brief Find the first multibyte element whose codepoint >=
            /// codepoint_index
            ///
            /// This implements a binary search on the multibyte entries with
            /// the exception that only elements at step sizes are considered.
            /// After that, we can use a linear search to find what we want.
            ///
            /// We often need to find the last multibyte element whose codepoint
            /// < new_codepoint_index, or the element before the first multibyte
            /// element whose codepoint >=new_codepoint_index.
            ///
            /// So we need a binary search for that, but we also need this
            /// binary search to skip the iterators to multibyte indexes for
            /// which we don't know the codepoints. \param codepoint_index
            /// Codepoint index we are looking for \return An iterator to the
            /// first multibyte element in the table whose codepoint is >=
            /// codepoint_index \return The codepoint of this element, which is
            /// useful to avoid recalculating when != codepoint_index
            constexpr std::pair<iterator, size_type>
            lower_bound_codepoint(const key_type &codepoint_index) {
                // Trivial case
                if (empty()) {
                    return std::
                        make_pair(end(), std::numeric_limits<size_type>::max());
                }
                // 1) Use a binary search to find the step size entry that
                // matches the codepoint index better
                iterator it;
                difference_type step{ 0 };
                auto first = begin();
                auto last = end();
                difference_type search_range = std::distance(first, last);
                while (cmp_greater(search_range, step_size - 1)) {
                    it = first;
                    step = search_range / 2;
                    std::advance(it, step);
                    if (not it.contains_codepoint_information()) {
                        // We can go a few indexes ahead or behind to fix this
                        const size_type indexes_behind = (step_size - 1)
                                                         - (it.multibyte_index()
                                                            % step_size);
                        const size_type indexes_ahead = step_size
                                                        - indexes_behind;
                        const bool move_back_is_possible = it.multibyte_index()
                                                           > indexes_behind;
                        const bool move_forward_is_possible = it + indexes_ahead
                                                              < end();
                        if (move_back_is_possible
                            && (indexes_behind < indexes_ahead
                                || move_forward_is_possible))
                        {
                            std::advance(it, -difference_type(indexes_behind));
                        } else if (move_forward_is_possible) {
                            std::advance(it, indexes_ahead);
                        } else {
                            // No more codepoints to find
                            break;
                        }
                    }
                    if (it.codepoint_index() < codepoint_index) {
                        first = ++it;
                        search_range -= step + 1;
                    } else {
                        search_range = step;
                    }
                }
                // 2) Move or advance around this iterator to find a better match
                size_type first_codepoint_index = first.codepoint_index();
                if (first_codepoint_index < codepoint_index) {
                    // The best match is the first _not less_ than the codepoint
                    // we are looking for This snippet is then equivalent to:
                    // while (first.codepoint_index() < codepoint_index) {
                    //    ++first;
                    // }
                    // However, we know we are going to iterate through
                    // codepoints which don't know they codepoint indexes now.
                    // So we accumulate the indexes ourselves.
                    while (first != last
                           && first_codepoint_index < codepoint_index) {
                        const size_type prev_byte_index = first.byte_index();
                        uint8_t prev_utf_size = first.utf8_size();
                        ++first;
                        const size_type cur_byte_index = first.byte_index();
                        const size_type byte_distance = cur_byte_index
                                                        - prev_byte_index;
                        const size_type codepoint_distance
                            = byte_distance - prev_utf_size + 1;
                        first_codepoint_index += codepoint_distance;
                    }
                } else if (first_codepoint_index > codepoint_index) {
                    // We do the opposite of the previous operation here
                    // We move backwards until we find the element we want
                    iterator begin_it = begin();
                    while (first != begin_it
                           && first_codepoint_index >= codepoint_index) {
                        const size_type prev_byte_index = first.byte_index();
                        --first;
                        uint8_t cur_utf_size = first.utf8_size();
                        const size_type cur_byte_index = first.byte_index();
                        const size_type byte_distance = prev_byte_index
                                                        - cur_byte_index;
                        const size_type codepoint_distance = byte_distance
                                                             - cur_utf_size + 1;
                        first_codepoint_index -= codepoint_distance;
                    }
                    // Move to the next, where the codepoint is greater or
                    // equal, which is what we expect from this function
                    if (first != last
                        && first_codepoint_index < codepoint_index) {
                        // Move to next and adjust the codepoint index we are
                        // going to return
                        const size_type prev_byte_index = first.byte_index();
                        uint8_t prev_utf_size = first.utf8_size();
                        ++first;
                        const size_type cur_byte_index = first.byte_index();
                        const size_type byte_distance = cur_byte_index
                                                        - prev_byte_index;
                        const size_type codepoint_distance
                            = byte_distance - prev_utf_size + 1;
                        first_codepoint_index += codepoint_distance;
                    }
                }
                return std::make_pair(first, first_codepoint_index);
            }

            /// \brief Find the first multibyte element whose codeunit >=
            /// codeunit_index
            constexpr iterator
            lower_bound_codeunit(const key_type &codeunit_index) {
                // Trivial case
                if (empty()) {
                    return end();
                }
                // Since all iterators have an associated code unit, we use a
                // regular binary search on the iterators Each iterator returns
                // a pair with <multibyte index, multibyte code unit>
                return std::lower_bound(
                    begin(),
                    end(),
                    codeunit_index,
                    [](const auto &p1, const key_type &codeunit_index) {
                    return p1.second < codeunit_index;
                    });
            }

            /// \brief Check if the table has something associated with the k-th
            /// multibyte index
            constexpr bool
            contains(const key_type &k) const {
                return k < size();
            }

            /// \brief Find the first multibyte index >= k
            constexpr iterator
            lower_bound(const key_type &k) {
                return find(k);
            }

            constexpr const_iterator
            lower_bound(const key_type &k) const {
                return find(k);
            }

            /// \brief Find the first multibyte index > k
            constexpr iterator
            upper_bound(const key_type &k) {
                iterator it = find(k);
                if (it != end()) {
                    ++it;
                }
                return it;
            }

            constexpr const_iterator
            upper_bound(const key_type &k) const {
                const_iterator it = find(k);
                if (it != end()) {
                    ++it;
                }
                return it;
            }

            constexpr std::pair<iterator, iterator>
            equal_range(const key_type &k) {
                iterator first = find(k);
                iterator last = first != end() ? std::next(first) : first;
                return std::make_pair(first, last);
            }

            constexpr std::pair<const_iterator, const_iterator>
            equal_range(const key_type &k) const {
                const_iterator first = find(k);
                const_iterator last = first != end() ? std::next(first) : first;
                return std::make_pair(first, last);
            }

        private:
            /// \brief The lookup table is represented as a span of bytes
            /// This span is part of the string buffer, whose spare allocated
            /// bytes we use for the table This span has the following layout:
            /// (i) spare bytes
            /// (ii) codepoint indexes
            ///     - index of the n-th multibyte codepoint
            ///     - index of the n-1-th multibyte codepoint
            ///     - index of the n-2-th multibyte codepoint
            ///     - ...
            ///     - index of the 1-st multibyte codepoint
            /// (iii) number of codepoint indexes / size
            /// (iv) size of size / number of bytes needed to represent 4
            ///
            /// Note that:
            /// - In this layout, with the size at the last position, operations
            /// usually read from the span
            ///   data_.end() to its data_.begin() looking for elements. This is
            ///   purposefully so because this table is usually stored at the
            ///   extra reserved bytes of string buffers.
            /// - Unibyte codepoints are not represented in the table. They are
            /// simply inferred from the local
            ///   multibyte codepoints, so finding
            /// - The table size is only 1 byte when the table is empty, in
            /// which case it simply decays into
            ///   an indicator
            byte_span_type data_;

            /// \brief A reference to the underlying string
            string_view_type str_{};
        };

        template <
            class CharT,
            class Traits,
            size_t step_size = 10,
            class SizeType = size_t>
        using lookup_table_view
            = lookup_table_view_impl<false, CharT, Traits, step_size, SizeType>;

        template <
            class CharT,
            class Traits,
            size_t step_size = 10,
            class SizeType = size_t>
        using const_lookup_table_view
            = lookup_table_view_impl<true, CharT, Traits, step_size, SizeType>;

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_CONTAINER_LOOKUP_TABLE_VIEW_HPP
