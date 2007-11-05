	.file	1 "asm-offsets.c"
	.section .mdebug.abi32
	.previous

 # -G value = 0, Arch = mips32, ISA = 32
 # GNU C version 4.1.2 (mips-linux-uclibc)
 #	compiled by GNU C version 4.0.2 20050901 (prerelease) (SUSE Linux).
 # GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
 # options passed:  -nostdinc -Iinclude -Iinclude/asm-mips/mach-atheros
 # -Iinclude/asm-mips/mach-generic -iprefix -D__KERNEL__
 # -DVMLINUX_LOAD_ADDRESS=0xffffffff80041000 -DKBUILD_STR(s)=#s
 # -DKBUILD_BASENAME=KBUILD_STR(asm_offsets)
 # -DKBUILD_MODNAME=KBUILD_STR(asm_offsets) -isystem -include -MD -mabi=32
 # -mno-abicalls -msoft-float -march=mips32 -auxbase-strip -Os -Wall
 # -Wundef -Wstrict-prototypes -Wno-trigraphs
 # -Werror-implicit-function-declaration -Wdeclaration-after-statement
 # -Wno-pointer-sign -fno-strict-aliasing -fno-common -fno-pic
 # -ffreestanding -fomit-frame-pointer -fno-stack-protector
 # -funit-at-a-time -fverbose-asm
 # options enabled:  -falign-loops -fargument-alias -fbranch-count-reg
 # -fcaller-saves -fcprop-registers -fcrossjumping -fcse-follow-jumps
 # -fcse-skip-blocks -fdefer-pop -fdelete-null-pointer-checks
 # -fearly-inlining -feliminate-unused-debug-types
 # -fexpensive-optimizations -ffunction-cse -fgcse -fgcse-lm
 # -fguess-branch-probability -fident -fif-conversion -fif-conversion2
 # -finline-functions -finline-functions-called-once -fipa-pure-const
 # -fipa-reference -fipa-type-escape -fivopts -fkeep-static-consts
 # -fleading-underscore -floop-optimize -floop-optimize2 -fmath-errno
 # -fmerge-constants -fomit-frame-pointer -foptimize-register-move
 # -foptimize-sibling-calls -fpcc-struct-return -fpeephole -fpeephole2
 # -fregmove -freorder-functions -frerun-cse-after-loop -frerun-loop-opt
 # -fsched-interblock -fsched-spec -fsched-stalled-insns-dep
 # -fschedule-insns -fschedule-insns2 -fshow-column -fsplit-ivs-in-unroller
 # -fstrength-reduce -fthread-jumps -ftrapping-math -ftree-ccp
 # -ftree-copy-prop -ftree-copyrename -ftree-dce -ftree-dominator-opts
 # -ftree-dse -ftree-fre -ftree-loop-im -ftree-loop-ivcanon
 # -ftree-loop-optimize -ftree-lrs -ftree-salias -ftree-sink -ftree-sra
 # -ftree-store-ccp -ftree-store-copy-prop -ftree-ter
 # -ftree-vect-loop-version -ftree-vrp -funit-at-a-time -fverbose-asm
 # -fzero-initialized-in-bss -mcheck-zero-division -mdivide-traps
 # -mdouble-float -meb -mexplicit-relocs -mfp-exceptions -mfp32
 # -mfused-madd -mgp32 -mlong32 -mno-mips16 -mno-mips3d -msoft-float
 # -msplit-addresses

 # Compiler executable checksum: 1b6311ee8c3264a5a72275de71fd3c9f

