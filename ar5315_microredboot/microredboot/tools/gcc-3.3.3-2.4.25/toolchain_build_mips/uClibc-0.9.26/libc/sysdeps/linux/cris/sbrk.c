/* From libc-5.3.12 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include "sysdep.h"

extern void * ___brk_addr;

extern int __init_brk (void);

void *
sbrk(intptr_t increment)
{
	if (__init_brk () == 0) {
		void * tmp = ___brk_addr + increment;

		/* 
		 * Notice that we don't need to save/restore the GOT
		 * register since that is not call clobbered by the syscall.
		 */
		asm ("move.d %1,$r10\n\t"
		     "movu.w " STR(__NR_brk) ",$r9\n\t"
		     "break 13\n\t"
		     "move.d $r10, %0"
		     : "=r" (___brk_addr)
		     : "g" (tmp)
		     : "r9", "r10");

		if (___brk_addr == tmp)
			return tmp - increment;
		__set_errno(ENOMEM);
		return ((void *) -1);
	}
	return ((void *) -1);
}
