#!/bin/sh
# Verify that even with overlapping ranges of line numbers,
# only the selected lines are affected.

# Copyright (C) 2015-2022 Free Software Foundation, Inc.

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

printf '%s\n' 1 2 3 4 5 6 > in || framework_failure_
printf '%s\n' 1 5 6 > exp || framework_failure_

# Before sed-4.3, this would mistakenly modify line 5 like this:
# 1
# yx5
# 6
sed '2,4d;2,3s/^/x/;3,4s/^/y/' in > out 2> err || framework_failure_

compare exp out || fail=1
compare /dev/null err || fail=1

Exit $fail
