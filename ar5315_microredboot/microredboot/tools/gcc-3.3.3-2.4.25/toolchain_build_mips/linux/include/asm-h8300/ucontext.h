#ifndef _H8300_UCONTEXT_H
#define _H8300_UCONTEXT_H

typedef int greg_t;
#define NGREG 10
typedef greg_t gregset_t[NGREG];

struct mcontext {
	int version;
	gregset_t gregs;
};

#define MCONTEXT_VERSION 1

struct ucontext {
	unsigned long	  uc_flags;
	struct ucontext  *uc_link;
	stack_t		  uc_stack;
	struct mcontext	  uc_mcontext;
	sigset_t	  uc_sigmask;	/* mask last for extensibility */
};

#endif
