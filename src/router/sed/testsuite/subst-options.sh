#!/bin/sh
# Test Substitute options (for code-coverage purposes as well)

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

#
# Simple modifiers to s//
# (specific characters included as make_subst_opts's implementation
#  checks for them before returning control)
printf "%s\n" a a a a a a > subst-in1 || framework_failure_
printf "%s\n" x x x x x x > subst-exp1 || framework_failure_
cat << \EOF >> subst-prog1 || framework_failure_
1s/A/x/i
2s/A/x/I

# s// followed by '}'
3{s/./x/}
# s// followed by '#'
4s/./x/#
# s// followed by ';'
5s/./x/;
# s// followed by '\n
6s/./x/
EOF

sed -f subst-prog1 subst-in1 > subst-out1 || fail=1
compare_ subst-exp1 subst-out1 || fail=1


#
# Number modifiers to s//
#

cat << \EOF >subst-in2 || framework_failure_
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
bbbbbbbbbb
EOF

cat << \EOF >subst-prog2 || framework_failure_
1s/./x/g
2s/./x/1
3s/./x/2
4s/./x/3
5s/./x/4
6s/./x/5
7s/./x/6
8s/./x/7
9s/./x/8
10s/./x/9
EOF

cat << \EOF >subst-exp2
xxxxxxxxxx
xbbbbbbbbb
bxbbbbbbbb
bbxbbbbbbb
bbbxbbbbbb
bbbbxbbbbb
bbbbbxbbbb
bbbbbbxbbb
bbbbbbbxbb
bbbbbbbbxb
EOF

sed -f subst-prog2 subst-in2 > subst-out2 || fail=1
compare_ subst-exp2 subst-out2 || fail=1

#
# Multiline modifier: s///m
# ('N' will read and concatenate the second line
#  into the patten space, making it "foo\nbar".
#  s// will then operate on it as one string).
printf "foo\nbar\n" > subst-in3 || fail=1
printf "Xoo\nXar\n" > subst-exp3 || fail=1

sed 'N;s/^./X/gm' subst-in3 > subst-out3-1 || fail=1
compare_ subst-exp3 subst-out3-1 || fail=1
sed 'N;s/^./X/gM' subst-in3 > subst-out3-2 || fail=1
compare_ subst-exp3 subst-out3-2 || fail=1

# sanity-check: without m, only the first line should match
printf "Xoo\nbar\n" > subst-exp3-3 || fail=1
sed 'N;s/^./X/g' subst-in3 > subst-out3-3 || fail=1
compare_ subst-exp3-3 subst-out3-3 || fail=1


#
# s// followed by \r\n
#

printf "s/./X/\r\n" > subst-prog4 || framework_failure_
echo a > subst-in4 || framework_failure_
echo X > subst-exp4 || framework_failure_
sed -f subst-prog4 subst-in4 > subst-out4 || fail=1
compare_ subst-exp4 subst-out4 || fail=1




Exit $fail
