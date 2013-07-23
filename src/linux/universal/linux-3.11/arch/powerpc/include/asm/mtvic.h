#ifndef _ASM_POWERPC_MTVIC_H
#define _ASM_POWERPC_MTVIC_H

void __init mtvic_init(int def);
unsigned mtvic_get_irq(void);
unsigned rb_get_irq(void);

#endif
