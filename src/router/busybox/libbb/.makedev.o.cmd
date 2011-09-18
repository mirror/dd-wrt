cmd_libbb/makedev.o := arm-linux-uclibc-gcc -Wp,-MD,libbb/.makedev.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.19.2)" -DBB_BT=AUTOCONF_TIMESTAMP -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os  -DNEED_PRINTF -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(makedev)"  -D"KBUILD_MODNAME=KBUILD_STR(makedev)" -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
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
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/sysmacros.h \

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):
