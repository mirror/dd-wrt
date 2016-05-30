/********************************************************************************
 * Copyright (c) 2003, 2004														*
 * Panagiotis E. Hadjidoukas	(peh@hpclab.ceid.upatras.gr)					*
 * HPCLab - University of Patras. All Rights Reserved.							*
 * Unix ucontext_t on Windows Operating System, May 2004 (revision 1)			*
 *																				*
 * The author disclaims all warranties with regard to this software including	*
 * all implied warranties of merchantability and fitness for a particular		*
 * purpose. In no event shall HPCLab be liable for any special, indirect,		*
 * or consequential damages or any damages whatsoever resulting from			*
 * loss of use, data or profits, whether in action of contract negligence,		*
 * or other tortious action, arising out of or in connection with the use		*
 * or performance of this software.												*
 ********************************************************************************/

#include "ucontext.h"

int getcontext(ucontext_t *ucp)
{
	int ret;

	/* Retrieve the full machine context */
	ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;
	ret = GetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);

	return (ret == 0) ? -1: 0;
}

int setcontext(const ucontext_t *ucp)
{
	int ret;
	
	/* Restore the full machine context (already set) */
	ret = SetThreadContext(GetCurrentThread(), &ucp->uc_mcontext);

	return (ret == 0) ? -1: 0;
}

int makecontext(ucontext_t *ucp, void (*func)(), int argc, ...)
{
	int i;
    va_list ap;
	char *sp;

	/* Stack grows down */
	sp = (char *) (size_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;	

	/* Reserve stack space for the arguments (maximum possible: argc*(8 bytes per argument)) */
	sp -= argc*8;

	if ( sp < (char *)ucp->uc_stack.ss_sp) {
		/* errno = ENOMEM;*/
		return -1;
	}

	/* Set the instruction and the stack pointer */
	ucp->uc_mcontext.Eip = (unsigned long) func;
	ucp->uc_mcontext.Esp = (unsigned long) sp - 4;

	/* Save/Restore the full machine context */
	ucp->uc_mcontext.ContextFlags = CONTEXT_FULL;

	/* Copy the arguments */
	va_start (ap, argc);
	for (i=0; i<argc; i++) {
		memcpy(sp, ap, 8);
		ap +=8;
		sp += 8;
	}
	va_end(ap);

	return 0;
}

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
	int ret;

	if ((oucp == NULL) || (ucp == NULL)) {
		/*errno = EINVAL;*/
		return -1;
	}

	ret = getcontext(oucp);
	if (ret == 0) {
		ret = setcontext(ucp);
	}
	return ret;
}