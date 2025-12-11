#! /bin/sh
# Copyright (C) 1999, 2000, 2002, 2004, 2005, 2007 Red Hat, Inc.
# This file is part of elfutils.
# Written by Ulrich Drepper <drepper@redhat.com>, 1999.
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

testfiles testfile testfile2 testfile-define-file

testrun_compare ${abs_builddir}/get-files testfile testfile2 <<\EOF
cuhl = 11, o = 0, asz = 4, osz = 4, ncu = 191
 dirs[0] = "/home/drepper/gnu/new-bu/build/ttt"
 file[0] = "???"
 file[1] = "/home/drepper/gnu/new-bu/build/ttt/m.c"
cuhl = 11, o = 114, asz = 4, osz = 4, ncu = 5617
 dirs[0] = "/home/drepper/gnu/new-bu/build/ttt"
 file[0] = "???"
 file[1] = "/home/drepper/gnu/new-bu/build/ttt/b.c"
 file[2] = "/usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h"
 file[3] = "/usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h"
 file[4] = "/usr/include/bits/types.h"
 file[5] = "/usr/include/bits/sched.h"
 file[6] = "/usr/include/bits/pthreadtypes.h"
 file[7] = "/usr/include/stdio.h"
 file[8] = "/usr/include/libio.h"
 file[9] = "/usr/include/wchar.h"
 file[10] = "/usr/include/_G_config.h"
 file[11] = "/usr/include/gconv.h"
cuhl = 11, o = 412, asz = 4, osz = 4, ncu = 5752
 dirs[0] = "/home/drepper/gnu/new-bu/build/ttt"
 file[0] = "???"
 file[1] = "/home/drepper/gnu/new-bu/build/ttt/f.c"
cuhl = 11, o = 0, asz = 4, osz = 4, ncu = 2418
 dirs[0] = "/shoggoth/drepper"
 file[0] = "???"
 file[1] = "/shoggoth/drepper/b.c"
 file[2] = "/home/geoffk/objs/laurel-000912-branch/lib/gcc-lib/powerpc-unknown-linux-gnu/2.96-laurel-000912/include/stddef.h"
 file[3] = "/home/geoffk/objs/laurel-000912-branch/lib/gcc-lib/powerpc-unknown-linux-gnu/2.96-laurel-000912/include/stdarg.h"
 file[4] = "/shoggoth/drepper/<built-in>"
 file[5] = "/usr/include/bits/types.h"
 file[6] = "/usr/include/stdio.h"
 file[7] = "/usr/include/libio.h"
 file[8] = "/usr/include/_G_config.h"
cuhl = 11, o = 213, asz = 4, osz = 4, ncu = 2521
 dirs[0] = "/shoggoth/drepper"
 file[0] = "???"
 file[1] = "/shoggoth/drepper/f.c"
cuhl = 11, o = 267, asz = 4, osz = 4, ncu = 2680
 dirs[0] = "/shoggoth/drepper"
 file[0] = "???"
 file[1] = "/shoggoth/drepper/m.c"
EOF

# see tests/testfile-dwarf-45.source
testfiles testfile-splitdwarf-4 testfile-hello4.dwo testfile-world4.dwo
testfiles testfile-splitdwarf-5 testfile-hello5.dwo testfile-world5.dwo

testrun_compare ${abs_builddir}/get-files testfile-splitdwarf-4 testfile-hello4.dwo testfile-world4.dwo <<\EOF
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 52
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include"
 file[0] = "???"
 file[1] = "/home/mark/src/elfutils/tests/hello.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include/stddef.h"
cuhl = 11, o = 26, asz = 8, osz = 4, ncu = 104
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/mark/src/elfutils/tests/world.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/usr/include/stdlib.h"
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 414
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include"
 file[0] = "???"
 file[1] = "/home/mark/src/elfutils/tests/hello.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include/stddef.h"
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 331
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/mark/src/elfutils/tests/world.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/usr/include/stdlib.h"
EOF

testrun_compare ${abs_builddir}/get-files testfile-splitdwarf-5 testfile-hello5.dwo testfile-world5.dwo <<\EOF
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 53
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include"
 file[0] = "/home/mark/src/elfutils/tests/hello.c"
 file[1] = "/home/mark/src/elfutils/tests/hello.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include/stddef.h"
cuhl = 20, o = 21, asz = 8, osz = 4, ncu = 106
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/mark/src/elfutils/tests/world.c"
 file[1] = "/home/mark/src/elfutils/tests/world.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/usr/include/stdlib.h"
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 386
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include"
 file[0] = "/home/mark/src/elfutils/tests/hello.c"
 file[1] = "/home/mark/src/elfutils/tests/hello.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/opt/local/install/gcc/lib/gcc/x86_64-pc-linux-gnu/9.0.0/include/stddef.h"
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 296
 dirs[0] = "/home/mark/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/mark/src/elfutils/tests/world.c"
 file[1] = "/home/mark/src/elfutils/tests/world.c"
 file[2] = "/home/mark/src/elfutils/tests/hello.h"
 file[3] = "/usr/include/stdlib.h"
EOF

