//
// Copyright (c) 2024 Yat Ho (lagoho7@gmail.com)
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_CONTAINER_ASSOCIATIVE_VECTOR_HPP
#define SMALL_DETAIL_CONTAINER_ASSOCIATIVE_VECTOR_HPP

#include <small/vector.hpp>
#include <small/detail/iterator/const_key_iterator.hpp>
#include <small/detail/iterator/iterator_type_traits.hpp>
#include <small/detail/traits/add_key_const.hpp>
#include <small/detail/traits/extract_value_type.hpp>
#include <small/detail/traits/has_allocator.hpp>
#include <map>
#include <variant>

namespace small::detail {
    /// \brief A set/map represented with any sorted vector type
    ///
    /// - More cache friendly node types
    /// - Small vectors for small maps
    /// - Optional random access with order statistic tree
    ///   - Each node stores its size so we can trivially find a position
    /// - Inline allocation for small number of elements
    /// - Custom expected size
    /// - Special treatment of relocatable types
    /// - Better default grow factors
    /// - Custom grow factors
    ///
    /// This is similar to boost::vector_map, with the main difference
    /// that it works as a container adapter for vector types rather
    /// than a map that uses a specific kind of vector internally.
    ///
    /// Unlike other std:: container adapters, it forces you to make what
    /// type of vector type to use explicit and then inherits its types
    /// from the vector type instead of requiring the same key/value type
    /// parameters twice.
    ///
    /// \tparam IsMap Whether this represents a map or a set
    /// \tparam IsMulti Whether repeated keys are allowed
    /// \tparam IsOrdered Whether the elements should be ordered
    /// \tparam Vector A vector-like container of pairs
    /// \tparam Compare Comparison function to sort elements
    template <
        bool IsMap,
        bool IsMulti,
        bool IsOrdered,
        class Vector,
        class Compare = std::less<>>
    class associative_vector
    {
    public /* types */:
        typedef Vector vector_type;

        template <class, class = void>
        struct first_type_is_const : std::false_type
        {};
        template <class T>
        struct first_type_is_const<T, std::void_t<typename T::first_type>>
            : std::is_const<typename T::first_type>
        {};
        template <class, class = void>
        struct second_type_is_const : std::false_type
        {};
        template <class T>
        struct second_type_is_const<T, std::void_t<typename T::second_type>>
            : std::is_const<typename T::second_type>
        {};
        template <class, class = void>
        struct first_type_or_void
        {
            using type = void;
        };
        template <class T>
        struct first_type_or_void<T, std::void_t<typename T::first_type>>
        {
            using type = typename T::first_type;
        };
        template <class, class = void>
        struct second_type_or_void
        {
            using type = void;
        };
        template <class T>
        struct second_type_or_void<T, std::void_t<typename T::second_type>>
        {
            using type = typename T::second_type;
        };
        template <class T>
        using first_type_or_void_t = typename first_type_or_void<T>::type;
        template <class T>
        using second_type_or_void_t = typename second_type_or_void<T>::type;
        template <class, class = void>
        struct is_transparent : std::false_type
        {};
        template <class Func>
        struct is_transparent<Func, std::void_t<typename Func::is_transparent>>
            : std::true_type
        {};
        template <class Func>
        static auto constexpr is_transparent_v = is_transparent<Func>::value;
        static auto constexpr is_comp_tr = is_transparent_v<Compare>;

        /* static asserts */
        static_assert(
            !IsMap || (is_pair<typename vector_type::value_type>::value),
            "vector_map: vector value_type must be std::pair");

        static_assert(
            !IsMap
                || (!first_type_is_const<
                    typename vector_type::value_type>::value),
            "vector_map: vector key_type should be mutable / cannot be "
            "constant");

        static_assert(
            !IsMap
                || (!second_type_is_const<
                    typename vector_type::value_type>::value),
            "vector_map: vector mapped_type should be mutable / cannot be "
            "constant");

        typedef typename vector_type::value_type mutable_value_type;

        using key_type = std::conditional_t<
            IsMap,
            first_type_or_void_t<typename vector_type::value_type>,
            typename vector_type::value_type>;
        using mapped_type = std::conditional_t<
            IsMap,
            second_type_or_void_t<typename vector_type::value_type>,
            typename vector_type::value_type>;

        using value_type = std::conditional_t<
            IsMap,
            typename add_key_const<mutable_value_type>::type,
            key_type>;

        static_assert(
            !IsMap
                || (std::is_const_v<first_type_or_void_t<
                        typename associative_vector::value_type>>),
            "vector_map: vector_map::value_type::first_type must be "
            "constant");

