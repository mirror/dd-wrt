#!/bin/sh
# Test -u/--unbuffered option

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

printf "1\n2\n" > in1 || framework_failure_

# expected output for both programs
printf "1\n" >> exp || framework_failure_


# in unbuffered mode,
# sed should consume and print the first line,
# wc should see the rest of the input (second line).
# The second sed trims optional leading whitespace.
( sed -u 1q > out-sed ; wc -l | sed 's/^  *//' > out-wc ) < in1

compare_ exp out-sed || fail=1
compare_ exp out-wc || fail=1


Exit $fail
