//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_CONTAINER_ALIGNED_STORAGE_VECTOR_HPP
#define SMALL_DETAIL_CONTAINER_ALIGNED_STORAGE_VECTOR_HPP

#include <small/detail/exception/throw.hpp>
#include <small/detail/iterator/iterator_type_traits.hpp>
#include <small/detail/iterator/pointer_wrapper.hpp>
#include <cassert>
#include <cstdlib>
#include <vector>

/// \file This is the type we used to use for max_size_vector
/// We now use the inline vector with an extra template paramater to forbids
/// going beyond inline storage This is only here for reference

namespace small {
    namespace detail {
        /// Forward-declare small_vector to make it a friend of
        /// aligned_storage_vector
        template <
            class T,
            size_t N,
            class Allocator,
            class AllowHeap,
            class SizeType,
            class GrowthFactor>
        class vector;

        /// \brief Small array of elements
        /// This is a class very similar to small_vector in the
        /// sense that it allocates elements in the stack.
        /// The main difference is once we reach the maximum
        /// aligned_storage_vector capacity, there's no buffer to allocate extra
        /// elements. This is useful when we know we won't need
        /// this buffer or we can erase elements we don't need
        /// at certain points.
        /// \note This is somewhat equivalent to boost::static_vector
        /// \tparam T Array type
        /// \tparam N Array maximum size
        template <
            class T,
            size_t N = std::
                max((sizeof(std::vector<T>) * 4) / sizeof(T), std::size_t(5))>
        class aligned_storage_vector
        {
            public /* types */:
            // There's no custom allocator for aligned_storage_vector
            typedef T &reference;
            typedef const T &const_reference;
            typedef size_t size_type;
            typedef ptrdiff_t difference_type;
            typedef T value_type;
            typedef T *pointer;
            typedef const T *const_pointer;
            typedef pointer_wrapper<pointer> iterator;
            typedef pointer_wrapper<const_pointer> const_iterator;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

            public /* constructors */:
            /// \brief Construct empty small array
            constexpr aligned_storage_vector() noexcept
                : aligned_storage_vector(0) {}

            /// \brief Construct small array with size n
            constexpr explicit aligned_storage_vector(size_type n) : size_(n) {
                for (size_type pos = 0; pos < size_; ++pos) {
                    construct_at(pos);
                }
                assert(invariants());
            }

            /// \brief Construct small array with size n and fill with single
            /// value
            constexpr aligned_storage_vector(
                size_type n,
                const value_type &value)
                : size_(n) {
                for (size_type pos = 0; pos < size_; ++pos) {
                    construct_at(pos, value);
                }
                assert(invariants());
            }

            /// \brief Construct small array from a pair of iterators
            template <class InputIterator>
            constexpr aligned_storage_vector(
                InputIterator first,
                enable_if_iterator_t<InputIterator, value_type> last)
                : size_(std::distance(first, last)) {
                auto it = first;
                for (size_type pos = 0; pos < size_; ++pos) {
                    construct_at(pos, *it);
                    ++it;
                }
                assert(invariants());
            }

            /// \brief Construct small array from initializer list
            constexpr aligned_storage_vector(
                std::initializer_list<value_type> il)
                : aligned_storage_vector(il.begin(), il.end()) {}

            public /* rule of five */:
            /// \brief Rule of 5: Destructor
            ~aligned_storage_vector() {
                // Call the destructor for element we previously allocated
                destroy_all();
            }

            /// \brief Rule of 5: Copy Constructor
            aligned_storage_vector(const aligned_storage_vector &other)
                : size_(other.size_) {
                // Replicate share
                for (size_type pos = 0; pos < size_; ++pos) {
                    // Construct each element with copy constructor
                    construct_at(pos, other[pos]);
                }
            }

            /// \brief Rule of 5: Move Constructor
            aligned_storage_vector(aligned_storage_vector &&other) noexcept
                : size_(other.size_) {
                // Move resource ownership
                // Construct each element with move constructor
                for (size_type pos = 0; pos < other.size(); ++pos) {
                    construct_at(pos, std::move(other[pos]));
                }
                // Destroy elements in other
                other.destroy_all();
                other.size_ = 0;
            }