# See testfile-dwp.source.
testfiles testfile-dwp-5 testfile-dwp-5.dwp
testfiles testfile-dwp-4 testfile-dwp-4.dwp

testrun_compare ${abs_builddir}/get-files testfile-dwp-5 << EOF
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 53
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[1] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 20, o = 21, asz = 8, osz = 4, ncu = 106
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[1] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 20, o = 42, asz = 8, osz = 4, ncu = 155
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/main.cc"
 file[1] = "/home/osandov/src/elfutils/tests/main.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
EOF

# Note that this one includes the type units in .debug_info.dwo as expected.
testrun_compare ${abs_builddir}/get-files testfile-dwp-5.dwp << EOF
cuhl = 24, o = 0, asz = 8, osz = 4, ncu = 112
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[1] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 376
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[1] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 24, o = 0, asz = 8, osz = 4, ncu = 486
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[1] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 606
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[1] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 20, o = 0, asz = 8, osz = 4, ncu = 1009
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "/home/osandov/src/elfutils/tests/main.cc"
 file[1] = "/home/osandov/src/elfutils/tests/main.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
EOF

testrun_compare ${abs_builddir}/get-files testfile-dwp-4 << EOF
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 56
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 11, o = 29, asz = 8, osz = 4, ncu = 108
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 11, o = 55, asz = 8, osz = 4, ncu = 160
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/main.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
EOF

testrun_compare ${abs_builddir}/get-files testfile-dwp-4.dwp << EOF
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 286
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/foo.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 404
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/bar.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
cuhl = 11, o = 0, asz = 8, osz = 4, ncu = 857
 dirs[0] = "/home/osandov/src/elfutils/tests"
 dirs[1] = "/usr/include"
 file[0] = "???"
 file[1] = "/home/osandov/src/elfutils/tests/main.cc"
 file[2] = "/home/osandov/src/elfutils/tests/foobar.h"
 file[3] = "/usr/include/stdc-predef.h"
EOF

tempfiles files define-files.out get-files-define-file.out

cat > files <<\EOF
 dirs[0] = "session"
 dirs[1] = "/home/wcohen/minimal_mod"
 dirs[2] = "include/asm"
 dirs[3] = "include/linux"
 dirs[4] = "include/asm-generic"
 file[0] = "???"
 file[1] = "/home/wcohen/minimal_mod/minimal_mod.c"
 file[2] = "include/asm/gcc_intrin.h"
 file[3] = "include/linux/kernel.h"
 file[4] = "include/asm/processor.h"
 file[5] = "include/asm/types.h"
 file[6] = "include/asm/ptrace.h"
 file[7] = "include/linux/sched.h"
 file[8] = "include/asm/thread_info.h"
 file[9] = "include/linux/thread_info.h"
 file[10] = "include/asm/atomic.h"
 file[11] = "include/linux/list.h"
 file[12] = "include/linux/cpumask.h"
 file[13] = "include/linux/rbtree.h"
 file[14] = "include/asm/page.h"
 file[15] = "include/linux/rwsem.h"
 file[16] = "include/asm/rwsem.h"
 file[17] = "include/asm/spinlock.h"
 file[18] = "include/linux/completion.h"
 file[19] = "include/linux/wait.h"
 file[20] = "include/linux/aio.h"
 file[21] = "include/linux/workqueue.h"
 file[22] = "include/linux/timer.h"
 file[23] = "include/linux/types.h"
 file[24] = "include/asm/posix_types.h"
 file[25] = "include/linux/pid.h"
 file[26] = "include/linux/time.h"
 file[27] = "include/linux/capability.h"
 file[28] = "include/linux/signal.h"
 file[29] = "include/linux/resource.h"
 file[30] = "include/linux/sem.h"
 file[31] = "include/asm/fpu.h"
 file[32] = "include/linux/fs_struct.h"
 file[33] = "include/asm/signal.h"
 file[34] = "include/asm/siginfo.h"
 file[35] = "include/asm-generic/siginfo.h"
 file[36] = "include/asm/nodedata.h"
 file[37] = "include/linux/mmzone.h"
 file[38] = "include/linux/jiffies.h"
 file[39] = "include/asm/io.h"
 file[40] = "include/asm/machvec.h"
 file[41] = "include/asm/smp.h"
 file[42] = "include/asm/numa.h"
 file[43] = "include/linux/slab.h"
EOF

# Files should be printed 3 times, followed by the two files from
# DW_LNE_define_file
cat files > define-files.out
cat files >> define-files.out
cat files >> define-files.out
echo ' file[44] = "include/asm/abc.c"' >> define-files.out
echo ' file[45] = "include/linux/01.c"' >> define-files.out

# testfile-define-file is a copy of testfile36.debug but with a modified
# line program.  The line program in testfile-define-file consists of
# two DW_LNE_define_file opcodes.
#
# xxd was used to create a hexdump of testfile36.debug and a text editor
# was used to modify the line program.  The modified hexdump was converted
# back to a binary with xxd -r.
cat define-files.out | testrun_compare ${abs_builddir}/get-files-define-file testfile-define-file

exit 0
