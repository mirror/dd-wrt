//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_VECTOR_HPP
#define SMALL_VECTOR_HPP

#include <small/detail/exception/scope_guard.hpp>
#include <small/detail/exception/throw.hpp>
#include <small/detail/iterator/iterator_type_traits.hpp>
#include <small/detail/iterator/pointer_wrapper.hpp>
#include <small/detail/traits/default_inline_storage.hpp>
#include <small/detail/traits/enable_allocator_from_this.hpp>
#include <small/detail/traits/is_range.hpp>
#include <small/detail/traits/is_relocatable.hpp>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <ratio>

namespace small {
    /// \brief Vector of elements with a buffer for small vectors
    ///
    /// A vector optimized for the case when it's small.
    ///
    /// - Inline allocation for small number of elements
    /// - Custom expected size
    /// - Special treatment of relocatable types
    ///   - Relocatable types are defined by default for POD types and aggregate
    ///   types of PODs
    ///   - Some types might be dynamically relocatable (don't have internal
    ///   pointers all the time)
    /// - Better grow factors
    /// - Consider the cache line size in allocations
    /// (https://github.com/NickStrupat/CacheLineSize)
    ///
    /// When there are less elements than a given threshold,
    /// the elements are kept in a stack buffer for small vectors.
    /// Otherwise, it works as usual. This makes cache locality
    /// even better.
    ///
    /// This is what is usually used as a data structure for
    /// small vector. However, if you are 100% sure you will never
    /// need more than N elements, a max_size_vector is a more
    /// appropriate container.
    ///
    /// At worst, a std::vector<T> would take sizeof(std::vector<T>)
    /// bytes. So a small vector with sizeof(std::vector<T>)/sizeof(T)
    /// elements is a very conservative default because you would have
    /// using sizeof(std::vector<T>) bytes on the stack anyway.
    /// For instance:
    /// - sizeof(std::vector<int>)/sizeof(int) == 6
    /// - sizeof(std::vector<int>)/sizeof(int) == 24
    /// - sizeof(std::vector<float>)/sizeof(float) == 6
    ///
    /// This works well as a conservative default for fundamental data
    /// structures, but this will result in 1 for most other data
    /// structures. Thus, we also use a minimum of 5 elements for
    /// this default because even the smallest vectors are probably
    /// going to have at least 5 elements.
    ///
    /// This implementation was used folly, abseil, and LLVM as a reference.
    ///
    /// \tparam T Array type
    /// \tparam N Array maximum expected size
    template <
        class T,
        size_t N = default_inline_storage_v<T>,
        class Allocator = std::allocator<T>,
        class AllowHeap = std::true_type,
        class SizeType = size_t,
        class GrowthFactor = std::ratio<3, 2>>
    class vector
        : public detail::
              enable_allocator_from_this<std::is_empty_v<Allocator>, Allocator>
    {
    private:
        using enable_allocator_type = detail::
            enable_allocator_from_this<std::is_empty_v<Allocator>, Allocator>;

    public:
        /// \section Common container types
        typedef SizeType size_type;
        typedef T value_type;
        typedef Allocator allocator_type;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef ptrdiff_t difference_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef detail::pointer_wrapper<pointer> iterator;
        typedef detail::pointer_wrapper<const_pointer> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        static_assert(
            std::is_unsigned<size_t>::value,
            "Size type should be an unsigned integral type");
        static_assert(
            (std::is_same<typename allocator_type::value_type, value_type>::
                 value),
            "Allocator::value_type must be same type as value_type");

    public:
        /// \section Compile-time inferences

        /// \brief Size of a pointer
        static constexpr size_t pointer_size = sizeof(value_type *);

        /// \brief Size of a size type
        static constexpr size_t size_type_size = sizeof(size_type);

        /// \brief Size of new for the heap pointers
        static constexpr size_t heap_storage_size = pointer_size
                                                    + size_type_size;

        /// \brief Size of a value
        static constexpr size_t value_size = sizeof(value_type);

        /// \brief How many values we can store in the space allocated for the
        /// heap
        static constexpr size_t inline_values_per_heap = heap_storage_size
                                                         / value_size;

        /// \brief Min reasonable inline
        /// It's reasonable that:
        /// - less than can fit is the heap pointers is a waste
        /// - 0 is doesn't make sense
        /// - 1 is no reason to create a vector
        static constexpr size_t min_reasonable_inline_elements = 2;

        /// \brief Pointers per value
        static constexpr size_t min_inline_elements = std::
            max(inline_values_per_heap, min_reasonable_inline_elements);

        /// \brief Pointers per value
        static constexpr auto requested_inline_size = N;

        /// \brief Number of elements we should inline
        /// If the requested number is below the number we can fit in the
        /// pointer itself, use that instead
        static constexpr std::size_t num_inline_elements = std::
            max(min_inline_elements, requested_inline_size);

        /// \brief The type of the raw array we would use to store inline vectors
        using raw_value_type_array = value_type[num_inline_elements];

        /// \brief Type we would use for inline storage if we do
        typedef typename std::aligned_storage_t<
            sizeof(raw_value_type_array),
            alignof(raw_value_type_array)>
            inline_storage_data_type;

        /// \brief True if inline storage would always be empty
        static constexpr bool inline_storage_empty
            = sizeof(value_type) * num_inline_elements == 0;

        /// \brief Final inline storage type
        /// This is the inline storage data type or (very rarely) just a pointer
        typedef typename std::conditional_t<
            !inline_storage_empty,
            inline_storage_data_type,
            pointer>
            inline_storage_type;

        /// \brief An assumption about the size of a cache line
        /// \note Clang unfortunately defines
        /// __cpp_lib_hardware_interference_size without defining
        /// hardware_constructive_interference_size.
        static constexpr std::size_t cache_line_size =
#if defined(__cpp_lib_hardware_interference_size) && !defined(__clang__)
            std::hardware_constructive_interference_size;
#else
            2 * sizeof(std::max_align_t);
#endif

        /// \brief True if we should just copy the inline storage
        static constexpr bool should_copy_inline
            = std::is_trivially_copyable_v<value_type>
              && sizeof(inline_storage_type) <= cache_line_size / 2;

        /// \brief True if we are using the std::allocator
        static constexpr bool using_std_allocator = std::
            is_same<allocator_type, std::allocator<value_type>>::value;

        /// \brief An empty type when we want to remove a member from the class
        using empty_type = std::tuple<>;

        /// \brief Type we use to internally store the allocator
        /// We don't really represent the std::allocator in the class because it
        /// has no members We can just recreate it at get_alloc()
        using internal_allocator_type = std::
            conditional_t<using_std_allocator, empty_type, allocator_type>;

        /// \brief Use memcpy to copy items
        /// If type is relocatable, we just use memcpy
        static constexpr bool relocate_use_memcpy = is_relocatable_v<T>
                                                    && using_std_allocator;

    public:
        /// \section Rule of five constructors

        /// \brief Destructor
        /// Deallocate memory if it's not inline
        ~vector() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (auto &t: *this) {
                    (&t)->~value_type();
                }
            }
            free_heap();
        }

        /// \brief Copy constructor
        vector(const vector &rhs) : enable_allocator_type() {
            if constexpr (std::is_empty_v<allocator_type>) {
                enable_allocator_type::set_allocator(allocator_type(
                    std::allocator_traits<allocator_type>::
                        select_on_container_copy_construction(
                            rhs.get_allocator())));
            } else {
                enable_allocator_type::set_allocator(allocator_type(
                    std::allocator_traits<allocator_type>::
                        select_on_container_copy_construction(rhs.alloc_)));
            }

            if constexpr (should_copy_inline) {
                if (rhs.is_inline()) {
                    copy_inline_trivial(rhs);
                    return;
                }
            }

            auto n = rhs.size();
            make_size(n);
            if constexpr (std::is_trivially_copy_constructible_v<value_type>) {
                std::memcpy(
                    (void *) begin().base(),
                    (void *) rhs.begin().base(),
                    n * sizeof(T));
            } else {
                {
                    auto rollback = make_guard([&] { free_heap(); });
                    std::uninitialized_copy(
                        rhs.begin(),
                        rhs.begin() + n,
                        begin());
                    rollback.dismiss();
                }
            }
            this->set_internal_size(n);
        }

        /// \brief Move constructor
        vector(vector &&rhs) noexcept(
            std::is_nothrow_move_constructible<value_type>::value)
            : enable_allocator_type() {
            if constexpr (!std::is_empty_v<allocator_type>) {
                enable_allocator_type::set_allocator(std::move(rhs.alloc_));
            } else {
                enable_allocator_type::set_allocator(
                    enable_allocator_type::get_allocator());
            }

            if (rhs.is_external()) {
                this->data_.heap_storage_.pointer_ = rhs.data_.heap_storage_
                                                         .pointer_;
                rhs.data_.heap_storage_.pointer_ = nullptr;
                size_ = rhs.size_;
                rhs.size_ = 0;
                this->data_.set_capacity(rhs.data_.get_capacity());
                rhs.data_.set_capacity(0);
            } else {
                if (should_copy_inline) {
                    copy_inline_trivial(rhs);
                    rhs.size_ = 0;
                } else {
                    auto n = rhs.size();
                    if constexpr (std::is_trivially_move_constructible_v<
                                      value_type>)
                    {
                        std::memcpy(
                            (void *) begin().base(),
                            (void *) rhs.begin().base(),
                            n * sizeof(T));
                    } else {
                        std::uninitialized_copy(
                            std::make_move_iterator(rhs.begin()),
                            std::make_move_iterator(rhs.end()),
                            begin());
                    }
                    this->set_internal_size(n);
                    rhs.clear();
                }
            }
        }

        /// \brief Copy assignment
        vector &
        operator=(const vector &rhs) {
            if (this == &rhs) {
                return *this;
            }

            constexpr bool should_copy_alloc = std::allocator_traits<
                allocator_type>::propagate_on_container_copy_assignment::value;
            if constexpr (should_copy_alloc) {
                enable_allocator_type::set_allocator(rhs.alloc_);
            }

            if constexpr (should_copy_inline) {
                if (this->is_inline() && rhs.is_inline()) {
                    // cheap copy the inline buffer
                    copy_inline_trivial(rhs);
                    return *this;
                }
            }

            if (rhs.size() < capacity()) {
                // rhs fits in lhs capacity
                const size_t n = rhs.size();
                if constexpr (std::is_trivially_copy_assignable_v<value_type>) {
                    std::memcpy(
                        (void *) begin().base(),
                        (void *) rhs.begin().base(),
                        n * sizeof(T));
                } else {
                    partially_uninitialized_copy(rhs.begin(), n, begin(), size());
                }
                this->set_internal_size(n);
            } else {
                // rhs does not fit in lhs current capacity
                assign(rhs.begin(), rhs.end());
            }

            return *this;
        }

        /// \brief Move assignment
        vector &
        operator=(vector &&rhs) noexcept(
            std::is_nothrow_move_constructible<value_type>::value
            && (allocator_type::propagate_on_container_move_assignment::value
                || allocator_type::is_always_equal::value)) {
            const bool should_move = std::allocator_traits<
                allocator_type>::propagate_on_container_move_assignment::value;
            if constexpr (should_move) {
                if constexpr (std::is_empty_v<allocator_type>) {
                    enable_allocator_type::set_allocator(
                        enable_allocator_type::get_allocator());
                } else {
                    enable_allocator_type::set_allocator(std::move(rhs.alloc_));
                }
                if (this != &rhs) {
                    if (this->is_external() || rhs.is_external()) {
                        reset();
                    }
                    if (rhs.is_inline()) {
                        if (should_copy_inline) {
                            copy_inline_trivial(rhs);
                            rhs.size_ = 0;
                        } else {
                            const size_t n = rhs.size();
                            if constexpr (std::is_trivially_move_assignable_v<
                                              value_type>)
                            {
                                std::memcpy(
                                    (void *) data_.buffer(),
                                    (void *) rhs.data_.buffer(),
                                    n * sizeof(T));
                            } else {
                                partially_uninitialized_copy(
                                    std::make_move_iterator(rhs.data_.buffer()),
                                    n,
                                    this->data_.buffer(),
                                    size());
                            }
                            this->set_internal_size(n);
                            rhs.clear();
                        }
                    } else {
                        // rhs is external
                        this->data_.heap_storage_.pointer_
                            = rhs.data_.heap_storage_.pointer_;
                        rhs.data_.heap_storage_.pointer_ = nullptr;
                        // this was already reset above, so it's empty and
                        // internal.
                        size_ = rhs.size_;
                        rhs.size_ = 0;
                        this->data_.set_capacity(rhs.data_.get_capacity());
                        rhs.data_.set_capacity(0);
                    }
                }
            } else {
                assign(
                    std::make_move_iterator(rhs.begin()),
                    std::make_move_iterator(rhs.end()));
            }
            return *this;
        }

    private:
        /// \section Lambda constructor from which we can construct
        template <
            typename InitFunc,
            std::enable_if_t<std::is_invocable_v<InitFunc, void *>, int> = 0>
        vector(size_type n, InitFunc &&func, const allocator_type &alloc)
            : enable_allocator_type() {
            enable_allocator_type::set_allocator(alloc);
            make_size(n);
            assert(size() == 0);
            this->increment_internal_size(n);
            {
                auto rollback = make_guard([&] { free_heap(); });
                populate_mem_forward(data(), n, std::forward<InitFunc>(func));
                rollback.dismiss();
            }
            assert(invariants());
        }

    public /* constructors */:
        /// \section Initialization constructors

        /// \brief Construct empty small array
        /// Create the default allocator
        constexpr vector() noexcept(
            std::is_nothrow_default_constructible<allocator_type>::value)
            : vector(allocator_type()) {}

        /// \brief Construct small vector with a given allocator
        constexpr explicit vector(const allocator_type &alloc)
            : vector(0, alloc) {}

        /// \brief Construct small array with size n
        /// Any of the n values should be constructed with new (p) value_type();
        constexpr explicit vector(
            size_type n,
            const allocator_type &alloc = allocator_type())
            : vector(n, [&](void *p) { new (p) value_type(); }, alloc) {}

        /// \brief Construct small array with size n and fill with single value
        constexpr vector(
            size_type n,
            const value_type &value,
            const allocator_type &alloc = allocator_type())
            : vector(n, [&](void *p) { new (p) value_type(value); }, alloc) {}

        /// \brief Construct small array from a pair of iterators
        template <class Iterator>
        constexpr vector(
            Iterator first,
            detail::enable_if_iterator_t<Iterator, value_type> last,
            const allocator_type &alloc = allocator_type()) {
            enable_allocator_type::set_allocator(alloc);
            // Handle input iterators
            constexpr bool is_input_iterator = std::is_same_v<
                typename std::iterator_traits<Iterator>::iterator_category,
                std::input_iterator_tag>;
            if (is_input_iterator) {
                while (first != last) {
                    emplace_back(*first++);
                }
                return;
            }

            // Handle inline vector
            size_type distance = std::distance(first, last);
            if (distance <= num_inline_elements) {
                this->increment_internal_size(distance);
                populate_mem_forward(data_.buffer(), distance, [&](void *p) {
                    new (p) value_type(*first++);
                });
                return;
            }

            // Handle external vector
            make_size(distance);
            this->increment_internal_size(distance);
            {
                auto rollback = make_guard([&] { free_heap(); });
                populate_mem_forward(data_.heap(), distance, [&](void *p) {
                    new (p) value_type(*first++);
                });
                rollback.dismiss();
            }

            assert(invariants());
        }

        /// \brief Construct small array from initializer list
        constexpr vector(
            std::initializer_list<value_type> il,
            const allocator_type &alloc = allocator_type())
            : vector(il.begin(), il.end(), alloc) {}

        /// \brief Construct small array from a range
        /// This range might also be a std::vector
        template <
            class Range,
            std::enable_if_t<detail::is_range_v<Range>, int> = 0>
        constexpr explicit vector(
            Range &&r,
            const allocator_type &alloc = allocator_type())
            : vector(r.begin(), r.end(), alloc) {}

        /// \brief Assign small array from initializer list
        constexpr vector &
        operator=(std::initializer_list<value_type> il) {
            assign(il.begin(), il.end());
            return *this;
        }

        /// \brief Assign small array from iterators
        template <class InputIterator>
        constexpr void
        assign(
            InputIterator first,
            detail::enable_if_iterator_t<InputIterator, value_type> last) {
            clear();
            insert(end(), first, last);
        }

        /// \brief Assign small array from size and fill with value
        constexpr void
        assign(size_type n, const value_type &u) {
            clear();
            insert(end(), n, u);
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
        swap(vector &rhs) noexcept(
            std::is_nothrow_move_constructible_v<value_type>
            && std::is_nothrow_swappable_v<value_type>) {
            // Allow ADL on swap for our value_type.
            using std::swap;

            const bool both_external = this->is_external() && rhs.is_external();
            if (both_external) {
                std::swap(size_, rhs.size_);
                auto *tmp = data_.heap_storage_.pointer_;
                data_.heap_storage_.pointer_ = rhs.data_.heap_storage_.pointer_;
                rhs.data_.heap_storage_.pointer_ = tmp;
                const auto capacity_ = this->data_.get_capacity();
                this->set_capacity(rhs.data_.get_capacity());
                rhs.data_.set_capacity(capacity_);
                return;
            }

            bool both_inline = this->is_inline() && rhs.is_inline();
            if (both_inline) {
                auto &old_small = size() < rhs.size() ? *this : rhs;
                auto &old_large = size() < rhs.size() ? rhs : *this;

                // Swap all elements up of the smaller one
                for (size_type i = 0; i < old_small.size(); ++i) {
                    swap(old_small[i], old_large[i]);
                }

                // Move elements from the larger one
                size_type i = old_small.size();
                const size_type ci = i;
                {
                    // If it fails, destruct the old large values we haven't
                    // moved
                    auto rollback = make_guard([&] {
                        old_small.set_internal_size(i);
                        if constexpr (!std::is_trivially_destructible_v<T>) {
                            for (; i < old_large.size(); ++i) {
                                old_large[i].~value_type();
                            }
                        }
                        old_large.set_internal_size(ci);
                    });
                    // Move elements from the larger vector to the small one
                    for (; i < old_large.size(); ++i) {
                        auto element_address = (old_small.begin() + i).base();
                        new (element_address)
                            value_type(std::move(old_large[i]));
                        if constexpr (!std::is_trivially_destructible_v<T>) {
                            old_large[i].~value_type();
                        }
                    }
                    rollback.dismiss();
                }
                old_small.set_internal_size(i);
                old_large.set_internal_size(ci);
                return;
            }

            // swap an external with an internal
            auto &old_external = rhs.is_external() ? rhs : *this;
            auto &old_internal = rhs.is_external() ? *this : rhs;
            auto old_external_capacity = old_external.capacity();
            auto old_external_heap = old_external.data_.heap_storage_.pointer_;

            // Store a pointer to the old external / new internal buffer
            auto old_external_buffer = old_external.data_.buffer();

            // Move elements from old internal to old external
            size_type i = 0;
            {
                // In case anything goes wrong
                auto rollback = make_guard([&] {
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        // Destroy elements from old external
                        for (size_type kill = 0; kill < i; ++kill) {
                            old_external_buffer[kill].~value_type();
                        }
                        // Destroy elements from old internal
                        for (; i < old_internal.size(); ++i) {
                            old_internal[i].~value_type();
                        }
                    }
                    // Reset sizes
                    old_internal.size_ = 0;
                    old_external.data_.heap_storage_.pointer_
                        = old_external_heap;
                    old_external.set_capacity(old_external_capacity);
                });
                // Move elements from old internal to old external buffer
                for (; i < old_internal.size(); ++i) {
                    new (&old_external_buffer[i])
                        value_type(std::move(old_internal[i]));
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        old_internal[i].~value_type();
                    }
                }
                rollback.dismiss();
            }

            // Adjust pointers
            old_internal.data_.heap_storage_.pointer_ = old_external_heap;
            std::swap(size_, rhs.size_);
            old_internal.set_capacity(old_external_capacity);

            // Change allocators
            const bool should_swap = std::allocator_traits<
                allocator_type>::propagate_on_container_swap::value;
            if constexpr (should_swap) {
                swap_allocator(rhs.alloc_);
            }

            assert(invariants());
        }

    public /* iterators */:
        /// \brief Get iterator to first element
        constexpr iterator
        begin() noexcept {
            return iterator(data());
        }

        /// \brief Get constant iterator to first element[[nodiscard]]
        constexpr const_iterator
        begin() const noexcept {
            return const_iterator(data());
        }

        /// \brief Get iterator to last element
        constexpr iterator
        end() noexcept {
            return iterator(data() + size());
        }

        /// \brief Get constant iterator to last element
        constexpr const_iterator
        end() const noexcept {
            return const_iterator(data() + size());
        }

        /// \brief Get iterator to first element in reverse order
        constexpr reverse_iterator
        rbegin() noexcept {
            return std::reverse_iterator<iterator>(end());
        }

        /// \brief Get constant iterator to first element in reverse order
        constexpr const_reverse_iterator
        rbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(end());
        }

        /// \brief Get iterator to last element in reverse order
        constexpr reverse_iterator
        rend() noexcept {
            return std::reverse_iterator<iterator>(begin());
        }

        /// \brief Get constant iterator to last element in reverse order
        constexpr const_reverse_iterator
        rend() const noexcept {
            return std::reverse_iterator<const_iterator>(begin());
        }

        /// \brief Get constant iterator to first element
        constexpr const_iterator
        cbegin() const noexcept {
            return const_iterator(begin());
        }

        /// \brief Get constant iterator to last element
        constexpr const_iterator
        cend() const noexcept {
            return cbegin() + size();
        }

        /// \brief Get constant iterator to first element in reverse order
        constexpr const_reverse_iterator
        crbegin() const noexcept {
            return std::reverse_iterator<const_iterator>(cend());
        }

        /// \brief Get constant iterator to last element in reverse order
        constexpr const_reverse_iterator
        crend() const noexcept {
            return std::reverse_iterator<const_iterator>(cbegin());
        }

    public /* capacity */:
        /// \brief Get small array size
        [[nodiscard]] constexpr size_type
        size() const noexcept {
            return get_unmasked_size();
        }

        /// \brief Get small array max size
        [[nodiscard]] constexpr size_type
        max_size() const noexcept {
            if constexpr (!should_use_heap) {
                return static_cast<size_type>(num_inline_elements);
            } else {
                constexpr size_type max_with_mask = size_type(clear_size_mask);
                return (std::min)(
                    std::allocator_traits<allocator_type>::max_size(
                        enable_allocator_type::get_allocator()),
                    max_with_mask);
            }
        }

        /// \brief Get small array capacity (same as max_size())
        [[nodiscard]] constexpr size_type
        capacity() const noexcept {
            if constexpr (!should_use_heap) {
                return num_inline_elements;
            } else {
                if (is_external()) {
                    return data_.get_capacity();
                } else {
                    return num_inline_elements;
                }
            }
        }

        /// \brief Check if small array is empty
        [[nodiscard]] constexpr bool
        empty() const noexcept {
            return !size();
        }

        /// \brief Check if vector is inline
        [[nodiscard]] constexpr bool
        is_inline() const {
            return !is_external();
        }

        /// \brief Reserve space for n elements
        /// We concentrate the logic to switch the variant types in these
        /// functions
        void
        reserve(size_type n) {
            make_size(n);
        }

        /// \brief Reserve space for n elements
        /// We concentrate the logic to switch the variant types in these
        /// functions The behaviour of this function might change
        void
        reserve() {
            shrink_to_fit();
        }

        /// \brief Shrink internal array to fit only the current elements
        /// We concentrate the logic to switch the variant types in these
        /// functions
        void
        shrink_to_fit() noexcept {
            if (this->is_inline()) {
                return;
            }
            vector tmp(begin(), end());
            tmp.swap(*this);
        }

    public /* element access */:
        /// \brief Get reference to n-th element in small array
        constexpr reference
        operator[](size_type n) {
            assert(n < size() && "vector::operator[] index out of bounds");
            return *(begin() + n);
        }

        /// \brief Get constant reference to n-th element in small array
        constexpr const_reference
        operator[](size_type n) const {
            assert(n < size() && "vector::operator[] index out of bounds");
            return *(begin() + n);
        }

        /// \brief Check bounds and get reference to n-th element in small array
        constexpr reference
        at(size_type n) {
            if (n >= size()) {
                detail::throw_exception<std::out_of_range>(
                    "at: cannot access element after vector::size()");
            }
            return (*this)[n];
        }

        /// \brief Check bounds and get constant reference to n-th element in
        /// small array
        constexpr const_reference
        at(size_type n) const {
            if (n >= size()) {
                detail::throw_exception<std::out_of_range>(
                    "at const: cannot access element after vector::size()");
            }
            return (*this)[n];
        }

        /// \brief Get reference to first element in small array
        constexpr reference
        front() {
            assert(!empty() && "front() called for empty small array");
            return *begin();
        }

        /// \brief Get constant reference to first element in small array
        constexpr const_reference
        front() const {
            assert(!empty() && "front() called for empty small array");
            return *begin();
        }

        /// \brief Get reference to last element in small array
        constexpr reference
        back() {
            assert(!empty() && "back() called for empty small array");
            return *(end() - 1);
        }

        /// \brief Get constant reference to last element in small array
        constexpr const_reference
        back() const {
            assert(!empty() && "back() called for empty small array");
            return *(end() - 1);
        }

        /// \brief Get reference to internal pointer to small array data
        constexpr T *
        data() noexcept {
            return this->is_external() ? data_.heap() : data_.buffer();
        }

        /// \brief Get constant reference to internal pointer to small array data
        constexpr const T *
        data() const noexcept {
            return this->is_external() ? data_.heap() : data_.buffer();
        }

    public /* modifiers */:
        /// \brief Copy element to end of small array
        constexpr void
        push_back(const value_type &v) {
            emplace_back(v);
        }

        /// \brief Move element to end of small array
        constexpr void
        push_back(value_type &&v) {
            emplace_back(std::move(v));
        }

        /// \brief Emplace element to end of small array
        template <class... Args>
        constexpr reference
        emplace_back(Args &&...args) {
            // Handle inline vector
            if (size_ < num_inline_elements) {
                auto *ptr = new (data_.buffer() + size_)
                    value_type(std::forward<Args>(args)...);
                this->increment_internal_size(1);
                return *ptr;
            } else {
                if constexpr (!should_use_heap) {
                    detail::throw_exception<std::length_error>(
                        "emplace_back: max_size exceeded in small_vector");
                } else {
                    // Handle external vectors
                    size_type old_size = size();
                    size_type old_capacity = capacity();
                    const bool needs_to_grow = old_capacity == old_size;
                    if (needs_to_grow) {
                        // Internal vector
                        make_size(old_size + 1, [&](void *p) {
                            new (p) value_type(std::forward<Args>(args)...);
                        }, old_size);
                    } else {
                        // External vector
                        new (data_.heap() + old_size)
                            value_type(std::forward<Args>(args)...);
                    }
                    this->increment_internal_size(1);
                    return *(data_.heap() + old_size);
                }
            }
        }

        /// \brief Remove element from end of small array
        constexpr void
        pop_back() {
            destroy_and_downsize(size() - 1);
        }

        /// \brief Emplace element to a position in small array
        template <class... Args>
        constexpr iterator
        emplace(const_iterator position, Args &&...args) {
            // Special case
            const bool is_emplace_back = position == cend();
            if (is_emplace_back) {
                emplace_back(std::forward<Args>(args)...);
                return end() - 1;
            }

            // Construct and insert
            return insert(position, value_type(std::forward<Args>(args)...));
        }

        /// \brief Copy element to a position in small array
        constexpr iterator
        insert(const_iterator position, const value_type &x) {
            return insert(position, 1, x);
        }

        /// \brief Move element to a position in small array
        constexpr iterator
        insert(const_iterator position, value_type &&x) {
            iterator p = unconst(position);
            const bool is_push_back = p == end();
            if (is_push_back) {
                push_back(std::move(x));
                return end() - 1;
            }

            auto offset = p - begin();
            auto old_size = size();
            const bool must_grow = capacity() == old_size;
            if (must_grow) {
                make_size(old_size + 1, [&x](void *ptr) {
                    new (ptr) value_type(std::move(x));
                }, offset);
                this->increment_internal_size(1);
            } else {
                shift_right_and_construct(
                    data() + offset,
                    data() + old_size,
                    data() + old_size + 1,
                    [&]() mutable -> value_type && { return std::move(x); });
                this->increment_internal_size(1);
            }
            return begin() + offset;
        }

        /// \brief Copy n elements to a position in small array
        constexpr iterator
        insert(const_iterator position, size_type n, const value_type &x) {
            auto offset = position - begin();
            auto old_size = size();
            make_size(old_size + n);
            shift_right_and_construct(
                data() + offset,
                data() + old_size,
                data() + old_size + n,
                [&]() mutable -> value_type const & { return x; });
            this->increment_internal_size(n);
            return begin() + offset;
        }

        /// \brief Copy element range stating at a position in small array
        template <class Iterator>
        constexpr iterator
        insert(
            const_iterator position,
            Iterator first,
            detail::enable_if_iterator_t<Iterator, value_type> last) {
            // Handle input iterators
            using category = typename std::iterator_traits<
                Iterator>::iterator_category;
            using it_ref = typename std::iterator_traits<Iterator>::reference;
            if constexpr (std::is_same_v<category, std::input_iterator_tag>) {
                auto offset = position - begin();
                while (first != last) {
                    position = insert(position, *first++);
                    ++position;
                }
                return begin() + offset;
            }

            // Other iterator types
            auto const distance = std::distance(first, last);
            auto const offset = position - begin();
            auto old_size = size();
            assert(distance >= 0);
            assert(offset >= 0);
            make_size(old_size + distance);
            shift_right_and_construct(
                data() + offset,
                data() + old_size,
                data() + old_size + distance,
                [&, in = last]() mutable -> it_ref { return *--in; });
            this->increment_internal_size(distance);
            return begin() + offset;
        }

        /// \brief Copy elements from initializer list to a position in small
        /// array
        constexpr iterator
        insert(const_iterator position, std::initializer_list<value_type> il) {
            return insert(position, il.begin(), il.end());
        }

        /// \brief Erase element at a position in small array
        constexpr iterator
        erase(const_iterator position) {
            return erase(position, std::next(position));
        }

        /// \brief Erase range of elements in the small array
        constexpr iterator
        erase(const_iterator first, const_iterator last) {
            if (first == last) {
                return unconst(first);
            }

            const auto n_erase = last - first;
            if constexpr (is_relocatable_v<value_type> && using_std_allocator) {
                // Move elements directly in memory
                const auto n_after_erase = cend() - last;
                if (n_erase >= n_after_erase) {
                    std::memcpy(
                        (void *) first.base(),
                        (void *) last.base(),
                        n_after_erase * sizeof(value_type));
                } else {
                    std::memmove(
                        (void *) first.base(),
                        (void *) last.base(),
                        n_after_erase * sizeof(value_type));
                }
            } else {
                // Move elements in memory
                std::move(last, cend(), unconst(first));

                // Destruct elements that were moved from and no longer in-use
                // N.B. Only do this for non-relocatable types, otherwise you'd
                // be running the destructor on exact byte copies of in-use
                // elements, and you might free their internal buffers (oh no!).
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    std::destroy_n(crbegin(), n_erase);
                }
            }

            // Directly set internal size. Elements are already destroyed.
            set_internal_size(size() - std::distance(first, last));
            return unconst(first);
        }

        /// \brief Clear elements in the small array
        constexpr void
        clear() noexcept {
            destroy_and_downsize(0);
        }

        /// \brief Resize the small array
        /// If we are using a vector to store the data, resize will not
        /// move the data back to the stack even if the new size is less
        /// than the small_vector capacity
        constexpr void
        resize(size_type n) {
            if (n <= size()) {
                destroy_and_downsize(n);
                return;
            }
            auto extra = n - size();
            make_size(n);
            populate_mem_forward((begin() + size()).base(), extra, [&](void *p) {
                new (p) value_type();
            });
            this->increment_internal_size(extra);
        }

        /// \brief Resize and fill the small array
        constexpr void
        resize(size_type n, const value_type &v) {
            if (n < size()) {
                erase(begin() + n, end());
                return;
            }
            auto extra = n - size();
            make_size(n);
            populate_mem_forward((begin() + size()).base(), extra, [&](void *p) {
                new (p) value_type(v);
            });
            this->increment_internal_size(extra);
        }

    private:
        /// \brief Check if small array invariants are ok
        [[nodiscard]] constexpr bool
        invariants() const {
            if (size() > capacity()) {
                return false;
            }
            if (begin() > end()) {
                return false;
            }
            if (end() > begin() + capacity()) {
                return false;
            }
            return true;
        }

    private:
        /// \brief Whether this vector should also use the heap
        static constexpr bool should_use_heap = AllowHeap::value;

        /// \brief A mask with the most significant bit of the size type
        static size_type constexpr size_type_most_significant_bit_mask
            = size_type(1) << (sizeof(size_type) * 8 - 1);

        /// \brief A mask to identify if the array is external
        static size_type constexpr is_external_mask
            = should_use_heap ? size_type_most_significant_bit_mask : 0;

        /// \brief Identify if we are using the heap
        /// A mask to extract the size bits identifying whether the current
        /// vector it's external The two most significant digits can tell us if
        /// something is external / allocated in the heap
        [[nodiscard]] constexpr bool
        is_external() const {
            return is_external_mask & size_;
        }

        /// \brief Set the flag indicating the vector is external
        void
        set_external(bool b) {
            if (b) {
                size_ |= is_external_mask;
            } else {
                size_ &= ~is_external_mask;
            }
        }

        /// \brief Set the capacity of the external vector
        void
        set_capacity(size_type new_capacity) {
            assert(this->is_external());
            assert(new_capacity < std::numeric_limits<size_type>::max());
            data_.set_capacity(new_capacity);
        }

        /// \brief Return the max size of this vector
        /// We remove the final digit we use to identify the use of the heap
        static size_type constexpr clear_size_mask = ~is_external_mask;

        /// \brief Set the size variable
        /// This sets the size and maintains the inline bit
        void
        set_internal_size(std::size_t sz) {
            assert(sz <= clear_size_mask);
            size_ = (is_external_mask & size_) | size_type(sz);
        }

        /// \brief Get the size value without the mask
        [[nodiscard]] constexpr size_t
        get_unmasked_size() const {
            return size_ & clear_size_mask;
        }

        /// \brief Free the vector heap
        void
        free_heap() {
            if (is_external()) {
                auto alloc_instance = enable_allocator_type::get_allocator();
                std::allocator_traits<allocator_type>::
                    deallocate(alloc_instance, data(), capacity());
                data_.heap_storage_.pointer_ = nullptr;
            }
        }

        /// \brief Copy the inline storage from rhs vector when type is
        /// trivially copyable
        template <
            class T2 = value_type,
            std::enable_if_t<std::is_trivially_copyable_v<T2>, int> = 0>
        void
        copy_inline_trivial(vector const &rhs) {
            // Copy the whole buffer to maintain the loop with fixed size
            std::copy(
                rhs.data_.buffer(),
                rhs.data_.buffer() + num_inline_elements,
                data_.buffer());
            this->set_internal_size(rhs.size());
        }

        /// \brief Copy the inline storage from rhs vector when type is not
        /// trivially copyable
        template <
            class T2 = value_type,
            std::enable_if_t<!std::is_trivially_copyable_v<T2>, int> = 0>
        void
        copy_inline_trivial(vector const &) {
            throw std::logic_error(
                "Attempting to trivially copy not trivially copyable type");
        }

        /// \brief Make it empty and with no heap
        void
        reset() {
            clear();
            free_heap();
            size_ = 0;
        }

        /// \brief Change the size to a new size
        void
        make_size(size_type new_size) {
            if (new_size <= capacity()) {
                return;
            }
            make_size_internal<std::false_type>(new_size, [](void *) {
                detail::throw_exception<std::logic_error>(
                    "Should not emplace when changing size");
            }, 0);
        }

        /// \brief Change the size and emplace the elements as we go
        template <typename EmplaceFunc>
        void
        make_size(
            size_type new_size,
            EmplaceFunc &&emplace_function,
            size_type new_emplaced_size) {
            assert(size() == capacity());
            make_size_internal<std::true_type>(
                new_size,
                std::forward<EmplaceFunc>(emplace_function),
                new_emplaced_size);
        }

        /// \brief Change the current size to new size and emplace the elements
        /// This will heapify the vector if it's inline
        template <typename InsertVersion, typename EmplaceFunc>
        void
        make_size_internal(
            size_type new_size,
            EmplaceFunc &&emplace_func,
            size_type new_emplaced_size) {
            // Invariants
            if (new_size > max_size()) {
                detail::throw_exception<std::length_error>(
                    "make_size: max_size exceeded in small_vector");
            } else {
                if constexpr (!should_use_heap) {
                    return;
                } else {
                    // New heap pointer
                    size_type new_capacity = std::
                        max(new_size, compute_new_size());
                    auto alloc_instance = enable_allocator_type::get_allocator();
                    value_type *new_heap_ptr = std::allocator_traits<
                        allocator_type>::allocate(alloc_instance, new_capacity);

                    // Copy data
                    {
                        auto rollback = make_guard([&] {
                            std::allocator_traits<allocator_type>::deallocate(
                                alloc_instance,
                                new_heap_ptr,
                                new_capacity);
                        });
                        if constexpr (InsertVersion::value) {
                            // move the begin()/end() range to the
                            // new_heap_ptr/new_emplaced_size range and insert
                            // the new element
                            this->move_to_uninitialized_emplace(
                                begin().base(),
                                end().base(),
                                new_heap_ptr,
                                new_emplaced_size,
                                std::forward<EmplaceFunc>(emplace_func));
                        } else {
                            // move the begin()/end() range to the range
                            // starting at new_heap_ptr
                            this->move_to_uninitialized(
                                begin().base(),
                                end().base(),
                                new_heap_ptr);
                        }
                        rollback.dismiss();
                    }

                    // Destruct values we already copied
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        for (auto &val: *this) {
                            val.~value_type();
                        }
                    }

                    // Free the old heap
                    free_heap();

                    // Store the new pointer
                    data_.heap_storage_.pointer_ = new_heap_ptr;
                    this->set_external(true);
                    this->set_capacity(new_capacity);
                }
            }
        }

        /// \brief Move from begin/end range to uninitialized range out/pos and
        /// call the emplace function at pos
        template <class EmplaceFunc>
        void
        move_to_uninitialized_emplace(
            value_type *begin,
            value_type *end,
            value_type *out,
            size_type pos,
            EmplaceFunc &&emplace_func) {
            // Emplace the element at the position
            emplace_func(out + pos);

            {
                // Move old elements to the left of the new one
                auto rollback = make_guard([&] { //
                    out[pos].~T();
                });
                this->move_to_uninitialized(begin, begin + pos, out);
                rollback.dismiss();
            }

            {
                // move old elements to the right of the new one
                auto rollback = make_guard([&] {
                    for (size_type i = 0; i <= pos; ++i) {
                        out[i].~T();
                    }
                });
                if (begin + pos < end) {
                    this->move_to_uninitialized(begin + pos, end, out + pos + 1);
                }
                rollback.dismiss();
            }
        }

        /// \brief Move the range first/last to uninitialized memory starting at
        /// out
        template <
            class T2 = value_type,
            std::enable_if_t<!std::is_trivially_copyable_v<T2>, int> = 0>
        void
        move_to_uninitialized(T *first, T *last, T *out) {
            std::size_t idx = 0;
            {
                // Call destructor for all elements we emplaced correctly if
                // something fails
                auto rollback = make_guard([&] {
                    for (std::size_t i = 0; i < idx; ++i) {
                        out[i].~T();
                    }
                });
                // Move elements
                for (; first != last; ++first, ++idx) {
                    new (&out[idx]) T(std::move(*first));
                }
                rollback.dismiss();
            }
        }

        /// \brief Move the range first/last to uninitialized memory starting at
        /// out
        template <
            class T2 = value_type,
            std::enable_if_t<std::is_trivially_copyable_v<T2>, int> = 0>
        void
        move_to_uninitialized(T *first, T *last, T *out) {
            // Just memmove all the data
            std::memmove(
                static_cast<void *>(out),
                static_cast<void const *>(first),
                (last - first) * sizeof *first);
        }

        /// \brief Compute the new size this vector should have after growing
        /// This is a growth factor of 1.5
        constexpr size_type
        compute_new_size() const {
            const auto old_capacity = capacity();
            // Set the initial capacity
            if (old_capacity == 0) {
                return (std::max)(64 / sizeof(value_type), size_type(5));
            }
            // Blocks larger than or equal to 4096 bytes can be expanded in place
            constexpr size_t min_in_place_expansion = 4096;
            const bool cannot_be_in_place = old_capacity
                                            < min_in_place_expansion / sizeof(T);
            if (cannot_be_in_place) {
                return capacity() * 2;
            }
            const bool very_very_large_already = capacity()
                                                 > 4096 * 32 / sizeof(T);
            if (very_very_large_already) {
                return capacity() * 2;
            }
            // Apply usual growth factor
            return (std::min)(
                (static_cast<size_type>(GrowthFactor::num) * capacity())
                        / static_cast<size_type>(GrowthFactor::den)
                    + 1,
                max_size());
        }

        /// \brief Copy some elements as initialized and some as uninitialized
        template <class Iterator1, class Iterator2>
        static void
        partially_uninitialized_copy(
            Iterator1 from,
            size_t from_size,
            Iterator2 to,
            size_t to_size) {
            const size_t min_size = (std::min)(from_size, to_size);
            std::copy(from, from + min_size, to);
            if (from_size > to_size) {
                std::uninitialized_copy(
                    from + min_size,
                    from + from_size,
                    to + min_size);
            } else {
                for (auto it = to + min_size; it != to + to_size; ++it) {
                    using Value = typename std::decay<decltype(*it)>::type;
                    it->~Value();
                }
            }
        }

        /// \brief A std::memcpy implementation that ensures the values are
        /// moved byte by byte This is what std::memcpy usually does, but some
        /// compilers implement it differently
        static void
        byte_copy(void *destination, void *source, size_t n) {
            char *char_source = (char *) source;
            char *char_destination = (char *) destination;
            std::copy(char_source, char_source + n, char_destination);
        }

        /// \brief Move elements to the right a construct at the new location
        template <
            class Construct,
            class T2 = value_type,
            std::enable_if_t<!std::is_trivially_copyable_v<T2>, int> = 0>
        void
        shift_right_and_construct(
            T *const first,
            T *const last,
            T *const new_last,
            Construct &&create) {
            // Input is same as output
            if (last == new_last) {
                return;
            }

            // Input and output for the elements moving
            T *out = new_last;
            T *in = last;
            {
                // In case anything goes wrong
                auto rollback = make_guard([&] {
                    // Destroy the out/last range
                    if (out < last) {
                        out = last - 1;
                    }
                    for (auto it = out + 1; it != new_last; ++it) {
                        it->~T();
                    }
                });

                // Move elements from "in" to uninitialized "out"
                while (in != first && out > last) {
                    // Out must be decremented before an exception can be thrown
                    // so that the rollback guard knows where to start.
                    --out;
                    --in;
                    if constexpr (
                        is_relocatable_v<value_type> && using_std_allocator)
                    {
                        byte_copy(out, in, sizeof(value_type));
                    } else {
                        new (out) T(std::move(*in));
                    }
                }

                // Move elements from "in" to initialized "out"
                while (in != first) {
                    --out;
                    --in;
                    if constexpr (
                        is_relocatable_v<value_type> && using_std_allocator)
                    {
                        byte_copy(out, in, sizeof(value_type));
                    } else {
                        *out = std::move(*in);
                    }
                }

                // Construct elements in uninitialized "out"
                while (out > last) {
                    --out;
                    new (out) T(create());
                }

                // Construct elements in initialized "out"
                while (out != first) {
                    --out;
                    if constexpr (
                        is_relocatable_v<value_type> && using_std_allocator)
                    {
                        new (out) T(create());
                    } else {
                        *out = create();
                    }
                }

                rollback.dismiss();
            }
        }

        /// \brief Shirt the range [first, last] to [new_first, new_last] and
        /// fill the range [first, new_first] with the `create` function
        template <
            class Construct,
            class T2 = value_type,
            std::enable_if_t<std::is_trivially_copyable_v<T2>, int> = 0>
        void
        shift_right_and_construct(
            T *const first,
            T *const last,
            T *const new_last,
            Construct &&create) {
            // Move elements backward from [first, last] to [new_first, new_last]
            std::move_backward(first, last, new_last);

            // Create elements in the range [first, new_first] backward
            const auto n_create = new_last - last;
            if (n_create > 0) {
                T *create_out = first + n_create;
                do {
                    --create_out;
                    *create_out = create();
                }
                while (create_out != first);
            }
        }

        /// \brief Populate memory with the given function
        template <class Function>
        void
        populate_mem_forward(
            value_type *mem,
            std::size_t n,
            Function const &op) {
            std::size_t idx = 0;
            {
                auto rollback = make_guard([&] {
                    for (std::size_t i = 0; i < idx; ++i) {
                        mem[i].~T();
                    }
                });
                for (size_t i = 0; i < n; ++i) {
                    op(&mem[idx]);
                    ++idx;
                }
                rollback.dismiss();
            }
        }

        /// \brief Increment the internal size
        void
        increment_internal_size(std::size_t n) {
            assert(get_unmasked_size() + n <= max_size());
            size_ += size_type(n);
        }

        /// \brief Destroy elements after n and decrement the internal size
        /// If elements are relocatable and we are erasing elements, we should
        /// directly destroy the appropriate elements and call set_internal_size
        void
        destroy_and_downsize(std::size_t n) {
            assert(n <= size());
            // Destroy extra elements
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (auto it = (begin() + n); it != end(); ++it) {
                    it->~value_type();
                }
            }
            // Set internal size
            this->set_internal_size(n);
        }

        /// Unconst an iterator
        static iterator
        unconst(const_iterator it) {
            return iterator(const_cast<value_type *>(it.base()));
        }

    private:
        /// \brief A pointer to the heap and the capacity of the array in the
        /// heap This is usual storage we use when the vector is not inline.
        /// This class doesn't handle allocations directly though.
        struct heap_storage_type
        {
            value_type *pointer_{ nullptr };
            size_type capacity_;
            size_type
            get_capacity() const {
                return capacity_;
            }
            void
            set_capacity(size_type c) {
                capacity_ = c;
            }
        };

        /// \brief Data type used to represent the vector
        /// This is a union that might be storing a inline or heap array at a
        /// given time
        union data_type
        {
            /// \brief Storage when the vector is inline
            inline_storage_type inline_storage_;

            /// \brief Storage when the vector is in the heap
            heap_storage_type heap_storage_;

            /// \brief By default, we have a heap element pointing to nullptr
            /// (size == 0)
            explicit data_type() {
                heap_storage_.pointer_ = nullptr;
                heap_storage_.capacity_ = 0;
            }

            /// \brief Get a pointer to the buffer if it's inline
            value_type *
            buffer() noexcept {
                void *vp = &inline_storage_;
                return static_cast<value_type *>(vp);
            }

            /// \brief Get a const pointer to the buffer if it's inline
            value_type const *
            buffer() const noexcept {
                return const_cast<data_type *>(this)->buffer();
            }

            /// \brief Get a pointer to the array if it's not inline
            value_type *
            heap() noexcept {
                return heap_storage_.pointer_;
            }

            /// \brief Get a const pointer to the array if it's not inline
            value_type const *
            heap() const noexcept {
                return heap_storage_.pointer_;
            }

            /// \brief Get the current allocated capacity if it's not inline
            size_type
            get_capacity() const {
                return heap_storage_.get_capacity();
            }

            /// \brief Set the current allocated capacity if it's not inline
            void
            set_capacity(size_type c) {
                heap_storage_.set_capacity(c);
            }
        };

        /// \brief Internal array or vector
        data_type data_{};

        /// \brief The number of elements in the storage
        size_type size_{ 0 };
    };

    /// Type deduction
    template <class T, class... U>
    vector(T, U...) -> vector<T, 1 + sizeof...(U)>;

    /// \brief operator== for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator==(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return std::equal(x.begin(), x.end(), y.begin(), y.end());
    }

    /// \brief operator!= for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator!=(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return !(x == y);
    }

    /// \brief operator< for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator<(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return std::
            lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

    /// \brief operator> for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator>(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return y < x;
    }

    /// \brief operator<= for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator<=(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return !(y < x);
    }

    /// \brief operator>= for small arrays
    template <class T, size_t N, class A, class H, class S>
    constexpr bool
    operator>=(const vector<T, N, A, H, S> &x, const vector<T, N, A, H, S> &y) {
        return !(x < y);
    }

    /// \brief swap the contents of two small arrays
    template <class T, size_t N, class A, class H, class S>
    void
    swap(vector<T, N, A, H, S> &x, vector<T, N, A, H, S> &y) noexcept(
        noexcept(x.swap(y))) {
        x.swap(y);
    }

    /// \brief Create a vector from a raw array
    /// This is similar to std::to_array
    template <
        class T,
        size_t N_INPUT,
        size_t N_OUTPUT = (std::max)(default_inline_storage_v<T>, N_INPUT)>
    constexpr vector<std::remove_cv_t<T>, N_OUTPUT>
    to_vector(T (&a)[N_INPUT]) {
        return vector<std::remove_cv_t<T>, N_OUTPUT>(a, a + N_INPUT);
    }

    /// \brief Create a vector from a raw array
    /// This is similar to std::to_array
    template <
        class T,
        size_t N_INPUT,
        size_t N_OUTPUT = (std::max)(default_inline_storage_v<T>, N_INPUT)>
    constexpr vector<std::remove_cv_t<T>, N_OUTPUT>
    to_vector(T (&&a)[N_INPUT]) {
        return vector<std::remove_cv_t<T>, N_OUTPUT>(a, a + N_INPUT);
    }

    template <
        class T,
        size_t N = (std::max)((24 * 2) / sizeof(T), std::size_t(5)),
        class Allocator = std::allocator<T>,
        class SizeType = size_t>
    using max_size_vector = vector<T, N, Allocator, std::false_type, SizeType>;

    template <
        class T,
        size_t N = (std::max)((24 * 2) / sizeof(T), std::size_t(5)),
        class Allocator = std::allocator<T>,
        class SizeType = size_t>
    using inline_vector = vector<T, N, Allocator, std::true_type, SizeType>;

} // namespace small

#endif // SMALL_VECTOR_HPP
