	.file	"asm-offsets.c"

 # rs6000/powerpc options: -msdata=data -G 8
 # GNU C version 4.1.2 (powerpc-linux-uclibc)
 #	compiled by GNU C version 4.1.2 20061115 (prerelease) (SUSE Linux).
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc -Iinclude -Iarch/ppc -Iarch/ppc/include
 # -Iarch/ppc -iprefix -D__unix__ -D__gnu_linux__ -D__linux__ -Dunix
 # -D__unix -Dlinux -D__linux -Asystem=linux -Asystem=unix -Asystem=posix
 # -D__KERNEL__ -DKBUILD_STR(s)=#s
 # -DKBUILD_BASENAME=KBUILD_STR(asm_offsets)
 # -DKBUILD_MODNAME=KBUILD_STR(asm_offsets) -isystem -include -MD -m32
 # -msoft-float -mmultiple -mno-altivec -mstring -auxbase-strip -O2 -Wall
 # -Wundef -Wstrict-prototypes -Wno-trigraphs
 # -Werror-implicit-function-declaration -Wdeclaration-after-statement
 # -Wno-pointer-sign -fno-strict-aliasing -fno-common -ffixed-r2
 # -fomit-frame-pointer -fno-stack-protector -funit-at-a-time -fverbose-asm
 # options enabled:  -falign-loops -fargument-alias -fbranch-count-reg
 # -fcaller-saves -fcprop-registers -fcrossjumping -fcse-follow-jumps
 # -fcse-skip-blocks -fdefer-pop -fearly-inlining
 # -feliminate-unused-debug-types -fexpensive-optimizations -ffunction-cse
 # -fgcse -fgcse-lm -fguess-branch-probability -fident -fif-conversion
 # -fif-conversion2 -finline-functions-called-once -fipa-pure-const
 # -fipa-reference -fipa-type-escape -fivopts -fkeep-static-consts
 # -fleading-underscore -floop-optimize -floop-optimize2 -fmath-errno
 # -fmerge-constants -fomit-frame-pointer -foptimize-register-move
 # -foptimize-sibling-calls -fpeephole -fpeephole2 -freg-struct-return
 # -fregmove -freorder-blocks -freorder-functions -frerun-cse-after-loop
 # -frerun-loop-opt -fsched-interblock -fsched-spec
 # -fsched-stalled-insns-dep -fschedule-insns -fschedule-insns2
 # -fshow-column -fsplit-ivs-in-unroller -fstrength-reduce -fthread-jumps
 # -ftrapping-math -ftree-ccp -ftree-ch -ftree-copy-prop -ftree-copyrename
 # -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-fre -ftree-loop-im
 # -ftree-loop-ivcanon -ftree-loop-optimize -ftree-lrs -ftree-pre
 # -ftree-salias -ftree-sink -ftree-sra -ftree-store-ccp
 # -ftree-store-copy-prop -ftree-ter -ftree-vect-loop-version -ftree-vrp
 # -funit-at-a-time -fverbose-asm -fzero-initialized-in-bss -m32
 # -maix-struct-return -mbig -mbig-endian -mbss-plt -mfp-in-toc
 # -mfused-madd -mmultiple -mnew-mnemonics -mpowerpc -msched-prolog
 # -msoft-float -mstring -mupdate

 # Compiler executable checksum: aaeed27dc624c9a605ea623582fd180f

	.section	".text"
	.align 2
	.globl main
	.type	main, @function
main:
#APP
	
->THREAD 496 offsetof(struct task_struct, thread)	 #
	
->THREAD_INFO 4 offsetof(struct task_struct, stack)	 #
	
->MM 176 offsetof(struct task_struct, mm)	 #
	
->PTRACE 16 offsetof(struct task_struct, ptrace)	 #
	
->KSP 0 offsetof(struct thread_struct, ksp)	 #
	
->PGDIR 12 offsetof(struct thread_struct, pgdir)	 #
	
->PT_REGS 4 offsetof(struct thread_struct, regs)	 #
	
->THREAD_FPEXC_MODE 288 offsetof(struct thread_struct, fpexc_mode)	 #
	
->THREAD_FPR0 24 offsetof(struct thread_struct, fpr[0])	 #
	
->THREAD_FPSCR 280 offsetof(struct thread_struct, fpscr)	 #
	
->THREAD_DBCR0 16 offsetof(struct thread_struct, dbcr0)	 #
	
->PT_PTRACED 1 PT_PTRACED	 #
	
->STACK_FRAME_OVERHEAD 16 STACK_FRAME_OVERHEAD	 #
	
->INT_FRAME_SIZE 192 STACK_FRAME_OVERHEAD + sizeof(struct pt_regs)	 #
	
->GPR0 16 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[0])	 #
	
->GPR1 20 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[1])	 #
	
->GPR2 24 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[2])	 #
	
->GPR3 28 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[3])	 #
	
->GPR4 32 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[4])	 #
	
->GPR5 36 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[5])	 #
	
->GPR6 40 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[6])	 #
	
->GPR7 44 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[7])	 #
	
->GPR8 48 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[8])	 #
	
->GPR9 52 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[9])	 #
	
->GPR10 56 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[10])	 #
	
->GPR11 60 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[11])	 #
	
->GPR12 64 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[12])	 #
	
->GPR13 68 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[13])	 #
	
->GPR14 72 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[14])	 #
	
->GPR15 76 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[15])	 #
	
->GPR16 80 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[16])	 #
	
->GPR17 84 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[17])	 #
	
