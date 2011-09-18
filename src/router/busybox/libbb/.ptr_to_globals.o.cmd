cmd_libbb/ptr_to_globals.o := arm-linux-uclibc-gcc -Wp,-MD,libbb/.ptr_to_globals.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.19.2)" -DBB_BT=AUTOCONF_TIMESTAMP -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wunused-function -Wunused-value -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Wold-style-definition -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os  -DNEED_PRINTF -Os -pipe -march=armv6k -mtune=mpcore -mfloat-abi=softfp -mfpu=vfp -fno-caller-saves    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ptr_to_globals)"  -D"KBUILD_MODNAME=KBUILD_STR(ptr_to_globals)" -c -o libbb/ptr_to_globals.o libbb/ptr_to_globals.c

deps_libbb/ptr_to_globals.o := \
  libbb/ptr_to_globals.c \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/features.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/uClibc_config.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/sys/cdefs.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/bits/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/linux/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/errno.h \
  /home/xfs/toolchains/toolchain-arm_v6k_gcc-linaro_uClibc-0.9.32_eabi/bin/../lib/gcc/arm-openwrt-linux-uclibcgnueabi/4.5.4/../../../../arm-openwrt-linux-uclibcgnueabi/sys-include/asm-generic/errno-base.h \

libbb/ptr_to_globals.o: $(deps_libbb/ptr_to_globals.o)

$(deps_libbb/ptr_to_globals.o):
