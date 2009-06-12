#ifndef _ASMnios_SIGCONTEXT_H
#define _ASMnios_SIGCONTEXT_H

#include <asm/ptrace.h>

struct sigcontext {
	struct pt_regs 	regs;   /* this must be first */
	int oldmask;            /* sigmask to restore */
};

#endif /* !(_ASMnios_SIGCONTEXT_H) */

