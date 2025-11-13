#!/bin/sh
# Verify that a ":" command with no label is now rejected.

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

echo 'sed: -e expression #1, char 1: ":" lacks a label' > exp-err \
  || framework_failure_

# Before sed-4.3, sed would mistakenly accept a ":" with no following
# label name.
echo x | returns_ 1 sed : > out 2> err || fail=1

compare /dev/null out || fail=1
compare exp-err err || fail=1

Exit $fail