->GPR18 88 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[18])	 #
	
->GPR19 92 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[19])	 #
	
->GPR20 96 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[20])	 #
	
->GPR21 100 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[21])	 #
	
->GPR22 104 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[22])	 #
	
->GPR23 108 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[23])	 #
	
->GPR24 112 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[24])	 #
	
->GPR25 116 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[25])	 #
	
->GPR26 120 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[26])	 #
	
->GPR27 124 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[27])	 #
	
->GPR28 128 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[28])	 #
	
->GPR29 132 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[29])	 #
	
->GPR30 136 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[30])	 #
	
->GPR31 140 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, gpr[31])	 #
	
->_NIP 144 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, nip)	 #
	
->_MSR 148 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, msr)	 #
	
->_CTR 156 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, ctr)	 #
	
->_LINK 160 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, link)	 #
	
->_CCR 168 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, ccr)	 #
	
->_MQ 172 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, mq)	 #
	
->_XER 164 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, xer)	 #
	
->_DAR 180 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, dar)	 #
	
->_DSISR 184 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, dsisr)	 #
	
->_DEAR 180 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, dar)	 #
	
->_ESR 184 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, dsisr)	 #
	
->ORIG_GPR3 152 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, orig_gpr3)	 #
	
->RESULT 188 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, result)	 #
	
->TRAP 176 STACK_FRAME_OVERHEAD+offsetof(struct pt_regs, trap)	 #
	
->CLONE_VM 256 CLONE_VM	 #
	
->CLONE_UNTRACED 8388608 CLONE_UNTRACED	 #
	
->MM_PGD 36 offsetof(struct mm_struct, pgd)	 #
	
->CPU_SPEC_ENTRY_SIZE 68 sizeof(struct cpu_spec)	 #
	
->CPU_SPEC_PVR_MASK 0 offsetof(struct cpu_spec, pvr_mask)	 #
	
->CPU_SPEC_PVR_VALUE 4 offsetof(struct cpu_spec, pvr_value)	 #
	
->CPU_SPEC_FEATURES 12 offsetof(struct cpu_spec, cpu_features)	 #
	
->CPU_SPEC_SETUP 36 offsetof(struct cpu_spec, cpu_setup)	 #
	
->TI_TASK 0 offsetof(struct thread_info, task)	 #
	
->TI_EXECDOMAIN 4 offsetof(struct thread_info, exec_domain)	 #
	
->TI_FLAGS 52 offsetof(struct thread_info, flags)	 #
	
->TI_LOCAL_FLAGS 48 offsetof(struct thread_info, local_flags)	 #
	
->TI_CPU 8 offsetof(struct thread_info, cpu)	 #
	
->TI_PREEMPT 12 offsetof(struct thread_info, preempt_count)	 #
	
->pbe_address 0 offsetof(struct pbe, address)	 #
	
->pbe_orig_address 4 offsetof(struct pbe, orig_address)	 #
	
->pbe_next 8 offsetof(struct pbe, next)	 #
	
->TASK_SIZE -2147483648 TASK_SIZE	 #
	
->NUM_USER_SEGMENTS 8 TASK_SIZE>>28	 #
	
->CFG_TB_ORIG_STAMP 0 offsetof(struct vdso_data, tb_orig_stamp)	 #
	
->CFG_TB_TICKS_PER_SEC 8 offsetof(struct vdso_data, tb_ticks_per_sec)	 #
	
->CFG_TB_TO_XS 16 offsetof(struct vdso_data, tb_to_xs)	 #
	
->CFG_STAMP_XSEC 24 offsetof(struct vdso_data, stamp_xsec)	 #
	
->CFG_TB_UPDATE_COUNT 32 offsetof(struct vdso_data, tb_update_count)	 #
	
->CFG_TZ_MINUTEWEST 36 offsetof(struct vdso_data, tz_minuteswest)	 #
	
->CFG_TZ_DSTTIME 40 offsetof(struct vdso_data, tz_dsttime)	 #
	
->CFG_SYSCALL_MAP32 52 offsetof(struct vdso_data, syscall_map_32)	 #
	
->WTOM_CLOCK_SEC 44 offsetof(struct vdso_data, wtom_clock_sec)	 #
	
->WTOM_CLOCK_NSEC 48 offsetof(struct vdso_data, wtom_clock_nsec)	 #
	
->TVAL32_TV_SEC 0 offsetof(struct timeval, tv_sec)	 #
	
->TVAL32_TV_USEC 4 offsetof(struct timeval, tv_usec)	 #
	
->TSPEC32_TV_SEC 0 offsetof(struct timespec, tv_sec)	 #
	
->TSPEC32_TV_NSEC 4 offsetof(struct timespec, tv_nsec)	 #
	
->TZONE_TZ_MINWEST 0 offsetof(struct timezone, tz_minuteswest)	 #
	
->TZONE_TZ_DSTTIME 4 offsetof(struct timezone, tz_dsttime)	 #
	
->CLOCK_REALTIME 0 CLOCK_REALTIME	 #
	
->CLOCK_MONOTONIC 1 CLOCK_MONOTONIC	 #
	
->NSEC_PER_SEC 1000000000 NSEC_PER_SEC	 #
	
->CLOCK_REALTIME_RES 10000000 TICK_NSEC	 #
#NO_APP
	li 3,0	 # <result>,
	blr	 #
	.size	main, .-main
	.ident	"GCC: (GNU) 4.1.2"
	.section	.note.GNU-stack,"",@progbits
