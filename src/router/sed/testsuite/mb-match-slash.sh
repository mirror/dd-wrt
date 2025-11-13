#!/bin/sh
# Test slash following an incomplete multibyte character

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

require_en_utf8_locale_

# before sed-4.3, a slash following an incomplete multibyte character
# would be ignored during program compilation, leading to an error.


# Test 1: match_slash in 's' command.
# Before sed-4.3, this would fail with "unterminated `s' command".
printf 's/\316/X/' > p1 || framework_failure_
LC_ALL=en_US.UTF-8 sed -f p1 </dev/null >out1 || fail=1
compare_ /dev/null out1 || fail=1

# Test 2: match_slash in address regex.
# Before sed-4.3, this would fail with "unterminated address regex".
printf '/\316/p' >p2 || framework_failure_
LC_ALL=en_US.UTF-8 sed -f p2 </dev/null >out2 || fail=1
compare_ /dev/null out2 || fail=1

# Test 3: match_slash in 'y' command..
# Before sed-4.3, this would fail with "unterminated `y' command".
printf 'y/\316/X/' >p3 || framework_failure_
LC_ALL=en_US.UTF-8 sed -f p3 </dev/null >out3 || fail=1
compare_ /dev/null out3 || fail=1


Exit $fail
