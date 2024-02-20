cmd_/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o := x86_64-linux-uclibc-gcc -Wp,-MMD,/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/.main.o.d  -nostdinc -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -fmacro-prefix-map=./= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu11 -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fcf-protection=none -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -Wno-sign-compare -fno-asynchronous-unwind-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-address-of-packed-member -O2 -fno-allow-store-data-races -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -Wframe-larger-than=1024 -fno-stack-protector -Wno-main -Wno-unused-but-set-variable -Wno-unused-const-variable -Wno-dangling-pointer -fomit-frame-pointer -ftrivial-auto-var-init=zero -fno-stack-clash-protection -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wcast-function-type -Wno-stringop-truncation -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized -Wno-array-bounds -Wno-alloc-size-larger-than -Wimplicit-fallthrough=5 -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=incompatible-pointer-types -Werror=designated-init -Wno-packed-not-aligned -flto=32 -g -I/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include -I/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib -I/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../libre -I/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include -DHAVE_CONFIG_H -DNDPI_LIB_COMPILATION -DOPENDPI_NETFILTER_MODULE -DNDPI_DETECTION_SUPPORT_IPV6 -DNDPI_STATIC=static -Wno-unused-function -Wno-declaration-after-statement -femit-struct-debug-detailed=any  -DMODULE  -DKBUILD_BASENAME='"main"' -DKBUILD_MODNAME='"xt_ndpi"' -D__KBUILD_MODNAME=kmod_xt_ndpi -c -o /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.c  

source_/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o := /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.c

