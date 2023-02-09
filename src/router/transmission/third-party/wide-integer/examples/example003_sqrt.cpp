///////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2018 - 2022.                 //
//  Distributed under the Boost Software License,                //
//  Version 1.0. (See accompanying file LICENSE_1_0.txt          //
//  or copy at http://www.boost.org/LICENSE_1_0.txt)             //
///////////////////////////////////////////////////////////////////

#include <examples/example_uintwide_t.h>
#include <math/wide_integer/uintwide_t.h>

#if defined(WIDE_INTEGER_NAMESPACE)
auto WIDE_INTEGER_NAMESPACE::math::wide_integer::example003_sqrt() -> bool
#else
auto math::wide_integer::example003_sqrt() -> bool
#endif
{
  using math::wide_integer::uint256_t;

  WIDE_INTEGER_CONSTEXPR uint256_t a("0xF4DF741DE58BCB2F37F18372026EF9CBCFC456CB80AF54D53BDEED78410065DE");

  WIDE_INTEGER_CONSTEXPR uint256_t s = sqrt(a);

  WIDE_INTEGER_CONSTEXPR bool result_is_ok = (s == "0xFA5FE7853F1D4AD92BDF244179CA178B");

  #if defined(WIDE_INTEGER_CONSTEXPR_IS_COMPILE_TIME_CONST) && (WIDE_INTEGER_CONSTEXPR_IS_COMPILE_TIME_CONST != 0)
  static_assert(result_is_ok, "Error: example003_sqrt not OK!");
  #endif

  return result_is_ok;
}

// Enable this if you would like to activate this main() as a standalone example.
#if defined(WIDE_INTEGER_STANDALONE_EXAMPLE003_SQRT)

#if !defined(WIDE_INTEGER_DISABLE_IOSTREAM)
#include <iomanip>
#include <iostream>
#endif

auto main() -> int
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  const auto result_is_ok = WIDE_INTEGER_NAMESPACE::math::wide_integer::example003_sqrt();
  #else
  const auto result_is_ok = math::wide_integer::example003_sqrt();
  #endif

  #if !defined(WIDE_INTEGER_DISABLE_IOSTREAM)
  std::cout << "result_is_ok: " << std::boolalpha << result_is_ok << std::endl;
  #endif

  return (result_is_ok ? 0 : -1);
}

#endif
