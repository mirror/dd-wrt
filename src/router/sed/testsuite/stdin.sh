#!/bin/sh

# Test runner for old 'stdin' test

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


cat << \EOF > stdin-in || framework_failure_
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
EOF

# This checks for a bug in 3.02 and 3.02.80
( sed d ; sed G ) < stdin-in > stdin-out1 || fail=1
cat stdin | ( sed d ; sed G ) > stdin-out2 || fail=1

remove_cr_inplace stdin-out1
remove_cr_inplace stdin-out2

compare stdin-out1 stdin-out2 || fail=1

Exit $fail
