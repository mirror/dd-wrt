#!/bin/sh
# Ensure GNU address extensions are rejected in posix mode

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

cat <<\EOF> exp-err-addr0 || framework_failure_
sed: -e expression #1, char 6: invalid usage of line address 0
EOF

cat <<\EOF >exp-err-bad-addr || framework_failure_
sed: -e expression #1, char 3: unexpected `,'
EOF

printf "%s\n" A B A C D E F G H I J >in1 || framework_failure_

# The expected output with zero-line address '0,/A/'
# the regex will match the first line
printf "A\n" >exp-l0 || framework_failure_

# The expected output with one-line address '1,/A/'
# the regex will not be checked against the first line,
# will match the third line
printf "%s\n" A B A >exp-l1 || framework_failure_

# The expected output with address '2,+1'
# (from line 2, count 1 addition line = line 3)
printf "%s\n" B A >exp-plus || framework_failure_

# The expected output with address '5,~4'
# (from line 5 till a multiple of 4 = line 8)
printf "%s\n" D E F G >exp-mult || framework_failure_


#
# Addressing extension: 0,/regexp/
#

# sanity check: address line=1 is valid for both posix and gnu
sed -n '1,/A/p' in1 > out-l1 || fail=1
compare_ exp-l1 out-l1 || fail=1

# address line=0 is a gnu extension
sed -n '0,/A/p' in1 > out-gnu-l0 || fail=1
compare_ exp-l0 out-gnu-l0 || fail=1
# rejected in posix mode
returns_ 1 sed --posix -n '0,/A/p' in1 2>err-posix-l0 || fail=1
compare_ exp-err-addr0 err-posix-l0 || fail=1



#
# Addressing extension: addr,+N
#
sed -n '2,+1p' in1 > out-plus || fail=1
compare_ exp-plus out-plus || fail=1

returns_ 1 sed --posix -n '2,+1p' in1 2> err-plus || fail=1
compare_ exp-err-bad-addr err-plus || fail=1



#
# Addressing extension: addr,~N
#

sed -n '5,~4p' in1 > out-mult || fail=1
compare_ exp-mult out-mult || fail=1

returns_ 1 sed --posix -n '5,~4p' in1 2> err-mult || fail=1
compare_ exp-err-bad-addr err-mult || fail=1



Exit $fail
