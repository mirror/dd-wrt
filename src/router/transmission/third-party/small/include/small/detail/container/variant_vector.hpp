//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_CONTAINER_VARIANT_VECTOR_HPP
#define SMALL_DETAIL_CONTAINER_VARIANT_VECTOR_HPP

#include <small/iterator/iterator_type_traits.hpp>
#include <small/iterator/pointer_wrapper.hpp>
#include <small/max_size_vector.hpp>
#include <small/vector.hpp>
#include <cstdlib>
#include <variant>
#include <vector>
#include <cpp-manifest/fmt_tools/fmt_error.h>

/// \brief A small vector that uses a variant for inline elements
/// This is only kept here for reference

namespace small {
    namespace detail {
        /// \brief Vector of elements with a buffer for small vectors
        ///
        /// A vector optimized for the case when it's small.
        ///
        /// - Inline allocation for small number of elements
        /// - Custom expected size
        /// - Special treatment of relocatable types
        ///   - Relocatable types are defined by default for POD types and
        ///   aggregate types of PODs
        ///   - Some types might be dynamically relocatable (don't have internal
        ///   pointers all the time)
        /// - Better default grow factors
        /// - Custom grow factors
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
        /// \tparam T Array type
        /// \tparam N Array maximum expected size
        template <
            class T,
            size_t N = std::
                max((sizeof(std::vector<T>) * 2) / sizeof(T), std::size_t(5)),
            class Allocator = std::allocator<T>>
        class small_vector
        {
        private:
            /// Classes of small vector errors
            enum class small_vector_error_code : int
            {
                none = 0,
                out_of_range,
            };

            /// \brief An unknown error category for new error types
            /// This is better than generic category, which has very specific
            /// messages associated with its error codes
            class small_vector_error_category_type : public std::error_category
            {
            public:
                /// \brief Constructs the error category object
                constexpr small_vector_error_category_type() noexcept = default;

                /// \brief destructs an error_category
                ~small_vector_error_category_type() override = default;

                /// \brief Obtains the name of the category
                [[nodiscard]] const char *
                name() const noexcept override {
                    return "small_vector";
                }

                /// \brief Maps error_code to error_condition
                [[nodiscard]] std::error_condition
                default_error_condition(int code) const noexcept override {
                    return std::error_condition(code, *this);
                }

                /// \brief Checks whether error code is equivalent to an error
                /// condition for the error category
                [[nodiscard]] bool
                equivalent(int code, const std::error_condition &condition)
                    const noexcept override {
                    return default_error_condition(code) == condition;
                }

                /// \brief Checks whether error code is equivalent to an error
                /// condition for the error category
                [[nodiscard]] bool
                equivalent(const std::error_code &code, int condition)
                    const noexcept override {
                    return *this == code.category()
                           && code.value() == condition;
                }

                /// \brief Obtains the explanatory string
                [[nodiscard]] std::string
                message(int code) const override {
                    // return the same string for all error codes
                    switch (code) {
                    case static_cast<int>(small_vector_error_code::none):
                        return "<no error>";
                    case static_cast<int>(
                        small_vector_error_code::out_of_range):
                        return "Out of range";
                    default:
                        return "Small vector unknown error";
                    }
                }
            };

            /// \brief Obtains a reference to the static error category object
            /// for unknown errors
            static const std::error_category &
            small_vector_error_category() noexcept {
                static small_vector_error_category_type c;
                return c;
            }

            public /* types */:
            typedef Allocator allocator_type;
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

            static_assert(
                (std::is_same<typename allocator_type::value_type, value_type>::
                     value),
                "Allocator::value_type must be same type as value_type");

            private /* types */:
            /// Small array type to keep array in stack
            using small_vector_type = max_size_vector<T, N>;

            /// Vector type to keep array in heap
            using large_vector_type = std::vector<T, allocator_type>;

            /// Variant type to keep array in stack or heap
            using variant_type = std::
                variant<small_vector_type, large_vector_type>;

            public /* constructors */:
            /// \brief Construct empty small array
            constexpr small_vector() noexcept(
                std::is_nothrow_default_constructible<allocator_type>::value)
                : small_vector(allocator_type()) {}

            /// \brief Construct small vector with a given allocator
            constexpr explicit small_vector(const allocator_type &alloc)
                : small_vector(0, alloc) {}

            /// \brief Construct small array with size n
            constexpr explicit small_vector(
                size_type n,
                const allocator_type &alloc = allocator_type())
                : alloc_(alloc),
                  data_(
                      n > N ? variant_type(large_vector_type(n, alloc)) :
                              variant_type(small_vector_type(n))) {
                assert(invariants());
            }

