//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef SMALL_DETAIL_TRAITS_CPP_VERSION_HPP
#define SMALL_DETAIL_TRAITS_CPP_VERSION_HPP

/// \headerfile Feature testing support for C++17, C++14, C++11, ...
/// Whenever the C++20 feature testing macros are unavailable, we recur to
/// inferences according to the compiler version, the current C++ version, and
/// the stdlib version. \see https://en.cppreference.com/w/cpp/feature_test

/*
 * General
 */
/// \section Identify compiler version and exception support
#ifndef __cplusplus
#    error "__cplusplus macro undefined"
#else
#    if defined(_MSC_VER) && __cplusplus == 199711L
// It's common for MSVC to report 199711L regardless of the C++ version
// This is a MSVC hack we use when it doesn't report __cplusplus correctly
// \see
// https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-160
// \see
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#        if _MSC_VER <= 1924 // Visual Studio 2019 -> C++17
#            define cplusplus 201603L
#        elif _MSC_VER < 1915 // Visual Studio 2017 -> C++14
#            define cplusplus 201309L
#        elif _MSC_VER < 1900 // Visual Studio 2015 -> C++14
#            define cplusplus 201309L
#        elif _MSC_VER < 1800 // Visual Studio 2013 -> C++11
#            define cplusplus 200809L
#        elif _MSC_VER < 1700 // Visual Studio 2012 -> C++11
#            define cplusplus 200809L
#        elif _MSC_VER < 1600 // Visual Studio 2010 -> C++97
#            define cplusplus 199711L
#        elif _MSC_VER < 1500 // Visual Studio 2008 -> C++97
#            define cplusplus 199711L
#        elif _MSC_VER < 1400 // Visual Studio 2005 -> C++97
#            define cplusplus 199711L
#        elif _MSC_VER < 1310 // Visual Studio 2003 -> C++97
#            define cplusplus 199711L
#        elif _MSC_VER < 1300 // Visual Studio 2002 -> C++97
#            define cplusplus 199711L
#        else // Assume C++17 by default
#            define cplusplus 201603L
#        endif
#    else
#        define cplusplus __cplusplus
#    endif
#endif

#ifdef __cpp_exceptions
#    define cpp_exceptions __cpp_exceptions
#elif cplusplus >= 199711L
#    define cpp_exceptions 199711L
#endif

#ifdef __cpp_rtti
#    define cpp_rtti __cpp_rtti
#elif cplusplus >= 199711L
#    define cpp_rtti 199711L
#endif

/*
 * Language features
 */
#ifdef __has_include
#    if __has_include(<version>)
#        include <version>
#    endif
#    if __has_include(<ciso646>)
#        include <ciso646>
#    endif
#endif

#include <small/detail/traits/hedley.hpp>


// Feature test to check if we can rely on C++20 feature-testing
// This is a C++ feature but most compilers implement it in C++17
// We can check this with the oldest feature there is, which is
// __cpp_static_assert If feature testing is available, we use it for other
// features Else, we recur to inferences about the compiler version
#if __cpp_static_assert
#    define cpp_feature_testing 201911L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_aggregate_bases
#        define cpp_aggregate_bases __cpp_aggregate_bases
#    endif
#elif cplusplus >= 201603L
#    define cpp_aggregate_bases 201603L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_aggregate_nsdmi
#        define cpp_aggregate_nsdmi __cpp_aggregate_nsdmi
#    endif
#elif cplusplus >= 201304L
#    define cpp_aggregate_nsdmi 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_aggregate_paren_init
#        define cpp_aggregate_paren_init __cpp_aggregate_paren_init
#    endif
#elif cplusplus >= 201902L
#    define cpp_aggregate_paren_init 201902L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_alias_templates
#        define cpp_alias_templates __cpp_alias_templates
#    endif
#elif cplusplus >= 200704L
#    define cpp_alias_templates 200704L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_aligned_new
#        define cpp_aligned_new __cpp_aligned_new
#    endif
#elif cplusplus >= 201606L
#    define cpp_aligned_new 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_attributes
#        define cpp_attributes __cpp_attributes
#    endif
#elif cplusplus >= 200809L
#    define cpp_attributes 200809L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_binary_literals
#        define cpp_binary_literals __cpp_binary_literals
#    endif
#elif cplusplus >= 201304L
#    define cpp_binary_literals 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_capture_star_this
#        define cpp_capture_star_this __cpp_capture_star_this
#    endif
#elif cplusplus >= 201603L
#    define cpp_capture_star_this 201603L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_char8_t
#        define cpp_char8_t __cpp_char8_t
#    endif
#elif cplusplus >= 201811L
#    define cpp_char8_t 201811L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_concepts
#        define cpp_concepts __cpp_concepts
#    endif
#elif cplusplus >= 201907L
#    define cpp_concepts 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_conditional_explicit
#        define cpp_conditional_explicit __cpp_conditional_explicit
#    endif
#elif cplusplus >= 201806L
#    define cpp_conditional_explicit 201806L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_consteval
#        define cpp_consteval __cpp_consteval
#    endif
#elif cplusplus >= 201811L
#    define cpp_consteval 201811L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_constexpr
#        define cpp_constexpr __cpp_constexpr
#    endif
#elif cplusplus >= 201907L
#    define cpp_constexpr 201907L
#elif cplusplus >= 201603L
#    define cpp_constexpr 201603L
#elif cplusplus >= 201304L
#    define cpp_constexpr 201304L
#elif cplusplus >= 200704L
#    define cpp_constexpr 200704L
#endif

