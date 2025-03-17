/* compiler.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#define INTERNAL_HIDDEN __attribute__((visibility("hidden")))
#define ASSUME_ALIGNED __attribute__((assume_aligned(8)))
#define CONST __attribute__((const))

/* This isn't standard in C as [[nodiscard]] until C23 */
#define NO_DISCARD __attribute__((warn_unused_result))

#if UNIT_TESTING
#define EXTERNAL_API __attribute__((visibility("default")))
#endif

#if PERF_TEST_BUILD
#define INLINE
#define FLATTEN
#else
#if __clang__
#define INLINE __attribute__((always_inline))
#else
#define INLINE __attribute__((inline))
#endif
#define FLATTEN __attribute__((flatten))
#endif

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#if __clang__
#if __has_feature(address_sanitizer)
#define __SANITIZE_ADDRESS__ 1
#endif
#if __has_feature(memory_sanitizer)
#define __SANITIZE_MEMORY__ 1
#endif
#endif

#if DONT_USE_NEON == 0 && __ARM_NEON
#include <arm_neon.h>
#define USE_NEON 1
#else
#define USE_NEON 0
#endif

#if defined(__SANITIZE_ADDRESS__)
static_assert(ENABLE_ASAN == 1, "ENABLE_ASAN should be 1 to enable asan instead");
#endif

#if defined(__SANITIZE_MEMORY__)
static_assert(ENABLE_MSAN == 1, "ENABLE_MSAN should be 1 to enable msan instead");
#endif
