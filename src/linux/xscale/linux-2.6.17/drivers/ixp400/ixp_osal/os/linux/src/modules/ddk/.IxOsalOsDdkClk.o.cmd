cmd_drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o := armeb-linux-uclibc-gcc -Wp,-MD,drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/.IxOsalOsDdkClk.o.d  -nostdinc -isystem /opt/armb-4.1.0/bin/../lib/gcc/armeb-linux-uclibc/4.1.1/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -mbig-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O2 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=apcs-gnu -mno-thumb-interwork -D__LINUX_ARM_ARCH__=5 -march=armv5te -mtune=xscale -Wa,-mcpu=xscale  -msoft-float -Uarm -Wdeclaration-after-statement -Wno-pointer-sign -D__ixp42X -DIX_DEVICE=ixp42X -DIX_PLATFORM=ixdp42x -D__linux -DIX_TARGET=linuxbe -DIX_UTOPIAMODE=0 -DIX_PLATFORM=ixdp42x -DIX_DEVICE=ixp42X -DIX_LINUXVER=2.6 -DIX_MPHYSINGLEPORT=0 -DIX_NPEDL_READ_MICROCODE_FROM_FILE -DIX_ACC_DRAM_PHYS_OFFSET=0 -D__LINUX_ARM_ARCH__=5 -DCPU=33 -DXSCALE=33 -DSIMSPARCSOLARIS=0 -DNDEBUG -Idrivers/ixp400 -Idrivers/ixp400/ixp400_xscale_sw/src/include -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/core/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/ioMem/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/core/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/ddk/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/bufferMgt/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/platforms/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/platforms/ixp400/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/platforms/ixp400/ixp425/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/bufferMgt/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/ioMem/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/core/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/platforms/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/platforms/ixp400/ -Idrivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/platforms/ixp400/ixp425/ -Idrivers/ixp400/ixp400_xscale_sw/src/include -Idrivers/ixp400/ixp400_xscale_sw/src/errHdlAcc/include -Idrivers/ixp400/ixp400_xscale_sw/src/ethAcc/include -Idrivers/ixp400/ixp400_xscale_sw/src/hssAcc/include -Idrivers/ixp400/ixp400_xscale_sw/src/ethDB/include -Idrivers/ixp400/ixp400_xscale_sw/src/featureCtrl/include -Idrivers/ixp400/ixp400_xscale_sw/src/npeMh/include -Idrivers/ixp400/ixp400_xscale_sw/src/npeDl/include -Idrivers/ixp400/ixp400_xscale_sw/src/qmgr -Idrivers/ixp400/ixp400_xscale_sw/src/ -Idrivers/ixp400/ixp400_xscale_sw/src/ethDB  -DMODULE -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(IxOsalOsDdkClk)"  -D"KBUILD_MODNAME=KBUILD_STR(ixp400)" -c -o drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.c

