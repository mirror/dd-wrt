#!/bin/sh
# Test number conversion from escape sequences \xNN \oNNN \dNNN
# (compile.c:convert_number())

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

#
# Test \dNNN conversions
#
printf "%s\n" a a a a a a a > in-d || framework_failure_

# Each line is a separate test case
cat <<\EOF >prog-d
# Expected output: ASCII 0x0D '\r'
1s/./\d13/

# Expected output: ASCII 0xff '\3ff'
2s/./\d255/

# Expected (?) output: 'dB'
# (\d followed by character >= base 10, treated as '\d', which is 'd').
3s/./\dB/

# Expected (?) output: 'dQ'
# (\d followed by non-hex character, treated as '\d', which is 'd').
4s/./\dQ/

# Expected output: '{4'
# \dNNN is limited to three digits.
# The first three digits are 123 = 0x7b = '{'. '4' is treated as-is.
5s/./\d1234/

# Expected (?) output: '\1'
# undocumented implementation-specific limitation:
# After 3 digit limits, the 8-bit value is used,
# decimal 513 wraps-around to 1.
6s/./\d513/

# Expected output: '\0','7'
# (three digit limit)
7s/./\d0007/
EOF

printf '\r\n\377\ndB\ndQ\n{4\n\1\n\0007\n' > exp-d || framework_failure_

sed -f prog-d in-d > out-d || fail=1
compare_ exp-d out-d || fail=1

if test "$fail" -eq 1 ; then
    od -tx1c prog-d
    od -tx1c exp-d
    od -tx1c out-d
fi




#
# Test \oNNN conversions
#
printf "%s\n" a a a a a a > in-o || framework_failure_

# Each line is a separate test case
cat <<\EOF >prog-o
# Expected output: '\5'
1s/./\o5/

# Expected output: ASCII 0xff '\3ff'
2s/./\o377/

# Expected (?) output: 'o9'
# (\o followed by character >= base 18, treated as '\o', which is 'o').
3s/./\o9/

# Expected (?) output: 'oQ'
# (\o followed by non-hex character, treated as '\o', which is 'o').
4s/./\oQ/

# Expected output: 'S4'
# \oNNN is limited to three digits.
# The first three digits are o123 = 0x53 = 'S'. '4' is treated as-is.
5s/./\o1234/

# Expected (?) output: '\1'
# undocumented implementation-specific limitation:
# After 3 digit limits, the 8-bit value is used,
# octal 401 wraps-around to 1.
6s/./\o401/
EOF

printf '\5\n\377\no9\noQ\nS4\n\1\n' > exp-o || framework_failure_

sed -f prog-o in-o > out-o || fail=1
compare_ exp-o out-o || fail=1

if test "$fail" -eq 1 ; then
    od -tx1c prog-o
    od -tx1c exp-o
    od -tx1c out-o
fi





#
# Test \xNN conversions
#
printf "%s\n" a a a a > in-x || framework_failure_

# Each line is a separate test case
cat <<\EOF >prog-x
# Expected output: ASCII 0x06 '\6'
1s/./\x6/

# Expected output: ASCII 0xCE '\316'
2s/./\xce/

# Expected (?) output: 'xy'
# (\x followed by non-hex character, treated as '\x', which is 'x').
3s/./\xy/

# Expected output: '\253' 'c' (0xAB = 253 octal)
# \xNN is limited to two digits.
4s/./\xabc/
EOF

printf '\6\n\316\nxy\n\253c\n' > exp-x || framework_failure_

sed -f prog-x in-x > out-x || fail=1
compare_ exp-x out-x || fail=1

if test "$fail" -eq 1 ; then
    od -tx1c prog-x
    od -tx1c exp-x
    od -tx1c out-x
fi


# for completeness, cover all possible letters/digits

printf "%s\n" a a a a a a a a a a a > cnv-num-in || framework_failure_
cat << \EOF > cnv-num-prog || framework_failure_
1s/./\x01/
2s/./\x23/
3s/./\x45/
4s/./\x67/
5s/./\x89/
6s/./\xAB/
7s/./\xab/
8s/./\xCD/
9s/./\xcd/
10s/./\xef/
11s/./\xEF/
EOF

printf '\1\n#\nE\ng\n\211\n\253\n\253\n\315\n\315\n\357\n\357\n' \
    > cnv-num-exp || framework_failure_

sed -f cnv-num-prog cnv-num-in > cnv-num-out || fail=1
compare_ cnv-num-exp cnv-num-out || fail=1

Exit $fail
