///////////////////////////////////////////////////////////////
//  Copyright 2022 - 2025 Christopher Kormanyos.
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include <boost/version.hpp>

#if !defined(BOOST_VERSION)
#error BOOST_VERSION is not defined. Ensure that <boost/version.hpp> is properly included.
#endif

#if ((BOOST_VERSION >= 107900) && !defined(BOOST_MP_STANDALONE))
#define BOOST_MP_STANDALONE
#endif

#if ((BOOST_VERSION >= 108000) && !defined(BOOST_NO_EXCEPTIONS))
#define BOOST_NO_EXCEPTIONS
#endif

#if (BOOST_VERSION < 108000)
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#endif

#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 12))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
#endif

#if (BOOST_VERSION < 108000)
#if ((defined(__clang__) && (__clang_major__ > 9)) && !defined(__APPLE__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#endif

#include <test/test_arithmetic.hpp>
#include <test/test_uintwide_t.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/uintwide_t_backend.hpp>

// cd /mnt/c/Users/ckorm/Documents/Ks/PC_Software/NumericalPrograms/ExtendedNumberTypes/wide_integer
// g++ -march=native -mtune=native -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -std=c++20 -I. -I/mnt/c/boost/boost_1_88_0 test/test_uintwide_t_boost_backend_via_test_arithmetic.cpp -o test_uintwide_t_boost_backend_via_test_arithmetic.exe
// ./test_uintwide_t_boost_backend_via_test_arithmetic.exe

auto main() -> int
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  using local_size_type = WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t;
  #else
  using local_size_type = ::math::wide_integer::size_t;
  #endif

  using local_big_uint_backend_type =
    ::boost::multiprecision::uintwide_t_backend<local_size_type { UINT32_C(1024) }, std::uint32_t, std::allocator<void>>;

  using local_big_uint_type = ::boost::multiprecision::number<local_big_uint_backend_type, boost::multiprecision::et_off>;

  test<local_big_uint_type>();

  return boost::report_errors();
}

#if (BOOST_VERSION < 108000)
#if ((defined(__clang__) && (__clang_major__ > 9)) && !defined(__APPLE__))
#pragma GCC diagnostic pop
#endif
#endif

#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 12))
#pragma GCC diagnostic pop
#endif

#if (BOOST_VERSION < 108000)
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif
#endif
