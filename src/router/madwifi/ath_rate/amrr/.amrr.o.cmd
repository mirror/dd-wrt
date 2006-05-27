cmd_/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o := mipsel-linux-gcc -Wp,-MD,/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/.amrr.o.d  -nostdinc -isystem /opt/staging_dir_mipsel.old/bin/../lib/gcc/mipsel-linux-uclibc/4.1.0/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer  -G 0 -mno-abicalls -fno-pic -pipe   -msoft-float  -mabi=32 -march=4kc -Wa,-32 -Wa,-march=4kc -Wa,-mips32 -Wa,--trap -Iinclude/asm-mips/rc32434 -Iinclude/asm-mips/mach-generic -funit-at-a-time -Wdeclaration-after-statement -Wno-pointer-sign -include /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../include/compat.h -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../include -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../hal -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../hal/linux -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../ath -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211 -I/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../.. -O2 -pipe -mips32 -mtune=mips32 -funit-at-a-time  -Werror -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN -O2 -G 0 -mno-abicalls -fno-pic -pipe -mips32 -mtune=mips32 -funit-at-a-time -Wa,--trap -fno-strict-aliasing -fno-common -fomit-frame-pointer -mlong-calls -DATH_SUPERG_FF=1 -DATH_SUPERG_DYNTURBO=1 -DATH_TURBO_SCAN=1 -DATH_SUPERG_XR=1 -Werror -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN -O2 -G 0 -mno-abicalls -fno-pic -pipe -mips32 -mtune=mips32 -funit-at-a-time -Wa,--trap -fno-strict-aliasing -fno-common -fomit-frame-pointer -mlong-calls -DATH_SUPERG_FF=1 -DATH_SUPERG_DYNTURBO=1 -DATH_TURBO_SCAN=1 -DATH_SUPERG_XR=1 -Werror -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN -O2 -G 0 -mno-abicalls -fno-pic -pipe -mips32 -mtune=mips32 -funit-at-a-time -Wa,--trap -fno-strict-aliasing -fno-common -fomit-frame-pointer -mlong-calls -DATH_SUPERG_FF=1 -DATH_SUPERG_DYNTURBO=1 -DATH_TURBO_SCAN=1 -DATH_SUPERG_XR=1  -DMODULE -mlong-calls -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(amrr)"  -D"KBUILD_MODNAME=KBUILD_STR(ath_rate_amrr)" -c -o /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.c