#if defined(do_constexpr)
#    undef do_constexpr
#endif
#ifdef HEDLEY_CONSTEXPR
#    define do_constexpr HEDLEY_CONSTEXPR
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_constexpr_dynamic_alloc
#        define cpp_constexpr_dynamic_alloc __cpp_constexpr_dynamic_alloc
#    endif
#elif cplusplus >= 201907L
#    define cpp_constexpr_dynamic_alloc 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_constexpr_in_decltype
#        define cpp_constexpr_in_decltype __cpp_constexpr_in_decltype
#    endif
#elif cplusplus >= 201711L
#    define cpp_constexpr_in_decltype 201711L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_constinit
#        define cpp_constinit __cpp_constinit
#    endif
#elif cplusplus >= 201907L
#    define cpp_constinit 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_decltype
#        define cpp_decltype __cpp_decltype
#    endif
#elif cplusplus >= 200707L
#    define cpp_decltype 200707L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_decltype_auto
#        define cpp_decltype_auto __cpp_decltype_auto
#    endif
#elif cplusplus >= 201304L
#    define cpp_decltype_auto 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_deduction_guides
#        define cpp_deduction_guides __cpp_deduction_guides
#    endif
#elif cplusplus >= 201703L
#    define cpp_deduction_guides 201703L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_deduction_guides
#        define cpp_deduction_guides __cpp_deduction_guides
#    endif
#elif cplusplus >= 201907L
#    define cpp_deduction_guides 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_delegating_constructors
#        define cpp_delegating_constructors __cpp_delegating_constructors
#    endif
#elif cplusplus >= 200604L
#    define cpp_delegating_constructors 200604L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_designated_initializers
#        define cpp_designated_initializers __cpp_designated_initializers
#    endif
#elif cplusplus >= 201707L
#    define cpp_designated_initializers 201707L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_enumerator_attributes
#        define cpp_enumerator_attributes __cpp_enumerator_attributes
#    endif
#elif cplusplus >= 201411L
#    define cpp_enumerator_attributes 201411L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_fold_expressions
#        define cpp_fold_expressions __cpp_fold_expressions
#    endif
#elif cplusplus >= 201603L
#    define cpp_fold_expressions 201603L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_generic_lambdas
#        define cpp_generic_lambdas __cpp_generic_lambdas
#    endif
#elif cplusplus >= 201304L
#    define cpp_generic_lambdas 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_generic_lambdas
#        define cpp_generic_lambdas __cpp_generic_lambdas
#    endif
#elif cplusplus >= 201707L
#    define cpp_generic_lambdas 201707L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_guaranteed_copy_elision
#        define cpp_guaranteed_copy_elision __cpp_guaranteed_copy_elision
#    endif
#elif cplusplus >= 201606L
#    define cpp_guaranteed_copy_elision 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_hex_float
#        define cpp_hex_float __cpp_hex_float
#    endif
#elif cplusplus >= 201603L
#    define cpp_hex_float 201603L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_if_consteval
#        define cpp_if_consteval __cpp_if_consteval
#    endif
#elif cplusplus >= 202106L
#    define cpp_if_consteval 202106L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_if_constexpr
#        define cpp_if_constexpr __cpp_if_constexpr
#    endif
#elif cplusplus >= 201606L
#    define cpp_if_constexpr 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_impl_coroutine
#        define cpp_impl_coroutine __cpp_impl_coroutine
#    endif
#elif cplusplus >= 201902L
#    define cpp_impl_coroutine 201902L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_impl_destroying_delete
#        define cpp_impl_destroying_delete __cpp_impl_destroying_delete
#    endif
#elif cplusplus >= 201806L
#    define cpp_impl_destroying_delete 201806L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_impl_three_way_comparison
#        define cpp_impl_three_way_comparison __cpp_impl_three_way_comparison
#    endif
#elif cplusplus >= 201907L
#    define cpp_impl_three_way_comparison 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_inheriting_constructors
#        define cpp_inheriting_constructors __cpp_inheriting_constructors
#    endif
#elif cplusplus >= 200802L
#    define cpp_inheriting_constructors 200802L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_inheriting_constructors
#        define cpp_inheriting_constructors __cpp_inheriting_constructors
#    endif
#elif cplusplus >= 201511L
#    define cpp_inheriting_constructors 201511L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_init_captures
#        define cpp_init_captures __cpp_init_captures
#    endif
#elif cplusplus >= 201304L
#    define cpp_init_captures 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_init_captures
#        define cpp_init_captures __cpp_init_captures
#    endif
#elif cplusplus >= 201803L
#    define cpp_init_captures 201803L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_initializer_lists
#        define cpp_initializer_lists __cpp_initializer_lists
#    endif
#elif cplusplus >= 200806L
#    define cpp_initializer_lists 200806L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_inline_variables
#        define cpp_inline_variables __cpp_inline_variables
#    endif
#elif cplusplus >= 201606L
#    define cpp_inline_variables 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_lambdas
#        define cpp_lambdas __cpp_lambdas
#    endif
#elif cplusplus >= 200907L
#    define cpp_lambdas 200907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_modules
#        define cpp_modules __cpp_modules
#    endif
#elif cplusplus >= 201907L
#    define cpp_modules 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_namespace_attributes
#        define cpp_namespace_attributes __cpp_namespace_attributes
#    endif
#elif cplusplus >= 201411L
#    define cpp_namespace_attributes 201411L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_noexcept_function_type
#        define cpp_noexcept_function_type __cpp_noexcept_function_type
#    endif
#elif cplusplus >= 201510L
#    define cpp_noexcept_function_type 201510L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_nontype_template_args
#        define cpp_nontype_template_args __cpp_nontype_template_args
#    endif
#elif cplusplus >= 201911L
#    define cpp_nontype_template_args 201911L
#elif cplusplus >= 201411L
#    define cpp_nontype_template_args 201411L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_nontype_template_parameter_auto
#        define cpp_nontype_template_parameter_auto \
            __cpp_nontype_template_parameter_auto
