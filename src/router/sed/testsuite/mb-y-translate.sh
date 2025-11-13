#!/bin/sh
# Test multibyte y/// translations

# Copyright (C) 2016-2022 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
. "${srcdir=.}/testsuite/init.sh"; path_prepend_ ./sed
print_ver_ sed

require_en_utf8_locale_

# These tests use the following unicode characters in various ways:
#   GREEK CAPITAL LETTER PHI (U+03A6)
#   UTF-8: hex: 0xCE     0xA6
#          oct: 0316     0246
#          bin: 11001110 10100110
#
#   GREEK CAPITAL LETTER DELTA (U+0394)
#   UTF-8: hex: 0xCE     0x94
#          oct: 0316     0224
#          bin: 11001110 10010100
#
# Octal encoding is used due to printf not supporting hex on older systems.
# Using the first octet alone (\316) causes various multibyte related functions
# to return '-2' (incomplete multibyte sequence).
# using the second octet alone (\246) causess same functions to return '-1'
# (invalid multibyte sequence).


#
# Test 1: valid multibyte 'dest-chars'
#
printf 'y/a/\316\246/' > p1 || framework_failure_
echo Xa > in1 || framework_failure_
printf 'X\316\246\n' > exp1 || framework_failure_

LC_ALL=en_US.UTF-8 sed -f p1 <in1 >out1 || fail=1
compare_ exp1 out1 || fail=1

# in C locale, report error of mismatched length
cat <<\EOF > exp-err1 || framework_failure_
sed: file p1 line 1: strings for `y' command are different lengths
EOF
returns_ 1 env LC_ALL=C sed -f p1 </dev/null 2>err1 || fail=1
compare_ exp-err1 err1 || fail=1


#
# Test 2: valid multibyte 'src-chars'
#
printf 'y/\316\246/a/' > p2 || framework_failure_
printf 'X\316\246\n' > in2 || framework_failure_
echo Xa > exp2 || framework_failure_

LC_ALL=en_US.UTF-8 sed -f p2 <in2 >out2 || fail=1
compare_ exp2 out2 || fail=1

# in C locale, report error of mismatched length
cat <<\EOF > exp-err2 || framework_failure_
sed: file p2 line 1: strings for `y' command are different lengths
EOF
returns_ 1 env LC_ALL=C sed -f p2 </dev/null 2>err2 || fail=1
compare_ exp-err2 err2 || fail=1


#
# Tests 3-6: invalid/incomplete multibyte characters in src/dest.
# All work as-is in C locale, treated as single-bytes in multibyte locales.
# None should fail.

# Test 3: invalid multibyte 'dest-chars'.
echo Xa > in3 || framework_failure_
printf 'y/a/\246/' > p3 || framework_failure_
printf 'X\246\n' > exp3 || framework_failure_

# Test 4: incomplete multibyte 'dest-chars'.
echo Xa > in4 || framework_failure_
printf 'y/a/\316/' > p4 || framework_failure_
printf 'X\316\n' > exp4 || framework_failure_

# Test 5: invalid multibyte 'src-chars'.
printf 'X\246\n' > in5 || framework_failure_
printf 'y/\246/a/' > p5 || framework_failure_
echo Xa > exp5 || framework_failure_

# Test 6: incomplete multibyte 'dest-chars'.
printf 'X\316\n' > in6 || framework_failure_
printf 'y/\316/a/' > p6 || framework_failure_
echo Xa > exp6 || framework_failure_

for t in 3 4 5 6 ;
do
    for l in C en_US.UTF-8 ;
    do
        LC_ALL=$l sed -f p$t <in$t >out$t-$l || fail=1
        compare_ exp$t out$t-$l || fail=1
    done
done


#
# Tests 7,8: length mismatch in multibyte locales
# Implementation note: the code path for length check differ between
# single-byte/multibyte locales. The actual characters don't have to be
# multibyte themselves.
printf 'y/abc/d/' > p7 || framework_failure_
cat <<\EOF > exp-err7 || framework_failure_
sed: file p7 line 1: strings for `y' command are different lengths
EOF

returns_ 1 env LC_ALL=en_US.UTF-8 sed -f p7 </dev/null 2>err7 || fail=1
compare_ exp-err7 err7 || fail=1

printf 'y/a/bcd/' > p8 || framework_failure_
cat <<\EOF > exp-err8 || framework_failure_
sed: file p8 line 1: strings for `y' command are different lengths
EOF

returns_ 1 env LC_ALL=en_US.UTF-8 sed -f p8 </dev/null 2>err8 || fail=1
compare_ exp-err8 err8 || fail=1


Exit $fail
