//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_EXCEPTION_SCOPE_GUARD_HPP
#define SMALL_DETAIL_EXCEPTION_SCOPE_GUARD_HPP

#include <small/detail/exception/throw.hpp>
#include <cstdint> // uintptr_t
#include <functional>
#include <iostream>
#include <utility>


namespace small {
    namespace detail {
        /// \brief The common functions in a scope guard
        /// \note Adapted from folly / abseil
        class scope_guard_impl_base
        {
        public:
            /// \brief Tell the scope guard not to run the function
            void
            dismiss() noexcept {
                dismissed_ = true;
            }

            /// \brief Tell the scope guard to run the function again
            void
            rehire() noexcept {
                dismissed_ = false;
            }

        protected:
            /// Create the guard
            explicit scope_guard_impl_base(bool dismissed = false) noexcept
                : dismissed_(dismissed) {}

            /// Terminate if we have an exception
            inline static void
            terminate() noexcept {
                // Ensure the availability of std::cerr
                std::ios_base::Init ioInit;
                std::cerr << "This program will now terminate because a "
                             "scope_guard callback "
                             "threw an \nexception.\n";
                std::rethrow_exception(std::current_exception());
            }

            static scope_guard_impl_base
            make_empty_scope_guard() noexcept {
                return scope_guard_impl_base{};
            }

            template <typename T>
            static const T &
            as_const(const T &t) noexcept {
                return t;
            }

            bool dismissed_;
        };

        /// \brief A scope guard that calls a function when being destructed
        /// unless told otherwise
        template <typename FunctionType, bool InvokeNoexcept>
        class scope_guard_impl : public scope_guard_impl_base
        {
        public:
            explicit scope_guard_impl(FunctionType &fn) noexcept(
                std::is_nothrow_copy_constructible<FunctionType>::value)
                : scope_guard_impl(
                    as_const(fn),
                    makeFailsafe(
                        std::is_nothrow_copy_constructible<FunctionType>{},
                        &fn)) {}

            explicit scope_guard_impl(const FunctionType &fn) noexcept(
                std::is_nothrow_copy_constructible<FunctionType>::value)
                : scope_guard_impl(
                    fn,
                    makeFailsafe(
                        std::is_nothrow_copy_constructible<FunctionType>{},
                        &fn)) {}

            explicit scope_guard_impl(FunctionType &&fn) noexcept(
                std::is_nothrow_move_constructible<FunctionType>::value)
                : scope_guard_impl(
                    std::move_if_noexcept(fn),
                    makeFailsafe(
                        std::is_nothrow_move_constructible<FunctionType>{},
                        &fn)) {}

            /// A tag for a dismissed scope guard
            struct scope_guard_dismissed
            {};

            explicit scope_guard_impl(
                FunctionType &&fn,
                scope_guard_dismissed) noexcept(std::
                                                    is_nothrow_move_constructible<
                                                        FunctionType>::value)
                // No need for failsafe in this case, as the guard is dismissed.
                : scope_guard_impl_base{ true },
                  function_(std::forward<FunctionType>(fn)) {}

            scope_guard_impl(scope_guard_impl &&other) noexcept(
                std::is_nothrow_move_constructible<FunctionType>::value)
                : function_(std::move_if_noexcept(other.function_)) {
                // If the above line attempts a copy and the copy throws, other
                // is left owning the cleanup action and will execute it (or
                // not) depending on the value of other.dismissed_. The
                // following lines only execute if the move/copy succeeded, in
                // which case *this assumes ownership of the cleanup action and
                // dismisses other.
                dismissed_ = std::exchange(other.dismissed_, true);
            }

            ~scope_guard_impl() noexcept(InvokeNoexcept) {
                if (!dismissed_) {
                    execute();
                }
            }

        private:
            static scope_guard_impl_base
            makeFailsafe(std::true_type, const void *) noexcept {
                return make_empty_scope_guard();
            }

            template <typename Fn>
            static auto
            makeFailsafe(std::false_type, Fn *fn) noexcept
                -> scope_guard_impl<decltype(std::ref(*fn)), InvokeNoexcept> {
                return scope_guard_impl<decltype(std::ref(*fn)), InvokeNoexcept>{
                    std::ref(*fn)
                };
            }

            template <typename Fn>
            explicit scope_guard_impl(Fn &&fn, scope_guard_impl_base &&failsafe)
                : scope_guard_impl_base{}, function_(std::forward<Fn>(fn)) {
                failsafe.dismiss();
            }

            void *operator new(std::size_t) = delete;

            void
            execute() noexcept(InvokeNoexcept) {
                if (InvokeNoexcept) {
                    using R = decltype(function_());
                    auto catcher_word = reinterpret_cast<uintptr_t>(&terminate);
                    auto catcher = reinterpret_cast<R (*)()>(catcher_word);
                    catch_exception(function_, catcher);
                } else {
                    function_();
                }
            }

            FunctionType function_;
        };

        /// A decayed type scope guard
        template <typename F, bool InvokeNoExcept>
        using scope_guard_impl_decay
            = scope_guard_impl<typename std::decay<F>::type, InvokeNoExcept>;

    } // namespace detail

    /// \brief The default scope guard alias for a function
    template <class F>
    using scope_guard = detail::scope_guard_impl_decay<F, true>;

    /// \brief Make a scope guard with a function
    template <typename F>
    [[nodiscard]] scope_guard<F>
    make_guard(F &&f) noexcept(noexcept(scope_guard<F>(static_cast<F &&>(f)))) {
        return scope_guard<F>(static_cast<F &&>(f));
    }
} // namespace small

#endif // SMALL_DETAIL_EXCEPTION_SCOPE_GUARD_HPP
