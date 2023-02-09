#
#  Copyright Christopher Kormanyos 2020 - 2022.
#  Distributed under the Boost Software License,
#  Version 1.0. (See accompanying file LICENSE_1_0.txt
#  or copy at http://www.boost.org/LICENSE_1_0.txt)
#

if [[ "$1" != "" ]]; then
    GCC="$1"
else
    GCC=g++
fi

if [[ "$2" != "" ]]; then
    STD="$2"
else
    STD=c++11
fi

mkdir -p bin

rm -f ./bin/*.*

echo run examples with GCC=$GCC STD=$STD

$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE000A_BUILTIN_CONVERT      ../../examples/example000a_builtin_convert.cpp                        -o ./bin/example000a_builtin_convert.exe
ls -la ./bin/example000a_builtin_convert.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE000_NUMERIC_LIMITS        ../../examples/example000_numeric_limits.cpp                          -o ./bin/example000_numeric_limits.exe
ls -la ./bin/example000_numeric_limits.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE001A_DIV_MOD              ../../examples/example001a_div_mod.cpp                                -o ./bin/example001a_div_mod.exe
ls -la ./bin/example001a_div_mod.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE001_MUL_DIV               ../../examples/example001_mul_div.cpp                                 -o ./bin/example001_mul_div.exe
ls -la ./bin/example001_mul_div.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE002_SHL_SHR               ../../examples/example002_shl_shr.cpp                                 -o ./bin/example002_shl_shr.exe
ls -la ./bin/example002_shl_shr.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE003A_CBRT                 ../../examples/example003a_cbrt.cpp                                   -o ./bin/example003a_cbrt.exe
ls -la ./bin/example003a_cbrt.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE003_SQRT                  ../../examples/example003_sqrt.cpp                                    -o ./bin/example003_sqrt.exe
ls -la ./bin/example003_sqrt.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE004_ROOTK_POW             ../../examples/example004_rootk_pow.cpp                               -o ./bin/example004_rootk_pow.exe
ls -la ./bin/example004_rootk_pow.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE005A_POW_FACTORS_OF_P99   ../../examples/example005a_pow_factors_of_p99.cpp                     -o ./bin/example005a_pow_factors_of_p99.exe
ls -la ./bin/example005a_pow_factors_of_p99.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE005_POWM                  ../../examples/example005_powm.cpp                                    -o ./bin/example005_powm.exe
ls -la ./bin/example005_powm.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE006_GCD                   ../../examples/example006_gcd.cpp                                     -o ./bin/example006_gcd.exe
ls -la ./bin/example006_gcd.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE007_RANDOM_GENERATOR      ../../examples/example007_random_generator.cpp                        -o ./bin/example007_random_generator.exe
ls -la ./bin/example007_random_generator.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE008_MILLER_RABIN_PRIME    ../../examples/example008_miller_rabin_prime.cpp                      -o ./bin/example008_miller_rabin_prime.exe
ls -la ./bin/example008_miller_rabin_prime.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE009A_TIMED_MUL_4_BY_4     ../../examples/example009a_timed_mul_4_by_4.cpp                       -o ./bin/example009a_timed_mul_4_by_4.exe
ls -la ./bin/example009a_timed_mul_4_by_4.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE009B_TIMED_MUL_8_BY_8     ../../examples/example009b_timed_mul_8_by_8.cpp                       -o ./bin/example009b_timed_mul_8_by_8.exe
ls -la ./bin/example009b_timed_mul_8_by_8.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE009_TIMED_MUL             ../../examples/example009_timed_mul.cpp                               -o ./bin/example009_timed_mul.exe
ls -la ./bin/example009_timed_mul.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE010_UINT48_T              ../../examples/example010_uint48_t.cpp                                -o ./bin/example010_uint48_t.exe
ls -la ./bin/example010_uint48_t.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE011_UINT24_T              ../../examples/example011_uint24_t.cpp                                -o ./bin/example011_uint24_t.exe
ls -la ./bin/example011_uint24_t.exe
$GCC -std=$STD -Wall -Werror -O3 -march=native -I../.. -DWIDE_INTEGER_STANDALONE_EXAMPLE012_RSA_CRYPTO            ../../examples/example012_rsa_crypto.cpp                              -o ./bin/example012_rsa_crypto.exe
ls -la ./bin/example012_rsa_crypto.exe

./bin/example000a_builtin_convert.exe
result_var_000a_builtin_convert=$?

./bin/example000_numeric_limits.exe
result_var_000_numeric_limits=$?

./bin/example001a_div_mod.exe
result_var_001a_div_mod=$?

./bin/example001_mul_div.exe
result_var_001_mul_div=$?

./bin/example002_shl_shr.exe
result_var_002_shl_shr=$?

./bin/example003a_cbrt.exe
result_var_003a_cbrt=$?

./bin/example003_sqrt.exe
result_var_003_sqrt=$?

./bin/example004_rootk_pow.exe
result_var_004_rootk_pow=$?

./bin/example005a_pow_factors_of_p99.exe
result_var_005a_pow_factors_of_p99=$?

./bin/example005_powm.exe
result_var_005_powm=$?

./bin/example006_gcd.exe
result_var_006_gcd=$?

./bin/example007_random_generator.exe
result_var_007_random_generator=$?

./bin/example008_miller_rabin_prime.exe
result_var_008_miller_rabin_prime=$?

./bin/example009a_timed_mul_4_by_4.exe
result_var_009a_timed_mul_4_by_4=$?

./bin/example009b_timed_mul_8_by_8.exe
result_var_009b_timed_mul_8_by_8=$?

./bin/example009_timed_mul.exe
result_var_009_timed_mul=$?

./bin/example010_uint48_t.exe
result_var_010_uint48_t=$?

./bin/example011_uint24_t.exe
result_var_011_uint24_t=$?

./bin/example012_rsa_crypto.exe
result_var_012_rsa_crypto=$?

echo "result_var_000a_builtin_convert     : "  "$result_var_000a_builtin_convert"
echo "result_var_000_numeric_limits       : "  "$result_var_000_numeric_limits"
echo "result_var_001a_div_mod             : "  "$result_var_001a_div_mod"
echo "result_var_001_mul_div              : "  "$result_var_001_mul_div"
echo "result_var_002_shl_shr              : "  "$result_var_002_shl_shr"
echo "result_var_003a_cbrt                : "  "$result_var_003a_cbrt"
echo "result_var_003_sqrt                 : "  "$result_var_003_sqrt"
echo "result_var_004_rootk_pow            : "  "$result_var_004_rootk_pow"
echo "result_var_005a_pow_factors_of_p99  : "  "$result_var_005a_pow_factors_of_p99"
echo "result_var_005_powm                 : "  "$result_var_005_powm"
echo "result_var_006_gcd                  : "  "$result_var_006_gcd"
echo "result_var_007_random_generator     : "  "$result_var_007_random_generator"
echo "result_var_008_miller_rabin_prime   : "  "$result_var_008_miller_rabin_prime"
echo "result_var_009a_timed_mul_4_by_4    : "  "$result_var_009a_timed_mul_4_by_4"
echo "result_var_009b_timed_mul_8_by_8    : "  "$result_var_009b_timed_mul_8_by_8"
echo "result_var_009_timed_mul            : "  "$result_var_009_timed_mul"
echo "result_var_010_uint48_t             : "  "$result_var_010_uint48_t"
echo "result_var_011_uint24_t             : "  "$result_var_011_uint24_t"
echo "result_var_012_rsa_crypto           : "  "$result_var_012_rsa_crypto"

result_total=$((result_var_000a_builtin_convert+result_var_000_numeric_limits+result_var_001a_div_mod+result_var_001_mul_div+result_var_002_shl_shr+result_var_003a_cbrt+result_var_003_sqrt+result_var_004_rootk_pow+result_var_005a_pow_factors_of_p99+result_var_005_powm+result_var_006_gcd+result_var_007_random_generator+result_var_008_miller_rabin_prime+result_var_009a_timed_mul_4_by_4+result_var_009b_timed_mul_8_by_8+result_var_009_timed_mul+result_var_010_uint48_t+result_var_011_uint24_t+result_var_012_rsa_crypto))

echo "result_total                        : "  "$result_total"

exit $result_total
