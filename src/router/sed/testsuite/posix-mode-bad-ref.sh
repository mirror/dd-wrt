#!/bin/sh
# Test non-posix-conforming gnu extensions when using --posix.

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

cat <<\EOF >exp-err || framework_failure_
sed: -e expression #1, char 10: invalid reference \1 on `s' command's RHS
EOF

# Invalid references are errors in non-posix mode
returns_ 1 sed 's/abc/\1/g' 2>err < /dev/null || fail=1
compare_ exp-err err || fail=1

# Invalid references are silently ignored in posix mode
sed --posix 's/abc/\1/g' < /dev/null || fail=1

Exit $fail
