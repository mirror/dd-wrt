#!/bin/sh

# Runner for old '8to7' test

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

# Generate the input file, containing non-ascii 8-bit octets.
# printf with octal escape sequences is the most portable way
# to produce these.
{
  printf '\344\306\244\342\244\350 \244\337\344\306\273\375\244\301\n' ;
  printf '\267\241\266\372\244\342\244\350 ' ;
  printf '\244\337\267\241\266\372\273\375\244\301\n' ;
  printf '\244\263\244\316\265\326\244\313 ' ;
  printf '\272\332\305\246\244\336\244\271\273\371\n' ;
  printf '\262\310\264\326\244\253\244\312 \271\360\244\351\244\265\244\315\n' ;
  printf '\244\275\244\351\244\337\244\304 ';
  printf '\302\347\317\302\244\316\271\361\244\317\n' ;
  printf '\244\252\244\267\244\343\244\312\244\331\244\306 ' ;
  printf '\244\357\244\354\244\263\244\275\265\357\244\354\n' ;
  printf '\244\267\244\255\244\312\244\331\244\306 ' ;
  printf '\244\357\244\354\244\263\244\275 ' ;
  printf '\272\302\244\273\n';
  printf '\244\357\244\313\244\263\244\275\244\317 ' ;
  printf '\271\360\244\351\244\341\n' ;
  printf '\262\310\244\362\244\342\314\276\244\362\244\342\n';
} > 8to7-inp || framework_failure_


# The expected output.
# NOTE:
# shell-escaping is OFF with here-doc preceded by a backslash
# i.e. the first 4 octets in the output will be the characters
# '\', '3', '4', '4'.
cat <<\EOF > 8to7-exp || framework_failure_
\344\306\244\342\244\350 \244\337\344\306\273\375\244\301$
\267\241\266\372\244\342\244\350 \244\337\267\241\266\372\273\375\244\
\301$
\244\263\244\316\265\326\244\313 \272\332\305\246\244\336\244\271\273\
\371$
\262\310\264\326\244\253\244\312 \271\360\244\351\244\265\244\315$
\244\275\244\351\244\337\244\304 \302\347\317\302\244\316\271\361\244\
\317$
\244\252\244\267\244\343\244\312\244\331\244\306 \244\357\244\354\244\
\263\244\275\265\357\244\354$
\244\267\244\255\244\312\244\331\244\306 \244\357\244\354\244\263\244\
\275 \272\302\244\273$
\244\357\244\313\244\263\244\275\244\317 \271\360\244\351\244\341$
\262\310\244\362\244\342\314\276\244\362\244\342$
EOF

sed -e 'l;d' 8to7-inp > 8to7-out || fail=1
remove_cr_inplace 8to7-out
compare 8to7-exp 8to7-out || fail=1


Exit $fail