#APP
	.macro _ssnop; sll $0, $0, 1; .endm
	.macro _ehb; sll $0, $0, 3; .endm
	.macro mtc0_tlbw_hazard; nop; nop; .endm
	.macro tlbw_use_hazard; nop; nop; nop; .endm
	.macro tlb_probe_hazard; nop; nop; nop; .endm
	.macro irq_enable_hazard; _ssnop; _ssnop; _ssnop;; .endm
	.macro irq_disable_hazard; nop; nop; nop; .endm
	.macro back_to_back_c0_hazard; _ssnop; _ssnop; _ssnop;; .endm
	.macro enable_fpu_hazard; nop; nop; nop; nop; .endm
	.macro disable_fpu_hazard; _ehb; .endm
		.macro	raw_local_irq_enable				
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1e						
	mtc0	$1,$12						
	irq_enable_hazard					
	.set	pop						
	.endm
		.macro	raw_local_irq_disable
	.set	push						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1f						
	.set	noreorder					
	mtc0	$1,$12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_save_flags flags			
	.set	push						
	.set	reorder						
	mfc0	\flags, $12					
	.set	pop						
	.endm							

		.macro	raw_local_irq_save result			
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	\result, $12					
	ori	$1, \result, 0x1f				
	xori	$1, 0x1f					
	.set	noreorder					
	mtc0	$1, $12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_irq_restore flags			
	.set	push						
	.set	noreorder					
	.set	noat						
	mfc0	$1, $12						
	andi	\flags, 1					
	ori	$1, 0x1f					
	xori	$1, 0x1f					
	or	\flags, $1					
	mtc0	\flags, $12					
	irq_disable_hazard					
	.set	pop						
	.endm							

#NO_APP
	.text
	.align	2
	.globl	output_ptreg_defines
	.ent	output_ptreg_defines
	.type	output_ptreg_defines, @function
output_ptreg_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* MIPS pt_regs offsets. */
	
@@@#define PT_R0     24	 #
	
@@@#define PT_R1     28	 #
	
@@@#define PT_R2     32	 #
	
@@@#define PT_R3     36	 #
	
@@@#define PT_R4     40	 #
	
@@@#define PT_R5     44	 #
	
@@@#define PT_R6     48	 #
	
@@@#define PT_R7     52	 #
	
@@@#define PT_R8     56	 #
	
@@@#define PT_R9     60	 #
	
@@@#define PT_R10    64	 #
	
@@@#define PT_R11    68	 #
	
@@@#define PT_R12    72	 #
	
@@@#define PT_R13    76	 #
	
@@@#define PT_R14    80	 #
	
@@@#define PT_R15    84	 #
	
@@@#define PT_R16    88	 #
	
@@@#define PT_R17    92	 #
	
@@@#define PT_R18    96	 #
	
@@@#define PT_R19    100	 #
	
@@@#define PT_R20    104	 #
	
@@@#define PT_R21    108	 #
	
@@@#define PT_R22    112	 #
	
@@@#define PT_R23    116	 #
	
@@@#define PT_R24    120	 #
	
@@@#define PT_R25    124	 #
	
@@@#define PT_R26    128	 #
	
@@@#define PT_R27    132	 #
	
@@@#define PT_R28    136	 #
	
@@@#define PT_R29    140	 #
	
@@@#define PT_R30    144	 #
	
@@@#define PT_R31    148	 #
	
@@@#define PT_LO     160	 #
	
@@@#define PT_HI     156	 #
	
@@@#define PT_EPC    172	 #
	
@@@#define PT_BVADDR 164	 #
	
@@@#define PT_STATUS 152	 #
	
@@@#define PT_CAUSE  168	 #
	
@@@#define PT_SIZE   176	 #
	
@@@
#NO_APP
	j	$31
	.end	output_ptreg_defines
	.align	2
	.globl	output_task_defines
	.ent	output_task_defines
	.type	output_task_defines, @function
output_task_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* MIPS task_struct offsets. */
	
@@@#define TASK_STATE         0	 #
	
@@@#define TASK_THREAD_INFO   4	 #
	
@@@#define TASK_FLAGS         12	 #
	
@@@#define TASK_MM            176	 #
	
@@@#define TASK_PID           212	 #
	
@@@#define TASK_STRUCT_SIZE   1056	 #
	
@@@
#NO_APP
	j	$31
	.end	output_task_defines
	.align	2
	.globl	output_thread_info_defines
	.ent	output_thread_info_defines
	.type	output_thread_info_defines, @function
output_thread_info_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* MIPS thread_info offsets. */
	
@@@#define TI_TASK            0	 #
	
@@@#define TI_EXEC_DOMAIN     4	 #
	
@@@#define TI_FLAGS           8	 #
	
@@@#define TI_TP_VALUE	   12	 #
	
@@@#define TI_CPU             16	 #
	
@@@#define TI_PRE_COUNT       20	 #
	
@@@#define TI_ADDR_LIMIT      24	 #
	
@@@#define TI_RESTART_BLOCK   28	 #
	
@@@#define TI_REGS            48	 #
	
@@@#define _THREAD_SIZE       0x2000	 #
	
@@@#define _THREAD_MASK       0x1fff	 #
	
