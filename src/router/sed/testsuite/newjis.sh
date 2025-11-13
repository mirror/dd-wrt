#!/bin/sh

# Test runner for newjis

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

# Input file. \033 is ASCII escape (0x1B).
{
  printf '\033$B$H$J$j$N$?$1$,$-$K\033(B\n' ;
  printf '\033$B$?$F$+$1$?$N$O\033(B\n' ;
  printf '\033$B$?$F$+$1$?$+$C$?$+$i\033(B\n' ;
  printf '\033$B$?$F$+$1$?!#\033(B\n' ;
} > newjis-inp || framework_failure_

# The expected output.
{
  printf '\033$B$H$J$j$NM9JX6I$K\033(B\n';
  printf '\033$B$?$F$+$1$?$N$O\033(B\n' ;
  printf '\033$B$?$F$+$1$?$+$C$?$+$i\033(B\n' ;
  printf '\033$B$?$F$+$1$?!#\033(B\n' ;
} > newjis-exp || framework_failure_

# The sed program.
cat <<\EOF > newjis.sed || framework_failure_
s/$?$1$,$-/M9JX6I/
EOF

sed -f newjis.sed < newjis-inp > newjis-out || fail=1
remove_cr_inplace newjis-out
compare newjis-exp newjis-out || fail=1


Exit $fail
