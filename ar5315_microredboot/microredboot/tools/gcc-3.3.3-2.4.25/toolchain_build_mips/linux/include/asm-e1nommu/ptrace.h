#ifndef _HYPERSTONE_PTRACE_H_
#define _HYPERSTONE_PTRACE_H_

struct pt_regs{
	unsigned long *L;
	unsigned long  FL;
};

/* If interrupted task was in User Mode return 1,
 * else if it was in System Mode return 0 */
/*#define user_mode(regs)		(( (regs)->L [(regs)->FL] & 0x1) == 0)*/

static inline int user_mode(struct pt_regs *regs )
{
		return(( (regs)->L [(regs)->FL] & 0x1) == 0);
}

#endif
