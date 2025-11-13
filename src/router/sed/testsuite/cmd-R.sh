#!/bin/sh
# Test 'R' command

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

printf "%s\n" x y > a || framework_failure_
printf "%s\n" 1 2 > b || framework_failure_
printf "%s\n" X > c || framework_failure_
touch d || framework_failure_

# Read and interleave two lines
printf "%s\n" x 1 y 2 > exp1 || framework_failure_
sed -e 1Rb -e 2Rb a > out1 || fail=1
compare_ exp1 out1 || fail=1

# Read a non-existing file, silently ignored
sed -e 1Rq a > out2 || fail=1
compare_ a out2

# Read two lines from a file, second time will be EOF
# (implementation note: EOF from get_delim())
printf "%s\n" x X y > exp3 || framework_failure_
sed -e 1Rc -e 2Rc a > out3 || fail=1
compare_ exp3 out3 || fail=1

# Read two lines from an empty file, both will be EOF
# (implementation note: EOF in before get_delim())
sed -e 1Rd -e 2Rd a > out4 || fail=1
compare_ a out4 || fail=1


Exit $fail