            /// \brief Rule of 5: Copy Assignment
            aligned_storage_vector &
            operator=(const aligned_storage_vector &other) {
                // Make sure they are not the same
                if (this == &other) {
                    return *this;
                }
                // Reuse copy constructor to replicate share
                destroy_all();
                for (size_type pos = 0; pos < other.size(); ++pos) {
                    construct_at(pos, other[pos]);
                }
                size_ = other.size_;
                return *this;
            }

            /// \brief Rule of 5: Move Assignment
            aligned_storage_vector &
            operator=(aligned_storage_vector &&other) noexcept {
                // Make sure they are not the same
                if (this == &other) {
                    return *this;
                }
                // Move resource ownership
                destroy_all();
                for (size_type pos = 0; pos < other.size(); ++pos) {
                    // Construct each element with move constructor
                    construct_at(pos, std::move(other[pos]));
                }
                // Destruct the elements from the original vector
                other.destroy_all();
                size_ = other.size_;
                other.size_ = 0;
                return *this;
            }

            public /* assign, fill, and swap */:
            /// \brief Assign small array from initializer list
            constexpr aligned_storage_vector &
            operator=(std::initializer_list<value_type> il) {
                assign(il);
                return *this;
            }

            /// \brief Assign small array from iterators
            template <class InputIterator>
            constexpr void
            assign(
                InputIterator first,
                enable_if_iterator_t<InputIterator, value_type> last) {
                const auto n = std::distance(first, last);
                assert(
                    (size_t(n) <= capacity())
                    && "assign() called with more elements than "
                       "aligned_storage_vector capacity");

                // Destroy all
                destroy_all();
                size_ = 0;

                // Construct copies
                for (auto it = first; it != last; ++it) {
                    // Construct each element with move constructor
                    emplace_back(*it);
                }

                assert(invariants());
            }

            /// \brief Assign small array from size and fill with value
            constexpr void
            assign(size_type n, const value_type &u) {
                assert(
                    (size_t(n) <= capacity())
                    && "assign() called with more elements than "
                       "aligned_storage_vector capacity");

                // Destroy all
                destroy_all();

                // Construct copies
                for (size_type pos = 0; pos < n; ++pos) {
                    construct_at(pos, u);
                }
                size_ = n;

                assert(invariants());
            }

            /// \brief Assign small array from initializer list
            constexpr void
            assign(std::initializer_list<value_type> il) {
                assign(il.begin(), il.end());
            }

            /// \brief Fill small array with value u
            constexpr void
            fill(const T &u) {
                std::fill(begin(), end(), u);
                assert(invariants());
            }

            /// \brief Swap the contents of two small arrays
            constexpr void
            swap(aligned_storage_vector &rhs) noexcept(
                std::is_nothrow_swappable_v<T>) {
                std::swap_ranges(data(), data() + N, rhs.data());
                std::swap(size_, rhs.size_);
                assert(invariants());
            }

            public /* iterators */:
            /// \brief Get iterator to first element
            constexpr iterator
            begin() noexcept {
                return iterator(data());
            }

            /// \brief Get constant iterator to first element[[nodiscard]]
            [[nodiscard]] constexpr const_iterator
            begin() const noexcept {
                return cbegin();
            }

            /// \brief Get iterator to last element
            constexpr iterator
            end() noexcept {
                return iterator(data() + size_);
            }

            /// \brief Get constant iterator to last element
            [[nodiscard]] constexpr const_iterator
            end() const noexcept {
                return cend();
            }

            /// \brief Get iterator to first element in reverse order
            constexpr reverse_iterator
            rbegin() noexcept {
                return std::reverse_iterator<iterator>(end());
            }

            /// \brief Get constant iterator to first element in reverse order
            [[nodiscard]] constexpr const_reverse_iterator
            rbegin() const noexcept {
                return std::reverse_iterator<const_iterator>(end());
            }

            /// \brief Get iterator to last element in reverse order
            constexpr reverse_iterator
            rend() noexcept {
                return std::reverse_iterator<iterator>(begin());
            }