#    endif
#elif cplusplus >= 201606L
#    define cpp_nontype_template_parameter_auto 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_nsdmi
#        define cpp_nsdmi __cpp_nsdmi
#    endif
#elif cplusplus >= 200809L
#    define cpp_nsdmi 200809L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_range_based_for
#        define cpp_range_based_for __cpp_range_based_for
#    endif
#elif cplusplus >= 200907L
#    define cpp_range_based_for 200907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_range_based_for
#        define cpp_range_based_for __cpp_range_based_for
#    endif
#elif cplusplus >= 201603L
#    define cpp_range_based_for 201603L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_raw_strings
#        define cpp_raw_strings __cpp_raw_strings
#    endif
#elif cplusplus >= 200710L
#    define cpp_raw_strings 200710L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_ref_qualifiers
#        define cpp_ref_qualifiers __cpp_ref_qualifiers
#    endif
#elif cplusplus >= 200710L
#    define cpp_ref_qualifiers 200710L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_return_type_deduction
#        define cpp_return_type_deduction __cpp_return_type_deduction
#    endif
#elif cplusplus >= 201304L
#    define cpp_return_type_deduction 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_rvalue_references
#        define cpp_rvalue_references __cpp_rvalue_references
#    endif
#elif cplusplus >= 200610L
#    define cpp_rvalue_references 200610L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_size_t_suffix
#        define cpp_size_t_suffix __cpp_size_t_suffix
#    endif
#elif cplusplus >= 202011L
#    define cpp_size_t_suffix 202011L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_sized_deallocation
#        define cpp_sized_deallocation __cpp_sized_deallocation
#    endif
#elif cplusplus >= 201309L
#    define cpp_sized_deallocation 201309L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_static_assert
#        define cpp_static_assert __cpp_static_assert
#    endif
#elif cplusplus >= 201411L
#    define cpp_static_assert 201411L
#elif cplusplus >= 200410L
#    define cpp_static_assert 200410L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_structured_bindings
#        define cpp_structured_bindings __cpp_structured_bindings
#    endif
#elif cplusplus >= 201606L
#    define cpp_structured_bindings 201606L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_template_template_args
#        define cpp_template_template_args __cpp_template_template_args
#    endif
#elif cplusplus >= 201611L
#    define cpp_template_template_args 201611L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_threadsafe_static_init
#        define cpp_threadsafe_static_init __cpp_threadsafe_static_init
#    endif
#elif cplusplus >= 200806L
#    define cpp_threadsafe_static_init 200806L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_unicode_characters
#        define cpp_unicode_characters __cpp_unicode_characters
#    endif
#elif cplusplus >= 200704L
#    define cpp_unicode_characters 200704L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_unicode_literals
#        define cpp_unicode_literals __cpp_unicode_literals
#    endif
#elif cplusplus >= 200710L
#    define cpp_unicode_literals 200710L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_user_defined_literals
#        define cpp_user_defined_literals __cpp_user_defined_literals
#    endif
#elif cplusplus >= 200809L
#    define cpp_user_defined_literals 200809L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_using_enum
#        define cpp_using_enum __cpp_using_enum
#    endif
#elif cplusplus >= 201907L
#    define cpp_using_enum 201907L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_variable_templates
#        define cpp_variable_templates __cpp_variable_templates
#    endif
#elif cplusplus >= 201304L
#    define cpp_variable_templates 201304L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_variadic_templates
#        define cpp_variadic_templates __cpp_variadic_templates
#    endif
#elif cplusplus >= 200704L
#    define cpp_variadic_templates 200704L
#endif

