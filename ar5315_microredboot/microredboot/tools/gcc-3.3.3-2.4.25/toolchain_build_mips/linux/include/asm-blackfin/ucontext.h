/** Changes made by Tony Kou   Lineo Inc.    May 2001
 *
 *  Based on: include/m68knommu/ucontext.h  
 */

#ifndef _FRIONOMMU_UCONTEXT_H
#define _FRIONOMMU_UCONTEXT_H

typedef int greg_t;
#define NGREG 20
/* including: r0-7, p0-5, a0/1w, a0/1x, astat, rets */

typedef greg_t gregset_t[NGREG];

struct mcontext {
	int version;
	gregset_t gregs;
};

#define MCONTEXT_VERSION 2

struct ucontext {
	unsigned long	  uc_flags; /* the others are necessary */
	struct ucontext  *uc_link;
	stack_t		  uc_stack;
	struct mcontext	  uc_mcontext;
	sigset_t	  uc_sigmask;	/* mask last for extensibility */
};

#endif /* _FRIONOMMU_UCONTEXT_H */
