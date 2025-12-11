#! /bin/sh
# Copyright (C) 2024 Mark J. Wielaard <mark@klomp.org>
# This file is part of elfutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# elfutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. $srcdir/test-subr.sh

# The test files are the native funcretval_test_struct files
# funcretval_test_struct.c
# See also run-funcretval-struct-native.sh

testfiles funcretval_test_struct_riscv

testrun_compare ${abs_top_builddir}/tests/funcretval \
	-e funcretval_test_struct_riscv <<\EOF
() main: return value location: {0x5a, 0}
() dmkpt: return value location: {0x90, 0x2a} {0x93, 0x8} {0x90, 0x2b} {0x93, 0x8}
() mkpt: return value location: {0x90, 0x2a}
() ldiv: return value location: {0x5a, 0} {0x93, 0x8} {0x5b, 0} {0x93, 0x8}
() div: return value location: {0x5a, 0}
EOF

exit 0
