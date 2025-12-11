#! /bin/sh
# Test readelf -r on file containing 64K+ functions and sections
# Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
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

# Tests readelf -r on file containing 64K+ functions and sections
# This forces the use of elf_scnshndx to lookup section indexes
# The test makes sure that for relocations against sectio symbols
# the section names are resolved even for section indexes > 64K.
#
# Note the sections are named after the symbols (with a '.' in front).

testrun ${abs_top_builddir}/src/readelf -r \
  ${abs_top_builddir}/tests/manyfuncs.o | grep -F ".y00000" || exit 1

testrun ${abs_top_builddir}/src/readelf -r \
  ${abs_top_builddir}/tests/manyfuncs.o | grep -F ".z00000" || exit 1

testrun ${abs_top_builddir}/src/readelf -r \
  ${abs_top_builddir}/tests/manyfuncs.o | grep -F ".y77777" || exit 1

testrun ${abs_top_builddir}/src/readelf -r \
  ${abs_top_builddir}/tests/manyfuncs.o | grep -F ".z77777" || exit 1

exit 0
