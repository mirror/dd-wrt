#!/bin/sh
# Test program file from STDIN

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

echo X > in1 || framework_failure_
printf "1\nX\n" > exp1 || framework_failure_

# program filename starts with '-'
printf "=\n" > ./-myprog || framework_failure_


# program from STDIN
printf "=\n" | sed -f - in1 > out1 || fail=1
compare_ exp1 out1 || fail=1

# program filename starting with '-'
# (if a buggy sed reads from STDIN, the 'v9' command will fail)
printf "v9\n" | sed -f -myprog in1 > out2 || fail=1
compare_ exp1 out2 || fail=1

Exit $fail
