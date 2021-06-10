/* System call table for x86-64. */

#include <linux/linkage.h>
#include <linux/sys.h>
#include <linux/cache.h>
#include <linux/syscalls.h>
#include <linux/compat.h>
#include <linux/syscalls.h>
#include <linux/compat.h>
#include <asm/asm-offsets.h>
#include <asm/syscall.h>
#include <asm/syscalls.h>

#define __SYSCALL_64_QUAL_(sym) sym
#define __SYSCALL_64_QUAL_ptregs(sym) ptregs_##sym
#define __SYSCALL_64(nr, sym, qual) [nr] = (sys_call_ptr_t)__SYSCALL_64_QUAL_(sym),

asmlinkage const sys_call_ptr_t sys_call_table[__NR_syscall_max+1] = {
	/*
	 * Smells like a compiler bug -- it doesn't work
	 * when the & below is removed.
	 */
	[0 ... __NR_syscall_max] = (sys_call_ptr_t)&sys_ni_syscall,
#include <asm/syscalls_64.h>
};
