//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_EXCEPTION_THROW_HPP
#define SMALL_DETAIL_EXCEPTION_THROW_HPP

#include <small/detail/traits/cpp_version.hpp>
#include <exception>
#include <utility>
#include <type_traits>

#ifndef cpp_exceptions
#    define SMALL_DISABLE_EXCEPTIONS
#endif

namespace small {
    namespace detail {

        /// \brief Throw an exception but terminate if we can't throw
        template <typename Ex>
        [[noreturn]] void
        throw_exception(Ex &&ex) {
#ifndef SMALL_DISABLE_EXCEPTIONS
            throw static_cast<Ex &&>(ex);
#else
            (void) ex;
            std::terminate();
#endif
        }

        /// \brief Construct and throw an exception but terminate otherwise
        template <typename Ex, typename... Args>
        [[noreturn]] void
        throw_exception(Args &&...args) {
            throw_exception(Ex(std::forward<Args>(args)...));
        }

        /// \brief Throw an exception but terminate if we can't throw
        template <typename ThrowFn, typename CatchFn>
        void
        catch_exception(ThrowFn &&thrower, CatchFn &&catcher) {
#ifndef SMALL_DISABLE_EXCEPTIONS
            try {
                return static_cast<ThrowFn &&>(thrower)();
            }
            catch (std::exception &) {
                return static_cast<CatchFn &&>(catcher)();
            }
#else
            return static_cast<ThrowFn &&>(thrower)();
#endif
        }

    } // namespace detail
} // namespace small

#endif // SMALL_DETAIL_EXCEPTION_THROW_HPP