        /// Default allocator type for a vector type with no allocator
        template <class T, typename = void>
        struct default_allocator_type
        {
            typedef std::allocator<mutable_value_type> type;
        };

        /// Default allocator type for a vector type with an allocator type
        template <class T>
        struct default_allocator_type<
            T,
            std::void_t<
                decltype(std::declval<T>().get_allocator()),
                typename T::allocator_type>>
        {
            typedef typename T::allocator_type type;
        };

        /* default container types */
        typedef Compare key_compare;
        typedef
            typename default_allocator_type<vector_type>::type allocator_type;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;

        typedef std::conditional_t<
            IsMap,
            const_key_iterator<typename vector_type::iterator>,
            typename vector_type::iterator>
            iterator;
        typedef std::conditional_t<
            IsMap,
            const_key_iterator<typename vector_type::const_iterator>,
            typename vector_type::const_iterator>
            const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        static_assert(
            (std::is_same<
                typename allocator_type::value_type,
                mutable_value_type>::value),
            "Allocator::value_type must be same type as value_type");

        /// \brief Construct vector depending the existence of an allocator
        /// type
        static constexpr vector_type
        construct_vector(const allocator_type &alloc) {
            if constexpr (has_allocator_v<vector_type>) {
                return vector_type(alloc);
            } else {
                return vector_type();
            }
        }

        /// \brief Construct vector depending the existence of an allocator
        /// type
        template <class InputIterator>
        static constexpr vector_type
        construct_vector(
            InputIterator first,
            enable_if_iterator_t<InputIterator, value_type> last,
            const allocator_type &alloc) {
            if constexpr (has_allocator_v<vector_type>) {
                return vector_type(first, last, alloc);
            } else {
                return vector_type(first, last);
            }
        }

    public /* Comparison type */:
        /// Object encapsulating the comparison function for values from
        /// the comparison function for keys
        class value_compare
        {
            friend class associative_vector;

        public:
            typedef value_type first_argument_type;
            typedef value_type second_argument_type;
            typedef bool result_type;

        protected:
            /// Function to compare keys
            key_compare comp;

            /// Construct value compare object
            constexpr explicit value_compare(key_compare c) : comp(c) {}

        public:
            /// Evaluate the comparison function to sort elements
            constexpr result_type
            operator()(
                const first_argument_type &x,
                const second_argument_type &y) const {
                return comp(maybe_first(x), maybe_first(y));
            }
        };

    public /* constructors */:
        /// \brief Construct empty small map with default allocator
        constexpr associative_vector() noexcept(
            std::is_nothrow_default_constructible<allocator_type>::value
            && std::is_nothrow_default_constructible<key_compare>::value
            && std::is_nothrow_copy_constructible<key_compare>::value)
            : associative_vector(key_compare(), allocator_type()) {}

        /// \brief Construct empty small map with comparison function
        explicit constexpr associative_vector(const key_compare &comp)
            : associative_vector(comp, allocator_type()) {}

        /// \brief Construct small vector with a given allocator
        constexpr explicit associative_vector(const allocator_type &alloc)
            : associative_vector(key_compare(), alloc) {}

        /// \brief Construct empty small vector with a comparison function
        /// and given allocator
        constexpr explicit associative_vector(
            const key_compare &comp,
            const allocator_type &alloc)
            : data_(construct_vector(alloc)), comp_(comp) {}