@@@
#NO_APP
	j	$31
	.end	output_thread_info_defines
	.align	2
	.globl	output_thread_defines
	.ent	output_thread_defines
	.type	output_thread_defines, @function
output_thread_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* MIPS specific thread_struct offsets. */
	
@@@#define THREAD_REG16   488	 #
	
@@@#define THREAD_REG17   492	 #
	
@@@#define THREAD_REG18   496	 #
	
@@@#define THREAD_REG19   500	 #
	
@@@#define THREAD_REG20   504	 #
	
@@@#define THREAD_REG21   508	 #
	
@@@#define THREAD_REG22   512	 #
	
@@@#define THREAD_REG23   516	 #
	
@@@#define THREAD_REG29   520	 #
	
@@@#define THREAD_REG30   524	 #
	
@@@#define THREAD_REG31   528	 #
	
@@@#define THREAD_STATUS  532	 #
	
@@@#define THREAD_FPU     536	 #
	
@@@#define THREAD_BVADDR  828	 #
	
@@@#define THREAD_BUADDR  832	 #
	
@@@#define THREAD_ECODE   836	 #
	
@@@#define THREAD_TRAPNO  840	 #
	
@@@#define THREAD_TRAMP   844	 #
	
@@@#define THREAD_OLDCTX  848	 #
	
@@@
#NO_APP
	j	$31
	.end	output_thread_defines
	.align	2
	.globl	output_thread_fpu_defines
	.ent	output_thread_fpu_defines
	.type	output_thread_fpu_defines, @function
output_thread_fpu_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@#define THREAD_FPR0    536	 #
	
@@@#define THREAD_FPR1    544	 #
	
@@@#define THREAD_FPR2    552	 #
	
@@@#define THREAD_FPR3    560	 #
	
@@@#define THREAD_FPR4    568	 #
	
@@@#define THREAD_FPR5    576	 #
	
@@@#define THREAD_FPR6    584	 #
	
@@@#define THREAD_FPR7    592	 #
	
@@@#define THREAD_FPR8    600	 #
	
@@@#define THREAD_FPR9    608	 #
	
@@@#define THREAD_FPR10   616	 #
	
@@@#define THREAD_FPR11   624	 #
	
@@@#define THREAD_FPR12   632	 #
	
@@@#define THREAD_FPR13   640	 #
	
@@@#define THREAD_FPR14   648	 #
	
@@@#define THREAD_FPR15   656	 #
	
@@@#define THREAD_FPR16   664	 #
	
@@@#define THREAD_FPR17   672	 #
	
@@@#define THREAD_FPR18   680	 #
	
@@@#define THREAD_FPR19   688	 #
	
@@@#define THREAD_FPR20   696	 #
	
@@@#define THREAD_FPR21   704	 #
	
@@@#define THREAD_FPR22   712	 #
	
@@@#define THREAD_FPR23   720	 #
	
@@@#define THREAD_FPR24   728	 #
	
@@@#define THREAD_FPR25   736	 #
	
@@@#define THREAD_FPR26   744	 #
	
@@@#define THREAD_FPR27   752	 #
	
@@@#define THREAD_FPR28   760	 #
	
@@@#define THREAD_FPR29   768	 #
	
@@@#define THREAD_FPR30   776	 #
	
@@@#define THREAD_FPR31   784	 #
	
@@@#define THREAD_FCR31   792	 #
	
@@@
#NO_APP
	j	$31
	.end	output_thread_fpu_defines
	.align	2
	.globl	output_mm_defines
	.ent	output_mm_defines
	.type	output_mm_defines, @function
output_mm_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* Size of struct page  */
	
@@@#define STRUCT_PAGE_SIZE   32	 #
	
@@@
	
@@@/* Linux mm_struct offsets. */
	
@@@#define MM_USERS      40	 #
	
@@@#define MM_PGD        36	 #
	
@@@#define MM_CONTEXT    344	 #
	
@@@
	
@@@#define _PAGE_SIZE     0x1000	 #
	
@@@#define _PAGE_SHIFT    0xc	 #
	
@@@
	
@@@#define _PGD_T_SIZE    0x4	 #
	
@@@#define _PMD_T_SIZE    0x4	 #
	
@@@#define _PTE_T_SIZE    0x4	 #
	
@@@
	
@@@#define _PGD_T_LOG2    0x2	 #
	
