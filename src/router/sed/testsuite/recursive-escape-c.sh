#!/bin/sh
# test \c escaping

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

unset POSIXLY_CORRECT
export LC_ALL=C

# input file, any 6 lines would do, each a different test case
printf "%s\n" a a a a a a >in1 || framework_failure_

# input program
cat << \EOF > prog1 || framework_failure_
1s/./\cA/
2s/./\cB/
3s/./\c[/
4s/./\c]/

# '\c' at end-of-buffer, a backslash is pushed up
# on level of interpretation, and the '.' match is replaced
# with one backslash.
5s/./\c/

# This would return incorrect results before 4.3,
# producing both \034 and another backslash.
6s/./\c\\/
EOF

# expected output:
printf '\001\n\002\n\033\n\035\n\\\n\034\n' > exp1 || framework_failure_

#
# Run simple test cases
#
sed -f prog1 in1 > out1 || fail=1
compare_ exp1 out1 || fail=1

# for easier troubleshooting, if users ever report errors
if test "$fail" -eq 1 ; then
    od -tx1c prog1
    od -tx1c exp1
    od -tx1c out1
fi

#
# Test invalid usage
#
cat << \EOF > exp-err || framework_failure_
sed: -e expression #1, char 10: recursive escaping after \c not allowed
EOF

# Before sed-4.3, this resulted in '\034d'. Now, it is rejected.
returns_ 1 sed '1s/./\c\d/' in1 2>err || fail=1
compare_ exp-err err || fail=1

Exit $fail