#ifdef cpp_feature_testing
#    ifdef __cpp_variadic_using
#        define cpp_variadic_using __cpp_variadic_using
#    endif
#elif cplusplus >= 201611L
#    define cpp_variadic_using 201611L
#endif

/*
 * C++ attributes
 */
#if defined(__has_cpp_attribute) && __has_cpp_attribute(carries_dependency)
#    define has_cpp_attribute_carries_dependency \
        __has_cpp_attribute(carries_dependency)
#elif __has_include(<version>) && cplusplus >= 200809L
#    define has_cpp_attribute_carries_dependency 200809L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(deprecated)
#    define has_cpp_attribute_deprecated __has_cpp_attribute(deprecated)
#elif __has_include(<version>) && cplusplus >= 201309L
#    define has_cpp_attribute_deprecated 201309L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(fallthrough)
#    define has_cpp_attribute_fallthrough __has_cpp_attribute(fallthrough)
#elif __has_include(<version>) && cplusplus >= 201603L
#    define has_cpp_attribute_fallthrough 201603L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(likely)
#    define has_cpp_attribute_likely __has_cpp_attribute(likely)
#elif __has_include(<version>) && cplusplus >= 201803L
#    define has_cpp_attribute_likely 201803L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(maybe_unused)
#    define has_cpp_attribute_maybe_unused __has_cpp_attribute(maybe_unused)
#elif __has_include(<version>) && cplusplus >= 201603L
#    define has_cpp_attribute_maybe_unused 201603L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#    define has_cpp_attribute_no_unique_address \
        __has_cpp_attribute(no_unique_address)
#elif __has_include(<version>) && cplusplus >= 201803L
#    define has_cpp_attribute_no_unique_address 201803L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(nodiscard)
#    define has_cpp_attribute_nodiscard __has_cpp_attribute(nodiscard)
#elif __has_include(<version>) && cplusplus >= 201603L
#    define has_cpp_attribute_nodiscard 201603L
#elif __has_include(<version>) && cplusplus >= 201907L
#    define has_cpp_attribute_nodiscard 201907L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(noreturn)
#    define has_cpp_attribute_noreturn __has_cpp_attribute(noreturn)
#elif __has_include(<version>) && cplusplus >= 200809L
#    define has_cpp_attribute_noreturn 200809L
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(unlikely)
#    define has_cpp_attribute_unlikely __has_cpp_attribute(unlikely)
#elif __has_include(<version>) && cplusplus >= 201803L
#    define has_cpp_attribute_unlikely 201803L
#endif

/*
 * Macros to use C++ attributes even if there are not formally available yet
 */
#if defined(warn_nodiscard)
#    undef warn_nodiscard
#endif
#ifdef HEDLEY_WARN_UNUSED_RESULT
#    define warn_nodiscard HEDLEY_WARN_UNUSED_RESULT
#endif

#if defined(warn_noreturn)
#    undef warn_noreturn
#endif
#ifdef HEDLEY_NO_RETURN
#    define warn_noreturn HEDLEY_NO_RETURN
#endif

#if defined(do_fallthrough)
#    undef do_fallthrough
#endif
#ifdef HEDLEY_FALL_THROUGH
#    define do_fallthrough HEDLEY_FALL_THROUGH
#endif

