#ifndef _NIOS_PTRACE_H
#define _NIOS_PTRACE_H

#include <asm/psr.h>

/* This struct defines the way the registers are stored on the 
 * stack during a system call and basically all traps.
 */

#ifndef __ASSEMBLY__

struct pt_regs {
	unsigned long psr;
	unsigned long pc;
	long orig_i0;
	long sysc_no;
	unsigned long u_regs[16]; /* globals and ins */
};

//vic#define PT_FLAG_SUPERVISOR	0x00000001	/* Emulated supervisor privilage */

#define UREG_G0        0
#define UREG_G1        1
#define UREG_G2        2
#define UREG_G3        3
#define UREG_G4        4
#define UREG_G5        5
#define UREG_G6        6
#define UREG_G7        7
#define UREG_I0        8
#define UREG_I1        9
#define UREG_I2        10
#define UREG_I3        11
#define UREG_I4        12
#define UREG_I5        13
#define UREG_I6        14
#define UREG_I7        15
#define UREG_FADDR     UREG_G0
#define UREG_FP        UREG_I6
#define UREG_RETPC     UREG_I7

/* A register window */
struct reg_window {
	unsigned long locals[8];
	unsigned long ins[8];
};


/* A Nios stack frame */
struct nios_stackf {
	unsigned long locals[8];
	unsigned long ins[6];
	struct nios_stackf *fp;
	unsigned long callers_pc;
	char *structptr;
	unsigned long xargs[6];
	unsigned long xxargs[1];
};	

#define TRACEREG_SZ   sizeof(struct pt_regs)
#define STACKFRAME_SZ sizeof(struct nios_stackf)
#define REGWIN_SZ     sizeof(struct reg_window)

#ifdef __KERNEL__
#define user_mode(regs) (!((regs)->psr & PSR_SUPERVISOR))
#define instruction_pointer(regs) ((regs)->pc)
extern void show_regs(struct pt_regs *);
#endif

#else /* __ASSEMBLY__ */

/* For assembly code. REGWIN_SZ is used by CRT0 in uClibc */
#define REGWIN_SZ         0x40
/* these are now defined in nios-defs.h */

#endif /* __ASSEMBLY__ */


/* These are for pt_regs. */
#define PT_PSR    0x0
#define PT_PC     0x4
#define PT_ORIG_I0  0x8
#define PT_SYSC_NO 0xc
#define PT_G0     0x10
#define PT_G1     0x14
#define PT_G2     0x18
#define PT_G3     0x1c
#define PT_G4     0x20
#define PT_G5     0x24
#define PT_G6     0x28
#define PT_G7     0x2c
#define PT_I0     0x30
#define PT_I1     0x34
#define PT_I2     0x38
#define PT_I3     0x3c
#define PT_I4     0x40
#define PT_I5     0x44
#define PT_I6     0x48
#define PT_FP     PT_I6
#define PT_I7     0x4c

/* Reg_window offsets */
#define RW_L0     0x00
#define RW_L1     0x04
#define RW_L2     0x08
#define RW_L3     0x0c
#define RW_L4     0x10
#define RW_L5     0x14
#define RW_L6     0x18
#define RW_L7     0x1c
#define RW_I0     0x20
#define RW_I1     0x24
#define RW_I2     0x28
#define RW_I3     0x2c
#define RW_I4     0x30
#define RW_I5     0x34
#define RW_I6     0x38
#define RW_I7     0x3c


#endif /* !(_NIOS_PTRACE_H) */
