#ifndef _HYPERSTONE_IRQ_H_
#define _HYPERSTONE_IRQ_H_

#include <asm/ptrace.h>

#define NR_IRQS  8

#define IRQ_EXT1	0
#define IRQ_EXT2	1	
#define IRQ_EXT3	2		
#define IRQ_EXT		3
#define IRQ_TIMER	7	

#ifndef __ASSEMBLY__

extern void disable_irq(unsigned int);
extern void disable_irq_nosync(unsigned int);
extern void enable_irq(unsigned int);

extern void do_IRQ(struct pt_regs regs, unsigned int irq);

#endif /* !__ASSEMBLY__ */

#endif /* _HYPERSTONE_IRQ_H_ */
