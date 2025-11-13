#!/bin/sh
# sed would access uninitialized memory for certain invalid backreference uses.
# Before sed 4.6 these would result in "Invalid read size of 4" reported
# by valgrind from execute.c:992

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

require_valgrind_

printf '1\n2\n' > in || framework_failure_
printf '1\n2\n\n' > exp-posix || framework_failure_
printf '1\n1\n2\n2\n' > exp-no-posix || framework_failure_

#
# Test 1: with "--posix"
#
# using "--posix" disables the backref safety check in
# regexp.c:compile_regex_1(), which is reported as:
#     "invalid reference \\%d on `s' command's RHS"

valgrind --quiet --error-exitcode=1 \
  sed --posix -e '/2/p ; 2s//\9/' in > out-posix 2> err-posix || fail=1

echo "valgrind report for 'posix' test:"
echo "=================================="
cat err-posix
echo "=================================="


# Work around a bug in CentOS 5.10's valgrind
# FIXME: remove in 2018 or when CentOS 5 is no longer officially supported
grep 'valgrind: .*Assertion.*failed' err-posix > /dev/null \
  && skip_ 'you seem to have a buggy version of valgrind'

compare exp-posix out-posix || fail=1
compare /dev/null err || fail=1



#
# Test 2: without "--posix"
#
# When not using "--posix", using a backref to a non-existing group
# would be caught in compile_regex_1.
# As reported in bugs.gnu.org/32082 by bugs@feusi.co,
# using the recent begline/endline optimization with a few "previous regex"
# tricks bypasses this check.

valgrind --quiet --error-exitcode=1 \
  sed -e  '/^/s///p ; 2s//\9/' in > out-no-posix 2> err-no-posix || fail=1

echo "valgrind report for 'no-posix' test:"
echo "===================================="
cat err-no-posix
echo "===================================="

# Work around a bug in CentOS 5.10's valgrind
# FIXME: remove in 2018 or when CentOS 5 is no longer officially supported
grep 'valgrind: .*Assertion.*failed' err-no-posix > /dev/null \
  && skip_ 'you seem to have a buggy version of valgrind'

compare exp-no-posix out-no-posix || fail=1
compare /dev/null err || fail=1


Exit $fail