            /// \brief Construct small array with size n and fill with single
            /// value
            constexpr small_vector(
                size_type n,
                const value_type &value,
                const allocator_type &alloc = allocator_type())
                : small_vector(n, alloc) {
                std::fill(begin(), end(), value);
                assert(invariants());
            }

            /// \brief Construct small array from a pair of iterators
            template <class InputIterator>
            constexpr small_vector(
                InputIterator first,
                enable_if_iterator_t<InputIterator, value_type> last,
                const allocator_type &alloc = allocator_type())
                : alloc_(alloc),
                  data_(
                      std::distance(first, last) > difference_type(N) ?
                          variant_type(large_vector_type(first, last, alloc)) :
                          variant_type(small_vector_type(first, last))) {
                assert(invariants());
            }

            /// \brief Construct small array from initializer list
            constexpr small_vector(
                std::initializer_list<value_type> il,
                const allocator_type &alloc = allocator_type())
                : small_vector(il.begin(), il.end(), alloc) {}

            /// \brief Assign small array from initializer list
            constexpr small_vector &
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
                auto n = std::distance(first, last);
                resize(n);
                std::copy(first, last, begin());
                assert(invariants());
            }

            /// \brief Assign small array from size and fill with value
            constexpr void
            assign(size_type n, const value_type &u) {
                resize(n);
                std::fill(begin(), end(), u);
                assert(invariants());
            }

            /// \brief Assign small array from initializer list
            constexpr void
            assign(std::initializer_list<value_type> il) {
                resize(il.size());
                std::copy(il.begin(), il.end(), begin());
                assert(invariants());
            }

            /// \brief Fill small array with value u
            constexpr void
            fill(const T &u) {
                std::fill(begin(), end(), u);
                assert(invariants());
            }

            /// \brief Swap the contents of two small arrays
            constexpr void
            swap(small_vector &a) noexcept(std::is_nothrow_swappable_v<T>) {
                data_.swap(a.data_);
                std::swap(alloc_, a.alloc_);
                assert(invariants());
            }

            /// \brief Get copy of allocator for dynamic vector
            allocator_type
            get_allocator() const noexcept {
                return alloc_;
            }

            public /* iterators */:
            /// \brief Get iterator to first element
            constexpr iterator
            begin() noexcept {
                return iterator(
                    std::visit([](auto &x) { return x.data(); }, data_));
            }

            /// \brief Get constant iterator to first element[[nodiscard]]
            constexpr const_iterator
            begin() const noexcept {
                return const_iterator(
                    std::visit([](auto &x) { return x.data(); }, data_));
            }

            /// \brief Get iterator to last element
            constexpr iterator
            end() noexcept {
                return begin() + size();
            }

            /// \brief Get constant iterator to last element
            constexpr const_iterator
            end() const noexcept {
                return begin() + size();
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
                return const_iterator(
                    std::visit([](auto &x) { return &x[0]; }, data_));
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
                return std::visit([](auto &x) { return x.size(); }, data_);
            }

            /// \brief Get small array max size
            [[nodiscard]] constexpr size_type
            max_size() const noexcept {
                return (std::min)<size_type>(
                    std::allocator_traits<allocator_type>::max_size(alloc_),
                    std::numeric_limits<difference_type>::max());
            }

            /// \brief Get small array capacity (same as max_size())
            [[nodiscard]] constexpr size_type
            capacity() const noexcept {
                return std::visit([](auto &x) { return x.capacity(); }, data_);
            }

            /// \brief Check if small array is empty
            [[nodiscard]] constexpr bool
            empty() const noexcept {
                return std::visit([](auto &x) { return x.empty(); }, data_);
            }

            /// \brief Reserve space for n elements
            /// We concentrate the logic to switch the variant types in these
            /// functions
            void
            reserve(size_type n) {
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    if (n > data_as_array.max_size()) {
                        std::vector<T, allocator_type>
                            tmp(data_as_array.begin(), data_as_array.end());
                        tmp.reserve(n);
                        data_ = std::move(tmp);
                    }
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    data_as_vector.reserve(n);
                }
            }

            /// \brief Shrink internal array to fit only the current elements
            /// We concentrate the logic to switch the variant types in these
            /// functions
            void
            shrink_to_fit() noexcept {
                if (std::holds_alternative<large_vector_type>(data_)) {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    if (data_as_vector.size() > N) {
                        data_as_vector.shrink_to_fit();
                    } else {
                        small_vector_type
                            tmp(data_as_vector.begin(), data_as_vector.end());
                        data_ = std::move(tmp);
                    }
                }
            }

            public /* element access */:
            /// \brief Get reference to n-th element in small array
            constexpr reference
            operator[](size_type n) {
                assert(n < size() && "small_vector[] index out of bounds");
                return begin()[n];
            }

