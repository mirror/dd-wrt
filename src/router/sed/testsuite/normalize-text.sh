#!/bin/sh
# Text text escaping (compile.c:normalize_text()).
# NOTE:
#    \dNNN \xNN \oNNN - tested in 'convert-number.sh'
#    character-classes in POSIX mode - tested in 'posix-char-class.sh'

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
# Common backslash combinations
#
printf "%s\n" a a a a a a >in1 || framework_failure_
cat <<\EOF >prog1 || framework_failure_
1y/a/\a/
2y/a/\f/
3y/a/\n/
4y/a/\r/
5y/a/\t/
6y/a/\v/
EOF
printf "\a\n\f\n\n\n\r\n\t\n\v\n" > exp1 || framework_failure_

sed -f prog1 in1 > out1 || fail=1
compare_ exp1 out1 || fail=1

#
# test '\\\n' (backslash followed by ASCII 0x0A)
# normalized to a simple '\n' .
#
echo a > in2 || framework_failure_
printf "y/a/\\\n/" > prog2 || framework_failure_
printf "\n\n" > exp2 || framework_failure_
sed -f prog2 in2 > out2 || fail=1
compare_ exp2 out2 || fail=1

#
# \cX combination
#
printf "%s\n" a a a a a a a a a a > in3 || framework_failure_
cat <<\EOF >prog3 || framework_failure_
1y/a/\cA/
2y/a/\ca/
3y/a/\cZ/
4y/a/\cz/
5y/a/\c{/
6y/a/\c;/
7y/a/\c#/
8y/a/\c[/
9y/a/\c\\/
10y/a/\c]/
EOF

printf "\1\n\1\n\32\n\32\n;\n{\nc\n\33\n\34\n\35\n" > exp3 || framework_failure_
sed -f prog3 in3 > out3 || fail=1
compare_ exp3 out3 || fail=1

# \c at end of (valid) text - normalize_text() stops, returns control to caller.
# TODO: is this a bug?
#       compare with 'y/a/\d/' and 'y/a/\x/'
cat <<\EOF >exp-err-c || framework_failure_
sed: -e expression #1, char 7: strings for `y' command are different lengths
EOF
returns_ 1 sed 'y/a/\c/' </dev/null 2>err-c || fail=1
compare_ exp-err-c err-c || fail=1

Exit $fail
