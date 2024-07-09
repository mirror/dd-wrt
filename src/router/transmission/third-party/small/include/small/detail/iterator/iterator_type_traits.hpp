//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_ITERATOR_ITERATOR_TYPE_TRAITS_HPP
#define SMALL_DETAIL_ITERATOR_ITERATOR_TYPE_TRAITS_HPP

#include <small/detail/traits/cpp_version.hpp>
#include <iterator>

namespace small {
    namespace detail {
        /// Difference type for a custom iterator
        /// \note Adapted from https://github.com/danra/shift_proposal/
        template <class I>
        using difference_type_type = typename std::iterator_traits<
            I>::difference_type;

        /// Category type for a custom iterator
        /// \note Adapted from https://github.com/danra/shift_proposal/
        template <class I>
        using iterator_category_type = typename std::iterator_traits<
            I>::iterator_category;

        /// Whether iterator is in a category Tag (base case = false)
        /// \note Adapted from https://github.com/danra/shift_proposal/
        template <class I, class Tag, class = void>
        static constexpr bool is_category_convertible = false;

        /// Whether iterator is in a category Tag (usual case)
        /// \note Adapted from https://github.com/danra/shift_proposal/
        template <class I, class Tag>
        static constexpr bool is_category_convertible<
            I,
            Tag,
            std::enable_if_t<
                std::is_convertible_v<iterator_category_type<I>, Tag>>> = true;

        /// Check if a type has an iterator category
        template <class I>
        struct has_iterator_category
        {
        private:
            /// An arbitrary type of size == 2
            struct two
            {
                char lx;
                char lxx;
            };

            /// Return type of size == 2 when UP doesn't has iterator category
            template <class UP>
            static two
            test(...) {
                return two{ char(), char() };
            }

            /// Return type of size == 1 when UP has iterator category
            template <class UP>
            static char
            test(typename UP::iterator_category * = 0) {
                return char();
            }

        public:
            /// Indicates if I has a category iterator (when return type of test
            /// has size == 1)
            static const bool value = sizeof(test<I>(0)) == 1;
        };

        /// Check if type has an iterator category convertible to UP
        template <
            class I,
            class UP,
            bool = has_iterator_category<std::iterator_traits<I>>::value>
        struct has_iterator_category_convertible_to
            : public std::integral_constant<
                  bool,
                  std::is_convertible<
                      typename std::iterator_traits<I>::iterator_category,
                      UP>::value>
        {};

        /// Check if type has an iterator category convertible to UP
        template <class I, class UP>
        struct has_iterator_category_convertible_to<I, UP, false>
            : public std::false_type
        {};

        /// Check if type is convertible to input_iterator
        template <class I>
        struct is_input_iterator
            : public has_iterator_category_convertible_to<
                  I,
                  std::input_iterator_tag>
        {};

        /// Type that is only valid if input type is convertible to an input
        /// iterator
        template <class I, class value_type>
        using enable_if_iterator_t = typename std::enable_if_t<
            is_input_iterator<I>::value
                && std::is_constructible<
                    value_type,
                    typename std::iterator_traits<I>::reference>::value,
            I>;

        template <class I>
        constexpr bool is_input_iterator_v = is_input_iterator<I>::value;

        /// Common iterator category primary template.
        /// A variant iterator will always have the minimal iterator category
        /// requirements for its final type. The implementation should always
        /// fall into one of the specializations though.
        template <typename... Types>
        struct common_iterator_tag;

        /// Base iterator: 1 Iterator type
        template <class Tag1>
        struct common_iterator_tag<Tag1>
        {
            using type = Tag1;
        };

        /// Base iterator: 2 Iterator types
        template <class Tag1, class Tag2>
        struct common_iterator_tag<Tag1, Tag2>
        {
            template <class B, class T1, class T2>
            static constexpr bool both_base_of_v = std::is_base_of_v<B, T1>
                &&std::is_base_of_v<B, T2>;

            static constexpr bool both_input
                = both_base_of_v<std::input_iterator_tag, Tag1, Tag2>;
            static constexpr bool both_forward
                = both_base_of_v<std::forward_iterator_tag, Tag1, Tag2>;
            static constexpr bool both_bidirectional
                = both_base_of_v<std::bidirectional_iterator_tag, Tag1, Tag2>;
            static constexpr bool both_random_access
                = both_base_of_v<std::random_access_iterator_tag, Tag1, Tag2>;
            static constexpr bool both_output
                = both_base_of_v<std::output_iterator_tag, Tag1, Tag2>;

            using type = std::conditional_t<
                std::is_same_v<Tag1, Tag2>,
                Tag1,
                std::conditional_t<
                    both_random_access,
                    std::random_access_iterator_tag,
                    std::conditional_t<
                        both_bidirectional,
                        std::bidirectional_iterator_tag,
                        std::conditional_t<
                            both_forward,
                            std::forward_iterator_tag,
                            std::conditional_t<
                                both_input,
                                std::input_iterator_tag,
                                std::conditional_t<
                                    both_output,
                                    std::output_iterator_tag,
                                    void>>>>>>;
        };

        /// Base iterator: > 2 Iterator types
        template <class Tag1, class Tag2, typename... Types>
        struct common_iterator_tag<Tag1, Tag2, Types...>
        {
            using type = typename common_iterator_tag<
                typename common_iterator_tag<Tag1, Tag2>::type,
                typename common_iterator_tag<Types...>::type>::type;
        };

        /// Base iterator type
        template <class... Types>
        using common_iterator_tag_t = typename common_iterator_tag<
            Types...>::type;

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_ITERATOR_ITERATOR_TYPE_TRAITS_HPP
