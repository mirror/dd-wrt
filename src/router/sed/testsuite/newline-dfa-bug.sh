#!/bin/sh
# sed may access to uninitialized memory if transit to 15th dfa state
# with newline.  This bug affected sed version 4.3.

# Copyright (C) 2017-2022 Free Software Foundation, Inc.

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

require_valgrind_

printf 'abcdefg abcdefg\nB\n' > in || framework_failure_
printf 'B\n' > exp || framework_failure_

valgrind --quiet --error-exitcode=1 \
  sed 'N;s/abcdefg.*\n//' in > out 2> err || fail=1

# Work around a bug in CentOS 5.10's valgrind
# FIXME: remove in 2018 or when CentOS 5 is no longer officially supported
grep 'valgrind: .*Assertion.*failed' err > /dev/null \
  && skip_ 'you seem to have a buggy version of valgrind'

# Remove any valgrind-added diagnostics from stderr.
sed -i '/^==/d' err

compare exp out || fail=1
compare /dev/null err || fail=1

Exit $fail
