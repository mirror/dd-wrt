#!/bin/sh
# Test '0rFILE' command

# Copyright (C) 2021-2022 Free Software Foundation, Inc.

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

cat <<\EOF >in1 || framework_failure_
HELLO
WORLD
EOF

cat <<\EOF >in2 || framework_failure_
1
2
3
EOF

cat <<\EOF >exp1 || framework_failure_
HELLO
WORLD
1
2
3
EOF

cat <<\EOF >exp2 || framework_failure_
1
HELLO
WORLD
2
HELLO
WORLD
3
EOF

cat <<\EOF> exp-err-addr0 || framework_failure_
sed: -e expression #1, char 4: invalid usage of line address 0
EOF

# Typical usage
sed '0rin1' in2 >out1 || fail=1
compare_ exp1 out1 || fail=1

# Ensure no regression for '0,/REGEXP/r'
sed '0,/2/rin1' in2 >out2 || fail=1
compare_ exp2 out2 || fail=1

# Ensure '0r' doesn't accept a numeric address range
returns_ 1 sed '0,4rin1' in2 2>err3 || fail=1
compare_ exp-err-addr0 err3 || fail=1

# Test with -i
sed -i '0rin1' in2 || fail=1
compare_ exp1 in2 || fail=1

Exit $fail
