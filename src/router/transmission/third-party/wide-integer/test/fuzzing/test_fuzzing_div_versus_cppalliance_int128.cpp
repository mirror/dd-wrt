///////////////////////////////////////////////////////////////////////////////
//  Copyright Matt Borland 2024 - 2025.
//  Copyright Christopher Kormanyos 2024 - 2025.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// cd /mnt/c/Users/ckorm/Documents/Ks/PC_Software/NumericalPrograms/ExtendedNumberTypes/wide_integer
// clang++ -std=c++20 -g -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -fsanitize=fuzzer -I. -I/mnt/c/ChrisGitRepos/cppalliance/int128/include -I../NumericalPrograms/ExtendedNumberTypes/wide_integer test/fuzzing/test_fuzzing_div_versus_cppalliance_int128.cpp -o test_fuzzing_div_versus_cppalliance_int128
// ./test_fuzzing_div_versus_cppalliance_int128 -max_total_time=1200 -max_len=32

#include <math/wide_integer/uintwide_t.h>

#include <boost/int128.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

extern "C"
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

namespace fuzzing
{
  template <typename CntrlUintType,
            typename LocalUintType>
  auto eval_op(const CntrlUintType& a_cntrl,
               const CntrlUintType& b_cntrl,
               const LocalUintType& a_local,
               const LocalUintType& b_local) -> bool;
}

template <typename CntrlUintType,
          typename LocalUintType>
auto fuzzing::eval_op(const CntrlUintType& a_cntrl,
                      const CntrlUintType& b_cntrl,
                      const LocalUintType& a_local,
                      const LocalUintType& b_local) -> bool
{
  using cntrl_uint_type = CntrlUintType;
  using local_uint_type = LocalUintType;

  static_assert
  (
       (std::numeric_limits<cntrl_uint_type>::digits == std::numeric_limits<local_uint_type>::digits)
    && (std::numeric_limits<cntrl_uint_type>::digits == int { INT32_C(128) }),
    "Error: the control and local types must both have 128 binary digits"
  );

  const local_uint_type result_local { local_uint_type(a_local) /= b_local };
  const cntrl_uint_type result_cntrl { cntrl_uint_type(a_cntrl) /= b_cntrl };

  const std::uint64_t result_local_lo = static_cast<std::uint64_t>(result_local);
  const std::uint64_t result_local_hi = static_cast<std::uint64_t>(result_local >> unsigned { UINT8_C(64) });

  const std::uint64_t result_cntrl_lo = static_cast<std::uint64_t>(result_cntrl);
  const std::uint64_t result_cntrl_hi = static_cast<std::uint64_t>(result_cntrl >> unsigned { UINT8_C(64) });

  // Verify that both the local (test) type as well as the
  // control type obtain the same numerical result.

  const bool
    result_is_ok
    {
         (result_local_lo == result_cntrl_lo)
      && (result_local_hi == result_cntrl_hi)
    };

  if(!result_is_ok)
  {
    std::cout << "Error: lhs: " << a_local << ", rhs: " << b_local << ", result obtained: " << result_local << std::endl;
  }

  return result_is_ok;
}

// The fuzzing entry point.
extern "C"
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  constexpr std::size_t max_size { UINT8_C(32) };
  constexpr std::size_t min_size { UINT8_C(17) };

  bool result_is_ok { true };

  if(((size >= min_size) && (size <= max_size)) && (data != nullptr))
  {
    using local_data_array_type = std::array<std::uint8_t, max_size>;

    local_data_array_type tmp_data { };

    tmp_data.fill(UINT8_C(0));

    static_cast<void>(std::copy(data, data + size, tmp_data.begin()));

    const std::uint64_t a_lo64 { *reinterpret_cast<const std::uint64_t*>(tmp_data.data() + std::size_t { UINT8_C(0) }) };
    const std::uint64_t a_hi64 { *reinterpret_cast<const std::uint64_t*>(tmp_data.data() + std::size_t { UINT8_C(8) }) };
    const std::uint64_t b_lo64 { *reinterpret_cast<const std::uint64_t*>(tmp_data.data() + std::size_t { UINT8_C(16) }) };
    const std::uint64_t b_hi64 { *reinterpret_cast<const std::uint64_t*>(tmp_data.data() + std::size_t { UINT8_C(24) }) };

    // Import data into the uint values.
    using local_uint_type = ::boost::int128::uint128_t;

    #if defined(WIDE_INTEGER_NAMESPACE)
    using cntrl_uint_type = ::WIDE_INTEGER_NAMESPACE::math::wide_integer::uint128_t;
    #else
    using cntrl_uint_type = ::math::wide_integer::uint128_t;
    #endif

    cntrl_uint_type a_cntrl { a_hi64 }; a_cntrl <<= unsigned { UINT8_C(64) }; a_cntrl |= a_lo64;
    cntrl_uint_type b_cntrl { b_hi64 }; b_cntrl <<= unsigned { UINT8_C(64) }; b_cntrl |= b_lo64;

    local_uint_type a_local { a_hi64 }; a_local <<= unsigned { UINT8_C(64) }; a_local |= a_lo64;
    local_uint_type b_local { b_hi64 }; b_local <<= unsigned { UINT8_C(64) }; b_local |= b_lo64;

    if(a_local < b_local)
    {
      std::swap(a_local, b_local);
      std::swap(a_cntrl, b_cntrl);
    }

    if(b_local != 0U)
    {
      const bool result_op_is_ok { fuzzing::eval_op(a_cntrl, b_cntrl, a_local, b_local) };

      if(!result_op_is_ok)
      {
        assert(result_op_is_ok);
      }

      result_is_ok = (result_op_is_ok && result_is_ok);
    }
  }

  return (result_is_ok ? 0 : -1);
}