            /// \brief Get constant iterator to last element in reverse order
            [[nodiscard]] constexpr const_reverse_iterator
            rend() const noexcept {
                return std::reverse_iterator<const_iterator>(begin());
            }

            /// \brief Get constant iterator to first element
            [[nodiscard]] constexpr const_iterator
            cbegin() const noexcept {
                return const_iterator(data());
            }

            /// \brief Get constant iterator to last element
            [[nodiscard]] constexpr const_iterator
            cend() const noexcept {
                return const_iterator(data() + size_);
            }

            /// \brief Get constant iterator to first element in reverse order
            [[nodiscard]] constexpr const_reverse_iterator
            crbegin() const noexcept {
                return std::reverse_iterator<const_iterator>(cend());
            }

            /// \brief Get constant iterator to last element in reverse order
            [[nodiscard]] constexpr const_reverse_iterator
            crend() const noexcept {
                return std::reverse_iterator<const_iterator>(cbegin());
            }

            public /* capacity */:
            /// \brief Get small array size
            [[nodiscard]] constexpr size_type
            size() const noexcept {
                return size_;
            }

            /// \brief Get small array max size
            [[nodiscard]] constexpr size_type
            max_size() const noexcept {
                return N;
            }

            /// \brief Check if small array is empty
            [[nodiscard]] constexpr bool
            empty() const noexcept {
                return size_ == 0;
            }

            /// \brief Get small array capacity (same as max_size())
            [[nodiscard]] constexpr size_type
            capacity() const noexcept {
                return max_size();
            }

            /// \brief Check if small array is full
            /// This is a convenience for small arrays only.
            /// Like empty() is better than size() == 0, full() is better than
            /// size() == capacity()
            [[nodiscard]] constexpr bool
            full() const noexcept {
                return size_ == N;
            }

            public /* element access */:
            /// \brief Get reference to n-th element in small array
            constexpr reference
            operator[](size_type n) {
                assert(
                    n < size()
                    && "aligned_storage_vector[] index out of bounds");
                return *(data() + n);
            }

            /// \brief Get constant reference to n-th element in small array
            constexpr const_reference
            operator[](size_type n) const {
                assert(
                    n < size()
                    && "aligned_storage_vector[] index out of bounds");
                return *(data() + n);
            }

            /// \brief Check bounds and get reference to n-th element in small
            /// array
            constexpr reference
            at(size_type n) {
                if (n >= size()) {
                    throw_exception<std::out_of_range>(
                        "at: cannot access element after vector::size()");
                }
                return operator[](n);
            }

            /// \brief Check bounds and get constant reference to n-th element
            /// in small array
            constexpr const_reference
            at(size_type n) const {
                if (n >= size()) {
                    throw_exception<std::out_of_range>(
                        "at: cannot access element after vector::size()");
                }
                return operator[](n);
            }

            /// \brief Get reference to first element in small array
            constexpr reference
            front() {
                assert(!empty() && "front() called for empty small array");
                return operator[](0);
            }

            /// \brief Get constant reference to first element in small array
            constexpr const_reference
            front() const {
                assert(!empty() && "front() called for empty small array");
                return operator[](0);
            }

            /// \brief Get reference to last element in small array
            constexpr reference
            back() {
                assert(!empty() && "back() called for empty small array");
                return operator[](size_ - 1);
            }

            /// \brief Get constant reference to last element in small array
            constexpr const_reference
            back() const {
                assert(!empty() && "back() called for empty small array");
                return operator[](size_ - 1);
            }

            /// \brief Get reference to internal pointer to small array data
            constexpr T *
            data() noexcept {
                return reinterpret_cast<T *>(&data_[0]);
            }

            /// \brief Get constant reference to internal pointer to small array
            /// data
            constexpr const T *
            data() const noexcept {
                return reinterpret_cast<const T *>(&data_[0]);
            }

            public /* modifiers */:
            /// \brief Copy element to end of small array
            constexpr void
            push_back(const value_type &x) {
                emplace_back(x);
            }

