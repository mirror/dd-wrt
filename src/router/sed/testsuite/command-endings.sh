#!/bin/sh
# Test command separators and endings

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

# Allowed endings/separators after most commands:
#   newline, comment, closing brace, semicolon, EOF
# they are also allowed after opening and closing braces themselves.
#
# Not tested here:
#   r/R/w/R/e and s///[we] which use read_filename() and do not
#   accept comments or semicolons.



# Test commands and braces followed by:
# closing braces, comment, semicolons, EOF (newlines are tested later).
#
# sed-4.3 wrongly rejected y/// followed by '}' or '#' (bug#22460).
#
# Implementation notes (see compile.c):
#   Simple commands, '}', and 'y///' commands use read_end_of_cmd().
#
#   q/Q/l/L have additional check for optional integer,
#   then call read_end_of_cmd().
#
#   labels use 'read_label()'.
#
#   's///' has special handling, depending on additional flags
#    (with 's///[we]' commands and semicolons are not allowed).
#   Implemented in mark_subst_opts().
#
for p in \
    'h'         \
    'h;'        \
    'h ;'       \
    'h# foo'    \
    'h # foo'   \
    '{h}'       \
    '{h } '     \
    '{ h } '    \
    \
    '{h}# foo'  \
    '{h} # foo' \
    '{h};'      \
    '{h} ;'     \
    '{;h;} '    \
    '{{h}}'     \
    '{;{h};}'   \
    \
    'y/1/a/'    \
    'y/1/a/;d'  \
    'y/1/a/ ;d' \
    '{y/1/a/}'  \
    'y/1/a/#foo'\
    'y/1/a/ #fo'\
    \
    's/1/a/'    \
    's/1/a/;d'  \
    's/1/a/ ;d' \
    '{s/1/a/}'  \
    's/1/a/#foo'\
    's/1/a/ #fo'\
    \
    's/1/a/i ;' \
    's/1/a/i #foo' \
    '{ s/1/a/i }' \
    \
    'bx; :x'      \
    'bx; :x;'     \
    'bx; :x ;'    \
    'bx; :x#foo'  \
    'bx; :x #foo' \
    '{ bx; :x }'  \
    \
    'l'           \
    'l;'          \
    'l ;'         \
    'l#foo'       \
    'l #foo'      \
    '{l}'         \
    '{l }'        \
    'l1'          \
    'l1;'         \
    'l1 ;'        \
    'l1#foo'      \
    'l1 #foo'     \
    '{l1}'        \
    '{l1 }'       \
    ;
do
  sed -n "$p" < /dev/null >out 2>err || fail=1
  compare /dev/null err || fail=1
  compare /dev/null out || fail=1
done


# Create files to test newlines after commands
# (instead of having to embed newlines in shell variables in a portable way)
printf 'd\n'         > nl1 || framework_failure_
printf '{\nd}'       > nl2 || framework_failure_
printf '{d\n}'       > nl3 || framework_failure_
printf '{d}\n'       > nl4 || framework_failure_
printf 'y/1/a/\n'    > nl5 || framework_failure_
printf 's/1/a/\n'    > nl6 || framework_failure_
printf 'bx\n:x\n'    > nl7 || framework_failure_
printf 'l\n'         > nl8 || framework_failure_
printf 'l1\n'        > nl9 || framework_failure_
# s/// has special allowance for \r in mark_subst_opts(),
# even if not on windows.
# TODO: should other commands allow it ?
printf 's/1/a/\r\n'  > nl10 || framework_failure_

for i in 1 2 3 4 5 6 7 8 9 10 ;
do
  sed -n -f "nl$i" </dev/null >out 2>err || fail=1
  compare /dev/null err || fail=1
  compare /dev/null out || fail=1
done


Exit $fail