            /// \brief Get constant reference to n-th element in small array
            constexpr const_reference
            operator[](size_type n) const {
                assert(n < size() && "small_vector[] index out of bounds");
                return cbegin()[n];
            }

            /// \brief Check bounds and get reference to n-th element in small
            /// array
            constexpr reference
            at(size_type n) {
                if (n >= size()) {
                    this->throw_out_of_range(
                        "at({0}): cannot access element {0} in array of size "
                        "{1}",
                        n,
                        size());
                }
                return begin()[n];
            }

            /// \brief Check bounds and get constant reference to n-th element
            /// in small array
            constexpr const_reference
            at(size_type n) const {
                if (n >= size()) {
                    this->throw_out_of_range(
                        "at({0}) const: cannot access element {0} in array of "
                        "size {1}",
                        n,
                        size());
                }
                return begin()[n];
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
                return operator[](size() - 1);
            }

            /// \brief Get constant reference to last element in small array
            constexpr const_reference
            back() const {
                assert(!empty() && "back() called for empty small array");
                return operator[](size() - 1);
            }

            /// \brief Get reference to internal pointer to small array data
            constexpr T *
            data() noexcept {
                return begin().base();
            }

            /// \brief Get constant reference to internal pointer to small array
            /// data
            constexpr const T *
            data() const noexcept {
                return begin().base();
            }

