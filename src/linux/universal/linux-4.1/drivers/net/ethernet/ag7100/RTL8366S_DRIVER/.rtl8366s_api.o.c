cmd_drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o := mips-linux-uclibc-gcc -Wp,-MD,drivers/net/ag7100/RTL8366S_DRIVER/.rtl8366s_api.o.d  -nostdinc -isystem /home/xfs/toolchains/toolchain-mips_r2_gcc-linaro_uClibc-0.9.32/bin/../lib/gcc/mips-openwrt-linux-uclibc/4.5.4/include -I/home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include -Iinclude  -include include/generated/autoconf.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0xffffffff80002000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -march=mips32r2 -Wa,-mips32r2 -Wa,--trap -I/home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/mach-ar7100 -I/home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -fconserve-stack  -DMODULE -mno-long-calls -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(rtl8366s_api)"  -D"KBUILD_MODNAME=KBUILD_STR(ag7100_mod)"  -c -o drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.c

deps_drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o := \
  drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.c \
    $(wildcard include/config/buffalo.h) \
  drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_errno.h \
  drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_asicdrv.h \
  drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_types.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
  include/asm-generic/int-ll64.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/posix_types.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/sgidefs.h \
  /home/xfs/toolchains/toolchain-mips_r2_gcc-linaro_uClibc-0.9.32/bin/../lib/gcc/mips-openwrt-linux-uclibc/4.5.4/include/stdarg.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/string.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/cpu/r3000.h) \
  drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.h \
  include/linux/delay.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  include/linux/linkage.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/linkage.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/bitops.h \
    $(wildcard include/config/cpu/mipsr2.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/irqflags.h \
    $(wildcard include/config/mips/mt/smtc.h) \
    $(wildcard include/config/irq/cpu.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/hazards.h \
    $(wildcard include/config/cpu/cavium/octeon.h) \
    $(wildcard include/config/cpu/mipsr1.h) \
    $(wildcard include/config/mach/alchemy.h) \
    $(wildcard include/config/cpu/loongson2.h) \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/r5500.h) \
    $(wildcard include/config/cpu/rm9000.h) \
    $(wildcard include/config/cpu/sb1.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/cpu-features.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/cpu.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/cpu-info.h \
    $(wildcard include/config/mips/mt/smp.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/mach-ar7100/cpu-feature-overrides.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/sgi/ip28.h) \
    $(wildcard include/config/cpu/has/wb.h) \
    $(wildcard include/config/weak/ordering.h) \
    $(wildcard include/config/weak/reordering/beyond/llsc.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/byteorder.h \
  include/linux/byteorder/big_endian.h \
  include/linux/swab.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/war.h \
    $(wildcard include/config/cpu/r4000/workarounds.h) \
    $(wildcard include/config/cpu/r4400/workarounds.h) \
    $(wildcard include/config/cpu/daddi/workarounds.h) \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/mach-ar7100/war.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/dynamic_debug.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/delay.h \
  include/linux/param.h \
  /home/seg/DEV/pb42/src/linux/pb4x/linux-2.6.34.6/arch/mips/include/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \

drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o: $(deps_drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o)

$(deps_drivers/net/ag7100/RTL8366S_DRIVER/rtl8366s_api.o):
