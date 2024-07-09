//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ALGORITHM_SHIFT_HPP
#define SMALL_DETAIL_ALGORITHM_SHIFT_HPP

/// \headerfile Adapted from https://github.com/danra/shift_proposal

#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>

namespace small {
    namespace detail {
        namespace shift {

            template <class I>
            using difference_type_t = typename std::iterator_traits<
                I>::difference_type;

            template <class I>
            using iterator_category_t = typename std::iterator_traits<
                I>::iterator_category;

            template <class I, class Tag, class = void>
            constexpr bool is_category = false;
            template <class I, class Tag>
            constexpr bool is_category<
                I,
                Tag,
                std::enable_if_t<
                    std::is_convertible_v<iterator_category_t<I>, Tag>>> = true;

            /// Increment (decrement for negative n) i |n| times or until i ==
            /// bound, whichever comes first. Returns n - the difference between
            /// i's final position and its initial position. (Note: "advance"
            /// has overloads with this behavior in the Ranges TS.)
            template <class I>
            constexpr difference_type_t<I>
            bounded_advance(I &i, difference_type_t<I> n, I const bound) {
                if constexpr (is_category<I, std::bidirectional_iterator_tag>) {
                    for (; n < 0 && i != bound; ++n, void(--i)) {
                        ;
                    }
                }

                for (; n > 0 && i != bound; --n, void(++i)) {
                    ;
                }

                return n;
            }

            template <class ForwardIt>
            ForwardIt
            shift_left(
                ForwardIt first,
                ForwardIt last,
                difference_type_t<ForwardIt> n) {
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

            template <class ForwardIt>
            ForwardIt
            shift_right(
                ForwardIt first,
                ForwardIt last,
                difference_type_t<ForwardIt> n) {
                if (n <= 0) {
                    return first;
                }

                if constexpr (is_category<
                                  ForwardIt,
                                  std::bidirectional_iterator_tag>) {
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
                                //   ... --|-- n elements --|
                                //   ^-first            mid-^         result-^
                                //   ^-trail     last-^
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
        } // namespace shift
    }     // namespace detail
} // namespace small

#endif // !defined(SMALL_DETAIL_ALGORITHM_SHIFT_HPP)