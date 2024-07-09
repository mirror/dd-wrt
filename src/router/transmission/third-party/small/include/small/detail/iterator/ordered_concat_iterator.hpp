//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ITERATOR_ORDERED_CONCAT_ITERATOR_HPP
#define SMALL_DETAIL_ITERATOR_ORDERED_CONCAT_ITERATOR_HPP

#include <algorithm>
#include <iterator>
#include <cpp-manifest/common/type_traits.h>
#include <type_traits>

namespace small {
    namespace detail {
        /// \brief Iterator that iterates two ordered containers as one
        template <class ITERATOR1, class ITERATOR2>
        class ordered_concat_iterator
        {
        public:
            // Base iterator
            typedef ITERATOR1 base_iterator_type1;
            typedef
                typename std::iterator_traits<base_iterator_type1>::value_type
                    base_value_type1;
            typedef ITERATOR2 base_iterator_type2;
            typedef
                typename std::iterator_traits<base_iterator_type2>::value_type
                    base_value_type2;

            // Wrapped types
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef
                typename std::iterator_traits<base_iterator_type1>::value_type
                    value_type;
            typedef typename std::iterator_traits<
                base_iterator_type1>::difference_type difference_type;
            typedef typename std::add_pointer_t<value_type> pointer;
            typedef typename std::add_lvalue_reference_t<value_type> reference;

            static_assert(
                (std::is_same_v<
                    typename std::iterator_traits<
                        base_iterator_type1>::value_type,
                    typename std::iterator_traits<
                        base_iterator_type2>::value_type>),
                "ordered_concat_iterator: iterators should have the same value "
                "type");

            public /* constructors */:
            /// \brief Construct empty iterator wrapper
            constexpr ordered_concat_iterator() noexcept
                : base1_(), base2_(), begin1_(), begin2_(), end1_(), end2_() {}

            /// \brief Construct iterator wrapper from pointer x
            constexpr explicit ordered_concat_iterator(
                base_iterator_type1 x1,
                base_iterator_type2 x2,
                base_iterator_type1 begin1,
                base_iterator_type2 begin2,
                base_iterator_type1 end1,
                base_iterator_type2 end2) noexcept
                : base1_(x1), base2_(x2), begin1_(begin1), begin2_(begin2),
                  end1_(end1), end2_(end2) {}

            /// \brief Construct const iterator wrapper from another wrapper u,
            /// which might be another type
            template <class UP1, class UP2>
            constexpr ordered_concat_iterator( // NOLINT(google-explicit-constructor):
                                               // expected for iterators
                const ordered_concat_iterator<UP1, UP2> &u,
                typename std::enable_if_t<
                    std::is_convertible_v<
                        UP1,
                        base_iterator_type1> && std::is_convertible_v<UP2, base_iterator_type2>>
                    * = 0) noexcept
                : base1_(u.base1()), base2_(u.base2()), begin1_(u.begin1_),
                  begin2_(u.begin2_), end1_(u.end1_), end2_(u.end2_) {}

            public /* element access */:
            /// \brief Dereference iterator
            constexpr reference
            operator*() const noexcept {
                return *(operator->());
            }

            /// \brief Dereference iterator and get member
            constexpr pointer
            operator->() const noexcept {
                const bool can_dereference_first = base1_ != end1_;
                const bool dereference_first = can_dereference_first
                                               && (base2_ == end2_
                                                   || *base1_ < *base2_);
                if (dereference_first) {
                    const auto base_pointer = std::addressof(*base1_);
                    auto mutable_base_pointer = const_cast<std::add_pointer_t<
                        std::remove_const_t<base_value_type1>>>(base_pointer);
                    return reinterpret_cast<pointer>(mutable_base_pointer);
                } else {
                    const auto base_pointer = std::addressof(*base2_);
                    auto mutable_base_pointer = const_cast<std::add_pointer_t<
                        std::remove_const_t<base_value_type2>>>(base_pointer);
                    return reinterpret_cast<pointer>(mutable_base_pointer);
                }
            }

            /// \brief Dereference iterator n positions ahead
            constexpr reference
            operator[](difference_type n) const noexcept {
                auto it = *this;
                std::advance(it, n);
                return it;
            }

            /// \brief Get base pointer
            constexpr base_iterator_type1
            base1() const noexcept {
                return base1_;
            }
            constexpr base_iterator_type2
            base2() const noexcept {
                return base2_;
            }

            public /* modifiers */:
            /// \brief Advance iterator
            constexpr ordered_concat_iterator &
            operator++() noexcept {
                const bool can_advance_first = base1_ != end1_;
                const bool can_advance_second = base2_ != end2_;
                const bool should_advance_first
                    = can_advance_first
                      && (!can_advance_second || *base1_ < *base2_);
                if (should_advance_first) {
                    base1_.operator++();
                } else {
                    base2_.operator++();
                }
                return *this;
            }