            public /* modifiers */:
            /// \brief Copy element to end of small array
            constexpr void
            push_back(const value_type &x) {
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    data_as_array.push_back(x);
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    data_as_vector.push_back(x);
                }
            }

            /// \brief Move element to end of small array
            constexpr void
            push_back(value_type &&x) {
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    data_as_array.push_back(std::move(x));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    data_as_vector.push_back(std::move(x));
                }
            }

            /// \brief Emplace element to end of small array
            template <class... Args>
            constexpr reference
            emplace_back(Args &&...args) {
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return data_as_array.emplace_back(
                        std::forward<Args>(args)...);
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return data_as_vector.emplace_back(
                        std::forward<Args>(args)...);
                }
            }

            /// \brief Remove element from end of small array
            constexpr void
            pop_back() {
                return std::visit([](auto &d) { return d.pop_back(); }, data_);
            }

            /// \brief Emplace element to a position in small array
            template <class... Args>
            constexpr iterator
            emplace(const_iterator position, Args &&...args) {
                size_t p = position - begin();
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(&*data_as_array.emplace(
                        data_as_array.begin() + p,
                        std::forward<Args>(args)...));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(&*data_as_vector.emplace(
                        data_as_vector.begin() + p,
                        std::forward<Args>(args)...));
                }
            }

            /// \brief Copy element to a position in small array
            constexpr iterator
            insert(const_iterator position, const value_type &x) {
                size_t p = position - begin();
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array.insert(data_as_array.begin() + p, x));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector.insert(data_as_vector.begin() + p, x));
                }
            }

            /// \brief Move element to a position in small array
            constexpr iterator
            insert(const_iterator position, value_type &&x) {
                size_t p = position - begin();
                reserve(size() + 1);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array
                              .insert(data_as_array.begin() + p, std::move(x)));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector
                              .insert(data_as_vector.begin() + p, std::move(x)));
                }
            }

            /// \brief Copy n elements to a position in small array
            constexpr iterator
            insert(const_iterator position, size_type n, const value_type &x) {
                size_t p = position - begin();
                reserve(size() + n);
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array.insert(data_as_array.begin() + p, n, x));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector
                              .insert(data_as_vector.begin() + p, n, x));
                }
            }

            /// \brief Copy element range stating at a position in small array
            template <class InputIterator>
            constexpr iterator
            insert(
                const_iterator position,
                InputIterator first,
                enable_if_iterator_t<InputIterator, value_type> last) {
                size_t p = position - begin();
                reserve(size() + std::distance(first, last));
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array
                              .insert(data_as_array.begin() + p, first, last));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector
                              .insert(data_as_vector.begin() + p, first, last));
                }
            }

            /// \brief Copy elements from initializer list to a position in
            /// small array
            constexpr iterator
            insert(
                const_iterator position,
                std::initializer_list<value_type> il) {
                size_t p = position - begin();
                reserve(size() + il.size());
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array.insert(data_as_array.begin() + p, il));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector.insert(data_as_vector.begin() + p, il));
                }
            }

            /// \brief Erase element at a position in small array
            constexpr iterator
            erase(const_iterator position) {
                size_t p = position - begin();
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(
                        &*data_as_array.erase(data_as_array.begin() + p));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(
                        &*data_as_vector.erase(data_as_vector.begin() + p));
                }
            }

            /// \brief Erase range of elements in the small array
            constexpr iterator
            erase(const_iterator first, const_iterator last) {
                size_t p_first = first - begin();
                size_t p_last = last - begin();
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    return iterator(&*data_as_array.erase(
                        data_as_array.begin() + p_first,
                        data_as_array.begin() + p_last));
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    return iterator(&*data_as_vector.erase(
                        data_as_vector.begin() + p_first,
                        data_as_vector.begin() + p_last));
                }
            }

            /// \brief Clear elements in the small array
            constexpr void
            clear() noexcept {
                resize(0);
            }

            /// \brief Resize the small array
            /// If we are using a vector to store the data, resize will not
            /// move the data back to the stack even if the new size is less
            /// than the small_vector capacity
            constexpr void
            resize(size_type n) {
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    if (n > data_as_array.max_size()) {
                        large_vector_type
                            tmp(data_as_array.begin(),
                                data_as_array.end(),
                                alloc_);
                        tmp.resize(n);
                        data_ = std::move(tmp);
                    } else {
                        data_as_array.resize(n);
                    }
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    data_as_vector.resize(n);
                }
            }

            /// \brief Resize and fill the small array
            constexpr void
            resize(size_type n, const value_type &c) {
                if (std::holds_alternative<small_vector_type>(data_)) {
                    auto &data_as_array = std::get<small_vector_type>(data_);
                    if (n > data_as_array.max_size()) {
                        large_vector_type
                            tmp(data_as_array.begin(),
                                data_as_array.end(),
                                alloc_);
                        tmp.resize(n, c);
                        data_ = std::move(tmp);
                    } else {
                        data_as_array.resize(n, c);
                    }
                } else {
                    auto &data_as_vector = std::get<large_vector_type>(data_);
                    data_as_vector.resize(n, c);
                }
            }

        private:
            /// \brief Check if small array invariants are ok
            [[nodiscard]] constexpr bool
            invariants() const {
                if (size() > capacity()) {
                    return false;
                }
                if (begin().base() == nullptr) {
                    return false;
                }
                if (end().base() == nullptr) {
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

            /// \brief Throw an out of range error
            template <typename... Args>
            void
            throw_out_of_range(const char *what_arg, const Args &...args) {
                throw_error_code(
                    small_vector_error_code::out_of_range,
                    what_arg,
                    args...);
            }

            /// \brief Throw any error related to small vectors
            template <typename... Args>
            void
            throw_error_code(
                small_vector_error_code e,
                const char *what_arg,
                const Args &...args) {
                throw fmt_error(
                    e,
                    small_vector_error_category(),
                    what_arg,
                    args...);
            }

        private:
            /// \brief A copy of the allocator so we can allocate vectors if
            /// needed
            allocator_type alloc_{};

            /// \brief Internal array or vector
            variant_type data_{};
        };

        /// Type deduction
        template <class T, class... U>
        small_vector(T, U...) -> small_vector<T, 1 + sizeof...(U)>;

        /// \brief operator== for small arrays
        template <class T, size_t N>
        constexpr bool
        operator==(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return std::equal(x.begin(), x.end(), y.begin(), y.end());
        }

        /// \brief operator!= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator!=(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return !(x == y);
        }

        /// \brief operator< for small arrays
        template <class T, size_t N>
        constexpr bool
        operator<(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return std::
                lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
        }

        /// \brief operator> for small arrays
        template <class T, size_t N>
        constexpr bool
        operator>(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return y < x;
        }

        /// \brief operator<= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator<=(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return !(y < x);
        }

        /// \brief operator>= for small arrays
        template <class T, size_t N>
        constexpr bool
        operator>=(const small_vector<T, N> &x, const small_vector<T, N> &y) {
            return !(x < y);
        }

        /// \brief swap the contents of two small arrays
        template <class T, size_t N>
        void
        swap(small_vector<T, N> &x, small_vector<T, N> &y) noexcept(
            noexcept(x.swap(y))) {
            x.swap(y);
        }

        /// \brief create small array from raw array
        template <class T, size_t N_INPUT, size_t N_OUTPUT = N_INPUT>
        constexpr small_vector<std::remove_cv_t<T>, N_OUTPUT>
        to_small_vector(T (&a)[N_INPUT]) {
            return small_vector<std::remove_cv_t<T>, N_OUTPUT>(a, a + N_INPUT);
        }

        /// \brief create small array from raw array
        template <class T, size_t N_INPUT, size_t N_OUTPUT = N_INPUT>
        constexpr small_vector<std::remove_cv_t<T>, N_OUTPUT>
        to_small_vector(T (&&a)[N_INPUT]) {
            return small_vector<std::remove_cv_t<T>, N_OUTPUT>(a, a + N_INPUT);
        }

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_CONTAINER_VARIANT_VECTOR_HPP
