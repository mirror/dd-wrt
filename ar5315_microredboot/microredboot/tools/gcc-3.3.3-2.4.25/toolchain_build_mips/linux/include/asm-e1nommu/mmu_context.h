#ifndef _HYPERSTONE_MMU_CONTEXT_H_
#define _HYPERSTONE_MMU_CONTEXT_H_

#define destroy_context(mm)             do { } while(0)
#define init_new_context(tsk,mm)        0
#define switch_mm(prev,next,tsk,cpu)    do { } while(0)
#define activate_mm(prev,next)		do { } while(0)
#define enter_lazy_tlb(mm,tsk,cpu)	do { } while(0)

#endif
