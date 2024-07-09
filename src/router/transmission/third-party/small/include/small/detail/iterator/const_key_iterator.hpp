//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ITERATOR_CONST_KEY_ITERATOR_HPP
#define SMALL_DETAIL_ITERATOR_CONST_KEY_ITERATOR_HPP

#include <small/detail/traits/add_key_const.hpp>
#include <small/detail/traits/is_pair.hpp>
#include <algorithm>
#include <iterator>
#include <type_traits>

namespace small {
    namespace detail {
        /// \brief Iterator wrapper that makes the key of the underlying pair
        /// constant
        template <class ITERATOR>
        class const_key_iterator
        {
        public:
            // Base iterator
            typedef ITERATOR base_iterator_type;
            typedef
                typename std::iterator_traits<base_iterator_type>::value_type
                    base_value_type;

            static_assert(
                (is_pair_v<base_value_type>),
                "const_key_iterator: base_value_type must be std::pair");

            // Wrapped types
            typedef typename std::iterator_traits<
                base_iterator_type>::iterator_category iterator_category;
            typedef typename add_key_const<base_value_type>::type value_type;
            typedef typename std::iterator_traits<
                base_iterator_type>::difference_type difference_type;
            typedef typename std::add_pointer_t<value_type> pointer;
            typedef typename std::add_lvalue_reference_t<value_type> reference;

            public /* constructors */:
            /// \brief Construct empty iterator wrapper
            constexpr const_key_iterator() noexcept : base_(nullptr) {}

            /// \brief Construct iterator wrapper from pointer x
            constexpr explicit const_key_iterator(base_iterator_type x) noexcept
                : base_(x) {}

            /// \brief Construct const iterator wrapper from another wrapper u,
            /// which might be another type
            template <class UP>
            constexpr const_key_iterator( // NOLINT(google-explicit-constructor):
                                          // expected for iterators
                const const_key_iterator<UP> &u,
                typename std::enable_if_t<
                    std::is_convertible_v<UP, base_iterator_type>>
                    * = 0) noexcept
                : base_(u.base()) {}

            public /* element access */:
            /// \brief Dereference iterator
            constexpr reference
            operator*() const noexcept {
                return *(operator->());
            }

            /// \brief Dereference iterator and get member
            constexpr pointer
            operator->() const noexcept {
                const auto base_pointer = std::addressof(*base_);
                auto mutable_base_pointer = const_cast<
                    std::add_pointer_t<std::remove_const_t<base_value_type>>>(
                    base_pointer);
                return reinterpret_cast<pointer>(mutable_base_pointer);
            }

            /// \brief Dereference iterator n positions ahead
            constexpr reference
            operator[](difference_type n) const noexcept {
                return reinterpret_cast<value_type>(base_[n]);
            }

            /// \brief Get base pointer
            constexpr base_iterator_type
            base() const noexcept {
                return base_;
            }

            public /* modifiers */:
            /// \brief Advance iterator
            constexpr const_key_iterator &
            operator++() noexcept {
                ++base_;
                return *this;
            }

            /// \brief Advance iterator (postfix)
            constexpr const_key_iterator
            operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
                const_key_iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            /// \brief Rewind iterator
            constexpr const_key_iterator &
            operator--() noexcept {
                --base_;
                return *this;
            }

            /// \brief Rewind iterator (postfix)
            constexpr const_key_iterator
            operator--(int) noexcept { // NOLINT(cert-dcl21-cpp)
                const_key_iterator tmp(*this);
                --(*this);
                return tmp;
            }

            /// \brief Return copy of iterator advanced by n positions
            constexpr const_key_iterator
            operator+(difference_type n) const noexcept {
                const_key_iterator w(*this);
                w += n;
                return w;
            }

            /// \brief Advance iterator by n positions
            constexpr const_key_iterator &
            operator+=(difference_type n) noexcept {
                base_ += n;
                return *this;
            }

            /// \brief Return copy of iterator n positions behind
            constexpr const_key_iterator
            operator-(difference_type n) const noexcept {
                return *this + (-n);
            }

            /// \brief Rewind iterator by n positions
            constexpr const_key_iterator &
            operator-=(difference_type n) noexcept {
                *this += -n;
                return *this;
            }

            public /* relational operators */:
            /// Make any other iterator wrapper a friend
            template <class UP>
            friend class const_key_iterator;

            public /* friends */:
            /// \brief Get distance between iterators
            template <class Iter1, class Iter2>
            constexpr friend auto
            operator-(
                const const_key_iterator<Iter1> &x,
                const const_key_iterator<Iter2> &y) noexcept
                -> decltype(x.base() - y.base());

            /// \brief Sum iterators
            template <class Iter1>
            constexpr friend const_key_iterator<Iter1> operator+(
                typename const_key_iterator<Iter1>::difference_type,
                const_key_iterator<Iter1>) noexcept;

        private:
            /// Base pointer
            base_iterator_type base_;
        };

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator==(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return x.base() == y.base();
        }

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator!=(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return !(x == y);
        }

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator<(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return x.base() < y.base();
        }

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator>(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return y < x;
        }

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator>=(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return !(x < y);
        }

        template <class Iter1, class Iter2>
        inline constexpr bool
        operator<=(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept {
            return !(y < x);
        }

        template <class Iter>
        inline constexpr bool
        operator==(
            const const_key_iterator<Iter> &x,
            const const_key_iterator<Iter> &y) noexcept {
            return x.base() == y.base();
        }

        template <class Iter>
        inline constexpr bool
        operator!=(
            const const_key_iterator<Iter> &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(x == y);
        }

        template <class Iter>
        inline constexpr bool
        operator>(
            const const_key_iterator<Iter> &x,
            const const_key_iterator<Iter> &y) noexcept {
            return y < x;
        }

        template <class Iter>
        inline constexpr bool
        operator>=(
            const const_key_iterator<Iter> &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(x < y);
        }

        template <class Iter>
        inline constexpr bool
        operator<=(
            const const_key_iterator<Iter> &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(y < x);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator==(
            const const_key_iterator<Iter> &x,
            const Pointer &y) noexcept {
            return x.base() == y;
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator!=(
            const const_key_iterator<Iter> &x,
            const Pointer &y) noexcept {
            return !(x == y);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator>(const const_key_iterator<Iter> &x, const Pointer &y) noexcept {
            return y < x;
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator>=(
            const const_key_iterator<Iter> &x,
            const Pointer &y) noexcept {
            return !(x < y);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator<=(
            const const_key_iterator<Iter> &x,
            const Pointer &y) noexcept {
            return !(y < x);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator==(
            const Pointer &x,
            const const_key_iterator<Iter> &y) noexcept {
            return x.base() == y.base();
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator!=(
            const Pointer &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(x == y);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator>(const Pointer &x, const const_key_iterator<Iter> &y) noexcept {
            return y < x;
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator>=(
            const Pointer &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(x < y);
        }

        template <class Iter, class Pointer>
        inline constexpr bool
        operator<=(
            const Pointer &x,
            const const_key_iterator<Iter> &y) noexcept {
            return !(y < x);
        }

        template <class Iter1, class Iter2>
        inline constexpr auto
        operator-(
            const const_key_iterator<Iter1> &x,
            const const_key_iterator<Iter2> &y) noexcept
            -> decltype(x.base() - y.base()) {
            return x.base() - y.base();
        }

        template <class Iter>
        inline constexpr const_key_iterator<Iter>
        operator+(
            typename const_key_iterator<Iter>::difference_type n,
            const_key_iterator<Iter> x) noexcept {
            x += n;
            return x;
        }
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ITERATOR_CONST_KEY_ITERATOR_HPP
