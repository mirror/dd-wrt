#!/bin/sh
# Test 'N' command with/without posix conformity

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

# in/exp as identical, but using 'exp' for both input and output
# will cause unneeded confusion when looking at the logs.
printf "A\nB\n" > in1   || framework_failure_
cp in1 exp1             || framework_failure_
printf "A\n" > in2      || framework_failure_
cp in2 exp2             || framework_failure_

# If there is a 'next' line, N behaves the same regardless of posixicity
sed N in1 > out1 || fail=1
compare exp1 out1 || fail=1

sed --posix N in1 > out2 || fail=1
compare exp1 out2 || fail=1

POSIXLY_CORRECT=y sed N in1 > out3 || fail=1
compare exp1 out3 || fail=1


# If there is no 'next' line,
# gnu-N quits with printing
# posix-N quits without printing.
sed N in2 > out4 || fail=1
compare exp2 out4 || fail=1

sed --posix N in2 > out5 || fail=1
compare /dev/null out5 || fail=1

POSIXLY_CORRECT=y sed N in2 > out6 || fail=1
compare /dev/null out6 || fail=1

# exception: gnu-mode N but no default output, should not print anything.
sed -n N in2 > out7 || fail=1
compare /dev/null out7 || fail=1


Exit $fail
