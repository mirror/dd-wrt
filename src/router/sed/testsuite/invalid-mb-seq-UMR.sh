#!/bin/sh
# Inserting an invalid multibyte sequence could lead to
# reading uninitialized memory.

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

require_valgrind_

test "$LOCALE_JA" = none && skip_ found no Japanese EUC locale

# Ensure the implementation is not buggy (skip otherwise)
require_valid_ja_eucjp_locale_ "$LOCALE_JA"

echo a > in || framework_failure_
printf 'b\262C\n' > exp || framework_failure_
LC_ALL=$LOCALE_JA valgrind --quiet --error-exitcode=1 \
  sed 's/a/b\U\xb2c/' in > out 2> err || fail=1

# Work around a bug in CentOS 5.10's valgrind
# FIXME: remove in 2018 or when CentOS 5 is no longer officially supported
grep 'valgrind: .*Assertion.*failed' err > /dev/null \
  && skip_ 'you seem to have a buggy version of valgrind'

compare exp out || fail=1
compare /dev/null err || fail=1

Exit $fail
