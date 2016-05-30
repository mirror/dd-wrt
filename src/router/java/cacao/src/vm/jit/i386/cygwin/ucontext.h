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

#ifndef UCONTEXT_H
#define UCONTEXT_H

#include <windows.h>

typedef struct __stack {
	void *ss_sp;
	size_t ss_size;
	int ss_flags;
} stack_t;

typedef CONTEXT mcontext_t;
typedef unsigned long __sigset_t;

typedef struct __ucontext {
	unsigned long int	uc_flags;
	struct __ucontext	*uc_link;
	stack_t				uc_stack;
	mcontext_t			uc_mcontext; 
	__sigset_t			uc_sigmask;
} ucontext_t;


int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);
int makecontext(ucontext_t *, void (*)(), int, ...);
int swapcontext(ucontext_t *, const ucontext_t *);

#endif /* UCONTEXT_H */
