#!/bin/sh

# Test runner for dc.sed

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

# Compute Easter of 2002...
# usage:   (echo YEAR; cat easter.dc) | dc.sed
cat << \EOF > easter.dc || framework_failure_
[ddsf[lfp[too early
]Pq]s@1583>@
ddd19%1+sg100/1+d3*4/12-sx8*5+25/5-sz5*4/lx-10-sdlg11*20+lz+lx-30%
d[30+]s@0>@d[[1+]s@lg11<@]s@25=@d[1+]s@24=@se44le-d[30+]s@21>@dld+7%-7+
[March ]smd[31-[April ]sm]s@31<@psnlmPpsn1z>p]splpx
EOF

cat <<\EOF > easter-exp || framework_failure_
31
March 2002
EOF


# Compute square root of 2
cat << \EOF > sqrt2-inp || framework_failure_
16oAk2vpq
EOF


cat << \EOF > sqrt2-exp || framework_failure_
1.6A09E667A
EOF


# location of external test files
dir="$abs_top_srcdir/testsuite"

# Easter 2002
( echo 2002 ; cat easter.dc ) | sed -n -f "$dir/dc.sed" > easter-out|| fail=1
compare easter-exp easter-out || fail=1

# Square root of 2
sed -n -f "$dir/dc.sed" sqrt2-inp > sqrt2-out || fail=1
compare sqrt2-exp sqrt2-out || fail=1



Exit $fail
