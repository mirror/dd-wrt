#ifndef _NIOS_CURRENT_H
#define _NIOS_CURRENT_H

struct task_struct;

static inline struct task_struct * get_current(void)
{
	struct task_struct *current;
	__asm__("mov %0, %%sp\n\t"
		"lsri %0, 13\n\t"
		"lsli %0, 13\n\t"
		:"=r" (current));
	return current;
 }
 
#define current get_current()

#endif /* _NIOS_CURRENT_H */
