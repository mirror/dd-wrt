#! /bin/sh
# Data race test for parallelized dwarf-die-addr-die
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

die_test_files=("testfile-debug-types"
                "testfile_multi_main" "testfile_multi.dwz"
                "testfilebazdbgppc64.debug"
                "testfile-dwarf-4" "testfile-dwarf-5"
                "testfile-splitdwarf-4" "testfile-hello4.dwo" "testfile-world4.dwo"
                "testfile-splitdwarf-5" "testfile-hello5.dwo" "testfile-world5.dwo")

testfiles "${die_test_files[@]}"

for file in "${die_test_files[@]}"; do
    testrun ${abs_builddir}/eu_search_die $file
done
