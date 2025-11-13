#!/bin/sh
# Test r/R/w/W commands without a file name.

# Copyright (C) 2018-2022 Free Software Foundation, Inc.

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

# Same error message, different character position in the sed program.
for i in 1 7 ; do
  err="sed: -e expression #1, char $i: missing filename in r/R/w/W commands"
  echo "$err" > exp-err$i || framework_failure_
done

# r/R/w/W commands
for cmd in r R w W ; do
  returns_ 1 sed $cmd </dev/null >/dev/null 2>err1 || fail=1
  compare exp-err1 err1 || fail=1
done

returns_ 1 sed 's/1/2/w' </dev/null >/dev/null 2>err7 || fail=1
compare exp-err7 err7 || fail=1

Exit $fail