#if defined(do_likely)
#    undef do_likely
#endif
#ifdef HEDLEY_LIKELY
#    define do_likely(expr) HEDLEY_LIKELY(expr)
#endif

#if defined(do_unlikely)
#    undef do_unlikely
#endif
#ifdef HEDLEY_UNLIKELY
#    define do_unlikely(expr) HEDLEY_UNLIKELY(expr)
#endif

/*
 * Compiler attributes
 */
#if defined(cplusplus) && defined(__has_attribute)
#    define has_attribute(x) __has_attribute(x) || has_cpp_attribute(x)
#else
#    define has_attribute(x) 0
#endif

/*
 * Macros to use compiler attributes if they are available
 */
#if defined(warn_return_non_null)
#    undef warn_return_non_null
#endif
#ifdef HEDLEY_RETURNS_NON_NULL
#    define warn_return_non_null HEDLEY_RETURNS_NON_NULL
#endif

#if defined(warn_non_null)
#    undef warn_non_null
#endif
#ifdef HEDLEY_RETURNS_NON_NULL
#    define warn_non_null(...) HEDLEY_NON_NULL(__VA_ARGS__)
#endif

#if defined(warn_bit_flags)
#    undef warn_bit_flags
#endif
#ifdef HEDLEY_RETURNS_NON_NULL
#    define warn_bit_flags HEDLEY_FLAGS
#endif

#if defined(do_predict)
#    undef do_predict
#endif
#ifdef HEDLEY_PREDICT
#    define do_predict(expr, expected, probability) \
        HEDLEY_PREDICT(expr, expected, probability)
#endif

/*
 * Library features
 * If <version> is available, we count on it to provide these macros
 * If <version> is not available, we infer the values from the current C++
 * version
 */
