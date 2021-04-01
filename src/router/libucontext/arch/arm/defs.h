#ifndef __ARCH_ARM_DEFS_H

#define REG_SZ		(4)
#define MCONTEXT_GREGS	(32)

#define TYPE(__proc)

#define FETCH_LINKPTR(dest) \
	asm("movs    %0, r4" : "=r" ((dest)))

#include "common-defs.h"

#endif
