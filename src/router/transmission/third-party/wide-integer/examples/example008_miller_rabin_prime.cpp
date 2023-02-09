///////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2018 - 2022.                 //
//  Distributed under the Boost Software License,                //
//  Version 1.0. (See accompanying file LICENSE_1_0.txt          //
//  or copy at http://www.boost.org/LICENSE_1_0.txt)             //
///////////////////////////////////////////////////////////////////

// This Miller-Rabin primality test is loosely based on
// an adaptation of some code from Boost.Multiprecision.
// The Boost.Multiprecision code can be found here:
// https://www.boost.org/doc/libs/1_73_0/libs/multiprecision/doc/html/boost_multiprecision/tut/primetest.html

#include <random>

#include <examples/example_uintwide_t.h>
#include <math/wide_integer/uintwide_t.h>

#if defined(WIDE_INTEGER_NAMESPACE)
auto WIDE_INTEGER_NAMESPACE::math::wide_integer::example008_miller_rabin_prime() -> bool
#else
auto math::wide_integer::example008_miller_rabin_prime() -> bool
#endif
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  using wide_integer_type   = WIDE_INTEGER_NAMESPACE::math::wide_integer::uintwide_t<static_cast<WIDE_INTEGER_NAMESPACE::math::wide_integer::size_t>(UINT32_C(512))>;
  using distribution_type   = WIDE_INTEGER_NAMESPACE::math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #else
  using wide_integer_type   = math::wide_integer::uintwide_t<static_cast<math::wide_integer::size_t>(UINT32_C(512))>;
  using distribution_type   = math::wide_integer::uniform_int_distribution<wide_integer_type::my_width2, typename wide_integer_type::limb_type>;
  #endif

  using random_engine1_type = std::linear_congruential_engine<std::uint32_t, UINT32_C(48271), UINT32_C(0), UINT32_C(2147483647)>;
  using random_engine2_type = std::mt19937;

  // Use a fixed seed in order to obtain deterministic
  // and reproducible result for this test.

  random_engine1_type generator1(static_cast<typename random_engine1_type::result_type>(std::clock()));
  random_engine2_type generator2(static_cast<typename random_engine2_type::result_type>(std::clock()));

  distribution_type distribution1;
  distribution_type distribution2;

  wide_integer_type p0;
  wide_integer_type p1;

  for(;;)
  {
    p0 = distribution1(generator1);

    const bool miller_rabin_result = miller_rabin(p0,
                                                  25U,
                                                  distribution2,
                                                  generator2);

    if(miller_rabin_result)
    {
      break;
    }
  }

  for(;;)
  {
    p1 = distribution1(generator1);

    const bool miller_rabin_result = miller_rabin(p1,
                                                  25U,
                                                  distribution2,
                                                  generator2);

    if(miller_rabin_result)
    {
      break;
    }
  }

  const auto gd = gcd(p0, p1);

  const auto result_is_ok = (   (p0 != 0U)
                             && (p1 != 0U)
                             && (p0 != p1)
                             && (gd == 1U));

  return result_is_ok;
}

// Enable this if you would like to activate this main() as a standalone example.
#if defined(WIDE_INTEGER_STANDALONE_EXAMPLE008_MILLER_RABIN_PRIME)

#include <iomanip>
#include <iostream>

auto main() -> int
{
  #if defined(WIDE_INTEGER_NAMESPACE)
  const auto result_is_ok = WIDE_INTEGER_NAMESPACE::math::wide_integer::example008_miller_rabin_prime();
  #else
  const auto result_is_ok = math::wide_integer::example008_miller_rabin_prime();
  #endif

  std::cout << "result_is_ok: " << std::boolalpha << result_is_ok << std::endl;

  return (result_is_ok ? 0 : -1);
}

#endif