#if __has_include(<version>)
#    ifdef __cpp_lib_chrono_udls
#        define cpp_lib_chrono_udls __cpp_lib_chrono_udls
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_chrono_udls 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_complex_udls
#        define cpp_lib_complex_udls __cpp_lib_complex_udls
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_complex_udls 201309L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_exchange_function
#        define cpp_lib_exchange_function __cpp_lib_exchange_function
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_exchange_function 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_generic_associative_lookup
#        define cpp_lib_generic_associative_lookup \
            __cpp_lib_generic_associative_lookup
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_generic_associative_lookup 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_integer_sequence
#        define cpp_lib_integer_sequence __cpp_lib_integer_sequence
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_integer_sequence 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_integral_constant_callable
#        define cpp_lib_integral_constant_callable \
            __cpp_lib_integral_constant_callable
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_integral_constant_callable 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_final
#        define cpp_lib_is_final __cpp_lib_is_final
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_final 201402L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_null_pointer
#        define cpp_lib_is_null_pointer __cpp_lib_is_null_pointer
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_null_pointer 201309L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_make_reverse_iterator
#        define cpp_lib_make_reverse_iterator __cpp_lib_make_reverse_iterator
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_make_reverse_iterator 201402L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_make_unique
#        define cpp_lib_make_unique __cpp_lib_make_unique
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_make_unique 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_null_iterators
#        define cpp_lib_null_iterators __cpp_lib_null_iterators
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_null_iterators 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_quoted_string_io
#        define cpp_lib_quoted_string_io __cpp_lib_quoted_string_io
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_quoted_string_io 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_result_of_sfinae
#        define cpp_lib_result_of_sfinae __cpp_lib_result_of_sfinae
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_result_of_sfinae 201210L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_robust_nonmodifying_seq_ops
#        define cpp_lib_robust_nonmodifying_seq_ops \
            __cpp_lib_robust_nonmodifying_seq_ops
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_robust_nonmodifying_seq_ops 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_shared_timed_mutex
#        define cpp_lib_shared_timed_mutex __cpp_lib_shared_timed_mutex
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_shared_timed_mutex 201402L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_string_udls
#        define cpp_lib_string_udls __cpp_lib_string_udls
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_string_udls 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_transformation_trait_aliases
#        define cpp_lib_transformation_trait_aliases \
            __cpp_lib_transformation_trait_aliases
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_transformation_trait_aliases 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_transparent_operators
#        define cpp_lib_transparent_operators __cpp_lib_transparent_operators
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_transparent_operators 201210L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_tuple_element_t
#        define cpp_lib_tuple_element_t __cpp_lib_tuple_element_t
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_tuple_element_t 201402L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_tuples_by_type
#        define cpp_lib_tuples_by_type __cpp_lib_tuples_by_type
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_tuples_by_type 201304L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_addressof_constexpr
#        define cpp_lib_addressof_constexpr __cpp_lib_addressof_constexpr
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_addressof_constexpr 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_allocator_traits_is_always_equal
#        define cpp_lib_allocator_traits_is_always_equal \
            __cpp_lib_allocator_traits_is_always_equal
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_allocator_traits_is_always_equal 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_any
#        define cpp_lib_any __cpp_lib_any
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_any 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_apply
#        define cpp_lib_apply __cpp_lib_apply
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_apply 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_array_constexpr
#        define cpp_lib_array_constexpr __cpp_lib_array_constexpr
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_array_constexpr 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_as_const
#        define cpp_lib_as_const __cpp_lib_as_const
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_as_const 201510L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_is_always_lock_free
#        define cpp_lib_atomic_is_always_lock_free \
            __cpp_lib_atomic_is_always_lock_free
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_is_always_lock_free 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_bool_constant
#        define cpp_lib_bool_constant __cpp_lib_bool_constant
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_bool_constant 201505L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_boyer_moore_searcher
#        define cpp_lib_boyer_moore_searcher __cpp_lib_boyer_moore_searcher
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_boyer_moore_searcher 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_byte
#        define cpp_lib_byte __cpp_lib_byte
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_byte 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_chrono
#        define cpp_lib_chrono __cpp_lib_chrono
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_chrono 201611L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_clamp
#        define cpp_lib_clamp __cpp_lib_clamp
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_clamp 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_enable_shared_from_this
#        define cpp_lib_enable_shared_from_this \
            __cpp_lib_enable_shared_from_this
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_enable_shared_from_this 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_execution
#        define cpp_lib_execution __cpp_lib_execution
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_execution 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_filesystem
#        define cpp_lib_filesystem __cpp_lib_filesystem
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_filesystem 201703L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_gcd_lcm
#        define cpp_lib_gcd_lcm __cpp_lib_gcd_lcm
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_gcd_lcm 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_hardware_interference_size
#        define cpp_lib_hardware_interference_size \
            __cpp_lib_hardware_interference_size
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_hardware_interference_size 201703L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_has_unique_object_representations
#        define cpp_lib_has_unique_object_representations \
            __cpp_lib_has_unique_object_representations
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_has_unique_object_representations 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_hypot
#        define cpp_lib_hypot __cpp_lib_hypot
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_hypot 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_incomplete_container_elements
#        define cpp_lib_incomplete_container_elements \
            __cpp_lib_incomplete_container_elements
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_incomplete_container_elements 201505L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_invoke
#        define cpp_lib_invoke __cpp_lib_invoke
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_invoke 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_aggregate
#        define cpp_lib_is_aggregate __cpp_lib_is_aggregate
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_aggregate 201703L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_invocable
#        define cpp_lib_is_invocable __cpp_lib_is_invocable
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_invocable 201703L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_swappable
#        define cpp_lib_is_swappable __cpp_lib_is_swappable
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_swappable 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_launder
#        define cpp_lib_launder __cpp_lib_launder
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_launder 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_logical_traits
#        define cpp_lib_logical_traits __cpp_lib_logical_traits
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_logical_traits 201510L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_make_from_tuple
#        define cpp_lib_make_from_tuple __cpp_lib_make_from_tuple
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_make_from_tuple 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_map_try_emplace
#        define cpp_lib_map_try_emplace __cpp_lib_map_try_emplace
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_map_try_emplace 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_math_special_functions
#        define cpp_lib_math_special_functions __cpp_lib_math_special_functions
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_math_special_functions 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_memory_resource
#        define cpp_lib_memory_resource __cpp_lib_memory_resource
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_memory_resource 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_node_extract
#        define cpp_lib_node_extract __cpp_lib_node_extract
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_node_extract 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_nonmember_container_access
#        define cpp_lib_nonmember_container_access \
            __cpp_lib_nonmember_container_access
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_nonmember_container_access 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_not_fn
#        define cpp_lib_not_fn __cpp_lib_not_fn
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_not_fn 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_optional
#        define cpp_lib_optional __cpp_lib_optional
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_optional 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_parallel_algorithm
#        define cpp_lib_parallel_algorithm __cpp_lib_parallel_algorithm
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_parallel_algorithm 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_raw_memory_algorithms
#        define cpp_lib_raw_memory_algorithms __cpp_lib_raw_memory_algorithms
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_raw_memory_algorithms 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_sample
#        define cpp_lib_sample __cpp_lib_sample
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_sample 201603L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_scoped_lock
#        define cpp_lib_scoped_lock __cpp_lib_scoped_lock
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_scoped_lock 201703L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_shared_mutex
#        define cpp_lib_shared_mutex __cpp_lib_shared_mutex
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_shared_mutex 201505L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_shared_ptr_arrays
#        define cpp_lib_shared_ptr_arrays __cpp_lib_shared_ptr_arrays
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_shared_ptr_arrays 201611L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_shared_ptr_weak_type
#        define cpp_lib_shared_ptr_weak_type __cpp_lib_shared_ptr_weak_type
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_shared_ptr_weak_type 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_string_view
#        define cpp_lib_string_view __cpp_lib_string_view
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_string_view 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_to_chars
#        define cpp_lib_to_chars __cpp_lib_to_chars
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_to_chars 201611L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_type_trait_variable_templates
#        define cpp_lib_type_trait_variable_templates \
            __cpp_lib_type_trait_variable_templates
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_type_trait_variable_templates 201510L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_uncaught_exceptions
#        define cpp_lib_uncaught_exceptions __cpp_lib_uncaught_exceptions
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_uncaught_exceptions 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_unordered_map_try_emplace
#        define cpp_lib_unordered_map_try_emplace \
            __cpp_lib_unordered_map_try_emplace
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_unordered_map_try_emplace 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_variant
#        define cpp_lib_variant __cpp_lib_variant
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_variant 201606L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_void_t
#        define cpp_lib_void_t __cpp_lib_void_t
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_void_t 201411L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_assume_aligned
#        define cpp_lib_assume_aligned __cpp_lib_assume_aligned
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_assume_aligned 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_flag_test
#        define cpp_lib_atomic_flag_test __cpp_lib_atomic_flag_test
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_flag_test 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_float
#        define cpp_lib_atomic_float __cpp_lib_atomic_float
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_float 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_lock_free_type_aliases
#        define cpp_lib_atomic_lock_free_type_aliases \
            __cpp_lib_atomic_lock_free_type_aliases
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_lock_free_type_aliases 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_ref
#        define cpp_lib_atomic_ref __cpp_lib_atomic_ref
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_ref 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_shared_ptr
#        define cpp_lib_atomic_shared_ptr __cpp_lib_atomic_shared_ptr
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_shared_ptr 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_value_initialization
#        define cpp_lib_atomic_value_initialization \
            __cpp_lib_atomic_value_initialization
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_value_initialization 201911L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_atomic_wait
#        define cpp_lib_atomic_wait __cpp_lib_atomic_wait
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_atomic_wait 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_barrier
#        define cpp_lib_barrier __cpp_lib_barrier
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_barrier 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_bind_front
#        define cpp_lib_bind_front __cpp_lib_bind_front
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_bind_front 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_bit_cast
#        define cpp_lib_bit_cast __cpp_lib_bit_cast
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_bit_cast 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_bitops
#        define cpp_lib_bitops __cpp_lib_bitops
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_bitops 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_bounded_array_traits
#        define cpp_lib_bounded_array_traits __cpp_lib_bounded_array_traits
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_bounded_array_traits 201902L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_char8_t
#        define cpp_lib_char8_t __cpp_lib_char8_t
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_char8_t 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_concepts
#        define cpp_lib_concepts __cpp_lib_concepts
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_concepts 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_algorithms
#        define cpp_lib_constexpr_algorithms __cpp_lib_constexpr_algorithms
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_algorithms 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_complex
#        define cpp_lib_constexpr_complex __cpp_lib_constexpr_complex
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_complex 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_dynamic_alloc
#        define cpp_lib_constexpr_dynamic_alloc \
            __cpp_lib_constexpr_dynamic_alloc
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_dynamic_alloc 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_functional
#        define cpp_lib_constexpr_functional __cpp_lib_constexpr_functional
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_functional 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_iterator
#        define cpp_lib_constexpr_iterator __cpp_lib_constexpr_iterator
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_iterator 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_memory
#        define cpp_lib_constexpr_memory __cpp_lib_constexpr_memory
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_memory 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_numeric
#        define cpp_lib_constexpr_numeric __cpp_lib_constexpr_numeric
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_numeric 201911L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_string
#        define cpp_lib_constexpr_string __cpp_lib_constexpr_string
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_string 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_string_view
#        define cpp_lib_constexpr_string_view __cpp_lib_constexpr_string_view
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_string_view 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_tuple
#        define cpp_lib_constexpr_tuple __cpp_lib_constexpr_tuple
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_tuple 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_utility
#        define cpp_lib_constexpr_utility __cpp_lib_constexpr_utility
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_utility 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_constexpr_vector
#        define cpp_lib_constexpr_vector __cpp_lib_constexpr_vector
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_constexpr_vector 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_coroutine
#        define cpp_lib_coroutine __cpp_lib_coroutine
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_coroutine 201902L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_destroying_delete
#        define cpp_lib_destroying_delete __cpp_lib_destroying_delete
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_destroying_delete 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_endian
#        define cpp_lib_endian __cpp_lib_endian
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_endian 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_erase_if
#        define cpp_lib_erase_if __cpp_lib_erase_if
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_erase_if 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_generic_unordered_lookup
#        define cpp_lib_generic_unordered_lookup \
            __cpp_lib_generic_unordered_lookup
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_generic_unordered_lookup 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_int_pow2
#        define cpp_lib_int_pow2 __cpp_lib_int_pow2
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_int_pow2 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_integer_comparison_functions
#        define cpp_lib_integer_comparison_functions \
            __cpp_lib_integer_comparison_functions
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_integer_comparison_functions 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_interpolate
#        define cpp_lib_interpolate __cpp_lib_interpolate
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_interpolate 201902L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_constant_evaluated
#        define cpp_lib_is_constant_evaluated __cpp_lib_is_constant_evaluated
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_constant_evaluated 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_layout_compatible
#        define cpp_lib_is_layout_compatible __cpp_lib_is_layout_compatible
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_layout_compatible 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_nothrow_convertible
#        define cpp_lib_is_nothrow_convertible __cpp_lib_is_nothrow_convertible
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_nothrow_convertible 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_pointer_interconvertible
#        define cpp_lib_is_pointer_interconvertible \
            __cpp_lib_is_pointer_interconvertible
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_pointer_interconvertible 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_jthread
#        define cpp_lib_jthread __cpp_lib_jthread
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_jthread 201911L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_latch
#        define cpp_lib_latch __cpp_lib_latch
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_latch 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_list_remove_return_type
#        define cpp_lib_list_remove_return_type \
            __cpp_lib_list_remove_return_type
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_list_remove_return_type 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_math_constants
#        define cpp_lib_math_constants __cpp_lib_math_constants
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_math_constants 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_polymorphic_allocator
#        define cpp_lib_polymorphic_allocator __cpp_lib_polymorphic_allocator
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_polymorphic_allocator 201902L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_ranges
#        define cpp_lib_ranges __cpp_lib_ranges
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_ranges 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_remove_cvref
#        define cpp_lib_remove_cvref __cpp_lib_remove_cvref
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_remove_cvref 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_semaphore
#        define cpp_lib_semaphore __cpp_lib_semaphore
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_semaphore 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_shift
#        define cpp_lib_shift __cpp_lib_shift
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_shift 201806L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_smart_ptr_for_overwrite
#        define cpp_lib_smart_ptr_for_overwrite \
            __cpp_lib_smart_ptr_for_overwrite
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_smart_ptr_for_overwrite 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_source_location
#        define cpp_lib_source_location __cpp_lib_source_location
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_source_location 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_span
#        define cpp_lib_span __cpp_lib_span
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_span 202002L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_ssize
#        define cpp_lib_ssize __cpp_lib_ssize
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_ssize 201902L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_starts_ends_with
#        define cpp_lib_starts_ends_with __cpp_lib_starts_ends_with
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_starts_ends_with 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_syncbuf
#        define cpp_lib_syncbuf __cpp_lib_syncbuf
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_syncbuf 201803L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_three_way_comparison
#        define cpp_lib_three_way_comparison __cpp_lib_three_way_comparison
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_three_way_comparison 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_to_address
#        define cpp_lib_to_address __cpp_lib_to_address
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_to_address 201711L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_to_array
#        define cpp_lib_to_array __cpp_lib_to_array
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_to_array 201907L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_unwrap_ref
#        define cpp_lib_unwrap_ref __cpp_lib_unwrap_ref
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_unwrap_ref 201811L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_is_scoped_enum
#        define cpp_lib_is_scoped_enum __cpp_lib_is_scoped_enum
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_is_scoped_enum 202011L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_stacktrace
#        define cpp_lib_stacktrace __cpp_lib_stacktrace
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_stacktrace 202011L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_stdatomic_h
#        define cpp_lib_stdatomic_h __cpp_lib_stdatomic_h
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_stdatomic_h 202011L
#endif

#if __has_include(<version>)
#    ifdef __cpp_lib_string_contains
#        define cpp_lib_string_contains __cpp_lib_string_contains
#    endif
#elif cplusplus >= 201304L
#    define cpp_lib_string_contains 202011L
#endif

#endif // SMALL_DETAIL_TRAITS_CPP_VERSION_HPP