deps_/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o := \
    $(wildcard include/config/NF_CONNTRACK_CUSTOM) \
    $(wildcard include/config/LIVEPATCH) \
    $(wildcard include/config/NDPI_HOOK) \
    $(wildcard include/config/NF_CONNTRACK_LABELS) \
    $(wildcard include/config/NF_CONNTRACK_MARK) \
  include/linux/compiler-version.h \
    $(wildcard include/config/CC_VERSION_TEXT) \
  include/linux/kconfig.h \
    $(wildcard include/config/CPU_BIG_ENDIAN) \
    $(wildcard include/config/BOOGER) \
    $(wildcard include/config/FOO) \
  include/linux/compiler_types.h \
    $(wildcard include/config/DEBUG_INFO_BTF) \
    $(wildcard include/config/PAHOLE_HAS_BTF_TAG) \
    $(wildcard include/config/HAVE_ARCH_COMPILER_H) \
    $(wildcard include/config/OPTIMIZE_INLINING) \
    $(wildcard include/config/CC_HAS_ASM_INLINE) \
  include/linux/compiler_attributes.h \
    $(wildcard include/config/LTO_GCC) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/RETPOLINE) \
    $(wildcard include/config/ARCH_USE_BUILTIN_BSWAP) \
    $(wildcard include/config/SHADOW_CALL_STACK) \
    $(wildcard include/config/KCOV) \
  include/linux/kernel.h \
    $(wildcard include/config/PREEMPT_VOLUNTARY_BUILD) \
    $(wildcard include/config/PREEMPT_DYNAMIC) \
    $(wildcard include/config/HAVE_PREEMPT_DYNAMIC_CALL) \
    $(wildcard include/config/HAVE_PREEMPT_DYNAMIC_KEY) \
    $(wildcard include/config/PREEMPT_) \
    $(wildcard include/config/DEBUG_ATOMIC_SLEEP) \
    $(wildcard include/config/SMP) \
    $(wildcard include/config/MMU) \
    $(wildcard include/config/PROVE_LOCKING) \
    $(wildcard include/config/TRACING) \
    $(wildcard include/config/FTRACE_MCOUNT_RECORD) \
  include/linux/stdarg.h \
  include/linux/align.h \
  include/linux/const.h \
  include/vdso/const.h \
  include/uapi/linux/const.h \
  include/linux/limits.h \
  include/uapi/linux/limits.h \
  include/linux/types.h \
    $(wildcard include/config/HAVE_UID16) \
    $(wildcard include/config/UID16) \
    $(wildcard include/config/ARCH_DMA_ADDR_T_64BIT) \
    $(wildcard include/config/PHYS_ADDR_T_64BIT) \
    $(wildcard include/config/64BIT) \
    $(wildcard include/config/ARCH_32BIT_USTAT_F_TINODE) \
    $(wildcard include/config/MIPS) \
  include/uapi/linux/types.h \
  arch/x86/include/generated/uapi/asm/types.h \
  include/uapi/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/x86/include/uapi/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  include/linux/compiler_types.h \
  arch/x86/include/asm/posix_types.h \
    $(wildcard include/config/X86_32) \
  arch/x86/include/uapi/asm/posix_types_64.h \
  include/uapi/asm-generic/posix_types.h \
  include/vdso/limits.h \
  include/linux/linkage.h \
    $(wildcard include/config/ARCH_USE_SYM_ANNOTATIONS) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/MODVERSIONS) \
    $(wildcard include/config/HAVE_ARCH_PREL32_RELOCATIONS) \
    $(wildcard include/config/MODULES) \
    $(wildcard include/config/TRIM_UNUSED_KSYMS) \
  arch/x86/include/asm/linkage.h \
    $(wildcard include/config/X86_64) \
    $(wildcard include/config/X86_ALIGNMENT_16) \
    $(wildcard include/config/RETHUNK) \
    $(wildcard include/config/SLS) \
  arch/x86/include/asm/ibt.h \
    $(wildcard include/config/X86_KERNEL_IBT) \
  include/linux/compiler.h \
    $(wildcard include/config/TRACE_BRANCH_PROFILING) \
    $(wildcard include/config/PROFILE_ALL_BRANCHES) \
    $(wildcard include/config/OBJTOOL) \
  arch/x86/include/generated/asm/rwonce.h \
  include/asm-generic/rwonce.h \
  include/linux/kasan-checks.h \
    $(wildcard include/config/KASAN_GENERIC) \
    $(wildcard include/config/KASAN_SW_TAGS) \
  include/linux/kcsan-checks.h \
    $(wildcard include/config/KCSAN) \
    $(wildcard include/config/KCSAN_WEAK_MEMORY) \
    $(wildcard include/config/KCSAN_IGNORE_ATOMICS) \
  include/linux/container_of.h \
  include/linux/build_bug.h \
  include/linux/err.h \
  arch/x86/include/generated/uapi/asm/errno.h \
  include/uapi/asm-generic/errno.h \
  include/uapi/asm-generic/errno-base.h \
  include/linux/bitops.h \
  include/linux/bits.h \
  include/vdso/bits.h \
  include/linux/typecheck.h \
  include/uapi/linux/kernel.h \
  include/uapi/linux/sysinfo.h \
  include/asm-generic/bitops/generic-non-atomic.h \
  arch/x86/include/asm/barrier.h \
  arch/x86/include/asm/alternative.h \
  arch/x86/include/asm/asm.h \
    $(wildcard include/config/KPROBES) \
  arch/x86/include/asm/extable_fixup_types.h \
  arch/x86/include/asm/nops.h \
  include/asm-generic/barrier.h \
  arch/x86/include/asm/bitops.h \
    $(wildcard include/config/X86_CMOV) \
  arch/x86/include/asm/rmwcc.h \
  include/asm-generic/bitops/sched.h \
  arch/x86/include/asm/arch_hweight.h \
  arch/x86/include/asm/cpufeatures.h \
  arch/x86/include/asm/required-features.h \
    $(wildcard include/config/X86_MINIMUM_CPU_FAMILY) \
    $(wildcard include/config/MATH_EMULATION) \
    $(wildcard include/config/X86_PAE) \
    $(wildcard include/config/X86_CMPXCHG64) \
    $(wildcard include/config/X86_P6_NOP) \
    $(wildcard include/config/MATOM) \
    $(wildcard include/config/PARAVIRT_XXL) \
  arch/x86/include/asm/disabled-features.h \
    $(wildcard include/config/X86_UMIP) \
    $(wildcard include/config/X86_INTEL_MEMORY_PROTECTION_KEYS) \
    $(wildcard include/config/X86_5LEVEL) \
    $(wildcard include/config/PAGE_TABLE_ISOLATION) \
    $(wildcard include/config/CPU_UNRET_ENTRY) \
    $(wildcard include/config/INTEL_IOMMU_SVM) \
    $(wildcard include/config/X86_SGX) \
    $(wildcard include/config/INTEL_TDX_GUEST) \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/instrumented-atomic.h \
  include/linux/instrumented.h \
  include/linux/kmsan-checks.h \
    $(wildcard include/config/KMSAN) \
  include/asm-generic/bitops/instrumented-non-atomic.h \
    $(wildcard include/config/KCSAN_ASSUME_PLAIN_WRITES_ATOMIC) \
  include/asm-generic/bitops/instrumented-lock.h \
  include/asm-generic/bitops/le.h \
  arch/x86/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  arch/x86/include/uapi/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  include/linux/kstrtox.h \
  include/linux/log2.h \
    $(wildcard include/config/ARCH_HAS_ILOG2_U32) \
    $(wildcard include/config/ARCH_HAS_ILOG2_U64) \
  include/linux/math.h \
  arch/x86/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/linux/minmax.h \
  include/linux/panic.h \
    $(wildcard include/config/PANIC_TIMEOUT) \
  include/linux/printk.h \
    $(wildcard include/config/MESSAGE_LOGLEVEL_DEFAULT) \
    $(wildcard include/config/CONSOLE_LOGLEVEL_DEFAULT) \
    $(wildcard include/config/CONSOLE_LOGLEVEL_QUIET) \
    $(wildcard include/config/EARLY_PRINTK) \
    $(wildcard include/config/PRINTK) \
    $(wildcard include/config/PRINTK_INDEX) \
    $(wildcard include/config/DYNAMIC_DEBUG) \
    $(wildcard include/config/DYNAMIC_DEBUG_CORE) \
  include/linux/init.h \
    $(wildcard include/config/STRICT_KERNEL_RWX) \
    $(wildcard include/config/STRICT_MODULE_RWX) \
    $(wildcard include/config/LTO_CLANG) \
  include/linux/kern_levels.h \
  include/linux/ratelimit_types.h \
  include/uapi/linux/param.h \
  arch/x86/include/generated/uapi/asm/param.h \
  include/asm-generic/param.h \
    $(wildcard include/config/HZ) \
  include/uapi/asm-generic/param.h \
  include/linux/spinlock_types_raw.h \
    $(wildcard include/config/DEBUG_SPINLOCK) \
    $(wildcard include/config/DEBUG_LOCK_ALLOC) \
  arch/x86/include/asm/spinlock_types.h \
  include/asm-generic/qspinlock_types.h \
    $(wildcard include/config/NR_CPUS) \
  include/asm-generic/qrwlock_types.h \
  include/linux/lockdep_types.h \
    $(wildcard include/config/PROVE_RAW_LOCK_NESTING) \
    $(wildcard include/config/LOCKDEP) \
    $(wildcard include/config/LOCK_STAT) \
  include/linux/once_lite.h \
  include/linux/static_call_types.h \
    $(wildcard include/config/HAVE_STATIC_CALL) \
    $(wildcard include/config/HAVE_STATIC_CALL_INLINE) \
  include/linux/instruction_pointer.h \
  include/linux/module.h \
    $(wildcard include/config/MODULE_STRIPPED) \
    $(wildcard include/config/SYSFS) \
    $(wildcard include/config/MODULES_TREE_LOOKUP) \
    $(wildcard include/config/STACKTRACE_BUILD_ID) \
    $(wildcard include/config/ARCH_USES_CFI_TRAPS) \
    $(wildcard include/config/MODULE_SIG) \
    $(wildcard include/config/ARCH_WANTS_MODULES_DATA_IN_VMALLOC) \
    $(wildcard include/config/GENERIC_BUG) \
    $(wildcard include/config/KALLSYMS) \
    $(wildcard include/config/TRACEPOINTS) \
    $(wildcard include/config/TREE_SRCU) \
    $(wildcard include/config/BPF_EVENTS) \
    $(wildcard include/config/DEBUG_INFO_BTF_MODULES) \
    $(wildcard include/config/JUMP_LABEL) \
    $(wildcard include/config/EVENT_TRACING) \
    $(wildcard include/config/KUNIT) \
    $(wildcard include/config/MODULE_UNLOAD) \
    $(wildcard include/config/CONSTRUCTORS) \
    $(wildcard include/config/FUNCTION_ERROR_INJECTION) \
  include/linux/list.h \
    $(wildcard include/config/DEBUG_LIST) \
  include/linux/poison.h \
    $(wildcard include/config/ILLEGAL_POINTER_VALUE) \
  include/linux/stat.h \
  arch/x86/include/uapi/asm/stat.h \
  include/uapi/linux/stat.h \
  include/linux/time.h \
    $(wildcard include/config/POSIX_TIMERS) \
  include/linux/cache.h \
    $(wildcard include/config/ARCH_HAS_CACHE_LINE_SIZE) \
  arch/x86/include/asm/cache.h \
    $(wildcard include/config/X86_L1_CACHE_SHIFT) \
    $(wildcard include/config/X86_INTERNODE_CACHE_SHIFT) \
    $(wildcard include/config/X86_VSMP) \
  include/linux/math64.h \
    $(wildcard include/config/ARCH_SUPPORTS_INT128) \
  include/vdso/math64.h \
  include/linux/time64.h \
  include/vdso/time64.h \
  include/uapi/linux/time.h \
  include/uapi/linux/time_types.h \
  include/linux/time32.h \
  include/linux/timex.h \
  include/uapi/linux/timex.h \
  arch/x86/include/asm/timex.h \
    $(wildcard include/config/X86_TSC) \
  arch/x86/include/asm/processor.h \
    $(wildcard include/config/X86_VMX_FEATURE_NAMES) \
    $(wildcard include/config/X86_IOPL_IOPERM) \
    $(wildcard include/config/STACKPROTECTOR) \
    $(wildcard include/config/VM86) \
    $(wildcard include/config/X86_DEBUGCTLMSR) \
    $(wildcard include/config/CPU_SUP_AMD) \
    $(wildcard include/config/XEN) \
  arch/x86/include/asm/processor-flags.h \
  arch/x86/include/uapi/asm/processor-flags.h \
  include/linux/mem_encrypt.h \
    $(wildcard include/config/ARCH_HAS_MEM_ENCRYPT) \
    $(wildcard include/config/AMD_MEM_ENCRYPT) \
  arch/x86/include/asm/mem_encrypt.h \
    $(wildcard include/config/X86_MEM_ENCRYPT) \
  include/linux/cc_platform.h \
    $(wildcard include/config/ARCH_HAS_CC_PLATFORM) \
  arch/x86/include/uapi/asm/bootparam.h \
  include/linux/screen_info.h \
  include/uapi/linux/screen_info.h \
  include/linux/apm_bios.h \
  include/uapi/linux/apm_bios.h \
  include/uapi/linux/ioctl.h \
  arch/x86/include/generated/uapi/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/uapi/asm-generic/ioctl.h \
  include/linux/edd.h \
  include/uapi/linux/edd.h \
  arch/x86/include/asm/ist.h \
  arch/x86/include/uapi/asm/ist.h \
  include/video/edid.h \
    $(wildcard include/config/X86) \
  include/uapi/video/edid.h \
  arch/x86/include/asm/math_emu.h \
  arch/x86/include/asm/ptrace.h \
    $(wildcard include/config/PARAVIRT) \
    $(wildcard include/config/IA32_EMULATION) \
  arch/x86/include/asm/segment.h \
    $(wildcard include/config/XEN_PV) \
  arch/x86/include/asm/page_types.h \
    $(wildcard include/config/PHYSICAL_START) \
    $(wildcard include/config/PHYSICAL_ALIGN) \
    $(wildcard include/config/DYNAMIC_PHYSICAL_MASK) \
  arch/x86/include/asm/page_64_types.h \
    $(wildcard include/config/KASAN) \
    $(wildcard include/config/DYNAMIC_MEMORY_LAYOUT) \
    $(wildcard include/config/RANDOMIZE_BASE) \
  arch/x86/include/asm/kaslr.h \
    $(wildcard include/config/RANDOMIZE_MEMORY) \
  arch/x86/include/uapi/asm/ptrace.h \
  arch/x86/include/uapi/asm/ptrace-abi.h \
  arch/x86/include/asm/paravirt_types.h \
    $(wildcard include/config/PGTABLE_LEVELS) \
    $(wildcard include/config/ZERO_CALL_USED_REGS) \
    $(wildcard include/config/PARAVIRT_DEBUG) \
  arch/x86/include/asm/desc_defs.h \
  arch/x86/include/asm/pgtable_types.h \
    $(wildcard include/config/MEM_SOFT_DIRTY) \
    $(wildcard include/config/HAVE_ARCH_USERFAULTFD_WP) \
    $(wildcard include/config/PROC_FS) \
  arch/x86/include/asm/pgtable_64_types.h \
    $(wildcard include/config/DEBUG_KMAP_LOCAL_FORCE_MAP) \
  arch/x86/include/asm/sparsemem.h \
    $(wildcard include/config/SPARSEMEM) \
    $(wildcard include/config/NUMA_KEEP_MEMINFO) \
  include/asm-generic/pgtable-nop4d.h \
  arch/x86/include/asm/nospec-branch.h \
    $(wildcard include/config/CPU_SRSO) \
    $(wildcard include/config/CPU_IBPB_ENTRY) \
  include/linux/static_key.h \
  include/linux/jump_label.h \
    $(wildcard include/config/HAVE_ARCH_JUMP_LABEL_RELATIVE) \
  arch/x86/include/asm/jump_label.h \
    $(wildcard include/config/HAVE_JUMP_LABEL_HACK) \
  include/linux/objtool.h \
    $(wildcard include/config/FRAME_POINTER) \
  arch/x86/include/asm/msr-index.h \
  arch/x86/include/asm/unwind_hints.h \
  arch/x86/include/asm/orc_types.h \
  arch/x86/include/asm/percpu.h \
    $(wildcard include/config/X86_64_SMP) \
  include/asm-generic/percpu.h \
    $(wildcard include/config/DEBUG_PREEMPT) \
    $(wildcard include/config/HAVE_SETUP_PER_CPU_AREA) \
  include/linux/threads.h \
    $(wildcard include/config/BASE_SMALL) \
  include/linux/percpu-defs.h \
    $(wildcard include/config/DEBUG_FORCE_WEAK_PER_CPU) \
  arch/x86/include/asm/proto.h \
  arch/x86/include/uapi/asm/ldt.h \
  arch/x86/include/uapi/asm/sigcontext.h \
  arch/x86/include/asm/current.h \
  arch/x86/include/asm/page.h \
  arch/x86/include/asm/page_64.h \
    $(wildcard include/config/DEBUG_VIRTUAL) \
    $(wildcard include/config/FLATMEM) \
    $(wildcard include/config/X86_VSYSCALL_EMULATION) \
  include/linux/range.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/SPARSEMEM_VMEMMAP) \
  include/linux/pfn.h \
  include/asm-generic/getorder.h \
  arch/x86/include/asm/msr.h \
  arch/x86/include/asm/msr-index.h \
  arch/x86/include/asm/cpumask.h \
  include/linux/cpumask.h \
    $(wildcard include/config/FORCE_NR_CPUS) \
    $(wildcard include/config/HOTPLUG_CPU) \
    $(wildcard include/config/DEBUG_PER_CPU_MAPS) \
    $(wildcard include/config/CPUMASK_OFFSTACK) \
  include/linux/bitmap.h \
  include/linux/find.h \
  include/linux/string.h \
    $(wildcard include/config/BINARY_PRINTF) \
    $(wildcard include/config/FORTIFY_SOURCE) \
  include/linux/errno.h \
  include/uapi/linux/errno.h \
  include/linux/overflow.h \
  include/uapi/linux/string.h \
  arch/x86/include/asm/string.h \
  arch/x86/include/asm/string_64.h \
    $(wildcard include/config/ARCH_HAS_UACCESS_FLUSHCACHE) \
  include/linux/atomic.h \
  arch/x86/include/asm/atomic.h \
  arch/x86/include/asm/cmpxchg.h \
  arch/x86/include/asm/cmpxchg_64.h \
  arch/x86/include/asm/atomic64_64.h \
  include/linux/atomic/atomic-arch-fallback.h \
    $(wildcard include/config/GENERIC_ATOMIC64) \
  include/linux/atomic/atomic-long.h \
  include/linux/atomic/atomic-instrumented.h \
  include/linux/bug.h \
    $(wildcard include/config/BUG_ON_DATA_CORRUPTION) \
  arch/x86/include/asm/bug.h \
    $(wildcard include/config/DEBUG_BUGVERBOSE) \
  include/linux/instrumentation.h \
    $(wildcard include/config/NOINSTR_VALIDATION) \
  include/asm-generic/bug.h \
    $(wildcard include/config/BUG) \
    $(wildcard include/config/GENERIC_BUG_RELATIVE_POINTERS) \
  include/linux/gfp_types.h \
    $(wildcard include/config/KASAN_HW_TAGS) \
  include/linux/numa.h \
    $(wildcard include/config/NODES_SHIFT) \
    $(wildcard include/config/NUMA) \
    $(wildcard include/config/HAVE_ARCH_NODE_DEV_GROUP) \
  arch/x86/include/uapi/asm/msr.h \
  arch/x86/include/asm/shared/msr.h \
  include/linux/tracepoint-defs.h \
  arch/x86/include/asm/special_insns.h \
  include/linux/irqflags.h \
    $(wildcard include/config/TRACE_IRQFLAGS) \
    $(wildcard include/config/PREEMPT_RT) \
    $(wildcard include/config/IRQSOFF_TRACER) \
    $(wildcard include/config/PREEMPT_TRACER) \
    $(wildcard include/config/DEBUG_IRQFLAGS) \
    $(wildcard include/config/TRACE_IRQFLAGS_SUPPORT) \
  arch/x86/include/asm/irqflags.h \
    $(wildcard include/config/DEBUG_ENTRY) \
  arch/x86/include/asm/fpu/types.h \
  arch/x86/include/asm/vmxfeatures.h \
  arch/x86/include/asm/vdso/processor.h \
  include/linux/personality.h \
  include/uapi/linux/personality.h \
  arch/x86/include/asm/tsc.h \
  arch/x86/include/asm/cpufeature.h \
    $(wildcard include/config/X86_FEATURE_NAMES) \
  include/vdso/time32.h \
  include/vdso/time.h \
  include/linux/uidgid.h \
    $(wildcard include/config/MULTIUSER) \
    $(wildcard include/config/USER_NS) \
  include/linux/highuid.h \
  include/linux/buildid.h \
    $(wildcard include/config/CRASH_CORE) \
  include/linux/mm_types.h \
    $(wildcard include/config/HAVE_ALIGNED_STRUCT_PAGE) \
    $(wildcard include/config/MEMCG) \
    $(wildcard include/config/USERFAULTFD) \
    $(wildcard include/config/SWAP) \
    $(wildcard include/config/HAVE_ARCH_COMPAT_MMAP_BASES) \
    $(wildcard include/config/MEMBARRIER) \
    $(wildcard include/config/AIO) \
    $(wildcard include/config/MMU_NOTIFIER) \
    $(wildcard include/config/TRANSPARENT_HUGEPAGE) \
    $(wildcard include/config/NUMA_BALANCING) \
    $(wildcard include/config/ARCH_WANT_BATCHED_UNMAP_TLB_FLUSH) \
    $(wildcard include/config/HUGETLB_PAGE) \
    $(wildcard include/config/IOMMU_SVA) \
    $(wildcard include/config/KSM) \
    $(wildcard include/config/LRU_GEN) \
  include/linux/mm_types_task.h \
    $(wildcard include/config/SPLIT_PTLOCK_CPUS) \
    $(wildcard include/config/ARCH_ENABLE_SPLIT_PMD_PTLOCK) \
  arch/x86/include/asm/tlbbatch.h \
  include/linux/auxvec.h \
  include/uapi/linux/auxvec.h \
  arch/x86/include/uapi/asm/auxvec.h \
  include/linux/kref.h \
  include/linux/spinlock.h \
    $(wildcard include/config/PREEMPTION) \
  include/linux/preempt.h \
    $(wildcard include/config/PREEMPT_COUNT) \
    $(wildcard include/config/TRACE_PREEMPT_TOGGLE) \
    $(wildcard include/config/PREEMPT_NOTIFIERS) \
  arch/x86/include/asm/preempt.h \
  include/linux/thread_info.h \
    $(wildcard include/config/THREAD_INFO_IN_TASK) \
    $(wildcard include/config/GENERIC_ENTRY) \
    $(wildcard include/config/HAVE_ARCH_WITHIN_STACK_FRAMES) \
    $(wildcard include/config/HARDENED_USERCOPY) \
  include/linux/restart_block.h \
  arch/x86/include/asm/thread_info.h \
    $(wildcard include/config/COMPAT) \
  include/linux/bottom_half.h \
  include/linux/lockdep.h \
    $(wildcard include/config/DEBUG_LOCKING_API_SELFTESTS) \
  include/linux/smp.h \
    $(wildcard include/config/UP_LATE_INIT) \
  include/linux/smp_types.h \
  include/linux/llist.h \
    $(wildcard include/config/ARCH_HAVE_NMI_SAFE_CMPXCHG) \
  arch/x86/include/asm/smp.h \
    $(wildcard include/config/X86_LOCAL_APIC) \
    $(wildcard include/config/DEBUG_NMI_SELFTEST) \
  arch/x86/include/generated/asm/mmiowb.h \
  include/asm-generic/mmiowb.h \
    $(wildcard include/config/MMIOWB) \
  include/linux/spinlock_types.h \
  include/linux/rwlock_types.h \
  arch/x86/include/asm/spinlock.h \
  arch/x86/include/asm/paravirt.h \
    $(wildcard include/config/PARAVIRT_SPINLOCKS) \
  arch/x86/include/asm/frame.h \
  arch/x86/include/asm/qspinlock.h \
  include/asm-generic/qspinlock.h \
  arch/x86/include/asm/qrwlock.h \
  include/asm-generic/qrwlock.h \
  include/linux/rwlock.h \
    $(wildcard include/config/PREEMPT) \
  include/linux/spinlock_api_smp.h \
    $(wildcard include/config/INLINE_SPIN_LOCK) \
    $(wildcard include/config/INLINE_SPIN_LOCK_BH) \
    $(wildcard include/config/INLINE_SPIN_LOCK_IRQ) \
    $(wildcard include/config/INLINE_SPIN_LOCK_IRQSAVE) \
    $(wildcard include/config/INLINE_SPIN_TRYLOCK) \
    $(wildcard include/config/INLINE_SPIN_TRYLOCK_BH) \
    $(wildcard include/config/UNINLINE_SPIN_UNLOCK) \
    $(wildcard include/config/INLINE_SPIN_UNLOCK_BH) \
    $(wildcard include/config/INLINE_SPIN_UNLOCK_IRQ) \
    $(wildcard include/config/INLINE_SPIN_UNLOCK_IRQRESTORE) \
    $(wildcard include/config/GENERIC_LOCKBREAK) \
  include/linux/rwlock_api_smp.h \
    $(wildcard include/config/INLINE_READ_LOCK) \
    $(wildcard include/config/INLINE_WRITE_LOCK) \
    $(wildcard include/config/INLINE_READ_LOCK_BH) \
    $(wildcard include/config/INLINE_WRITE_LOCK_BH) \
    $(wildcard include/config/INLINE_READ_LOCK_IRQ) \
    $(wildcard include/config/INLINE_WRITE_LOCK_IRQ) \
    $(wildcard include/config/INLINE_READ_LOCK_IRQSAVE) \
    $(wildcard include/config/INLINE_WRITE_LOCK_IRQSAVE) \
    $(wildcard include/config/INLINE_READ_TRYLOCK) \
    $(wildcard include/config/INLINE_WRITE_TRYLOCK) \
    $(wildcard include/config/INLINE_READ_UNLOCK) \
    $(wildcard include/config/INLINE_WRITE_UNLOCK) \
    $(wildcard include/config/INLINE_READ_UNLOCK_BH) \
    $(wildcard include/config/INLINE_WRITE_UNLOCK_BH) \
    $(wildcard include/config/INLINE_READ_UNLOCK_IRQ) \
    $(wildcard include/config/INLINE_WRITE_UNLOCK_IRQ) \
    $(wildcard include/config/INLINE_READ_UNLOCK_IRQRESTORE) \
    $(wildcard include/config/INLINE_WRITE_UNLOCK_IRQRESTORE) \
  include/linux/refcount.h \
  include/linux/rbtree.h \
  include/linux/rbtree_types.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/PREEMPT_RCU) \
    $(wildcard include/config/TINY_RCU) \
    $(wildcard include/config/RCU_STRICT_GRACE_PERIOD) \
    $(wildcard include/config/TASKS_RCU_GENERIC) \
    $(wildcard include/config/RCU_STALL_COMMON) \
    $(wildcard include/config/NO_HZ_FULL) \
    $(wildcard include/config/KVM_XFER_TO_GUEST_WORK) \
    $(wildcard include/config/RCU_NOCB_CPU) \
    $(wildcard include/config/TASKS_RCU) \
    $(wildcard include/config/TASKS_TRACE_RCU) \
    $(wildcard include/config/TASKS_RUDE_RCU) \
    $(wildcard include/config/TREE_RCU) \
    $(wildcard include/config/DEBUG_OBJECTS_RCU_HEAD) \
    $(wildcard include/config/PROVE_RCU) \
    $(wildcard include/config/ARCH_WEAK_RELEASE_ACQUIRE) \
  include/linux/context_tracking_irq.h \
    $(wildcard include/config/CONTEXT_TRACKING_IDLE) \
  include/linux/rcutree.h \
  include/linux/maple_tree.h \
    $(wildcard include/config/MAPLE_RCU_DISABLED) \
    $(wildcard include/config/DEBUG_MAPLE_TREE_VERBOSE) \
    $(wildcard include/config/DEBUG_MAPLE_TREE) \
  include/linux/rwsem.h \
    $(wildcard include/config/RWSEM_SPIN_ON_OWNER) \
    $(wildcard include/config/DEBUG_RWSEMS) \
  include/linux/osq_lock.h \
  include/linux/completion.h \
  include/linux/swait.h \
  include/linux/wait.h \
  include/uapi/linux/wait.h \
  include/linux/uprobes.h \
    $(wildcard include/config/UPROBES) \
  include/linux/page-flags-layout.h \
  include/generated/bounds.h \
  include/linux/workqueue.h \
    $(wildcard include/config/DEBUG_OBJECTS_WORK) \
    $(wildcard include/config/FREEZER) \
    $(wildcard include/config/WQ_WATCHDOG) \
  include/linux/timer.h \
    $(wildcard include/config/DEBUG_OBJECTS_TIMERS) \
  include/linux/ktime.h \
  include/linux/jiffies.h \
  include/vdso/jiffies.h \
  include/generated/timeconst.h \
  include/vdso/ktime.h \
  include/linux/timekeeping.h \
    $(wildcard include/config/GENERIC_CMOS_UPDATE) \
  include/linux/clocksource_ids.h \
  include/linux/debugobjects.h \
    $(wildcard include/config/DEBUG_OBJECTS) \
    $(wildcard include/config/DEBUG_OBJECTS_FREE) \
  include/linux/seqlock.h \
  include/linux/mutex.h \
    $(wildcard include/config/MUTEX_SPIN_ON_OWNER) \
    $(wildcard include/config/DEBUG_MUTEXES) \
  include/linux/debug_locks.h \
  arch/x86/include/asm/mmu.h \
    $(wildcard include/config/MODIFY_LDT_SYSCALL) \
  include/linux/kmod.h \
  include/linux/umh.h \
  include/linux/gfp.h \
    $(wildcard include/config/HIGHMEM) \
    $(wildcard include/config/ZONE_DMA) \
    $(wildcard include/config/ZONE_DMA32) \
    $(wildcard include/config/ZONE_DEVICE) \
    $(wildcard include/config/PM_SLEEP) \
    $(wildcard include/config/CONTIG_ALLOC) \
    $(wildcard include/config/CMA) \
  include/linux/mmzone.h \
    $(wildcard include/config/ARCH_FORCE_MAX_ORDER) \
    $(wildcard include/config/MEMORY_ISOLATION) \
    $(wildcard include/config/ZSMALLOC) \
    $(wildcard include/config/LRU_GEN_STATS) \
    $(wildcard include/config/MEMORY_HOTPLUG) \
    $(wildcard include/config/COMPACTION) \
    $(wildcard include/config/PAGE_EXTENSION) \
    $(wildcard include/config/DEFERRED_STRUCT_PAGE_INIT) \
    $(wildcard include/config/HAVE_MEMORYLESS_NODES) \
    $(wildcard include/config/SPARSEMEM_EXTREME) \
    $(wildcard include/config/HAVE_ARCH_PFN_VALID) \
  include/linux/list_nulls.h \
  include/linux/nodemask.h \
  include/linux/random.h \
    $(wildcard include/config/VMGENID) \
  include/linux/once.h \
  include/uapi/linux/random.h \
  include/linux/irqnr.h \
  include/uapi/linux/irqnr.h \
  include/linux/prandom.h \
  include/linux/percpu.h \
    $(wildcard include/config/NEED_PER_CPU_EMBED_FIRST_CHUNK) \
    $(wildcard include/config/NEED_PER_CPU_PAGE_FIRST_CHUNK) \
  include/linux/mmdebug.h \
    $(wildcard include/config/DEBUG_VM) \
    $(wildcard include/config/DEBUG_VM_IRQSOFF) \
    $(wildcard include/config/DEBUG_VM_PGFLAGS) \
  arch/x86/include/asm/archrandom.h \
    $(wildcard include/config/UML) \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/HUGETLB_PAGE_SIZE_VARIABLE) \
  include/linux/page-flags.h \
    $(wildcard include/config/ARCH_USES_PG_UNCACHED) \
    $(wildcard include/config/MEMORY_FAILURE) \
    $(wildcard include/config/PAGE_IDLE_FLAG) \
    $(wildcard include/config/HUGETLB_PAGE_OPTIMIZE_VMEMMAP) \
  include/linux/local_lock.h \
  include/linux/local_lock_internal.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/HAVE_ARCH_NODEDATA_EXTENSION) \
    $(wildcard include/config/ARCH_HAS_ADD_PAGES) \
    $(wildcard include/config/MEMORY_HOTREMOVE) \
  include/linux/notifier.h \
  include/linux/srcu.h \
    $(wildcard include/config/TINY_SRCU) \
    $(wildcard include/config/SRCU) \
  include/linux/rcu_segcblist.h \
  include/linux/srcutree.h \
  include/linux/rcu_node_tree.h \
    $(wildcard include/config/RCU_FANOUT) \
    $(wildcard include/config/RCU_FANOUT_LEAF) \
  arch/x86/include/asm/mmzone.h \
  arch/x86/include/asm/mmzone_64.h \
  include/linux/topology.h \
    $(wildcard include/config/USE_PERCPU_NUMA_NODE_ID) \
    $(wildcard include/config/SCHED_SMT) \
  include/linux/arch_topology.h \
    $(wildcard include/config/ACPI_CPPC_LIB) \
    $(wildcard include/config/GENERIC_ARCH_TOPOLOGY) \
  arch/x86/include/asm/topology.h \
    $(wildcard include/config/SCHED_MC_PRIO) \
  arch/x86/include/asm/mpspec.h \
    $(wildcard include/config/EISA) \
    $(wildcard include/config/X86_MPPARSE) \
  arch/x86/include/asm/mpspec_def.h \
  arch/x86/include/asm/x86_init.h \
  arch/x86/include/asm/apicdef.h \
  include/asm-generic/topology.h \
  include/linux/sysctl.h \
    $(wildcard include/config/SYSCTL) \
  include/uapi/linux/sysctl.h \
  include/linux/elf.h \
    $(wildcard include/config/ARCH_USE_GNU_PROPERTY) \
    $(wildcard include/config/ARCH_HAVE_ELF_PROT) \
  arch/x86/include/asm/elf.h \
    $(wildcard include/config/X86_X32_ABI) \
  arch/x86/include/asm/user.h \
  arch/x86/include/asm/user_64.h \
  arch/x86/include/asm/fsgsbase.h \
  arch/x86/include/asm/vdso.h \
  include/uapi/linux/elf.h \
  include/uapi/linux/elf-em.h \
  include/linux/kobject.h \
    $(wildcard include/config/UEVENT_HELPER) \
    $(wildcard include/config/DEBUG_KOBJECT_RELEASE) \
  include/linux/sysfs.h \
  include/linux/kernfs.h \
    $(wildcard include/config/KERNFS) \
  include/linux/idr.h \
  include/linux/radix-tree.h \
  include/linux/xarray.h \
    $(wildcard include/config/XARRAY_MULTI) \
  include/linux/kconfig.h \
  include/linux/sched/mm.h \
    $(wildcard include/config/ARCH_HAS_MEMBARRIER_CALLBACKS) \
  include/linux/sched.h \
    $(wildcard include/config/VIRT_CPU_ACCOUNTING_NATIVE) \
    $(wildcard include/config/SCHED_INFO) \
    $(wildcard include/config/SCHEDSTATS) \
    $(wildcard include/config/SCHED_CORE) \
    $(wildcard include/config/FAIR_GROUP_SCHED) \
    $(wildcard include/config/RT_GROUP_SCHED) \
    $(wildcard include/config/RT_MUTEXES) \
    $(wildcard include/config/UCLAMP_TASK) \
    $(wildcard include/config/UCLAMP_BUCKETS_COUNT) \
    $(wildcard include/config/KMAP_LOCAL) \
    $(wildcard include/config/CGROUP_SCHED) \
    $(wildcard include/config/BLK_DEV_IO_TRACE) \
    $(wildcard include/config/PSI) \
    $(wildcard include/config/COMPAT_BRK) \
    $(wildcard include/config/CGROUPS) \
    $(wildcard include/config/BLK_CGROUP) \
    $(wildcard include/config/PAGE_OWNER) \
    $(wildcard include/config/EVENTFD) \
    $(wildcard include/config/CPU_SUP_INTEL) \
    $(wildcard include/config/TASK_DELAY_ACCT) \
    $(wildcard include/config/ARCH_HAS_SCALED_CPUTIME) \
    $(wildcard include/config/VIRT_CPU_ACCOUNTING_GEN) \
    $(wildcard include/config/POSIX_CPUTIMERS) \
    $(wildcard include/config/POSIX_CPU_TIMERS_TASK_WORK) \
    $(wildcard include/config/KEYS) \
    $(wildcard include/config/SYSVIPC) \
    $(wildcard include/config/DETECT_HUNG_TASK) \
    $(wildcard include/config/IO_URING) \
    $(wildcard include/config/AUDIT) \
    $(wildcard include/config/AUDITSYSCALL) \
    $(wildcard include/config/UBSAN) \
    $(wildcard include/config/UBSAN_TRAP) \
    $(wildcard include/config/TASK_XACCT) \
    $(wildcard include/config/CPUSETS) \
    $(wildcard include/config/X86_CPU_RESCTRL) \
    $(wildcard include/config/FUTEX) \
    $(wildcard include/config/PERF_EVENTS) \
    $(wildcard include/config/RSEQ) \
    $(wildcard include/config/FAULT_INJECTION) \
    $(wildcard include/config/LATENCYTOP) \
    $(wildcard include/config/FUNCTION_GRAPH_TRACER) \
    $(wildcard include/config/BCACHE) \
    $(wildcard include/config/VMAP_STACK) \
    $(wildcard include/config/SECURITY) \
    $(wildcard include/config/BPF_SYSCALL) \
    $(wildcard include/config/GCC_PLUGIN_STACKLEAK) \
    $(wildcard include/config/X86_MCE) \
    $(wildcard include/config/KRETPROBES) \
    $(wildcard include/config/RETHOOK) \
    $(wildcard include/config/ARCH_HAS_PARANOID_L1D_FLUSH) \
    $(wildcard include/config/RV) \
    $(wildcard include/config/ARCH_TASK_STRUCT_ON_STACK) \
    $(wildcard include/config/PREEMPT_NONE) \
    $(wildcard include/config/PREEMPT_VOLUNTARY) \
    $(wildcard include/config/DEBUG_RSEQ) \
  include/uapi/linux/sched.h \
  include/linux/pid.h \
  include/linux/rculist.h \
    $(wildcard include/config/PROVE_RCU_LIST) \
  include/linux/sem.h \
  include/uapi/linux/sem.h \
  include/linux/ipc.h \
  include/linux/rhashtable-types.h \
  include/uapi/linux/ipc.h \
  arch/x86/include/generated/uapi/asm/ipcbuf.h \
  include/uapi/asm-generic/ipcbuf.h \
  arch/x86/include/uapi/asm/sembuf.h \
  include/linux/shm.h \
  include/uapi/linux/shm.h \
  include/uapi/asm-generic/hugetlb_encode.h \
  arch/x86/include/uapi/asm/shmbuf.h \
  include/uapi/asm-generic/shmbuf.h \
  arch/x86/include/asm/shmparam.h \
  include/linux/kmsan_types.h \
  include/linux/plist.h \
    $(wildcard include/config/DEBUG_PLIST) \
  include/linux/hrtimer.h \
    $(wildcard include/config/HIGH_RES_TIMERS) \
    $(wildcard include/config/TIME_LOW_RES) \
    $(wildcard include/config/TIMERFD) \
  include/linux/hrtimer_defs.h \
  include/linux/timerqueue.h \
  include/linux/seccomp.h \
    $(wildcard include/config/SECCOMP) \
    $(wildcard include/config/HAVE_ARCH_SECCOMP_FILTER) \
    $(wildcard include/config/SECCOMP_FILTER) \
    $(wildcard include/config/CHECKPOINT_RESTORE) \
    $(wildcard include/config/SECCOMP_CACHE_DEBUG) \
  include/uapi/linux/seccomp.h \
  arch/x86/include/asm/seccomp.h \
  arch/x86/include/asm/unistd.h \
  arch/x86/include/uapi/asm/unistd.h \
  arch/x86/include/generated/uapi/asm/unistd_64.h \
  arch/x86/include/generated/asm/unistd_64_x32.h \
  arch/x86/include/generated/asm/unistd_32_ia32.h \
  include/asm-generic/seccomp.h \
  include/uapi/linux/unistd.h \
  include/linux/resource.h \
  include/uapi/linux/resource.h \
  arch/x86/include/generated/uapi/asm/resource.h \
  include/asm-generic/resource.h \
  include/uapi/asm-generic/resource.h \
  include/linux/latencytop.h \
  include/linux/sched/prio.h \
  include/linux/sched/types.h \
  include/linux/signal_types.h \
    $(wildcard include/config/OLD_SIGACTION) \
  include/uapi/linux/signal.h \
  arch/x86/include/asm/signal.h \
  arch/x86/include/uapi/asm/signal.h \
  include/uapi/asm-generic/signal-defs.h \
  arch/x86/include/uapi/asm/siginfo.h \
  include/uapi/asm-generic/siginfo.h \
  include/linux/syscall_user_dispatch.h \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/TASK_IO_ACCOUNTING) \
  include/linux/posix-timers.h \
  include/linux/alarmtimer.h \
    $(wildcard include/config/RTC_CLASS) \
  include/uapi/linux/rseq.h \
  include/linux/kcsan.h \
  include/linux/rv.h \
    $(wildcard include/config/RV_REACTORS) \
  arch/x86/include/generated/asm/kmap_size.h \
  include/asm-generic/kmap_size.h \
    $(wildcard include/config/DEBUG_KMAP_LOCAL) \
  include/linux/sync_core.h \
    $(wildcard include/config/ARCH_HAS_SYNC_CORE_BEFORE_USERMODE) \
  arch/x86/include/asm/sync_core.h \
  include/linux/ioasid.h \
    $(wildcard include/config/IOASID) \
  include/linux/kobject_ns.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/ALPHA) \
    $(wildcard include/config/IA64) \
    $(wildcard include/config/PPC64) \
  include/linux/rbtree_latch.h \
  include/linux/error-injection.h \
  include/asm-generic/error-injection.h \
  arch/x86/include/asm/module.h \
    $(wildcard include/config/UNWINDER_ORC) \
  include/asm-generic/module.h \
    $(wildcard include/config/HAVE_MOD_ARCH_SPECIFIC) \
    $(wildcard include/config/MODULES_USE_ELF_REL) \
    $(wildcard include/config/MODULES_USE_ELF_RELA) \
  arch/x86/include/asm/orc_types.h \
  include/generated/uapi/linux/version.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/PROC_PID_ARCH_STATUS) \
  include/linux/fs.h \
    $(wildcard include/config/READ_ONLY_THP_FOR_FS) \
    $(wildcard include/config/FS_POSIX_ACL) \
    $(wildcard include/config/CGROUP_WRITEBACK) \
    $(wildcard include/config/IMA) \
    $(wildcard include/config/FILE_LOCKING) \
    $(wildcard include/config/FSNOTIFY) \
    $(wildcard include/config/FS_ENCRYPTION) \
    $(wildcard include/config/FS_VERITY) \
    $(wildcard include/config/EPOLL) \
    $(wildcard include/config/UNICODE) \
    $(wildcard include/config/QUOTA) \
    $(wildcard include/config/FS_DAX) \
    $(wildcard include/config/BLOCK) \
  include/linux/wait_bit.h \
  include/linux/kdev_t.h \
  include/uapi/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist_bl.h \
  include/linux/list_bl.h \
  include/linux/bit_spinlock.h \
  include/linux/lockref.h \
    $(wildcard include/config/ARCH_USE_CMPXCHG_LOCKREF) \
  include/linux/stringhash.h \
    $(wildcard include/config/DCACHE_WORD_ACCESS) \
  include/linux/hash.h \
    $(wildcard include/config/HAVE_ARCH_HASH) \
  include/linux/path.h \
  include/linux/list_lru.h \
    $(wildcard include/config/MEMCG_KMEM) \
  include/linux/shrinker.h \
    $(wildcard include/config/SHRINKER_DEBUG) \
  include/linux/capability.h \
  include/uapi/linux/capability.h \
  include/linux/semaphore.h \
  include/linux/fcntl.h \
    $(wildcard include/config/ARCH_32BIT_OFF_T) \
  include/uapi/linux/fcntl.h \
  arch/x86/include/generated/uapi/asm/fcntl.h \
  include/uapi/asm-generic/fcntl.h \
  include/uapi/linux/openat2.h \
  include/linux/migrate_mode.h \
  include/linux/percpu-rwsem.h \
  include/linux/rcuwait.h \
  include/linux/sched/signal.h \
    $(wildcard include/config/SCHED_AUTOGROUP) \
    $(wildcard include/config/BSD_PROCESS_ACCT) \
    $(wildcard include/config/TASKSTATS) \
    $(wildcard include/config/STACK_GROWSUP) \
  include/linux/signal.h \
    $(wildcard include/config/DYNAMIC_SIGFRAME) \
  include/linux/sched/jobctl.h \
  include/linux/sched/task.h \
    $(wildcard include/config/HAVE_EXIT_THREAD) \
    $(wildcard include/config/ARCH_WANTS_DYNAMIC_TASK_STRUCT) \
    $(wildcard include/config/HAVE_ARCH_THREAD_STRUCT_WHITELIST) \
  include/linux/uaccess.h \
    $(wildcard include/config/ARCH_HAS_SUBPAGE_FAULTS) \
  include/linux/fault-inject-usercopy.h \
    $(wildcard include/config/FAULT_INJECTION_USERCOPY) \
  arch/x86/include/asm/uaccess.h \
    $(wildcard include/config/CC_HAS_ASM_GOTO_OUTPUT) \
    $(wildcard include/config/CC_HAS_ASM_GOTO_TIED_OUTPUT) \
    $(wildcard include/config/ARCH_HAS_COPY_MC) \
    $(wildcard include/config/X86_INTEL_USERCOPY) \
  arch/x86/include/asm/smap.h \
  arch/x86/include/asm/extable.h \
    $(wildcard include/config/BPF_JIT) \
  include/asm-generic/access_ok.h \
    $(wildcard include/config/ALTERNATE_USER_ADDRESS_SPACE) \
  arch/x86/include/asm/uaccess_64.h \
  include/linux/cred.h \
    $(wildcard include/config/DEBUG_CREDENTIALS) \
  include/linux/key.h \
    $(wildcard include/config/KEY_NOTIFICATIONS) \
    $(wildcard include/config/NET) \
  include/linux/assoc_array.h \
    $(wildcard include/config/ASSOCIATIVE_ARRAY) \
  include/linux/sched/user.h \
    $(wildcard include/config/VFIO_PCI_ZDEV_KVM) \
    $(wildcard include/config/WATCH_QUEUE) \
  include/linux/percpu_counter.h \
  include/linux/ratelimit.h \
  include/linux/rcu_sync.h \
  include/linux/delayed_call.h \
  include/linux/uuid.h \
  include/uapi/linux/uuid.h \
  include/linux/errseq.h \
  include/linux/ioprio.h \
  include/linux/sched/rt.h \
  include/linux/iocontext.h \
    $(wildcard include/config/BLK_ICQ) \
  include/uapi/linux/ioprio.h \
  include/linux/fs_types.h \
  include/linux/mount.h \
  include/linux/mnt_idmapping.h \
  include/linux/slab.h \
    $(wildcard include/config/DEBUG_SLAB) \
    $(wildcard include/config/FAILSLAB) \
    $(wildcard include/config/KFENCE) \
    $(wildcard include/config/SLAB) \
    $(wildcard include/config/SLUB) \
    $(wildcard include/config/SLOB) \
  include/linux/percpu-refcount.h \
  include/linux/kasan.h \
    $(wildcard include/config/KASAN_STACK) \
    $(wildcard include/config/KASAN_VMALLOC) \
  include/linux/kasan-enabled.h \
  include/uapi/linux/fs.h \
  include/linux/quota.h \
    $(wildcard include/config/QUOTA_NETLINK_INTERFACE) \
  include/uapi/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/projid.h \
  include/uapi/linux/quota.h \
  include/linux/nfs_fs_i.h \
  include/net/net_namespace.h \
    $(wildcard include/config/NF_CONNTRACK) \
    $(wildcard include/config/NF_FLOW_TABLE) \
    $(wildcard include/config/UNIX) \
    $(wildcard include/config/IPV6) \
    $(wildcard include/config/IEEE802154_6LOWPAN) \
    $(wildcard include/config/IP_SCTP) \
    $(wildcard include/config/NETFILTER) \
    $(wildcard include/config/NF_TABLES) \
    $(wildcard include/config/WEXT_CORE) \
    $(wildcard include/config/XFRM) \
    $(wildcard include/config/IP_VS) \
    $(wildcard include/config/MPLS) \
    $(wildcard include/config/CAN) \
    $(wildcard include/config/XDP_SOCKETS) \
    $(wildcard include/config/MCTP) \
    $(wildcard include/config/CRYPTO_USER) \
    $(wildcard include/config/SMC) \
    $(wildcard include/config/NET_NS) \
    $(wildcard include/config/NET_NS_REFCNT_TRACKER) \
  include/net/flow.h \
  include/linux/socket.h \
  arch/x86/include/generated/uapi/asm/socket.h \
  include/uapi/asm-generic/socket.h \
  arch/x86/include/generated/uapi/asm/sockios.h \
  include/uapi/asm-generic/sockios.h \
  include/uapi/linux/sockios.h \
  include/linux/uio.h \
  include/uapi/linux/uio.h \
  include/uapi/linux/socket.h \
  include/linux/in6.h \
  include/uapi/linux/in6.h \
  include/uapi/linux/libc-compat.h \
  include/net/flow_dissector.h \
  include/linux/siphash.h \
    $(wildcard include/config/HAVE_EFFICIENT_UNALIGNED_ACCESS) \
  include/uapi/linux/if_ether.h \
  include/net/netns/core.h \
  include/net/netns/mib.h \
    $(wildcard include/config/XFRM_STATISTICS) \
    $(wildcard include/config/TLS) \
    $(wildcard include/config/MPTCP) \
  include/net/snmp.h \
    $(wildcard include/config/PROC_STRIPPED) \
  include/uapi/linux/snmp.h \
  include/linux/u64_stats_sync.h \
  arch/x86/include/generated/asm/local64.h \
  include/asm-generic/local64.h \
  arch/x86/include/asm/local.h \
  include/net/netns/unix.h \
  include/net/netns/packet.h \
  include/net/netns/ipv4.h \
    $(wildcard include/config/IP_MULTIPLE_TABLES) \
    $(wildcard include/config/IP_ROUTE_CLASSID) \
    $(wildcard include/config/NET_L3_MASTER_DEV) \
    $(wildcard include/config/IP_MROUTE) \
    $(wildcard include/config/IP_MROUTE_MULTIPLE_TABLES) \
    $(wildcard include/config/IP_ROUTE_MULTIPATH) \
  include/net/inet_frag.h \
  include/net/netns/ipv6.h \
    $(wildcard include/config/IPV6_MULTIPLE_TABLES) \
    $(wildcard include/config/IPV6_SUBTREES) \
    $(wildcard include/config/IPV6_MROUTE) \
    $(wildcard include/config/IPV6_MROUTE_MULTIPLE_TABLES) \
    $(wildcard include/config/NF_DEFRAG_IPV6) \
  include/net/dst_ops.h \
  include/uapi/linux/icmpv6.h \
  include/net/netns/nexthop.h \
  include/net/netns/ieee802154_6lowpan.h \
  include/net/netns/sctp.h \
  include/net/netns/netfilter.h \
    $(wildcard include/config/NETFILTER_FAMILY_ARP) \
    $(wildcard include/config/NETFILTER_FAMILY_BRIDGE) \
    $(wildcard include/config/NF_DEFRAG_IPV4) \
  include/linux/netfilter_defs.h \
  include/uapi/linux/netfilter.h \
  include/linux/in.h \
  include/uapi/linux/in.h \
  include/net/netns/conntrack.h \
    $(wildcard include/config/NF_CT_PROTO_DCCP) \
    $(wildcard include/config/NF_CT_PROTO_SCTP) \
    $(wildcard include/config/NF_CT_PROTO_GRE) \
    $(wildcard include/config/NF_CONNTRACK_EVENTS) \
    $(wildcard include/config/NF_CONNTRACK_CHAIN_EVENTS) \
  include/linux/netfilter/nf_conntrack_tcp.h \
  include/uapi/linux/netfilter/nf_conntrack_tcp.h \
  include/net/netns/nftables.h \
  include/net/netns/xfrm.h \
  include/uapi/linux/xfrm.h \
  include/net/netns/mpls.h \
  include/net/netns/can.h \
  include/net/netns/xdp.h \
  include/net/netns/smc.h \
  include/net/netns/bpf.h \
  include/net/netns/mctp.h \
  include/net/net_trackers.h \
    $(wildcard include/config/NET_DEV_REFCNT_TRACKER) \
  include/linux/ref_tracker.h \
    $(wildcard include/config/REF_TRACKER) \
  include/linux/stackdepot.h \
    $(wildcard include/config/STACKDEPOT) \
  include/linux/ns_common.h \
  include/linux/skbuff.h \
    $(wildcard include/config/IMQ) \
    $(wildcard include/config/BRIDGE_NETFILTER) \
    $(wildcard include/config/NET_TC_SKB_EXT) \
    $(wildcard include/config/NET_SOCK_MSG) \
    $(wildcard include/config/SKB_EXTENSIONS) \
    $(wildcard include/config/NET_CLS_ACT) \
    $(wildcard include/config/IPV6_NDISC_NODETYPE) \
    $(wildcard include/config/NET_SWITCHDEV) \
    $(wildcard include/config/NET_REDIRECT) \
    $(wildcard include/config/NETFILTER_SKIP_EGRESS) \
    $(wildcard include/config/TLS_DEVICE) \
    $(wildcard include/config/NET_SCHED) \
    $(wildcard include/config/NET_RX_BUSY_POLL) \
    $(wildcard include/config/XPS) \
    $(wildcard include/config/NETWORK_SECMARK) \
    $(wildcard include/config/DEBUG_NET) \
    $(wildcard include/config/PAGE_POOL) \
    $(wildcard include/config/NETWORK_PHY_TIMESTAMPING) \
    $(wildcard include/config/MCTP_FLOWS) \
    $(wildcard include/config/NET_DSA_TAG_OOB) \
    $(wildcard include/config/NETFILTER_XT_TARGET_TRACE) \
  include/linux/bvec.h \
  include/linux/highmem.h \
  include/linux/cacheflush.h \
  arch/x86/include/asm/cacheflush.h \
  include/linux/mm.h \
    $(wildcard include/config/HAVE_ARCH_MMAP_RND_BITS) \
    $(wildcard include/config/HAVE_ARCH_MMAP_RND_COMPAT_BITS) \
    $(wildcard include/config/ARCH_USES_HIGH_VMA_FLAGS) \
    $(wildcard include/config/ARCH_HAS_PKEYS) \
    $(wildcard include/config/PPC) \
    $(wildcard include/config/PARISC) \
    $(wildcard include/config/SPARC64) \
    $(wildcard include/config/ARM64) \
    $(wildcard include/config/ARM64_MTE) \
    $(wildcard include/config/HAVE_ARCH_USERFAULTFD_MINOR) \
    $(wildcard include/config/SHMEM) \
    $(wildcard include/config/MIGRATION) \
    $(wildcard include/config/ARCH_HAS_PTE_SPECIAL) \
    $(wildcard include/config/ARCH_HAS_PTE_DEVMAP) \
    $(wildcard include/config/DEBUG_VM_RB) \
    $(wildcard include/config/HAVE_FAST_GUP) \
    $(wildcard include/config/PAGE_POISONING) \
    $(wildcard include/config/INIT_ON_ALLOC_DEFAULT_ON) \
    $(wildcard include/config/INIT_ON_FREE_DEFAULT_ON) \
    $(wildcard include/config/DEBUG_PAGEALLOC) \
    $(wildcard include/config/HUGETLBFS) \
    $(wildcard include/config/MAPPING_DIRTY_HELPERS) \
    $(wildcard include/config/ANON_VMA_NAME) \
  include/linux/mmap_lock.h \
  include/linux/page_ext.h \
  include/linux/stacktrace.h \
    $(wildcard include/config/ARCH_STACKWALK) \
    $(wildcard include/config/STACKTRACE) \
    $(wildcard include/config/HAVE_RELIABLE_STACKTRACE) \
  include/linux/page_ref.h \
    $(wildcard include/config/DEBUG_PAGE_REF) \
  include/linux/sizes.h \
  include/linux/pgtable.h \
    $(wildcard include/config/HIGHPTE) \
    $(wildcard include/config/ARCH_HAS_NONLEAF_PMD_YOUNG) \
    $(wildcard include/config/GUP_GET_PTE_LOW_HIGH) \
    $(wildcard include/config/HAVE_ARCH_TRANSPARENT_HUGEPAGE_PUD) \
    $(wildcard include/config/HAVE_ARCH_SOFT_DIRTY) \
    $(wildcard include/config/ARCH_ENABLE_THP_MIGRATION) \
    $(wildcard include/config/HAVE_ARCH_HUGE_VMAP) \
    $(wildcard include/config/X86_ESPFIX64) \
  arch/x86/include/asm/pgtable.h \
    $(wildcard include/config/DEBUG_WX) \
    $(wildcard include/config/PAGE_TABLE_CHECK) \
  arch/x86/include/asm/pkru.h \
  arch/x86/include/asm/fpu/api.h \
    $(wildcard include/config/X86_DEBUG_FPU) \
  arch/x86/include/asm/coco.h \
  include/asm-generic/pgtable_uffd.h \
  include/linux/page_table_check.h \
  arch/x86/include/asm/pgtable_64.h \
  arch/x86/include/asm/fixmap.h \
    $(wildcard include/config/PROVIDE_OHCI1394_DMA_INIT) \
    $(wildcard include/config/X86_IO_APIC) \
    $(wildcard include/config/PCI_MMCONFIG) \
    $(wildcard include/config/ACPI_APEI_GHES) \
    $(wildcard include/config/INTEL_TXT) \
  arch/x86/include/uapi/asm/vsyscall.h \
  include/asm-generic/fixmap.h \
  arch/x86/include/asm/pgtable-invert.h \
  include/linux/memremap.h \
    $(wildcard include/config/DEVICE_PRIVATE) \
    $(wildcard include/config/PCI_P2PDMA) \
  include/linux/ioport.h \
  include/linux/huge_mm.h \
  include/linux/sched/coredump.h \
    $(wildcard include/config/CORE_DUMP_DEFAULT_ELF_HEADERS) \
  include/linux/vmstat.h \
    $(wildcard include/config/VM_EVENT_COUNTERS) \
    $(wildcard include/config/DEBUG_TLBFLUSH) \
  include/linux/vm_event_item.h \
    $(wildcard include/config/MEMORY_BALLOON) \
    $(wildcard include/config/BALLOON_COMPACTION) \
    $(wildcard include/config/ZSWAP) \
  include/asm-generic/cacheflush.h \
  include/linux/kmsan.h \
  include/linux/dma-direction.h \
  include/linux/hardirq.h \
  include/linux/context_tracking_state.h \
    $(wildcard include/config/CONTEXT_TRACKING_USER) \
    $(wildcard include/config/CONTEXT_TRACKING) \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/HWLAT_TRACER) \
    $(wildcard include/config/OSNOISE_TRACER) \
  include/linux/vtime.h \
    $(wildcard include/config/VIRT_CPU_ACCOUNTING) \
    $(wildcard include/config/IRQ_TIME_ACCOUNTING) \
  arch/x86/include/asm/hardirq.h \
    $(wildcard include/config/KVM_INTEL) \
    $(wildcard include/config/HAVE_KVM) \
    $(wildcard include/config/X86_THERMAL_VECTOR) \
    $(wildcard include/config/X86_MCE_THRESHOLD) \
    $(wildcard include/config/X86_MCE_AMD) \
    $(wildcard include/config/X86_HV_CALLBACK_VECTOR) \
    $(wildcard include/config/HYPERV) \
  include/linux/highmem-internal.h \
  include/linux/net.h \
  include/linux/sockptr.h \
  include/uapi/linux/net.h \
  include/linux/textsearch.h \
  include/net/checksum.h \
  arch/x86/include/asm/checksum.h \
    $(wildcard include/config/GENERIC_CSUM) \
  arch/x86/include/asm/checksum_64.h \
  include/linux/dma-mapping.h \
    $(wildcard include/config/DMA_API_DEBUG) \
    $(wildcard include/config/HAS_DMA) \
    $(wildcard include/config/NEED_DMA_MAP_STATE) \
  include/linux/device.h \
    $(wildcard include/config/GENERIC_MSI_IRQ_DOMAIN) \
    $(wildcard include/config/GENERIC_MSI_IRQ) \
    $(wildcard include/config/ENERGY_MODEL) \
    $(wildcard include/config/PINCTRL) \
    $(wildcard include/config/DMA_OPS) \
    $(wildcard include/config/DMA_DECLARE_COHERENT) \
    $(wildcard include/config/DMA_CMA) \
    $(wildcard include/config/SWIOTLB) \
    $(wildcard include/config/ARCH_HAS_SYNC_DMA_FOR_DEVICE) \
    $(wildcard include/config/ARCH_HAS_SYNC_DMA_FOR_CPU) \
    $(wildcard include/config/ARCH_HAS_SYNC_DMA_FOR_CPU_ALL) \
    $(wildcard include/config/DMA_OPS_BYPASS) \
    $(wildcard include/config/OF) \
    $(wildcard include/config/DEVTMPFS) \
    $(wildcard include/config/SYSFS_DEPRECATED) \
  include/linux/dev_printk.h \
  include/linux/energy_model.h \
  include/linux/sched/cpufreq.h \
    $(wildcard include/config/CPU_FREQ) \
  include/linux/sched/topology.h \
    $(wildcard include/config/SCHED_DEBUG) \
    $(wildcard include/config/SCHED_CLUSTER) \
    $(wildcard include/config/SCHED_MC) \
    $(wildcard include/config/CPU_FREQ_GOV_SCHEDUTIL) \
  include/linux/sched/idle.h \
  include/linux/sched/sd_flags.h \
  include/linux/klist.h \
  include/linux/pm.h \
    $(wildcard include/config/VT_CONSOLE_SLEEP) \
    $(wildcard include/config/CXL_SUSPEND) \
    $(wildcard include/config/PM) \
    $(wildcard include/config/PM_CLK) \
    $(wildcard include/config/PM_GENERIC_DOMAINS) \
  include/linux/device/bus.h \
    $(wildcard include/config/ACPI) \
  include/linux/device/class.h \
  include/linux/device/driver.h \
  arch/x86/include/asm/device.h \
  include/linux/pm_wakeup.h \
  include/linux/scatterlist.h \
    $(wildcard include/config/NEED_SG_DMA_LENGTH) \
    $(wildcard include/config/DEBUG_SG) \
    $(wildcard include/config/SGL_ALLOC) \
    $(wildcard include/config/ARCH_NO_SG_CHAIN) \
    $(wildcard include/config/SG_POOL) \
  arch/x86/include/asm/io.h \
    $(wildcard include/config/MTRR) \
    $(wildcard include/config/X86_PAT) \
  arch/x86/include/generated/asm/early_ioremap.h \
  include/asm-generic/early_ioremap.h \
    $(wildcard include/config/GENERIC_EARLY_IOREMAP) \
  arch/x86/include/asm/shared/io.h \
  include/asm-generic/iomap.h \
    $(wildcard include/config/HAS_IOPORT_MAP) \
  include/asm-generic/pci_iomap.h \
    $(wildcard include/config/PCI) \
    $(wildcard include/config/NO_GENERIC_PCI_IOPORT_MAP) \
    $(wildcard include/config/GENERIC_PCI_IOMAP) \
  include/asm-generic/io.h \
    $(wildcard include/config/GENERIC_IOMAP) \
    $(wildcard include/config/TRACE_MMIO_ACCESS) \
    $(wildcard include/config/GENERIC_IOREMAP) \
  include/linux/logic_pio.h \
    $(wildcard include/config/INDIRECT_PIO) \
  include/linux/fwnode.h \
  include/linux/vmalloc.h \
    $(wildcard include/config/HAVE_ARCH_HUGE_VMALLOC) \
  arch/x86/include/asm/vmalloc.h \
  arch/x86/include/asm/pgtable_areas.h \
  include/linux/netdev_features.h \
  include/linux/sched/clock.h \
    $(wildcard include/config/HAVE_UNSTABLE_SCHED_CLOCK) \
  include/linux/splice.h \
  include/linux/pipe_fs_i.h \
  include/uapi/linux/if_packet.h \
  include/net/page_pool.h \
    $(wildcard include/config/PAGE_POOL_STATS) \
  include/linux/ptr_ring.h \
  include/linux/netfilter/nf_conntrack_common.h \
  include/uapi/linux/netfilter/nf_conntrack_common.h \
  include/linux/imq.h \
  include/net/net_debug.h \
  include/net/dropreason.h \
  include/linux/seq_file_net.h \
  include/linux/seq_file.h \
  include/linux/string_helpers.h \
  include/linux/ctype.h \
  include/net/netns/generic.h \
  include/linux/ip.h \
  include/uapi/linux/ip.h \
  include/linux/ipv6.h \
    $(wildcard include/config/IPV6_ROUTER_PREF) \
    $(wildcard include/config/IPV6_ROUTE_INFO) \
    $(wildcard include/config/IPV6_OPTIMISTIC_DAD) \
    $(wildcard include/config/IPV6_SEG6_HMAC) \
    $(wildcard include/config/IPV6_MIP6) \
  include/uapi/linux/ipv6.h \
  include/linux/tcp.h \
    $(wildcard include/config/BPF) \
    $(wildcard include/config/TCP_MD5SIG) \
  include/linux/win_minmax.h \
  include/net/sock.h \
    $(wildcard include/config/SOCK_RX_QUEUE_MAPPING) \
    $(wildcard include/config/SOCK_VALIDATE_XMIT) \
    $(wildcard include/config/RPS) \
    $(wildcard include/config/SOCK_CGROUP_DATA) \
    $(wildcard include/config/INET) \
  include/linux/netdevice.h \
    $(wildcard include/config/DCB) \
    $(wildcard include/config/HYPERV_NET) \
    $(wildcard include/config/WLAN) \
    $(wildcard include/config/AX25) \
    $(wildcard include/config/MAC80211_MESH) \
    $(wildcard include/config/NET_IPIP) \
    $(wildcard include/config/NET_IPGRE) \
    $(wildcard include/config/IPV6_SIT) \
    $(wildcard include/config/IPV6_TUNNEL) \
    $(wildcard include/config/NETPOLL) \
    $(wildcard include/config/BQL) \
    $(wildcard include/config/RFS_ACCEL) \
    $(wildcard include/config/FCOE) \
    $(wildcard include/config/XFRM_OFFLOAD) \
    $(wildcard include/config/NET_POLL_CONTROLLER) \
    $(wildcard include/config/LIBFCOE) \
    $(wildcard include/config/WIRELESS_EXT) \
    $(wildcard include/config/ETHERNET_PACKET_MANGLE) \
    $(wildcard include/config/VLAN_8021Q) \
    $(wildcard include/config/NET_DSA) \
    $(wildcard include/config/TIPC) \
    $(wildcard include/config/ATALK) \
    $(wildcard include/config/CFG80211_HEADERS) \
    $(wildcard include/config/IEEE802154) \
    $(wildcard include/config/6LOWPAN) \
    $(wildcard include/config/MPLS_ROUTING) \
    $(wildcard include/config/NETFILTER_INGRESS) \
    $(wildcard include/config/NETFILTER_EGRESS) \
    $(wildcard include/config/PCPU_DEV_REFCNT) \
    $(wildcard include/config/GARP) \
    $(wildcard include/config/MRP) \
    $(wildcard include/config/NET_DROP_MONITOR) \
    $(wildcard include/config/CGROUP_NET_PRIO) \
    $(wildcard include/config/MACSEC) \
    $(wildcard include/config/NET_FLOW_LIMIT) \
    $(wildcard include/config/NET_EGRESS) \
    $(wildcard include/config/ETHTOOL_NETLINK) \
  include/linux/delay.h \
  arch/x86/include/asm/delay.h \
  include/asm-generic/delay.h \
  include/linux/prefetch.h \
  include/linux/dynamic_queue_limits.h \
  include/net/netprio_cgroup.h \
  include/linux/cgroup.h \
    $(wildcard include/config/CGROUP_CPUACCT) \
    $(wildcard include/config/CGROUP_DATA) \
    $(wildcard include/config/CGROUP_BPF) \
  include/uapi/linux/cgroupstats.h \
  include/uapi/linux/taskstats.h \
  include/linux/nsproxy.h \
  include/linux/user_namespace.h \
    $(wildcard include/config/INOTIFY_USER) \
    $(wildcard include/config/FANOTIFY) \
    $(wildcard include/config/PERSISTENT_KEYRINGS) \
  include/linux/kernel_stat.h \
  include/linux/interrupt.h \
    $(wildcard include/config/IRQ_FORCED_THREADING) \
    $(wildcard include/config/GENERIC_IRQ_PROBE) \
    $(wildcard include/config/IRQ_TIMINGS) \
  include/linux/irqreturn.h \
  arch/x86/include/asm/irq.h \
  arch/x86/include/asm/irq_vectors.h \
    $(wildcard include/config/PCI_MSI) \
  arch/x86/include/asm/sections.h \
  include/asm-generic/sections.h \
    $(wildcard include/config/HAVE_FUNCTION_DESCRIPTORS) \
  include/linux/cgroup-defs.h \
    $(wildcard include/config/CGROUP_NET_CLASSID) \
  include/linux/bpf-cgroup-defs.h \
    $(wildcard include/config/BPF_LSM) \
  include/linux/psi_types.h \
  include/linux/kthread.h \
  include/linux/cgroup_subsys.h \
    $(wildcard include/config/CGROUP_DEVICE) \
    $(wildcard include/config/CGROUP_FREEZER) \
    $(wildcard include/config/CGROUP_PERF) \
    $(wildcard include/config/CGROUP_HUGETLB) \
    $(wildcard include/config/CGROUP_PIDS) \
    $(wildcard include/config/CGROUP_RDMA) \
    $(wildcard include/config/CGROUP_MISC) \
    $(wildcard include/config/CGROUP_DEBUG) \
  include/net/xdp.h \
  include/uapi/linux/neighbour.h \
  include/linux/netlink.h \
  include/net/scm.h \
    $(wildcard include/config/SECURITY_NETWORK) \
  include/linux/security.h \
    $(wildcard include/config/SECURITY_INFINIBAND) \
    $(wildcard include/config/SECURITY_NETWORK_XFRM) \
    $(wildcard include/config/SECURITY_PATH) \
    $(wildcard include/config/SECURITYFS) \
  include/linux/kernel_read_file.h \
  include/linux/file.h \
  include/uapi/linux/netlink.h \
  include/uapi/linux/netdevice.h \
  include/uapi/linux/if.h \
  include/uapi/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/if_link.h \
  include/uapi/linux/if_link.h \
  include/uapi/linux/if_bonding.h \
  include/uapi/linux/pkt_cls.h \
  include/uapi/linux/pkt_sched.h \
  include/linux/hashtable.h \
  include/linux/page_counter.h \
  include/linux/memcontrol.h \
  include/linux/vmpressure.h \
  include/linux/eventfd.h \
  include/linux/writeback.h \
  include/linux/flex_proportions.h \
  include/linux/backing-dev-defs.h \
    $(wildcard include/config/DEBUG_FS) \
  include/linux/blk_types.h \
    $(wildcard include/config/FAIL_MAKE_REQUEST) \
    $(wildcard include/config/BLK_CGROUP_IOCOST) \
    $(wildcard include/config/BLK_INLINE_ENCRYPTION) \
    $(wildcard include/config/BLK_DEV_INTEGRITY) \
  include/linux/rculist_nulls.h \
  include/linux/poll.h \
  include/uapi/linux/poll.h \
  arch/x86/include/generated/uapi/asm/poll.h \
  include/uapi/asm-generic/poll.h \
  include/uapi/linux/eventpoll.h \
  include/linux/indirect_call_wrapper.h \
  include/net/dst.h \
  include/linux/rtnetlink.h \
    $(wildcard include/config/NET_INGRESS) \
  include/uapi/linux/rtnetlink.h \
  include/uapi/linux/if_addr.h \
  include/net/neighbour.h \
  include/net/rtnetlink.h \
  include/net/netlink.h \
  include/net/tcp_states.h \
  include/uapi/linux/net_tstamp.h \
  include/net/l3mdev.h \
  include/net/fib_rules.h \
  include/uapi/linux/fib_rules.h \
  include/net/fib_notifier.h \
  include/net/inet_connection_sock.h \
  include/net/inet_sock.h \
  include/linux/jhash.h \
  include/linux/unaligned/packed_struct.h \
  include/net/request_sock.h \
  include/net/netns/hash.h \
  include/net/inet_timewait_sock.h \
  include/net/timewait_sock.h \
  include/uapi/linux/tcp.h \
  include/linux/udp.h \
  include/uapi/linux/udp.h \
  include/linux/icmp.h \
  include/uapi/linux/icmp.h \
  include/uapi/linux/errqueue.h \
  include/linux/inetdevice.h \
  include/linux/netfilter/x_tables.h \
    $(wildcard include/config/NETFILTER_XTABLES_COMPAT) \
  include/linux/netfilter.h \
    $(wildcard include/config/NF_NAT) \
  include/linux/netfilter/nf_conntrack_zones_common.h \
  include/uapi/linux/netfilter/nf_conntrack_tuple_common.h \
  include/uapi/linux/netfilter/x_tables.h \
  include/linux/netfilter_ipv4.h \
  include/uapi/linux/netfilter_ipv4.h \
  include/net/netfilter/nf_conntrack.h \
    $(wildcard include/config/NETFILTER_DEBUG) \
    $(wildcard include/config/NF_CONNTRACK_ZONES) \
    $(wildcard include/config/NF_CONNTRACK_SECMARK) \
    $(wildcard include/config/NETFILTER_XT_MATCH_LAYER7) \
  include/linux/netfilter/nf_conntrack_dccp.h \
  include/uapi/linux/netfilter/nf_conntrack_tuple_common.h \
  include/linux/netfilter/nf_conntrack_sctp.h \
  include/uapi/linux/netfilter/nf_conntrack_sctp.h \
  include/linux/netfilter/nf_conntrack_proto_gre.h \
  include/net/gre.h \
  include/net/ip_tunnels.h \
    $(wildcard include/config/DST_CACHE) \
    $(wildcard include/config/IPV6_SIT_6RD) \
  include/linux/if_tunnel.h \
  include/uapi/linux/if_tunnel.h \
  include/net/dsfield.h \
  include/net/gro_cells.h \
  include/net/inet_ecn.h \
  include/linux/if_vlan.h \
  include/linux/etherdevice.h \
  include/linux/crc32.h \
  include/linux/bitrev.h \
    $(wildcard include/config/HAVE_ARCH_BITREVERSE) \
  arch/x86/include/generated/asm/unaligned.h \
  include/asm-generic/unaligned.h \
  include/uapi/linux/if_vlan.h \
  include/net/lwtunnel.h \
    $(wildcard include/config/LWTUNNEL) \
  include/uapi/linux/lwtunnel.h \
  include/net/route.h \
  include/net/inetpeer.h \
  include/net/ipv6.h \
  include/linux/jump_label_ratelimit.h \
  include/net/if_inet6.h \
  include/net/inet_dscp.h \
  include/net/ip_fib.h \
  include/net/arp.h \
    $(wildcard include/config/KERNEL_ARP_SPOOFING_PROTECT) \
  include/linux/if_arp.h \
    $(wildcard include/config/FIREWIRE_NET) \
  include/uapi/linux/if_arp.h \
  include/net/ndisc.h \
  include/net/ipv6_stubs.h \
  include/linux/icmpv6.h \
  include/uapi/linux/in_route.h \
  include/uapi/linux/route.h \
  include/net/dst_cache.h \
  include/net/ip6_fib.h \
  include/linux/ipv6_route.h \
  include/uapi/linux/ipv6_route.h \
  include/uapi/linux/bpf.h \
    $(wildcard include/config/BPF_LIRC_MODE2) \
    $(wildcard include/config/EFFICIENT_UNALIGNED_ACCESS) \
    $(wildcard include/config/BPF_KPROBE_OVERRIDE) \
  include/uapi/linux/bpf_common.h \
  include/net/ip6_route.h \
  include/net/addrconf.h \
  include/net/nexthop.h \
  include/net/pptp.h \
  include/net/netfilter/nf_conntrack_tuple.h \
  include/net/netfilter/ipv4/nf_conntrack_ipv4.h \
    $(wildcard include/config/NF_CT_PROTO_UDPLITE) \
  include/net/netfilter/ipv6/nf_conntrack_ipv6.h \
  include/net/netfilter/nf_conntrack_extend.h \
    $(wildcard include/config/NF_CONNTRACK_TIMESTAMP) \
    $(wildcard include/config/NF_CONNTRACK_TIMEOUT) \
    $(wildcard include/config/NETFILTER_SYNPROXY) \
    $(wildcard include/config/NET_ACT_CT) \
  include/net/netfilter/nf_nat.h \
    $(wildcard include/config/NF_NAT_PPTP) \
    $(wildcard include/config/NF_NAT_MASQUERADE) \
  include/linux/netfilter/nf_conntrack_pptp.h \
  include/net/netfilter/nf_conntrack_expect.h \
  include/net/netfilter/nf_conntrack_zones.h \
  include/uapi/linux/netfilter/nf_nat.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_config.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_api.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_main.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_config.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_includes.h \
  include/uapi/linux/times.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_define.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_protocol_ids.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_kernel_compat.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_typedefs.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_patricia_typedefs.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_api.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_protocols.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_private.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/xt_ndpi.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_main.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/../lib/third_party/include/ndpi_patricia.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_patricia_typedefs.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_strcol.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_flow_info.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_main_netfilter.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/../lib/third_party/include/ahocorasick.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_main_common.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_generic.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_parsers.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_info.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_flow.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_hostdef.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_ipdef.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../libre/regexp.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_strcol.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_parsers.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_generic.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_info.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_flow.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_hostdef.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/ndpi_proc_ipdef.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../libre/regexp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_kernel_compat.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../libre/regexp.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_main.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_private.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ahocorasick.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/libcache.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_kernel_compat.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt_light.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_content_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_dga_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_azure_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_tor_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_whatsapp_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_amazon_aws_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_ethereum_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_zoom_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_cachefly_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_cloudflare_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_ms_office365_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_ms_onedrive_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_ms_outlook_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_ms_skype_teams_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_google_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_google_cloud_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_crawlers_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_icloud_private_relay_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_protonvpn_in_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_protonvpn_out_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_mullvad_match.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_telegram.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_apple.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_twitter.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_netflix.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_webex.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_teamviewer.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_facebook.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_tencent.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_opendns.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_dropbox.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_starcraft.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_ubuntuone.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_twitch.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_hotspotshield.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_github.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_steam.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_bloomberg.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_edgecast.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_goto.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_riotgames.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_threema.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_alibaba.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_avast.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_discord.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_line.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_vk.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_yandex.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_yandex_cloud.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_disney_plus.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_hulu.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_epicgames.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_nvidia.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/inc_generated/ndpi_asn_roblox.c.inc \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_patricia.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_md5.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_sha256.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_typedefs.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_sha1.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/ndpi_md5.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_md5.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/ndpi_sha1.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_sha1.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/ndpi_sha256.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_sha256.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/ahocorasick.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/../../ndpi_replace_printf.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/libcache.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/ndpi_patricia.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/ndpi_patricia.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt_light.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/common.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/error.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/aes.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/cipher.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/gcm.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/digest.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/gcrypt/cipher_wrap.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt/aes.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt/gcm.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt/cipher.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt/cipher_wrap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/gcrypt/digest.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/btlib.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/btlib.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/src/hll/MurmurHash3.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/MurmurHash3.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_includes.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/afp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_protocol_ids.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ajp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/amazon_video.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/among_us.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/amqp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/apple_push.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/armagetron.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/avast_securedns.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/bgp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/bittorrent.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/bjnp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/capwap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/cassandra.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/checkmk.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ciscovpn.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/citrix.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/coap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/collectd.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/corba.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/cpha.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/crossfire.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dcerpc.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dhcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dhcpv6.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/diameter.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dnp3.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dns.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dnscrypt.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dofus.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/drda.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/dropbox.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/eaq.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/edonkey.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ethernet_ip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/fix.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ftp_control.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ftp_data.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/genshin_impact.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/git.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/gnutella.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/gtp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/guildwars.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/h323.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/halflife2_and_mods.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/thrift.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hpvirtgrp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hsrp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/http.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/i3d.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/iax.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/icecast.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/iec60870-5-104.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/imo.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ipp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ipsec.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/irc.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/jabber.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kakaotalk_voice.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kerberos.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kontiki.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ldap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/lisp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/lotus_notes.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mail_imap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mail_pop.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mail_smtp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/maplestory.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/megaco.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/memcached.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mgcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mining.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/modbus.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mongodb.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mpegdash.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mpegts.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mqtt.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mssql_tds.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mysql.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/nats.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/nest_log_sink.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/netbios.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/netflow.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/nfs.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/nintendo.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/noe.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/non_tcp_udp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ntp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ookla.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/openvpn.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/oracle.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/postgres.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ppstream.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/pptp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/qq.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/quic.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/radius.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/raknet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rdp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/riotgames.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rsh.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rsync.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rtcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rtmp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rtp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rtsp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rx.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/s7comm.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/sd_rtn.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/sflow.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/sip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/skinny.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/skype.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/smb.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/smpp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/snmp_proto.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/soap.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/socks45.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/someip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/spotify.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ssdp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ssh.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/starcraft.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/steam.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/stun.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/syslog.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tcp_udp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/teamspeak.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/teamviewer.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/telegram.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/telnet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/teredo.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tftp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/threema.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tinc.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tls.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/include/ndpi_encryption.h \
  include/linux/sort.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tocaboca.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tvuplayer.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ubntac2.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ultrasurf.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/usenet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/vhua.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/viber.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/vmware.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/vnc.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/vxlan.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/warcraft3.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/websocket.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/whatsapp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/whoisdas.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/wireguard.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/world_of_kung_fu.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/world_of_warcraft.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/wsd.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/xbox.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/xdmcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/xiaomi.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/z3950.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/zabbix.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/zattoo.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/zeromq.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/discord.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/avast.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/alicloud.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/activision.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/crynet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/fastcgi.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kismet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/line.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/munin.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/natpmp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/softether.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/syncthing.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tivoconnect.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tplink_shp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/elastic_search.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tailscale.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tuya_lp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/merakicloud.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/bacnet.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hots.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/oicq.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/source_engine.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/bitcoin.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/epicgames.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/slp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/haproxy.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hislip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/beckhoff_ads.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/can.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ethereum.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ethersbus.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ethersio.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/fins.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hart-ip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ieee-c37118.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/iso9506-1-mms.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/monero.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/opc-ua.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/profinet_io.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/protobuf.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ptpv2.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rmcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/rtps.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/http2.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/ceph.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/cip.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/gaijin_entertainment.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/gearman.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/hl7.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/iec62056.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/json-rpc.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kafka.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/kcp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/mumble.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/nomachine.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/openflow.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/radmin.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/raft.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/resp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/roughtime.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/steam_datagram_relay.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/stomp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/tencent_games.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/uftp.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/yojimbo.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/protocols/zoom.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_utils.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_replace_printf.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_hash.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/MurmurHash3.h \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_serializer.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_memory.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/ndpi_analyze.c \
  /home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/../../src/lib/third_party/include/hll.h \

/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o: $(deps_/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o)

$(deps_/home/seg/DEV/x86_64/src/router/ndpi-netfilter/ndpi-netfilter/src/main.o):
