#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include "sysdep.h"

extern void * ___brk_addr;

extern int __init_brk (void);

int brk(void * end_data_seg)
{
	if (__init_brk () == 0) {
		/* 
		 * Notice that we don't need to save/restore the GOT
		 * register since that is not call clobbered by the syscall.
		 */
		asm ("move.d %1,$r10\n\t"
		     "movu.w " STR(__NR_brk) ",$r9\n\t"
		     "break 13\n\t"
		     "move.d $r10, %0"
		     : "=r" (___brk_addr)
		     : "g" (end_data_seg)
		     : "r9", "r10");
		
		if (___brk_addr == end_data_seg)
			return 0;
		__set_errno(ENOMEM);
	}
	return -1;
	
}
