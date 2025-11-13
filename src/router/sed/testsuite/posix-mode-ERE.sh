#!/bin/sh
# Ensure extended regular expressions work in posix mode

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

printf "hi+\n" > in1 || framework_failure_

printf "{hi+}\n" > exp-special || framework_failure_
printf "h{i+}\n" > exp-literal || framework_failure_

# '+' is special in ERE
sed -E 's/(.+)/{\1}/' in1 > out0 || fail=1
compare_ exp-special out0 || fail=1

# '+' is special in ERE, even if --posix is used.
# sed-4.4 and earlier did not treat it as special (bug#26409).
sed --posix -E 's/(.+)/{\1}/' in1 > out1 || fail=1
compare_ exp-special out1 || fail=1

# Escape the '+' it to remove special meaning in ERE
sed --posix -E 's/(.\+)/{\1}/' in1 > out2 || fail=1
compare_ exp-literal out2 || fail=1

# with BRE and --posix, '+' should have no special meaning
sed --posix 's/\(.+\)/{\1}/' in1 > out3 || fail=1
compare_ exp-literal out3 || fail=1

# with BRE without --posix, '+' should have no special meaning
sed 's/\(.+\)/{\1}/' in1 > out4 || fail=1
compare_ exp-literal out4 || fail=1

# with BRE without --posix, '\+' is special (GNU extension)
sed 's/\(.\+\)/{\1}/' in1 > out5 || fail=1
compare_ exp-special out5 || fail=1


Exit $fail