@@@#define _PMD_T_LOG2    0x2	 #
	
@@@#define _PTE_T_LOG2    0x2	 #
	
@@@
	
@@@#define _PGD_ORDER     0x0	 #
	
@@@#define _PMD_ORDER     0x1	 #
	
@@@#define _PTE_ORDER     0x0	 #
	
@@@
	
@@@#define _PMD_SHIFT     0x16	 #
	
@@@#define _PGDIR_SHIFT   0x16	 #
	
@@@
	
@@@#define _PTRS_PER_PGD  0x400	 #
	
@@@#define _PTRS_PER_PMD  0x1	 #
	
@@@#define _PTRS_PER_PTE  0x400	 #
	
@@@
#NO_APP
	j	$31
	.end	output_mm_defines
	.align	2
	.globl	output_sc_defines
	.ent	output_sc_defines
	.type	output_sc_defines, @function
output_sc_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* Linux sigcontext offsets. */
	
@@@#define SC_REGS       16	 #
	
@@@#define SC_FPREGS     272	 #
	
@@@#define SC_ACX        528	 #
	
@@@#define SC_MDHI       552	 #
	
@@@#define SC_MDLO       560	 #
	
@@@#define SC_PC         8	 #
	
@@@#define SC_FPC_CSR    532	 #
	
@@@#define SC_FPC_EIR    536	 #
	
@@@#define SC_HI1        568	 #
	
@@@#define SC_LO1        572	 #
	
@@@#define SC_HI2        576	 #
	
@@@#define SC_LO2        580	 #
	
@@@#define SC_HI3        584	 #
	
@@@#define SC_LO3        588	 #
	
@@@
#NO_APP
	j	$31
	.end	output_sc_defines
	.align	2
	.globl	output_signal_defined
	.ent	output_signal_defined
	.type	output_signal_defined, @function
output_signal_defined:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* Linux signal numbers. */
	
@@@#define _SIGHUP     0x1	 #
	
@@@#define _SIGINT     0x2	 #
	
@@@#define _SIGQUIT    0x3	 #
	
@@@#define _SIGILL     0x4	 #
	
@@@#define _SIGTRAP    0x5	 #
	
@@@#define _SIGIOT     0x6	 #
	
@@@#define _SIGABRT    0x6	 #
	
@@@#define _SIGEMT     0x7	 #
	
@@@#define _SIGFPE     0x8	 #
	
@@@#define _SIGKILL    0x9	 #
	
@@@#define _SIGBUS     0xa	 #
	
@@@#define _SIGSEGV    0xb	 #
	
@@@#define _SIGSYS     0xc	 #
	
@@@#define _SIGPIPE    0xd	 #
	
@@@#define _SIGALRM    0xe	 #
	
@@@#define _SIGTERM    0xf	 #
	
@@@#define _SIGUSR1    0x10	 #
	
@@@#define _SIGUSR2    0x11	 #
	
@@@#define _SIGCHLD    0x12	 #
	
@@@#define _SIGPWR     0x13	 #
	
@@@#define _SIGWINCH   0x14	 #
	
@@@#define _SIGURG     0x15	 #
	
@@@#define _SIGIO      0x16	 #
	
@@@#define _SIGSTOP    0x17	 #
	
@@@#define _SIGTSTP    0x18	 #
	
@@@#define _SIGCONT    0x19	 #
	
@@@#define _SIGTTIN    0x1a	 #
	
@@@#define _SIGTTOU    0x1b	 #
	
@@@#define _SIGVTALRM  0x1c	 #
	
@@@#define _SIGPROF    0x1d	 #
	
@@@#define _SIGXCPU    0x1e	 #
	
@@@#define _SIGXFSZ    0x1f	 #
	
@@@
#NO_APP
	j	$31
	.end	output_signal_defined
	.align	2
	.globl	output_irq_cpustat_t_defines
	.ent	output_irq_cpustat_t_defines
	.type	output_irq_cpustat_t_defines, @function
output_irq_cpustat_t_defines:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
#APP
	
@@@/* Linux irq_cpustat_t offsets. */
	
@@@#define IC_SOFTIRQ_PENDING 0	 #
	
@@@#define IC_IRQ_CPUSTAT_T   32	 #
	
@@@
#NO_APP
	j	$31
	.end	output_irq_cpustat_t_defines
	.ident	"GCC: (GNU) 4.1.2"
