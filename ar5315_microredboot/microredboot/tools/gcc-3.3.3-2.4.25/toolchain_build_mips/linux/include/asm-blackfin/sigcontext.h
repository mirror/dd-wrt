#ifndef _ASM_FRIONOMMU_SIGCONTEXT_H
#define _ASM_FRIONOMMU_SIGCONTEXT_H

struct sigcontext {
	unsigned long  sc_mask; 	/* old sigmask */
	unsigned long  sc_usp;		/* old user stack pointer */
	unsigned long  sc_r0;
	unsigned long  sc_r1;
	unsigned long  sc_p0;
	unsigned long  sc_p1;
	unsigned short sc_seqstat;
	unsigned long  sc_pc;
};

#endif
