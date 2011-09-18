cmd_coreutils/tty.o := arm-linux-uclibc-gcc -Wp,-MD,coreutils/.tty.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.19.2)" -DBB_BT=AUTOCONF_TIMESTAMP -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os  -DNEED_PRINTF -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(tty)"  -D"KBUILD_MODNAME=KBUILD_STR(tty)" -c -o coreutils/tty.o coreutils/tty.c

deps_coreutils/tty.o := \
  coreutils/tty.c \
    $(wildcard include/config/include/susv2.h) \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/feature/utmp.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/seamless/lzma.h) \
    $(wildcard include/config/feature/seamless/bz2.h) \
    $(wildcard include/config/feature/seamless/gz.h) \
    $(wildcard include/config/feature/seamless/z.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/long/opts.h) \
    $(wildcard include/config/feature/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/echo.h) \
    $(wildcard include/config/printf.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/kill.h) \
    $(wildcard include/config/chown.h) \
    $(wildcard include/config/ls.h) \
    $(wildcard include/config/xxx.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/feature/crond/d.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/feature/adduser/to/group.h) \
    $(wildcard include/config/feature/del/user/from/group.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/pmap.h) \
    $(wildcard include/config/feature/show/threads.h) \
    $(wildcard include/config/feature/ps/additional/columns.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/feature/top/smp/process.h) \
    $(wildcard include/config/killall.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/sestatus.h) \
    $(wildcard include/config/unicode/support.h) \
    $(wildcard include/config/feature/mtab/support.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config/big/endian.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/nommu.h) \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include-fixed/limits.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include-fixed/syslimits.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/limits.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/features.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_config.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/cdefs.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/posix1_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/local_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/linux/limits.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_local_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/posix2_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/xopen_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/stdio_lim.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/byteswap.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/byteswap.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/byteswap-common.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/endian.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/endian.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include/stdint.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/stdint.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/wchar.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/wordsize.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include/stdbool.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/unistd.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/posix_opt.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/environments.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/types.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include/stddef.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/typesizes.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/pthreadtypes.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/confname.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/getopt.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/ctype.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_touplow.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/dirent.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/dirent.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/linux/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/errno-base.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/fcntl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/fcntl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/types.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/time.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/select.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/select.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sigset.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/time.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/sysmacros.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uio.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/stat.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/stat.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/inttypes.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/netdb.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/netinet/in.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/socket.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/uio.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/socket.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sockaddr.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/socket.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/sockios.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/in.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/siginfo.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/netdb.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/setjmp.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/setjmp.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/signal.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/signum.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sigaction.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sigcontext.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/sigcontext.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sigstack.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/ucontext.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/procfs.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/time.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/user.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sigthread.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/stdio.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_stdio.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/wchar.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_mutex.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/pthread.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sched.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/sched.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_clk_tck.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_pthread.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/include/stdarg.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/stdlib.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/waitflags.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/waitstatus.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/alloca.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/string.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/poll.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/poll.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/ioctl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/ioctls.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/ioctls.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/ioctls.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/linux/ioctl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/ioctl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/ioctl.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/ioctl-types.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/ttydefaults.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/mman.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/mman.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/mman-common.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/wait.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/resource.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/resource.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/termios.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/termios.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/param.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/linux/param.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/param.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/mntent.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/paths.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/statfs.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/statfs.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/pwd.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/grp.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/arpa/inet.h \
  include/pwd_.h \
  include/grp_.h \
  include/xatonum.h \

coreutils/tty.o: $(deps_coreutils/tty.o)

$(deps_coreutils/tty.o):
