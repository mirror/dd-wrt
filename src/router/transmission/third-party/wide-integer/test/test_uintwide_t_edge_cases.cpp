///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2019 - 2022.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>

#include <boost/version.hpp>

#if !defined(BOOST_VERSION)
#error BOOST_VERSION is not defined. Ensure that <boost/version.hpp> is properly included.
#endif

#if ((BOOST_VERSION >= 107900) && !defined(BOOST_MP_STANDALONE))
#define BOOST_MP_STANDALONE
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
#if (defined(__clang__) && (__clang_major__ > 9)) && !defined(__APPLE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#endif

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/uintwide_t_backend.hpp>

#include <math/wide_integer/uintwide_t.h>
#include <test/test_uintwide_t.h>

namespace local_edge_cases
{
  constexpr std::size_t local_digits2 = 16384U;
} // namespace local_edge_cases

using local_uint_backend_type =
  boost::multiprecision::uintwide_t_backend<local_edge_cases::local_digits2,
                                            std::uint32_t,
                                            std::allocator<std::uint32_t>>;

using boost_uint_backend_allocator_type = void;

using boost_uint_backend_type =
  boost::multiprecision::cpp_int_backend<local_edge_cases::local_digits2,
                                         local_edge_cases::local_digits2,
                                         boost::multiprecision::unsigned_magnitude,
                                         boost::multiprecision::unchecked,
                                         boost_uint_backend_allocator_type>;

using local_uint_type =
  boost::multiprecision::number<local_uint_backend_type,
                                boost::multiprecision::et_off>;

using boost_uint_type =
  boost::multiprecision::number<boost_uint_backend_type,
                                boost::multiprecision::et_off>;

#if defined(WIDE_INTEGER_NAMESPACE)
auto WIDE_INTEGER_NAMESPACE::math::wide_integer::test_uintwide_t_edge_cases() -> bool
#else
auto math::wide_integer::test_uintwide_t_edge_cases() -> bool
#endif
{
  const local_uint_type u_max_local = (std::numeric_limits<local_uint_type>::max)();
  const boost_uint_type u_max_boost = (std::numeric_limits<boost_uint_type>::max)();

  local_uint_type result_local;
  boost_uint_type result_boost;

  result_local = u_max_local * u_max_local;
  result_boost = u_max_boost * u_max_boost;

  const bool result01_is_ok = ((result_local == 1U) && (result_boost == 1U));

  result_local = (u_max_local - 1U) * u_max_local;
  result_boost = (u_max_boost - 1U) * u_max_boost;

  const bool result02_is_ok = ((result_local == 2U) && (result_boost == 2U));

  const std::string str_seven_and_effs =
    "0x7" + std::string(std::string::size_type((local_edge_cases::local_digits2 / 4) - 1U), 'F');

  const local_uint_type u_seven_and_effs_local(str_seven_and_effs.c_str());
  const boost_uint_type u_seven_and_effs_boost(str_seven_and_effs.c_str());

  result_local = u_seven_and_effs_local * u_seven_and_effs_local;
  result_boost = u_seven_and_effs_boost * u_seven_and_effs_boost;

  const bool result03_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  const std::string str_three_quarter_effs_and_zeros =
      "0x"
    + std::string(std::string::size_type((local_edge_cases::local_digits2 / 4) * 3U), 'F')
    + std::string(std::string::size_type((local_edge_cases::local_digits2 / 4) * 1U), '0')
    ;

  const local_uint_type u_three_quarter_effs_and_zeros_local(str_three_quarter_effs_and_zeros.c_str());
  const boost_uint_type u_three_quarter_effs_and_zeros_boost(str_three_quarter_effs_and_zeros.c_str());

  result_local = u_three_quarter_effs_and_zeros_local * u_three_quarter_effs_and_zeros_local;
  result_boost = u_three_quarter_effs_and_zeros_boost * u_three_quarter_effs_and_zeros_boost;

  const bool result04_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  const std::string str_one_quarter_effs_and_zeros =
      "0x"
    + std::string(std::string::size_type((local_edge_cases::local_digits2 / 4) * 1U), 'F')
    + std::string(std::string::size_type((local_edge_cases::local_digits2 / 4) * 3U), '0')
    ;

  const local_uint_type u_one_quarter_effs_and_zeros_local(str_one_quarter_effs_and_zeros.c_str());
  const boost_uint_type u_one_quarter_effs_and_zeros_boost(str_one_quarter_effs_and_zeros.c_str());

  result_local = u_one_quarter_effs_and_zeros_local * u_one_quarter_effs_and_zeros_local;
  result_boost = u_one_quarter_effs_and_zeros_boost * u_one_quarter_effs_and_zeros_boost;

  const bool result05_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  const local_uint_type one_limb_effs_prior_to_half_and_zeros_local(local_uint_type(UINT32_C(0xFFFFFFFF)) << ((std::numeric_limits<local_uint_type>::digits / 2) - 32));
  const boost_uint_type one_limb_effs_prior_to_half_and_zeros_boost(boost_uint_type(UINT32_C(0xFFFFFFFF)) << ((std::numeric_limits<boost_uint_type>::digits / 2) - 32));

  result_local = one_limb_effs_prior_to_half_and_zeros_local * one_limb_effs_prior_to_half_and_zeros_local;
  result_boost = one_limb_effs_prior_to_half_and_zeros_boost * one_limb_effs_prior_to_half_and_zeros_boost;

  const bool result06_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  const local_uint_type u_mid_local = u_three_quarter_effs_and_zeros_local / typename local_uint_type::backend_type::representation_type::limb_type(2U);
  const boost_uint_type u_mid_boost = u_three_quarter_effs_and_zeros_boost / typename std::iterator_traits<boost_uint_type::backend_type::limb_pointer>::value_type(2U);

  constexpr int signed_shift_amount =
    -(std::numeric_limits<typename local_uint_type::backend_type::representation_type::limb_type>::digits + 7);

  result_local = u_mid_local;
  result_local.backend().representation() >>= signed_shift_amount;
  result_boost = u_mid_boost * (boost_uint_type(1U) << (-signed_shift_amount));

  const bool result07_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  result_local = u_mid_local;
  result_local.backend().representation() <<= signed_shift_amount;
  result_boost = u_mid_boost / (boost_uint_type(1U) << (-signed_shift_amount));

  const bool result08_is_ok = (result_local.convert_to<std::string>() == result_boost.convert_to<std::string>());

  const auto result_is_ok = (   result01_is_ok
                             && result02_is_ok
                             && result03_is_ok
                             && result04_is_ok
                             && result05_is_ok
                             && result06_is_ok
                             && result07_is_ok
                             && result08_is_ok);

  return result_is_ok;
}

#if (BOOST_VERSION < 108000)
#if (defined(__clang__) && (__clang_major__ > 9)) && !defined(__APPLE__)
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
