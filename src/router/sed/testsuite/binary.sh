#!/bin/sh

# Test runner for the binary-operation version of dc.sed.
# Adapted from old-style 'binary.sed' test.

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

cat << \EOF > inp || framework_failure_
192.168.1.2 br b8<r b16<r b24< R|R|R| D
255.255.255.0 br b8<r b16<r b24< R|R|R| D~r
& DDD 24>dpP 16>11111111& dpP 8>11111111& dpP 11111111& dpP
| DDD 24>dpP 16>11111111& dpP 8>11111111& dpP 11111111& dpP
EOF


cat << \EOF > exp || framework_failure_
192
168
1
0
192
168
1
255
EOF


# location of the external SED scripts
dir="$abs_top_srcdir/testsuite"


# Run the three variations of the sed script
sed -n -f "$dir/binary.sed" < inp > out1 || fail=1
remove_cr_inplace out1
compare exp out1 || fail=1

sed -n -f "$dir/binary2.sed" < inp > out2 || fail=1
remove_cr_inplace out2
compare exp out2 || fail=1

sed -n -f "$dir/binary3.sed" < inp > out3 || fail=1
remove_cr_inplace out3
compare exp out3 || fail=1


Exit $fail