            /// \brief Move element to end of small array
            constexpr void
            push_back(value_type &&x) {
                emplace_back(std::move(x));
            }

            /// \brief Emplace element to end of small array
            template <class... Args>
            constexpr reference
            emplace_back(Args &&...args) {
                assert(!full() && "emplace_back() called for full small array");
                construct_at(size_, std::forward<Args>(args)...);
                ++size_;
                assert(invariants());
                return back();
            }

            /// \brief Remove element from end of small array
            constexpr void
            pop_back() {
                assert(!empty() && "pop_back() called for empty small array");
                destroy_at(size_ - 1);
                --size_;
                assert(invariants());
            }

            /// \brief Emplace element to a position in small array
            template <class... Args>
            constexpr iterator
            emplace(const_iterator position, Args &&...args) {
                assert(!full() && "emplace() called for full small array");
                size_t p = position - begin();
                ++size_;
                const auto underlying_position = &data_[0] + p;
                const auto underlying_new_end = &data_[0] + size_;
                shift_right(underlying_position, underlying_new_end, 1);
                construct_at(p, std::forward<Args>(args)...);
                assert(invariants());
                return begin() + p;
            }

            /// \brief Copy element to a position in small array
            constexpr iterator
            insert(const_iterator position, const value_type &x) {
                assert(!full() && "insert() called for full small array");
                return emplace(position, x);
            }

            /// \brief Move element to a position in small array
            constexpr iterator
            insert(const_iterator position, value_type &&x) {
                assert(!full() && "insert() called for full small array");
                return emplace(position, std::move(x));
            }

            /// \brief Copy n elements to a position in small array
            constexpr iterator
            insert(const_iterator position, size_type n, const value_type &x) {
                assert(
                    (size_ + n <= capacity())
                    && "insert() called with more elements than small array "
                       "capacity");
                size_t p = position - begin();
                size_ += n;
                const auto underlying_position = &data_[0] + p;
                const auto underlying_new_end = &data_[0] + size_;
                shift_right(underlying_position, underlying_new_end, n);
                for (size_t i = p; i < p + n; ++i) {
                    construct_at(i, x);
                }
                assert(invariants());
                return begin() + p;
            }

            /// \brief Copy element range stating at a position in small array
            template <class InputIterator>
            constexpr iterator
            insert(
                const_iterator position,
                InputIterator first,
                enable_if_iterator_t<InputIterator, value_type> last) {
                const size_t n = std::distance(first, last);
                assert(
                    (size_ + n <= capacity())
                    && "insert() called with more elements than small array "
                       "capacity");
                size_t p = position - begin();
                size_ += n;
                // open space for n elements
                const auto underlying_position = &data_[0] + p;
                const auto underlying_new_end = &data_[0] + size_;
                shift_right(underlying_position, underlying_new_end, n);
                size_t i = p;
                for (auto it = first; it != last; ++it) {
                    construct_at(i++, *it);
                }
                assert(invariants());
                return begin() + p;
            }

            /// \brief Copy elements from initializer list to a position in
            /// small array
            constexpr iterator
            insert(
                const_iterator position,
                std::initializer_list<value_type> il) {
                assert(
                    (size_ + il.size() <= capacity())
                    && "insert() called with more elements than small array "
                       "capacity");
                return insert(position, il.begin(), il.end());
            }

            /// \brief Erase element at a position in small array
            constexpr iterator
            erase(const_iterator position) {
                assert(!empty() && "erase() called for empty small array");
                size_type p = position - begin();
                const auto underlying_position = &data_[0] + p;
                const auto underlying_end = &data_[0] + size_;
                shift_left(underlying_position, underlying_end, 1);
                destroy_at(size_ - 1);
                --size_;
                assert(invariants());
                return begin() + p;
            }

            /// \brief Erase range of elements in the small array
            constexpr iterator
            erase(const_iterator first, const_iterator last) {
                size_t n = std::distance(first, last);
                assert(
                    (n <= size_)
                    && "erase() called for more elements than small array "
                       "size");
                size_type p = first - begin();
                const auto underlying_position = &data_[0] + p;
                const auto underlying_end = &data_[0] + size_;
                shift_left(underlying_position, underlying_end, n);
                for (size_type i = size_ - n; i != size_; ++i) {
                    destroy_at(i);
                }
                size_ -= n;
                assert(invariants());
                return begin() + p;
            }