            /// \brief Advance iterator (postfix)
            constexpr ordered_concat_iterator
            operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
                ordered_concat_iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            /// \brief Decrement iterator
            /// This will undo the post-condition of operator++
            constexpr ordered_concat_iterator &
            operator--() noexcept {
                const bool can_decrement_first = base1_ != begin1_;
                const bool can_decrement_second = base2_ != begin2_;
                const bool can_dereference_first = base1_ != end1_;
                const bool can_dereference_second = base2_ != end2_;
                const auto prev_base1 = std::prev(base1_);
                const auto prev_base2 = std::prev(base2_);
                const bool should_decrement_first
                    = can_decrement_first
                      && (!can_decrement_second
                          || (can_dereference_second && *prev_base1 < *base2_)
                          || (can_dereference_first && !(*base1_ < *prev_base2))
                          || !(*prev_base1 < *prev_base2));
                // i.e.: if std::prev(base1_) is the iterator to which we
                // applied operator++ last time
                if (should_decrement_first) {
                    // Then we should undo that
                    base1_.operator--();
                } else {
                    base2_.operator--();
                }
                return *this;
            }

            /// \brief Rewind iterator (postfix)
            constexpr ordered_concat_iterator
            operator--(int) noexcept { // NOLINT(cert-dcl21-cpp)
                ordered_concat_iterator tmp(*this);
                --(*this);
                return tmp;
            }

            /// \brief Return copy of iterator advanced by n positions
            constexpr ordered_concat_iterator
            operator+(difference_type n) const noexcept {
                ordered_concat_iterator w(*this);
                w += n;
                return w;
            }

            /// \brief Advance iterator by n positions
            constexpr ordered_concat_iterator &
            operator+=(difference_type n) noexcept {
                auto it = *this;
                std::advance(it, n);
                return *this;
            }

            /// \brief Return copy of iterator n positions behind
            constexpr ordered_concat_iterator
            operator-(difference_type n) const noexcept {
                return *this + (-n);
            }

            /// \brief Rewind iterator by n positions
            constexpr ordered_concat_iterator &
            operator-=(difference_type n) noexcept {
                *this += -n;
                return *this;
            }

            public /* relational operators */:
            /// Make any other iterator wrapper a friend
            template <class UP1, class UP2>
            friend class ordered_concat_iterator;

            public /* friends */:
            /// \brief Get distance between iterators
            template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
            constexpr friend auto
            operator-(
                const ordered_concat_iterator<Iter1a, Iter1b> &x,
                const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept
                -> decltype(x.base() - y.base());

            /// \brief Sum iterators
            template <class Iter1a, class Iter1b>
            constexpr friend ordered_concat_iterator<Iter1a, Iter1b> operator+(
                typename ordered_concat_iterator<Iter1a, Iter1b>::
                    difference_type,
                ordered_concat_iterator<Iter1a, Iter1b>) noexcept;

        private:
            /// Base pointer
            base_iterator_type1 base1_;
            base_iterator_type2 base2_;

            /// We need to know where std::begin and std::end are so that we
            /// know which iterators we can/should advance
            base_iterator_type1 begin1_;
            base_iterator_type2 begin2_;
            base_iterator_type1 end1_;
            base_iterator_type2 end2_;
        };

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator==(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return x.base1() == y.base1() && x.base2() == y.base2();
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator!=(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return !(x == y);
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator<(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return x.base() < y.base();
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator>(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return y < x;
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator>=(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return !(x < y);
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr bool
        operator<=(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept {
            return !(y < x);
        }

        template <class Itera, class Iterb>
        inline constexpr bool
        operator==(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return x.base1() == y.base1() && x.base2() == y.base2();
        }

        template <class Itera, class Iterb>
        inline constexpr bool
        operator!=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(x == y);
        }

        template <class Itera, class Iterb>
        inline constexpr bool
        operator>(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return y < x;
        }

        template <class Itera, class Iterb>
        inline constexpr bool
        operator>=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(x < y);
        }

        template <class Itera, class Iterb>
        inline constexpr bool
        operator<=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(y < x);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator==(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const Pointer &y) noexcept {
            return x.base() == y;
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator!=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const Pointer &y) noexcept {
            return !(x == y);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator>(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const Pointer &y) noexcept {
            return y < x;
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator>=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const Pointer &y) noexcept {
            return !(x < y);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator<=(
            const ordered_concat_iterator<Itera, Iterb> &x,
            const Pointer &y) noexcept {
            return !(y < x);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator==(
            const Pointer &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return x.base() == y.base();
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator!=(
            const Pointer &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(x == y);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator>(
            const Pointer &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return y < x;
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator>=(
            const Pointer &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(x < y);
        }

        template <class Itera, class Iterb, class Pointer>
        inline constexpr bool
        operator<=(
            const Pointer &x,
            const ordered_concat_iterator<Itera, Iterb> &y) noexcept {
            return !(y < x);
        }

        template <class Iter1a, class Iter1b, class Iter2a, class Iter2b>
        inline constexpr auto
        operator-(
            const ordered_concat_iterator<Iter1a, Iter1b> &x,
            const ordered_concat_iterator<Iter2a, Iter2b> &y) noexcept
            -> decltype(x.base() - y.base()) {
            return x.base() - y.base();
        }

        template <class Itera, class Iterb>
        inline constexpr ordered_concat_iterator<Itera, Iterb>
        operator+(
            typename ordered_concat_iterator<Itera, Iterb>::difference_type n,
            ordered_concat_iterator<Itera, Iterb> x) noexcept {
            x += n;
            return x;
        }
    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ITERATOR_ORDERED_CONCAT_ITERATOR_HPP
