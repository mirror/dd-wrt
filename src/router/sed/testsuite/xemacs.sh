#!/bin/sh

# Test runner for xemacs.sed

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

# location of external test files
dir="$abs_top_srcdir/testsuite"

# Inspired by xemacs' config.status script
# submitted by John Fremlin (john@fremlin.de)
cat << \EOF > xemacs.sed || framework_failure_
/^# Generated/d
s%/\*\*/#.*%%
s/^ *# */#/
/^##/d
/^#/ {
  p
  d
}
/./ {
  s/\([\"]\)/\\\1/g
  s/^/"/
  s/$/"/
}
EOF


sed -f xemacs.sed < "$dir/xemacs.inp" > out || fail=1
remove_cr_inplace out
compare "$dir/xemacs.good" out || fail=1


Exit $fail