deps_drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o := \
  drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.c \
  include/asm/system.h \
    $(wildcard include/config/cpu/xsc3.h) \
    $(wildcard include/config/cpu/xscale.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/linux/delay.h \
  include/asm/delay.h \
  include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/time.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/posix_types.h \
  include/asm/types.h \
  include/linux/seqlock.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/preempt.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm/thread_info.h \
  include/asm/fpstate.h \
    $(wildcard include/config/iwmmxt.h) \
  include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
  /opt/armb-4.1.0/bin/../lib/gcc/armeb-linux-uclibc/4.1.1/include/stdarg.h \
  include/asm/byteorder.h \
  include/linux/byteorder/big_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm-generic/bug.h \
  include/linux/stringify.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm-generic/atomic.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsal.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsalTypes.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/core/IxOsalOsTypes.h \
    $(wildcard include/config/cpu/ixp46x.h) \
    $(wildcard include/config/arch/ixp465.h) \
  include/asm/semaphore.h \
  include/linux/wait.h \
  include/linux/list.h \
  include/linux/prefetch.h \
  include/asm/processor.h \
    $(wildcard include/config/mmu.h) \
  include/asm/procinfo.h \
  include/asm/cache.h \
  include/asm/current.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/asm/locks.h \
  include/linux/version.h \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/pm.h) \
  include/linux/capability.h \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
  include/asm/timex.h \
  include/asm/arch/timex.h \
  include/asm/hardware.h \
  include/asm/arch/hardware.h \
  include/asm/arch/ixp4xx-regs.h \
    $(wildcard include/config/mach/gtwx5715.h) \
  include/asm/arch/platform.h \
  include/asm/arch/ixdp425.h \
  include/asm/arch/coyote.h \
  include/asm/arch/prpmc1100.h \
  include/asm/arch/nslu2.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/linux/rbtree.h \
  include/linux/cpumask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/nodemask.h \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/asm/page.h \
    $(wildcard include/config/cpu/copy/v3.h) \
    $(wildcard include/config/cpu/copy/v4wt.h) \
    $(wildcard include/config/cpu/copy/v4wb.h) \
    $(wildcard include/config/cpu/copy/v6.h) \
    $(wildcard include/config/aeabi.h) \
  include/asm/glue.h \
    $(wildcard include/config/cpu/arm610.h) \
    $(wildcard include/config/cpu/arm710.h) \
    $(wildcard include/config/cpu/abrt/lv4t.h) \
    $(wildcard include/config/cpu/abrt/ev4.h) \
    $(wildcard include/config/cpu/abrt/ev4t.h) \
    $(wildcard include/config/cpu/abrt/ev5tj.h) \
    $(wildcard include/config/cpu/abrt/ev5t.h) \
    $(wildcard include/config/cpu/abrt/ev6.h) \
  include/asm/memory.h \
    $(wildcard include/config/discontigmem.h) \
  include/asm/arch/memory.h \
  include/asm/sizes.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/out/of/line/pfn/to/page.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/page.h \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/smp.h \
  include/linux/sem.h \
    $(wildcard include/config/sysvipc.h) \
  include/linux/ipc.h \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
  include/asm-generic/signal.h \
  include/asm/sigcontext.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/rcupdate.h \
  include/linux/cache.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/dma/is/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/acpi/hotplug/memory.h) \
    $(wildcard include/config/acpi/hotplug/memory/module.h) \
  include/linux/memory_hotplug.h \
  include/linux/notifier.h \
  include/linux/mutex.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/asm/topology.h \
  include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/large/allocs.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/futex.h \
    $(wildcard include/config/futex.h) \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
  include/linux/hrtimer.h \
    $(wildcard include/config/no/idle/hz.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/kthread.h \
  include/linux/err.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsalAssert.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/core/IxOsalOsAssert.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsalConfig.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/ioMem/IxOsalIoMem.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/ioMem/IxOsalEndianess.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/ioMem/IxOsalMemAccess.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/bufferMgt/IxOsalBufferMgt.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsal.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/modules/bufferMgt/IxOsalOsBufferMgt.h \
  include/linux/skbuff.h \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/vlan/8021q/module.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/nf/conntrack/module.h) \
    $(wildcard include/config/imq.h) \
    $(wildcard include/config/imq/module.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/have/arch/dev/alloc/skb.h) \
    $(wildcard include/config/highmem.h) \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
    $(wildcard include/config/security.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kref.h \
  include/linux/radix-tree.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
    $(wildcard include/config/64bit.h) \
  include/asm/pgtable.h \
  include/asm-generic/4level-fixup.h \
  include/asm/proc-fns.h \
    $(wildcard include/config/cpu/32.h) \
    $(wildcard include/config/cpu/arm720t.h) \
    $(wildcard include/config/cpu/arm920t.h) \
    $(wildcard include/config/cpu/arm922t.h) \
    $(wildcard include/config/cpu/arm925t.h) \
    $(wildcard include/config/cpu/arm926t.h) \
    $(wildcard include/config/cpu/arm1020.h) \
    $(wildcard include/config/cpu/arm1020e.h) \
    $(wildcard include/config/cpu/arm1022.h) \
    $(wildcard include/config/cpu/arm1026.h) \
    $(wildcard include/config/cpu/v6.h) \
  include/asm/cpu-single.h \
  include/asm/arch/vmalloc.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/swap.h) \
  include/linux/highmem.h \
  include/asm/cacheflush.h \
    $(wildcard include/config/cpu/cache/vipt.h) \
    $(wildcard include/config/cpu/cache/vivt.h) \
  include/asm/shmparam.h \
  include/linux/poll.h \
  include/asm/poll.h \
  include/asm/uaccess.h \
  include/linux/net.h \
  include/asm/socket.h \
  include/asm/sockios.h \
  include/linux/sysctl.h \
  include/linux/textsearch.h \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/asm/elf.h \
  include/asm/user.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm-generic/local.h \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/asm/irq.h \
  include/asm/arch/irqs.h \
  include/linux/irq_cpustat.h \
  include/asm/module.h \
  include/net/checksum.h \
  include/asm/checksum.h \
  include/linux/in6.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/modules/bufferMgt/IxOsalBufferMgtDefault.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/platforms/ixp400/IxOsalOem.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsalTypes.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/platforms/ixp400/IxOsalOsIxp400.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/platforms/ixp400/ixp425/IxOsalOsIxp425Sys.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/include/IxOsalUtilitySymbols.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/core/IxOsalOsUtilitySymbols.h \
  drivers/ixp400/ixp400_xscale_sw/../ixp_osal/os/linux/include/core/IxOsalOs.h \
  include/linux/dma-mapping.h \
  include/linux/device.h \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
  include/asm/dma-mapping.h \
    $(wildcard include/config/dmabounce.h) \
  include/asm/scatterlist.h \
  include/asm/io.h \
  include/asm/arch/io.h \
    $(wildcard include/config/ixp4xx/indirect/pci.h) \
  include/asm/pgalloc.h \
  include/asm/pgtable-hwdef.h \
  include/asm/tlbflush.h \
    $(wildcard include/config/cpu/tlb/v3.h) \
    $(wildcard include/config/cpu/tlb/v4wt.h) \
    $(wildcard include/config/cpu/tlb/v4wbi.h) \
    $(wildcard include/config/cpu/tlb/v4wb.h) \
    $(wildcard include/config/cpu/tlb/v6.h) \

drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o: $(deps_drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o)

$(deps_drivers/ixp400/ixp_osal/os/linux/src/modules/ddk/IxOsalOsDdkClk.o):
