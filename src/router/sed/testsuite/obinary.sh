#!/bin/sh
# Test CR/LF behaviour on platforms which support O_BINARY file mode
# (i.e. differentiates between text and binary files).

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

# Test if O_TEXT is enabled by default (i.e. lines terminated with "\r\n").
# If not, skip the test.
printf a | sed cb > out1 \
  || framework_failure_ "failed to run sed 'cb'"
size=$(LC_ALL=C wc -c < out1 | tr -d '[:space:]') \
  || framework_failure_ "failed to check size of 'out1'"
case $size in
  2) skip_ "platform does not enable O_TEXT by default" ;;
  3) ;;
  *) framework_failure_ "unexpected size '$size'" ;;
esac


# files with "\r\n" and with just "\n"
printf 'a\015\12' > inT || framework_failure_
printf 'a\12'     > inB || framework_failure_
cp inT inplaceT1 || framework_failure_
cp inT inplaceT2 || framework_failure_
cp inT inplaceT3 || framework_failure_
cp inB inplaceB1 || framework_failure_
cp inB inplaceB2 || framework_failure_

printf 'z\015\12' > expT || framework_failure_
printf 'z\12'     > expB || framework_failure_


# First round of tests. These all seem equivalent,
# but older seds had sublte implementation differences
# between STDIN and explicit input files (bug#25459).
# Similarly, also test --inplace type output.
sed 's/a/z/' inT > out1 || fail=1
sed 's/a/z/' < inT > out2 || fail=1
cat inT | sed 's/a/z/' > out3 || fail=1
sed -i 's/a/z/' inplaceT1 || fail=1

compare_ expT out1 || fail=1
compare_ expT out2 || fail=1
compare_ expT out3 || fail=1
compare_ expT inplaceT1 || fail=1

# Input file with only "\n". Output should contain "\r\n".
sed 's/a/z/' inB > out4 || fail=1
sed 's/a/z/' < inB > out5 || fail=1
cat inB | sed 's/a/z/' > out6 || fail=1
sed -i 's/a/z/' inplaceB1 || fail=1

compare_ expT out4 || fail=1
compare_ expT out5 || fail=1
compare_ expT out6 || fail=1
compare_ expT inplaceB1 || fail=1

# Input file with only "\n", with "sed -b" should output only "\n".
sed -b 's/a/z/' inB > out7 || fail=1
sed -b 's/a/z/' < inB > out8 || fail=1
cat inB | sed -b 's/a/z/' > out9 || fail=1
sed -b -i 's/a/z/' inplaceB2 || fail=1

compare_ expB out7 || fail=1
compare_ expB out8 || fail=1
compare_ expB out9 || fail=1
compare_ expB inplaceB2 || fail=1

# End-of-line tests on input file with "\r\n".
# In TEXT mode, "\r\n" is end-of-line, the "y" character will be added prior to
# it. In BINARY mode, "\r" is just another character - the "y" character will
# be added after the "\r".
printf 'ay\015\012' > expTeol || framework_failure_
printf 'a\015y\012' > expBeol || framework_failure_

sed    's/$/y/'   inT     > out10 || fail=1
sed    's/$/y/' < inT     > out11 || fail=1
cat inT | sed    's/$/y/' > out12 || fail=1
sed -i 's/$/y/'   inplaceT2       || fail=1

sed -b 's/$/y/'   inT     > out13 || fail=1
sed -b 's/$/y/' < inT     > out14 || fail=1
cat inT | sed -b 's/$/y/' > out15 || fail=1
sed -i -b 's/$/y/' inplaceT3      || fail=1

compare_ expTeol out10 || fail=1
compare_ expTeol out11 || fail=1
compare_ expTeol out12 || fail=1
compare_ expTeol inplaceT2 || fail=1

compare_ expBeol out13 || fail=1
compare_ expBeol out14 || fail=1
compare_ expBeol out15 || fail=1
compare_ expBeol inplaceT3 || fail=1

Exit $fail
