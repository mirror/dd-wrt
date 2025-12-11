#! /bin/sh
# Data race test for parallelizd dwarf-getmacros
# Copyright (C) 2015, 2018 Red Hat, Inc.
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

. $srcdir/thread-safety-subr.sh

check_thread_safety_enabled

testfiles testfile51 testfile-macros testfile-macros-0xff

testrun ${abs_builddir}/eu_search_macros testfile51 0xb
testrun ${abs_builddir}/eu_search_macros testfile51 0x84
testrun ${abs_builddir}/eu_search_macros testfile-macros 0xb
testrun ${abs_builddir}/eu_search_macros testfile-macros-0xff 0xb
