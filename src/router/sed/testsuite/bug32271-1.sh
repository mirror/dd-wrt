#!/bin/sh
# sed would incorrectly copy internal buffers under certain s/// uses.
# Before sed 4.6 these would result in an extraneous NUL at end of lines.
#

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

printf '0\n' > in || framework_failure_
printf '0\n' > exp || framework_failure_

# Before sed 4.6, this would result in: 0x30 0x00 0x0a.
sed -e 's/$/a/2' in > out 2> err || fail=1

compare exp out || fail=1
compare /dev/null err || fail=1

# To ease debugging / error reporting (the above 'compare'
# will report "binary files differ" - not very helpful here)
if test -n "$fail" ; then
    echo "---- TEST FAILED"
    echo "out:"
    od -tx1 out
    echo "exp:"
    od -tx1 exp
    echo "err:"
    od -tx1 err
fi


Exit $fail
