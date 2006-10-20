cmd_scripts/basic/fixdep := gcc -Wp,-MD,scripts/basic/.fixdep.d -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer       -o scripts/basic/fixdep scripts/basic/fixdep.c  

deps_scripts/basic/fixdep := \
  scripts/basic/fixdep.c \
    $(wildcard include/config/his/driver.h) \
    $(wildcard include/config/my/option.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/boom.h) \
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
  /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /usr/include/sys/mman.h \
  /usr/include/bits/mman.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix_opt.h \
  /usr/include/bits/confname.h \
  /usr/include/getopt.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/string.h \
  /usr/include/bits/string.h \
  /usr/include/bits/string2.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
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
  /usr/lib64/gcc/x86_64-suse-linux/4.0.2/include/limits.h \
  /usr/lib64/gcc/x86_64-suse-linux/4.0.2/include/syslimits.h \
  /usr/include/limits.h \
  /usr/include/bits/posix1_lim.h \
  /usr/include/bits/local_lim.h \
  /usr/include/linux/limits.h \
  /usr/include/bits/posix2_lim.h \
  /usr/include/ctype.h \
  /usr/include/arpa/inet.h \
  /usr/include/netinet/in.h \
  /usr/include/stdint.h \
  /usr/include/sys/socket.h \
  /usr/include/sys/uio.h \
  /usr/include/bits/uio.h \
  /usr/include/bits/socket.h \
  /usr/include/bits/sockaddr.h \
  /usr/include/asm/socket.h \
  /usr/include/asm-x86_64/socket.h \
  /usr/include/asm/sockios.h \
  /usr/include/asm-x86_64/sockios.h \
  /usr/include/bits/in.h \
  /usr/include/bits/byteswap.h \

scripts/basic/fixdep: $(deps_scripts/basic/fixdep)

$(deps_scripts/basic/fixdep):
