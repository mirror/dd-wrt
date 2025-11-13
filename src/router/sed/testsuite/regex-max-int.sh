#!/bin/sh
# Test regex on input buffers larger than 2GB

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
very_expensive_
print_ver_ sed

# Create a file larger than 2GB and containing a single line
# (resulting in a regex match against the entire file)
#
# This is a "very expensive" test, we can assume it is only run by
# developers or advanced users, and we can assume truncate(1) exists.
#
# On most modern file-systems, the file will be sparse and would not
# consume 2GB of physical storage.

truncate -s 1G  input || framework_failure_
printf aaaa >>  input || framework_failure_
truncate -s +1G input || framework_failure_
printf 'a\n' >> input || framework_failure_

# The expected error message
cat <<\EOF > exp-err1 || framework_failure_
sed: regex input buffer length larger than INT_MAX
EOF


# Before sed-4.5, this was silently a no-op: would not perform the subsitution
# but would not indicate any error either (https://bugs.gnu.org/30520).
# Exit code 4 is "panic".
returns_ 4 sed 's/a/b/g' input >/dev/null 2>err1 || fail=1
compare_ exp-err1 err1 || fail=1

Exit $fail
