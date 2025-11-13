#!/bin/sh
# Test 'l' command with different widths

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

#         10        20        30        40        50        60        70  75
cat <<\EOF >in1 || framework_failure_
0123456789012345678901234567890123456789012345678901234567890123456789012345
EOF

# default: 70 characters (including the \n)
cat <<\EOF >exp-def || framework_failure_
012345678901234567890123456789012345678901234567890123456789012345678\
9012345$
EOF

# 11 characters
cat <<\EOF >exp-11 || framework_failure_
0123456789\
0123456789\
0123456789\
0123456789\
0123456789\
0123456789\
0123456789\
012345$
EOF

# command 'l n' is a gnu extension, rejected in posix mode
cat <<\EOF >exp-err-posix-ln || framework_failure_
sed: -e expression #1, char 2: extra characters after command
EOF

# sed's default: 70 characters
sed -n l in1 >out-def || fail=1
compare_ exp-def out-def || fail=1

# limit with COLS envvar, sed subtracts one to avoid ttys linewraps
COLS=12 sed -n l in1 >out-cols12 || fail=1
compare_ exp-11 out-cols12 || fail=1

# invalid COLS envvar should be ignored (wrap at default=70)
COLS=0 sed -n l in1 >out-cols0 || fail=1
compare_ exp-def out-cols0 || fail=1
COLS=foo sed -n l in1 >out-cols-foo || fail=1
compare_ exp-def out-cols-foo || fail=1

# limit with -l parameter
sed -l 11 -n l in1 >out-l11 || fail=1
compare_ exp-11 out-l11 || fail=1

# limit with 'ln' command (gnu extension)
sed -n l11 in1 >out-ln-11 || fail=1
compare_ exp-11 out-ln-11 || fail=1

# limit with 'ln' command (gnu extension)
returns_ 1 sed --posix -n l11 in1 2>err-posix-ln || fail=1
compare_ exp-err-posix-ln err-posix-ln || fail=1

Exit $fail
