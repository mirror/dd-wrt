#!/bin/sh
# Test 's' and 'y' non-slash delimiters in multibyte locales

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

# These tests use the following unicode character in various ways:
#   GREEK CAPITAL LETTER PHI (U+03A6)
#   UTF-8: hex: 0xCE     0xA6
#          oct: 0316     0246
#          bin: 11001110 10100110
#
# Octal encoding is used due to printf not supporting hex on older systems.
# Using the first octet alone (\316) causes various multibyte related functions
# to return '-2' (incomplete multibyte sequence).
# using the second octet alone (\246) causess same functions to return '-1'
# (ivalid multibyte sequence).


# Reject a valid multibyte delimiter (instead of slash).
printf 's\316\246a\316\246b\316\246' > prog1 || framework_failure_

cat <<\EOF > exp-err1 || framework_failure_
sed: file prog1 line 1: delimiter character is not a single-byte character
EOF

returns_ 1 env LC_ALL=en_US.UTF-8 sed -f prog1 < /dev/null 2>err1 || fail=1
compare_ exp-err1 err1 || fail=1


# Reject an incomplete multibyte delimiter (instead of slash).
# This is an implmentation-specific behavior:
# error is triggered upon first octet, before entire multibyte character
# is scanned.
printf 's\316a\316b\316' > prog2 || framework_failure_

cat <<\EOF > exp-err2 || framework_failure_
sed: file prog2 line 1: delimiter character is not a single-byte character
EOF

returns_ 1 env LC_ALL=en_US.UTF-8 sed -f prog2 </dev/null 2>err2 || fail=1
compare_ exp-err2 err2 || fail=1

# ... but accept octet \316 as delimiter in C locale
echo a > in2 || framework_failure_
echo b > exp2 || framework_failure_
LC_ALL=C sed -f prog2 <in2 >out2 || fail=1
compare_ exp2 out2 || fail=1



# An invalid multibyte sequence is treated as a valid single byte,
# thus accepted as a delimter (instead of slash).
# This is an implmentation-specific behavior.
printf 's\246a\246b\246' > prog3 || framework_failure_
echo a > in3 || framework_failure_
echo b > exp3 || framework_failure_

LC_ALL=en_US.UTF-8 sed -f prog3 <in3 >out3 || fail=1
compare_ exp3 out3 || fail=1

# Expect identical result in C locale
LC_ALL=C sed -f prog3 <in3 >out4 || fail=1
compare_ exp3 out4 || fail=1


Exit $fail