        /// \brief Construct small map from a pair of iterators
        template <class InputIterator>
        constexpr associative_vector(
            InputIterator first,
            enable_if_iterator_t<InputIterator, value_type> last,
            const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
            : data_(construct_vector(first, last, alloc)), comp_(comp) {
            if constexpr (IsOrdered) {
                std::sort(data_.begin(), data_.end(), value_comp());
            }
            assert(invariants());
        }

        /// \brief Construct from another map but change the allocator
        associative_vector(const associative_vector &m, const allocator_type &a)
            : associative_vector(m.begin(), m.end(), m.comp_, a) {}

        /// \brief Construct small map from initializer list
        constexpr associative_vector(
            std::initializer_list<mutable_value_type> il,
            const key_compare &comp = key_compare(),
            const allocator_type &alloc = allocator_type())
            : associative_vector(il.begin(), il.end(), comp, alloc) {}

        /// \brief Construct from iterators and allocator
        template <class InputIterator>
        associative_vector(
            InputIterator first,
            InputIterator last,
            const allocator_type &a)
            : associative_vector(first, last, Compare(), a) {}

        /// \brief Construct from initializer list and allocator
        associative_vector(
            std::initializer_list<value_type> il,
            const allocator_type &a)
            : associative_vector(il, Compare(), a) {}

        /// \brief Assign small map from initializer list
        constexpr associative_vector &
        operator=(std::initializer_list<value_type> il) {
            assign(il);
            return *this;
        }

        /// \brief Assign small map from iterators
        template <class InputIterator>
        constexpr void
        assign(
            InputIterator first,
            enable_if_iterator_t<InputIterator, value_type> last) {
            clear();
            insert(first, last);
            assert(invariants());
        }

        /// \brief Assign small map from initializer list
        constexpr void
        assign(std::initializer_list<value_type> il) {
            assign(il.begin(), il.end());
        }

        /// \brief Swap the contents of two flat maps
        constexpr void
        swap(associative_vector &a) noexcept(
            std::allocator_traits<allocator_type>::is_always_equal::value
            && std::is_nothrow_swappable<key_compare>::value) {
            std::swap(comp_, a.comp_);
            data_.swap(a.data_);
            assert(invariants());
        }

    public /* observers */:
        /// \brief Get copy of allocator for dynamic vector
        allocator_type
        get_allocator() const noexcept {
            if constexpr (has_allocator_v<vector_type>) {
                return data_.get_allocator();
            } else {
                return allocator_type();
            }
        }

        /// \brief Get copy of key comparison function
        key_compare
        key_comp() const {
            return comp_;
        }

        /// \brief Get copy of value comparison function
        value_compare
        value_comp() const {
            return value_compare(comp_);
        }

    public /* iterators */:
        /// \brief Get iterator to first element
        constexpr iterator
        begin() noexcept {
            return iterator(data_.begin());
        }

        /// \brief Get constant iterator to first element[[nodiscard]]
        constexpr const_iterator
        begin() const noexcept {
            return const_iterator(data_.begin());
        }

        /// \brief Get iterator to last element
        constexpr iterator
        end() noexcept {
            return iterator(data_.end());
        }

        /// \brief Get constant iterator to last element
        constexpr const_iterator
        end() const noexcept {
            return const_iterator(data_.end());
        }

        /// \brief Get iterator to first element in reverse order
        constexpr reverse_iterator
        rbegin() noexcept {
            return reverse_iterator(end());
        }

        /// \brief Get constant iterator to first element in reverse order
        constexpr const_reverse_iterator
        rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /// \brief Get iterator to last element in reverse order
        constexpr reverse_iterator
        rend() noexcept {
            return reverse_iterator(begin());
        }

        /// \brief Get constant iterator to last element in reverse order
        constexpr const_reverse_iterator
        rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /// \brief Get constant iterator to first element
        constexpr const_iterator
        cbegin() const noexcept {
            return const_iterator(data_.cbegin());
        }

        /// \brief Get constant iterator to last element
        constexpr const_iterator
        cend() const noexcept {
            return const_iterator(data_.cend());
        }

        /// \brief Get constant iterator to first element in reverse order
        constexpr const_reverse_iterator
        crbegin() const noexcept {
            return const_reverse_iterator(cend());
        }

        /// \brief Get constant iterator to last element in reverse order
        constexpr const_reverse_iterator
        crend() const noexcept {
            return const_reverse_iterator(cbegin());
        }

    public /* capacity */:
        /// \brief Check if small map is empty
        [[nodiscard]] constexpr bool
        empty() const noexcept {
            return data_.empty();
        }

        /// \brief Get small map size
        [[nodiscard]] constexpr size_type
        size() const noexcept {
            return data_.size();
        }

        /// \brief Get small map max size
        [[nodiscard]] constexpr size_type
        max_size() const noexcept {
            return data_.max_size();
        }

        /// \brief Get small map current capacity (might be smaller for
        /// buffer)
        [[nodiscard]] constexpr size_type
        capacity() const noexcept {
            return data_.capacity();
        }

        /// \brief Reserve space for n elements
        /// This might reserve space in the buffer or switch to a map_type
        /// already We concentrate the logic to switch the variant types in
        /// these functions
        void
        reserve(size_type n) {
            if constexpr (has_allocator_v<vector_type>) {
                data_.reserve(n);
            }
        }

        /// \brief Shrink internal array to fit only the current elements
        /// We concentrate the logic to switch the variant types in these
        /// functions
        void
        shrink_to_fit() noexcept {
            if constexpr (has_allocator_v<vector_type>) {
                data_.shrink_to_fit();
            }
        }

    public /* element access */:
        /// \brief Get reference to element in buffered map
        constexpr mapped_type &
        operator[](const key_type &k) {
            return operator[]<decltype(k)>(k);
        }

        /// \brief Get reference to element in buffered map
        constexpr mapped_type &
        operator[](key_type &&k) {
            return operator[]<decltype(k)>(std::move(k));
        }

        /// \brief Get reference to element in buffered map
        template <class K>
        constexpr std::enable_if_t<
            is_comp_tr || std::is_same_v<K, key_type>,
            mapped_type &>
        operator[](K &&k) {
            return element_access_implementation<true>(std::forward<K>(k));
        }

        /// \brief Check bound and get reference to element in buffered map
        constexpr mapped_type &
        at(const key_type &k) {
            return at<decltype(k)>(k);
        }

        /// \brief Check bound and get reference to element in buffered map
        template <class K>
        constexpr std::enable_if_t<
            is_comp_tr || std::is_same_v<K, key_type>,
            mapped_type &>
        at(const K &k) {
            return element_access_implementation<false>(k);
        }

        /// \brief Check bound and get reference to element in buffered map
        constexpr const mapped_type &
        at(const key_type &k) const {
            return at<decltype(k)>(k);
        }

        /// \brief Check bound and get reference to element in buffered map
        template <class K>
        constexpr std::enable_if_t<
            is_comp_tr || std::is_same_v<K, key_type>,
            const mapped_type &>
        at(const K &k) const {
            return element_access_implementation(k);
        }

        /// \brief Get reference to first element in small array
        constexpr reference
        front() {
            return reinterpret_cast<reference>(data_.front());
        }

        /// \brief Get constant reference to first element in small array
        constexpr const_reference
        front() const {
            return reinterpret_cast<const_reference>(data_.front());
        }

        /// \brief Get reference to last element in small array
        constexpr reference
        back() {
            return reinterpret_cast<reference>(data_.back());
        }

        /// \brief Get constant reference to last element in small array
        constexpr const_reference
        back() const {
            return reinterpret_cast<const_reference>(data_.back());
        }

        /// \brief Get reference to internal pointer to small array data
        constexpr pointer
        data() noexcept {
            return reinterpret_cast<pointer>(data_.data());
        }

        /// \brief Get constant reference to internal pointer to small array
        /// data
        constexpr const_pointer
        data() const noexcept {
            return reinterpret_cast<const_pointer>(data_.data());
        }

    public /* modifiers */:
        /// \brief Emplace element at hint hint of the small map
        /// \param hint Position before element will be constructed
        template <class... Args>
        iterator
        emplace_hint(const_iterator hint, Args &&...args) {
            return emplace_hint_impl(std::move(hint), std::forward<Args>(args)...)
                .first;
        }

        /// \brief Emplace element to small map
        template <class... Args>
        constexpr std::pair<iterator, bool>
        emplace(Args &&...args) {
            return emplace_hint_impl(end(), std::forward<Args>(args)...);
        }

        /// \brief Insert value type - copy
        std::pair<iterator, bool>
        insert(const value_type &v) {
            return insert<decltype(v)>(v);
        }

        /// \brief Insert value type - move
        std::pair<iterator, bool>
        insert(value_type &&v) {
            return insert<decltype(v)>(std::move(v));
        }

        /// \brief Insert value - construct
        template <class P>
        std::enable_if_t<
            std::is_constructible_v<value_type, P &&>,
            std::pair<iterator, bool>>
        insert(P &&p) {
            return emplace(std::forward<P>(p));
        }

        /// \brief Insert in position - copy
        iterator
        insert(const_iterator position, const value_type &v) {
            return insert<decltype(v)>(std::move(position), v);
        }

        /// \brief Insert in position - move
        iterator
        insert(const_iterator position, value_type &&v) {
            return insert<decltype(v)>(std::move(position), std::move(v));
        }

        /// \brief Insert value with hint
        template <class P>
        std::enable_if_t<std::is_constructible_v<value_type, P &&>, iterator>
        insert(const_iterator position, P &&p) {
            return emplace_hint(std::move(position), std::forward<P>(p));
        }

        /// \brief Insert all values from iterators
        template <class InputIterator>
        void
        insert(
            InputIterator first,
            enable_if_iterator_t<InputIterator, value_type> last) {
            while (first != last) {
                insert(*first++);
            }
        }

        /// \brief Insert all values from initializer list
        void
        insert(std::initializer_list<value_type> il) {
            insert(il.begin(), il.end());
        }

        /// \brief Emplace if key doesn't exist yet
        template <class... Args>
        std::pair<iterator, bool>
        try_emplace(const key_type &k, Args &&...args) {
            return try_emplace<decltype(k), Args...>(
                k,
                std::forward<Args>(args)...);
        }

        /// \brief Emplace if key doesn't exist yet
        template <class... Args>
        std::pair<iterator, bool>
        try_emplace(key_type &&k, Args &&...args) {
            return try_emplace<decltype(k), Args...>(
                std::move(k),
                std::forward<Args>(args)...);
        }

        /// \brief Emplace if key doesn't exist yet
        template <class K, class... Args>
        std::enable_if_t<
            !std::is_convertible_v<K &&, iterator>
                && !std::is_convertible_v<K &&, const_iterator>,
            std::pair<iterator, bool>>
        try_emplace(K &&k, Args &&...args) {
            return emplace_hint_impl_key(
                end(),
                k,
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<K>(k)),
                std::forward_as_tuple(std::forward<Args>(args)...));
        }

