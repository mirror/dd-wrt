#!/bin/sh

# Test runner for old 'eval' test

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

cat << \EOF > eval-in || framework_failure_
17380: 2 2 5 11 79
abcd
cpu
  abcd
  cpu
EOF

# create a copy of the input file.
# Keep the name 'eval.in2' - it is used in the 'eval' commands in the
# sed program below.
cp eval-in eval.in2 || framework_failure_


# The sed program - containing multiple 'e' (eval) commands.
# NOTE: the program executes 'sed' using 'e' commands - and
#       assumes GNU sed is in the $PATH (which is the case here).
cat << \EOF > eval.sed || framework_failure_
1d

	#Try eval command
	/cpu/!b2
	esed 1q eval.in2

:2
p
i---
h

	#Try eval option
	s,.* *cpu *,sed 1q eval.in2; echo "&",e

:3
p
g
i---

	h
	#Try eval option with print
	s,.* *cpu.*,sed 1q eval.in2,ep
	g


:4
p
i---

$!d

#Do some more tests
s/.*/Doing some more tests -----------------------/p
s,.*,sed 1q eval.in2,ep
i---
s,.*,sed 1q eval.in2,pe
i---
s,.*,sed 1q eval.in2,
h
e
p
g
i---
s/^/echo /ep
i---
s/^fubar$/echo wozthis/e
EOF


# The expected output file
cat << \EOF > eval-exp || framework_failure_
abcd
---
abcd
---
abcd
---
17380: 2 2 5 11 79
cpu
---
17380: 2 2 5 11 79
cpu
---
17380: 2 2 5 11 79
cpu
---
  abcd
---
  abcd
---
  abcd
---
17380: 2 2 5 11 79
  cpu
---
17380: 2 2 5 11 79
  cpu
---
17380: 2 2 5 11 79
  cpu
---
Doing some more tests -----------------------
17380: 2 2 5 11 79
---
sed 1q eval.in2
---
17380: 2 2 5 11 79
---
sed 1q eval.in2
---
sed 1q eval.in2
EOF




sed -f eval.sed eval-in > eval-out || fail=1
remove_cr_inplace eval-out
compare eval-exp eval-out || fail=1


Exit $fail
