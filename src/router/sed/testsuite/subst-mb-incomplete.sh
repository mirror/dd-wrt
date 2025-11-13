#!/bin/sh
# Ensure that sed no longer writes beyond the end of a heap buffer when
# performing a substitution with a replacement string containing an
# incomplete multi-byte character.

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

require_en_utf8_locale_

echo > in || framework_failure_
printf '\233\375\200\n' > exp-out || framework_failure_

LC_ALL=en_US.utf8 sed $(printf 's/^/\\L\233\375\\\200/') in > out 2> err

compare exp-out out || fail=1
compare /dev/null err || fail=1

Exit $fail
