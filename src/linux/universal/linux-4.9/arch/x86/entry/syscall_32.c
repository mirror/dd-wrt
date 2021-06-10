/* System call table for i386. */

#include <linux/linkage.h>
#include <linux/sys.h>
#include <linux/cache.h>
#include <asm/asm-offsets.h>
#include <asm/syscall.h>
#include <linux/syscalls.h>
#include <asm/syscalls.h>
#include <asm/sys_ia32.h>

#define __SYSCALL_I386(nr, sym, qual) [nr] = (sys_call_ptr_t)sym,


__visible const sys_call_ptr_t ia32_sys_call_table[__NR_syscall_compat_max+1] = {
	/*
	 * Smells like a compiler bug -- it doesn't work
	 * when the & below is removed.
	 */
	[0 ... __NR_syscall_compat_max] = (sys_call_ptr_t)&sys_ni_syscall,
#include <asm/syscalls_32.h>
};