        /// \brief Emplace if key doesn't exist yet
        template <class... Args>
        iterator
        try_emplace(const_iterator hint, const key_type &k, Args &&...args) {
            return try_emplace<
                decltype(k),
                Args...>(std::move(hint), k, std::forward<Args>(args)...);
        }

        /// \brief Emplace if key doesn't exist yet
        template <class... Args>
        iterator
        try_emplace(const_iterator hint, key_type &&k, Args &&...args) {
            return try_emplace<decltype(k), Args...>(
                std::move(hint),
                std::move(k),
                std::forward<Args>(args)...);
        }

        /// \brief Emplace if key doesn't exist yet
        template <class K, class... Args>
        iterator
        try_emplace(const_iterator hint, K &&k, Args &&...args) {
            return emplace_hint_impl_key(
                       std::move(hint),
                       k,
                       std::piecewise_construct,
                       std::forward_as_tuple(std::forward<K>(k)),
                       std::forward_as_tuple(std::forward<Args>(args)...))
                .first;
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class M>
        std::pair<iterator, bool>
        insert_or_assign(const key_type &k, M &&obj) {
            return insert_or_assign<decltype(k), M>(k, std::forward<M>(obj));
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class M>
        std::pair<iterator, bool>
        insert_or_assign(key_type &&k, M &&obj) {
            return insert_or_assign<decltype(k), M>(
                std::move(k),
                std::forward<M>(obj));
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class K, class M>
        std::pair<iterator, bool>
        insert_or_assign(K &&k, M &&obj) {
            auto res = try_emplace(std::forward<K>(k), std::forward<M>(obj));
            if (!res.second) {
                res.first->second = std::forward<M>(obj);
            }
            return res;
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class M>
        iterator
        insert_or_assign(const_iterator hint, const key_type &k, M &&obj) {
            return insert_or_assign<
                decltype(k),
                M>(std::move(hint), k, std::forward<M>(obj));
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class M>
        iterator
        insert_or_assign(const_iterator hint, key_type &&k, M &&obj) {
            return insert_or_assign<
                decltype(k),
                M>(std::move(hint), std::move(k), std::forward<M>(obj));
        }

        /// \brief Insert if key doesn't exist, assign if key already exists
        template <class K, class M>
        iterator
        insert_or_assign(const_iterator hint, K &&k, M &&obj) {
            auto [it, ok] = emplace_hint_impl_key(
                std::move(hint),
                k,
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<K>(k)),
                std::forward_as_tuple(std::forward<M>(obj)));
            if (!ok) {
                it->second = std::forward<M>(obj);
            }
            return it;
        }

        /// \brief Erase element at a position in small map
        constexpr iterator
        erase(const_iterator position) {
            return iterator(data_.erase(maybe_base(position)));
        }

        /// \brief Erase element at a position in small map
        size_type
        erase(const key_type &k) {
            return erase<decltype(k)>(k);
        }

        /// \brief Erase element at a position in small map
        template <typename K>
        std::enable_if_t<
            !std::is_convertible_v<K &&, iterator>
                && !std::is_convertible_v<K &&, const_iterator>
                && (is_comp_tr || std::is_same_v<K, key_type>),
            size_type>
        erase(K &&x) {
            if constexpr (IsMulti && !IsOrdered) {
                return erase_if([this, &x](const value_type &v) {
                    (void)this; // clang warns -Wunused-lambda-capture otherwise
                    return keys_equivalent(maybe_first(v), x);
                });
            }

            auto const first = find(x);
            auto const last = std::
                find_if(first, end(), [this, &x](auto const &v) {
                (void)this; // clang warns -Wunused-lambda-capture otherwise
                return !keys_equivalent(maybe_first(v), x);
            });
            auto const ret = std::distance(first, last);
            erase(first, last);
            return ret;
        }

        /// \brief Erase range of elements in the small map
        constexpr iterator
        erase(const_iterator first, const_iterator last) {
            return iterator(data_.erase(maybe_base(first), maybe_base(last)));
        }

        /// \brief Erase elements that match the predicate
        template <typename Pred>
        size_type
        erase_if(Pred p) {
            auto const old_size = size();
            data_.erase(
                std::remove_if(data_.begin(), data_.end(), p),
                data_.end());
            return old_size - size();
        }

        /// \brief Clear elements in the small map
        constexpr void
        clear() noexcept {
            data_.clear();
        }

        /// \brief Merge two flat maps
        template <bool b1, bool b2, bool b3, class V2, class C2>
        void
        merge(associative_vector<b1, b2, b3, V2, C2> &source) {
            data_.insert(data_.end(), source.begin(), source.end());
            if constexpr (IsOrdered) {
                std::sort(data_.begin(), data_.end(), value_comp());
            }
        }

        /// \brief Merge two flat maps
        template <bool b1, bool b2, bool b3, class V2, class C2>
        void
        merge(associative_vector<b1, b2, b3, V2, C2> &&source) {
            data_.insert(
                std::move_iterator(source.begin()),
                std::move_iterator(source.end()));
            source.clear();
            if constexpr (IsOrdered) {
                std::sort(data_.begin(), data_.end(), value_comp());
            }
        }

    public /* map operations */:
        /// \brief Find element in the small map
        iterator
        find(const key_type &k) {
            return find<decltype(k)>(k);
        }

        /// \brief Find element in the small map
        const_iterator
        find(const key_type &k) const {
            return (const_cast<associative_vector *>(this))->find(k);
        }

        /// \brief Find element in the small map
        template <typename K>
        std::enable_if_t<is_comp_tr || std::is_same_v<K, key_type>, iterator>
        find(const K &x) {
            if constexpr (!IsOrdered) {
                // blame MSVC warning C4127 for this travesty of code >:(
                for (auto it = begin(), last = end(); it != last; ++it) {
                    if (keys_equivalent(maybe_first(*it), x)) {
                        return it;
                    }
                }
            } else if (size() < 100) {
                for (auto it = begin(), last = end(); it != last; ++it) {
                    if (keys_equivalent(maybe_first(*it), x)) {
                        return it;
                    }
                }
            } else if (auto it = lower_bound(x);
                       it != end() && keys_equivalent(maybe_first(*it), x))
            {
                return it;
            }
            return end();
        }

        /// \brief Find element in the small map
        template <typename K>
        const_iterator
        find(const K &x) const {
            return (const_cast<associative_vector *>(this))->find(x);
        }

        /// \brief Count elements with a given key
        template <typename K>
        std::enable_if_t<is_comp_tr || std::is_same_v<K, key_type>, size_type>
        count(const K &x) const {
            if constexpr (!IsMulti) {
                return contains(x) ? 1 : 0;
            } else if constexpr (IsOrdered) {
                auto [first, last] = equal_range(x);
                return std::distance(first, last);
            } else {
                return std::count_if(begin(), end(), [this, &x](auto const &v) {
                    (void) this; // clang warns -Wunused-lambda-capture otherwise
                    return keys_equivalent(maybe_first(v), x);
                });
            }
        }

        /// \brief Count elements with a given key
        size_type
        count(const key_type &k) const {
            return count<decltype(k)>(k);
        }

        /// \brief Check if elements with a given key exists
        template <typename K>
        std::enable_if_t<is_comp_tr || std::is_same_v<K, key_type>, bool>
        contains(const K &x) const {
            return find(x) != end();
        }

        /// \brief Check if elements with a given key exists
        bool
        contains(const key_type &k) const {
            return contains<decltype(k)>(k);
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        iterator
        lower_bound(const key_type &k) {
            return lower_bound<decltype(k)>(k);
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        const_iterator
        lower_bound(const key_type &k) const {
            return lower_bound<decltype(k)>(k);
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        template <typename K>
        iterator
        lower_bound(const K &x) {
            return lower_bound_partial(begin(), end(), x);
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        template <typename K>
        const_iterator
        lower_bound(const K &x) const {
            return const_cast<associative_vector *>(this)->lower_bound(x);
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        iterator
        upper_bound(const key_type &k) {
            return upper_bound<decltype(k)>(k);
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        const_iterator
        upper_bound(const key_type &k) const {
            return upper_bound<decltype(k)>(k);
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        template <typename K>
        iterator
        upper_bound(const K &x) {
            return upper_bound_partial(begin(), end(), x);
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        template <typename K>
        const_iterator
        upper_bound(const K &x) const {
            return const_cast<associative_vector *>(this)->upper_bound(x);
        }

        /// \brief Get pair with lower_bound and upper_bound
        /// This will only work properly for ordered containers
        std::pair<iterator, iterator>
        equal_range(const key_type &k) {
            return equal_range<decltype(k)>(k);
        }

        /// \brief Get pair with lower_bound and upper_bound
        /// This will only work properly for ordered containers
        std::pair<const_iterator, const_iterator>
        equal_range(const key_type &k) const {
            return equal_range<decltype(k)>(k);
        }

        /// \brief Get pair with lower_bound and upper_bound
        /// This will only work properly for ordered containers
        template <typename K>
        std::pair<iterator, iterator>
        equal_range(const K &x) {
            auto first = lower_bound(x);
            auto last = upper_bound_partial(first, end(), x);
            return std::make_pair(std::move(first), std::move(last));
        }

        /// \brief Get pair with lower_bound and upper_bound
        /// This will only work properly for ordered containers
        template <typename K>
        std::pair<const_iterator, const_iterator>
        equal_range(const K &x) const {
            return const_cast<associative_vector *>(this)->equal_range(x);
        }

    private /* modifier implementations */:
        /// \brief Emplace element at hint hint of the small map
        /// \param hint Position before element will be constructed
        /// In a usual map, the node would be constructed in-place and then
        /// attached to its proper position. We unfortunately cannot do this
        /// here because we need to construct the object before find its
        /// proper position in the vector. We can then move the object, if
        /// this is possible.
        template <class... Args>
        std::pair<iterator, bool>
        emplace_hint_impl(const_iterator hint, Args &&...args) {
            value_type obj(std::forward<Args>(args)...);
            return emplace_hint_impl_key(
                std::move(hint),
                maybe_first(obj),
                std::move(obj));
        }

        /// \brief Emplace element at hint hint of the small map
        /// \param hint Position before element will be constructed
        template <class K, class... Args>
        std::enable_if_t<
            is_comp_tr || std::is_same_v<K, key_type>,
            std::pair<iterator, bool>>
        emplace_hint_impl_key(const_iterator hint, const K &k, Args &&...args) {
            if constexpr (!IsOrdered) {
                if constexpr (!IsMulti) {
                    if (auto const it = find(k); it != end()) {
                        return std::make_pair(it, false);
                    }
                }
                return std::make_pair(
                    iterator(data_.emplace(
                        maybe_base(hint),
                        std::forward<Args>(args)...)),
                    true);
            }

            auto const it = upper_bound_hint(hint, k);
            if constexpr (!IsMulti)
            {
                if (it != begin() && !comp_(maybe_first(*std::prev(it)), k)) {
                    return std::make_pair(std::prev(it), false);
                }
            }
            return std::make_pair(
                iterator(
                    data_.emplace(maybe_base(it), std::forward<Args>(args)...)),
                true);
        }

    private /* element lookup */:
        /// \brief Iterator to first element not less than key (with hint)
        /// This will only work properly for ordered containers
        /// Adapted from libc++
        template <typename K>
        iterator
        upper_bound_hint(const_iterator hint, K &&x) {
            // x < hint
            if (hint == end() || comp_(x, maybe_first(*hint))) {
                // prev(hint) <= x < hint
                if (hint == begin() || !comp_(x, maybe_first(*std::prev(hint))))
                {
                    return unconst(hint);
                }

                return upper_bound_partial(begin(), hint, x);
            } // else hint <= x

            auto const next = std::next(hint);

            // hint <= x < next(hint)
            if (next == end() || comp_(x, maybe_first(*next))) {
                return unconst(next);
            }

            return upper_bound_partial(next, end(), x);
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        template <typename K>
        std::enable_if_t<is_comp_tr || std::is_same_v<K, key_type>, iterator>
        lower_bound_partial(
            const_iterator first,
            const_iterator last,
            const K &x) {
            return iterator(std::lower_bound(
                maybe_base(unconst(first)),
                maybe_base(unconst(last)),
                x,
                [this](const auto &p, const auto &v) {
                return comp_(maybe_first(p), v);
            }));
        }

        /// \brief Iterator to first element not less than key
        /// This will only work properly for ordered containers
        template <typename K>
        const_iterator
        lower_bound_partial(
            const_iterator begin,
            const_iterator end,
            const K &x) const {
            return const_cast<associative_vector *>(this)
                ->lower_bound_partial(begin, end, x);
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        template <typename K>
        std::enable_if_t<is_comp_tr || std::is_same_v<K, key_type>, iterator>
        upper_bound_partial(
            const_iterator first,
            const_iterator last,
            const K &x) {
            return iterator(std::upper_bound(
                maybe_base(unconst(first)),
                maybe_base(unconst(last)),
                x,
                [this](const auto &v, const auto &p) {
                return comp_(v, maybe_first(p));
            }));
        }

        /// \brief Iterator to first element greater than key
        /// This will only work properly for ordered containers
        template <typename K>
        const_iterator
        upper_bound_partial(
            const_iterator begin,
            const_iterator end,
            const K &x) const {
            return const_cast<associative_vector *>(this)
                ->upper_bound_partial(begin, end, x);
        }

    private /* element access */:
        /// \brief Logic to access a mapped_type
        template <bool create_if_not_found, class K>
        constexpr mapped_type &
        element_access_implementation(K &&k) {
            if constexpr (IsMap) {
                iterator it = find(k);
                if (it == end()) {
                    if constexpr (create_if_not_found) {
                        std::tie(it, std::ignore) = emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(std::forward<K>(k)),
                            std::tuple<>());
                    } else {
                        throw_exception<std::out_of_range>(
                            "at(): cannot find element in vector map");
                    }
                }
                return it->second;
            } else {
                return data_.at(k);
            }
        }

        /// \brief Logic to access a mapped_type
        template <class K>
        constexpr mapped_type &
        element_access_implementation(K &&k) const {
            return (const_cast<associative_vector *>(this))
                ->template element_access_implementation<false>(
                    std::forward<K>(k));
        }

        /// \brief Check if small map invariants are ok
        [[nodiscard]] constexpr bool
        invariants() const {
            if constexpr (IsOrdered) {
                return std::is_sorted(data_.begin(), data_.end(), value_comp());
            } else {
                return true;
            }
        }

    private /* helpers */:
        /// \brief Check if 2 keys are equivalent
        template <class K1, class K2>
        std::enable_if_t<
            is_comp_tr
                || (std::is_same_v<K1, key_type>
                    && std::is_same_v<K2, key_type>),
            bool>
        keys_equivalent(const K1 &lhs, const K2 &rhs) const {
            return !comp_(lhs, rhs) && !comp_(rhs, lhs);
        }

        template <class EL>
        constexpr static auto &
        maybe_first(EL &&el) {
            if constexpr (IsMap) {
                return el.first;
            } else {
                return el;
            }
        }

        template <class EL>
        constexpr static auto
        maybe_base(EL &&el) {
            if constexpr (IsMap) {
                return el.base();
            } else {
                return el;
            }
        }

        constexpr iterator
        unconst(const_iterator it) {
            return begin() + (it - cbegin());
        }

    private:
        /// \brief Internal vector or map
        vector_type data_{};

        /// \brief A copy of the allocator so we can allocate vectors if
        /// needed
        key_compare comp_{};
    };

    /// \brief operator== for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator==(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return std::equal(x.begin(), x.end(), y.begin(), y.end());
    }

    /// \brief operator!= for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator!=(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return !(x == y);
    }

    /// \brief operator< for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator<(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return std::
            lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

    /// \brief operator> for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator>(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return y < x;
    }

    /// \brief operator<= for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator<=(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return !(y < x);
    }

    /// \brief operator>= for flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    constexpr bool
    operator>=(
        const associative_vector<b1, b2, b3, V, C> &x,
        const associative_vector<b1, b2, b3, V, C> &y) {
        return !(x < y);
    }

    /// \brief swap the contents of two flat maps
    template <bool b1, bool b2, bool b3, class V, class C>
    void
    swap(
        associative_vector<b1, b2, b3, V, C> &x,
        associative_vector<b1, b2, b3, V, C> &y) noexcept(noexcept(x.swap(y))) {
        x.swap(y);
    }

    /// \brief Erase elements that match the predicate
    template <bool b1, bool b2, bool b3, class V, class C, class Pred>
    typename associative_vector<b1, b2, b3, V, C>::size_type
    erase_if(associative_vector<b1, b2, b3, V, C> &c, Pred p) {
        return c.erase_if(p);
    }
} // namespace small::detail

#endif // SMALL_DETAIL_CONTAINER_ASSOCIATIVE_VECTOR_HPP