deps_/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o := \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.c \
    $(wildcard include/config/sysctl.h) \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../include/compat.h \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/linux/version.h \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
  include/linux/sched.h \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/debug/mutexes.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/pm.h) \
  include/asm/param.h \
  include/asm-mips/mach-generic/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/capability.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/posix_types.h \
  include/asm/sgidefs.h \
  include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
    $(wildcard include/config/64bit.h) \
    $(wildcard include/config/lbd.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
    $(wildcard include/config/cpu/mips32.h) \
    $(wildcard include/config/cpu/mips64.h) \
    $(wildcard include/config/32bit.h) \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm/break.h \
  include/asm-generic/bug.h \
  include/asm/byteorder.h \
    $(wildcard include/config/cpu/mipsr2.h) \
  include/linux/byteorder/little_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/cpu-features.h \
    $(wildcard include/config/mips/mt.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  include/asm/cpu.h \
  include/asm/cpu-info.h \
    $(wildcard include/config/sgi/ip27.h) \
  include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  include/asm-mips/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  include/asm-mips/mach-generic/cpu-feature-overrides.h \
  include/asm/interrupt.h \
    $(wildcard include/config/irq/cpu.h) \
  include/asm/hazards.h \
    $(wildcard include/config/cpu/rm9000.h) \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/sb1.h) \
  include/asm/war.h \
    $(wildcard include/config/sgi/ip22.h) \
    $(wildcard include/config/sni/rm200/pci.h) \
    $(wildcard include/config/cpu/r5432.h) \
    $(wildcard include/config/sb1/pass/1/workarounds.h) \
    $(wildcard include/config/sb1/pass/2/workarounds.h) \
    $(wildcard include/config/mips/malta.h) \
    $(wildcard include/config/mips/atlas.h) \
    $(wildcard include/config/mips/sead.h) \
    $(wildcard include/config/cpu/tx49xx.h) \
    $(wildcard include/config/momenco/jaguar/atx.h) \
    $(wildcard include/config/pmc/yosemite.h) \
    $(wildcard include/config/momenco/ocelot/3.h) \
  include/asm/thread_info.h \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/page/size/8kb.h) \
    $(wildcard include/config/page/size/16kb.h) \
    $(wildcard include/config/page/size/64kb.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/processor.h \
    $(wildcard include/config/cpu/has/prefetch.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/asm/cachectl.h \
  include/asm/mipsregs.h \
    $(wildcard include/config/cpu/vr41xx.h) \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/asm/prefetch.h \
  include/asm/system.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
  include/asm/addrspace.h \
    $(wildcard include/config/cpu/r4300.h) \
    $(wildcard include/config/cpu/r4x00.h) \
    $(wildcard include/config/cpu/r5000.h) \
    $(wildcard include/config/cpu/nevada.h) \
    $(wildcard include/config/cpu/r8000.h) \
    $(wildcard include/config/cpu/sb1a.h) \
  include/asm-mips/mach-generic/spaces.h \
    $(wildcard include/config/dma/noncoherent.h) \
  include/asm/dsp.h \
  include/asm/ptrace.h \
  include/asm/isadep.h \
    $(wildcard include/config/cpu/r3000.h) \
    $(wildcard include/config/cpu/tx39xx.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
  /opt/staging_dir_mipsel.old/bin/../lib/gcc/mipsel-linux-uclibc/4.1.0/include/stdarg.h \
  include/linux/stringify.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm-generic/atomic.h \
  include/asm/current.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/asm/timex.h \
  include/asm-mips/mach-generic/timex.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/asm/compiler.h \
  include/linux/rbtree.h \
  include/linux/cpumask.h \
  include/linux/bitmap.h \
  include/linux/string.h \
  include/asm/string.h \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/nodemask.h \
  include/linux/numa.h \
    $(wildcard include/config/flatmem.h) \
  include/asm/semaphore.h \
  include/linux/wait.h \
  include/linux/list.h \
  include/linux/prefetch.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/asm/page.h \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/limited/dma.h) \
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
    $(wildcard include/config/trad/signals.h) \
    $(wildcard include/config/binfmt/irix.h) \
  include/asm/sigcontext.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
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
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
  include/linux/cache.h \
    $(wildcard include/config/x86.h) \
    $(wildcard include/config/sparc64.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/parisc.h) \
  include/linux/init.h \
    $(wildcard include/config/hotplug.h) \
  include/linux/memory_hotplug.h \
  include/linux/notifier.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
  include/asm/topology.h \
  include/asm-mips/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/kmalloc_sizes.h \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/large/allocs.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rcupdate.h \
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
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/asm/elf.h \
    $(wildcard include/config/mips32/n32.h) \
    $(wildcard include/config/mips32/o32.h) \
    $(wildcard include/config/mips32/compat.h) \
  include/linux/kobject.h \
    $(wildcard include/config/net.h) \
  include/linux/sysfs.h \
    $(wildcard include/config/sysfs.h) \
  include/linux/kref.h \
  include/linux/moduleparam.h \
  include/asm/local.h \
  include/asm/module.h \
    $(wildcard include/config/cpu/mips32/r1.h) \
    $(wildcard include/config/cpu/mips32/4kc.h) \
    $(wildcard include/config/cpu/mips32/r2.h) \
    $(wildcard include/config/cpu/mips64/r1.h) \
    $(wildcard include/config/cpu/mips64/r2.h) \
    $(wildcard include/config/cpu/r6000.h) \
    $(wildcard include/config/cpu/rm7000.h) \
  include/asm/uaccess.h \
  include/asm-generic/uaccess.h \
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
  include/linux/mm.h \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
    $(wildcard include/config/security.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/radix-tree.h \
  include/linux/mutex.h \
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
  include/linux/err.h \
  include/asm/pgtable.h \
  include/asm/pgtable-32.h \
  include/asm/fixmap.h \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm/io.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm/pgtable-bits.h \
    $(wildcard include/config/mips/uncached.h) \
  include/asm-mips/mach-generic/ioremap.h \
  include/asm-mips/mach-generic/mangle-port.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/swap.h) \
  include/linux/highmem.h \
  include/asm/cacheflush.h \
  include/linux/poll.h \
  include/asm/poll.h \
  include/linux/net.h \
  include/asm/socket.h \
  include/asm/sockios.h \
  include/linux/sysctl.h \
  include/linux/textsearch.h \
  include/net/checksum.h \
  include/asm/checksum.h \
  include/linux/in6.h \
  include/asm/unaligned.h \
  include/asm-generic/unaligned.h \
  include/linux/netdevice.h \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/ax25/module.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/ipv6/module.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/net/divert.h) \
    $(wildcard include/config/netpoll/trap.h) \
  include/linux/if.h \
  include/linux/socket.h \
    $(wildcard include/config/compat.h) \
  include/linux/sockios.h \
  include/linux/uio.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/if_packet.h \
  include/linux/device.h \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
  include/linux/interrupt.h \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/generic/irq/probe.h) \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/irqbalance.h) \
    $(wildcard include/config/pci/msi.h) \
    $(wildcard include/config/auto/irq/affinity.h) \
  include/asm/irq.h \
    $(wildcard include/config/i8259.h) \
  include/asm-mips/rc32434/irq.h \
  include/asm/hw_irq.h \
  include/linux/profile.h \
  include/linux/irq_cpustat.h \
  include/linux/random.h \
  include/linux/delay.h \
  include/asm/delay.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/if_arp.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/if_media.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_linux.h \
    $(wildcard include/config/net/wireless.h) \
  include/linux/wireless.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../include/sys/queue.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_var.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/_ieee80211.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_crypto.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_ioctl.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_node.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_proto.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_power.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../net80211/ieee80211_scan.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../ath/if_athrate.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../ath/if_athvar.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../hal/ah.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../hal/linux/ah_osdep.h \
    $(wildcard include/config/regparm.h) \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../ath/if_athioctl.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../hal/ah_desc.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../release.h \
  /home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/../../svnversion.h \

/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o: $(deps_/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o)

$(deps_/home/dd-wrt/mikrotik/router/madwifi-ng-merge/madwifi.dev/ath_rate/amrr/amrr.o):
