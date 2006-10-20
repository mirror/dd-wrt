cmd_scripts/kconfig/lxdialog/lxdialog.o := gcc -Wp,-MD,scripts/kconfig/lxdialog/.lxdialog.o.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer   -DCURSES_LOC="<ncurses.h>" -DLOCALE    -c -o scripts/kconfig/lxdialog/lxdialog.o scripts/kconfig/lxdialog/lxdialog.c

deps_scripts/kconfig/lxdialog/lxdialog.o := \
  scripts/kconfig/lxdialog/lxdialog.c \
  scripts/kconfig/lxdialog/dialog.h \
  /usr/include/sys/types.h \
  /usr/include/features.h \
  /usr/include/sys/cdefs.h \
  /usr/include/gnu/stubs.h \
  /usr/include/bits/types.h \
  /usr/include/bits/wordsize.h \
  /usr/lib64/gcc/x86_64-suse-linux/4.0.2/include/stddef.h \
  /usr/include/bits/typesizes.h \
  /usr/include/time.h \
  /usr/include/endian.h \
  /usr/include/bits/endian.h \
  /usr/include/sys/select.h \
  /usr/include/bits/select.h \
  /usr/include/bits/sigset.h \
  /usr/include/bits/time.h \
  /usr/include/sys/sysmacros.h \
  /usr/include/bits/pthreadtypes.h \
  /usr/include/bits/sched.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h \
  /usr/include/getopt.h \
  /usr/include/ctype.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/include/bits/string2.h \
  /usr/include/ncurses.h \
  /usr/include/ncurses_dll.h \
  /usr/include/stdio.h \
  /usr/include/libio.h \
  /usr/include/_G_config.h \
  /usr/include/wchar.h \
  /usr/include/bits/wchar.h \
  /usr/include/gconv.h \
  /usr/lib64/gcc/x86_64-suse-linux/4.0.2/include/stdarg.h \
  /usr/include/bits/stdio_lim.h \
  /usr/include/bits/sys_errlist.h \
  /usr/include/bits/stdio.h \
  /usr/include/unctrl.h \
  /usr/include/curses.h \
  /usr/lib64/gcc/x86_64-suse-linux/4.0.2/include/stdbool.h \
  /usr/include/locale.h \
  /usr/include/bits/locale.h \

scripts/kconfig/lxdialog/lxdialog.o: $(deps_scripts/kconfig/lxdialog/lxdialog.o)

$(deps_scripts/kconfig/lxdialog/lxdialog.o):