            /// \brief Clear elements in the small array
            constexpr void
            clear() noexcept {
                destroy_all();
                size_ = 0;
                assert(invariants());
            }

            /// \brief Resize the small array
            constexpr void
            resize(size_type sz) {
                assert(
                    (sz <= capacity())
                    && "resize() called for more elements than small array "
                       "capacity");
                if (sz > size_) {
                    for (size_type i = size_; i != sz; ++i) {
                        construct_at(i);
                    }
                } else if (sz < size_) {
                    for (size_type i = sz; i != size_; ++i) {
                        destroy_at(i);
                    }
                }
                size_ = sz;
                assert(invariants());
            }

            /// \brief Resize and fill the small array
            constexpr void
            resize(size_type sz, const value_type &c) {
                assert(
                    (sz <= capacity())
                    && "resize() called for more elements than small array "
                       "capacity");
                if (sz > size_) {
                    for (size_type i = size_; i != sz; ++i) {
                        construct_at(i, c);
                    }
                } else if (sz < size_) {
                    for (size_type i = sz; i != size_; ++i) {
                        destroy_at(i);
                    }
                }
                size_ = sz;
                assert(invariants());
            }

        private:
            /// \brief Check if small array invariants are ok
            [[nodiscard]] constexpr bool
            invariants() const {
                if (size_ > capacity()) {
                    return false;
                }
                return true;
            }

            /// \brief Construct element at a given position
            template <typename... Args>
            void
            construct_at(size_type pos, Args &&...args) {
                if (pos >= N) {
                    throw_exception<std::bad_alloc>();
                }
                new (&data_[pos]) T(std::forward<Args>(args)...);
            }

            /// \brief Destroy element at a given position
            void
            destroy_at(size_type pos) {
                if (pos >= N) {
                    throw_exception<std::bad_alloc>();
                }
                reinterpret_cast<T *>(&data_[pos])->~T();
            }

            /// \brief Destroy all elements allocated
            void
            destroy_all() {
                for (size_type pos = 0; pos < size_; ++pos) {
                    destroy_at(pos);
                }
            }

            /// \brief Increment/decrement the mid iterator i by n positions
            /// \note Adapted from https://github.com/danra/shift_proposal/
            /// Increment (decrement for negative n) i |n| times or until i ==
            /// bound, whichever comes first. Returns n - the difference between
            /// i's final position and its initial position. (Note: "advance"
            /// has overloads with this behavior in the Ranges TS.)
            template <class I>
            constexpr difference_type_type<I>
            bounded_advance(I &i, difference_type_type<I> n, I const bound) {
                if constexpr (is_category_convertible<
                                  I,
                                  std::bidirectional_iterator_tag>) {
                    while (n < 0 && i != bound) {
                        ++n;
                        void(--i);
                    }
                }

                while (n > 0 && i != bound) {
                    --n;
                    void(++i);
                }

                return n;
            }

            /// \brief Shift elements left
            /// \note Adapted from https://github.com/danra/shift_proposal/
            template <class ForwardIt>
            ForwardIt
            shift_left(
                ForwardIt first,
                ForwardIt last,
                difference_type_type<ForwardIt> n) {
                if (n <= 0) {
                    return last;
                }

                auto mid = first;
                if (bounded_advance(mid, n, last)) {
                    return first;
                }

                return std::
                    move(std::move(mid), std::move(last), std::move(first));
            }

