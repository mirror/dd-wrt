#!/bin/sh
# Test compilation less-common cases

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
# Special file names, with gnu extensions and without (if the host
# supports /dev/std{out,err} )
#
echo a > a || framework_failure_

# With gnu extension enabled, /dev/stderr is emulated internally
# regardless of the operating system.
sed 'w/dev/stderr' a >out 2>err|| fail=1
compare_ a out || fail=1
compare_ a err || fail=1

# In posix mode /dev/std* are not emulated internally. Skip if they
# don't exist. 'env' is used to avoid built-in 'test' which
# simulates /dev/stderr, e.g. bash on AIX.
if env test -w /dev/stderr ; then
    sed --posix 'w/dev/stderr' a >out-psx 2>err-psx || fail=1
    compare_ a out-psx || fail=1
    compare_ a err-psx || fail=1
fi


#
# labels followed by various characters
# (read_label)
echo a > lbl-in-exp || framework_failure_
cat << \EOF > lbl-prog || framework_failure_
bZ
:Z
bY;
:Y
{bX}
:X ;
b W
: W
EOF
sed -f lbl-prog lbl-in-exp > lbl-out || fail=1
compare_ lbl-in-exp lbl-out



#
# character classes (compile.c:snarf_char_class)
#

# open brackets followed by EOF
cat <<\EOF >exp-err-op-bracket || framework_failure_
sed: -e expression #1, char 2: unterminated address regex
EOF
returns_ 1 sed '/[' </dev/null 2>err-op-bracket1 || fail=1
compare_ exp-err-op-bracket err-op-bracket1 || fail=1


# open brackets followed by \n
printf "/[\n" > op-bracket-prog || framework_failure_
cat <<\EOF >exp-err-op-bracket || framework_failure_
sed: file op-bracket-prog line 1: unterminated address regex
EOF
returns_ 1 sed -f op-bracket-prog  </dev/null 2>err-op-bracket2 || fail=1
compare_ exp-err-op-bracket err-op-bracket2 || fail=1


# unterminated character class '[.'
# (snarf_char_class terminates on char 7, then returns)
cat <<\EOF >exp-chr-class || framework_failure_
sed: -e expression #1, char 7: unterminated `s' command
EOF
returns_ 1 sed 's/[[.//' </dev/null 2>err-chr-class || fail=1
compare_ exp-chr-class err-chr-class || fail=1


# closing bracket immediately after char-class opening
# sequence (e.g. '[:]' instead of '[:alpha:]' ).
cat<< \EOF >exp-chr-class2 || framework_failure_
sed: -e expression #1, char 9: unterminated `s' command
EOF
returns_ 1 sed 's/[[:]]//' </dev/null 2>err-chr-class2 || fail=1
compare_ exp-chr-class2 err-chr-class2 || fail=1


# EOF after backslash in a regex (compile.c:match_slash())
cat<< \EOF >exp-backslash-eof || framework_failure_
sed: -e expression #1, char 2: unterminated address regex
EOF
returns_ 1 sed '/\' </dev/null 2>err-backslash-eof || fail=1
compare_ exp-backslash-eof err-backslash-eof || fail=1


# Valid version requirement
sed 'v4' < /dev/null || fail=1

# Closing braces followed by another closing braces, and '#'
echo X > in-exp || framework_failure_
sed -n '{{p}}' in-exp > out-braces-1 || fail=1
compare_ in-exp out-braces-1 || fail=1

sed -n '{p}#foo' in-exp > out-braces-2 || fail=1
compare_ in-exp out-braces-2 || fail=1

# 'l' followed by closing braces, and '#'
printf 'X$\n' > exp-l || framework_failure_
sed -n  '{l}' in-exp > out-l-braces || fail=1
compare_ exp-l out-l-braces || fail=1
sed -n  'l#foo' in-exp > out-l-hash || fail=1
compare_ exp-l out-l-hash || fail=1


#
# unterminated a/c/i as last command
# (pending_text)
sed -e 'a\' in-exp > out-unterm-a1 || fail=1
compare_ in-exp out-unterm-a1 || fail=1


Exit $fail
