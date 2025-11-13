#!/bin/sh
# Test the '#n' silent mode (activated by first line comment)

# Copyright (C) 2016-2022 Free Software Foundation, Inc.

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

echo X > in1 || framework_failure_

# expected output with 'sed -n = in1' (silent mode)
echo 1 > exp-silent || framework_failure_

# expected output with 'sed = in1' (not silent mode)
printf "1\nX\n" > exp-norm || framework_failure_


# A comment '#n' in the first script, in the first line
sed -e '#n' in1 > out1 || fail=1
compare_ /dev/null out1 || fail=1

sed -e '#n' -e = in1 > out2 || fail=1
compare_ exp-silent out2 || fail=1

sed -e '#ni!' -e = in1 > out3 || fail=1
compare_ exp-silent out3 || fail=1

# not the first 2 characters, or space before n,
# or uppercase N - do not activate silent mode
sed -e '=#n' in1 > out4 || fail=1
compare_ exp-norm out4 || fail=1

sed -e '# n' -e = in1 > out5 || fail=1
compare_ exp-norm out5 || fail=1

sed -e '#N' -e = in1 > out6 || fail=1
compare_ exp-norm out6 || fail=1

sed -e = -e '#n' in1 > out7 || fail=1
compare_ exp-norm out7 || fail=1


#
# Test the same, with a program instead of -e.
#
cat << \EOF > prog1 || framework_failure_
#n
=
EOF
sed -f prog1 in1 > out8 || fail=1
compare_ exp-silent out8 || fail=1

# not in the first 2 characters
cat << \EOF > prog2 || framework_failure_
=
#n
EOF
sed -f prog2 in1 > out9 || fail=1
compare_ exp-norm out9 || fail=1

# not in the first 2 characters
cat << \EOF > prog3 || framework_failure_
# n
=
EOF
sed -f prog3 in1 > out10 || fail=1
compare_ exp-norm out10 || fail=1


# -e then a program file.
cat << \EOF > prog4 || framework_failure_
#n
EOF
sed -e = -f prog4 in1 > out11 || fail=1
compare_ exp-norm out11 || fail=1


# If the program comes before -e , silent mode is activated.
sed -f prog4 -e = in1 > out12 || fail=1
compare_ exp-silent out12 || fail=1


Exit $fail