            /// \brief Shift elements right
            /// \note Adapted from https://github.com/danra/shift_proposal/
            template <class ForwardIt>
            ForwardIt
            shift_right(
                ForwardIt first,
                ForwardIt last,
                difference_type_type<ForwardIt> n) {
                if (n <= 0) {
                    return first;
                }

                if constexpr (is_category_convertible<
                                  ForwardIt,
                                  std::bidirectional_iterator_tag>)
                {
                    auto mid = last;
                    if (bounded_advance(mid, -n, first)) {
                        return last;
                    }
                    return std::move_backward(
                        std::move(first),
                        std::move(mid),
                        std::move(last));
                } else {
                    auto result = first;
                    if (bounded_advance(result, n, last)) {
                        return last;
                    }

                    // Invariant: next(first, n) == result
                    // Invariant: next(trail, n) == lead
                    auto lead = result;
                    auto trail = first;

                    for (; trail != result; ++lead, void(++trail)) {
                        if (lead == last) {
                            // The range looks like:
                            //
                            //   |-- (n - k) elements --|-- k elements --|-- (n
                            //   - k) elements --|
                            //   ^-first          trail-^ ^-result last-^
                            //
                            // Note that distance(first, trail) ==
                            // distance(result, last)
                            std::move(
                                std::move(first),
                                std::move(trail),
                                std::move(result));
                            return result;
                        }
                    }

                    for (;;) {
                        for (auto mid = first; mid != result;
                             ++lead, void(++trail), ++mid) {
                            if (lead == last) {
                                // The range looks like:
                                //
                                //   |-- (n - k) elements --|-- k elements --|--
                                //   ... --|-- n elements
                                //   --|
                                //   ^-first            mid-^         result-^
                                //   ^-trail last-^
                                //
                                trail = std::move(mid, result, std::move(trail));
                                std::move(
                                    std::move(first),
                                    std::move(mid),
                                    std::move(trail));
                                return result;
                            }
                            std::iter_swap(mid, trail);
                        }
                    }
                }
            }

        private:
            /// \brief Internal array
            /// We need aligned storage rather than array because we need
            /// properly aligned *uninitialized* storage for the elements
            /// An array would try to initialize all elements in the array
            /// even if we don't use them
            /// \see https://en.cppreference.com/w/cpp/types/aligned_storage
            typename std::aligned_storage<sizeof(T), alignof(T)>::type data_[N];

            /// \brief How much of the internal array we are actually using
            size_t size_{ 0 };
        };

        /// Type deduction
        template <class T, class... U>
        aligned_storage_vector(T, U...)
            -> aligned_storage_vector<T, 1 + sizeof...(U)>;

        /// \brief operator== for small arrays
        template <class T, size_t N>
        constexpr bool
        operator==(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return std::equal(x.begin(), x.end(), y.begin(), y.end());
        }

        /// \brief operator!= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator!=(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return !(x == y);
        }

        /// \brief operator< for small arrays
        template <class T, size_t N>
        constexpr bool
        operator<(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return std::
                lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
        }

        /// \brief operator> for small arrays
        template <class T, size_t N>
        constexpr bool
        operator>(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return y < x;
        }

        /// \brief operator<= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator<=(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return !(y < x);
        }

        /// \brief operator>= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator>=(
            const aligned_storage_vector<T, N> &x,
            const aligned_storage_vector<T, N> &y) {
            return !(x < y);
        }

        /// \brief swap the contents of two small arrays
        template <class T, size_t N>
        void
        swap(
            aligned_storage_vector<T, N> &x,
            aligned_storage_vector<T, N> &y) noexcept(noexcept(x.swap(y))) {
            x.swap(y);
        }

        /// \brief create small array from raw array
        template <class T, size_t N_INPUT, size_t N_OUTPUT = N_INPUT>
        constexpr aligned_storage_vector<std::remove_cv_t<T>, N_OUTPUT>
        to_small_array(T (&a)[N_INPUT]) {
            return aligned_storage_vector<std::remove_cv_t<T>, N_OUTPUT>(
                a,
                a + N_INPUT);
        }

        /// \brief create small array from raw array
        template <class T, size_t N_INPUT, size_t N_OUTPUT = N_INPUT>
        constexpr aligned_storage_vector<std::remove_cv_t<T>, N_OUTPUT>
        to_small_array(T (&&a)[N_INPUT]) {
            return aligned_storage_vector<std::remove_cv_t<T>, N_OUTPUT>(
                a,
                a + N_INPUT);
        }

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_CONTAINER_ALIGNED_STORAGE_VECTOR_HPP
