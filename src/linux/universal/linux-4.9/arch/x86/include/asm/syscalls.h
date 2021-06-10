/*
 * syscalls.h - Linux syscall interfaces (arch-specific)
 *
 * Copyright (c) 2008 Jaswinder Singh Rajput
 *
 * This file is released under the GPLv2.
 * See the file COPYING for more details.
 */

#ifndef _ASM_X86_SYSCALLS_H
#define _ASM_X86_SYSCALLS_H

#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/compat.h>

/* Common in X86_32 and X86_64 */
/* kernel/ioport.c */
asmlinkage long sys_ioperm(unsigned long, unsigned long, int);
asmlinkage long sys_iopl(unsigned int);

/* kernel/ldt.c */
asmlinkage long sys_modify_ldt(int, void __user *, unsigned long);

/* kernel/signal.c */
asmlinkage long sys_rt_sigreturn(void);

/* kernel/tls.c */
asmlinkage long sys_set_thread_area(struct user_desc __user *);
asmlinkage long sys_get_thread_area(struct user_desc __user *);

/* X86_32 only */
#ifdef CONFIG_X86_32

/* kernel/signal.c */
asmlinkage unsigned long sys_sigreturn(void);

/* kernel/vm86_32.c */
struct vm86_struct;
asmlinkage long sys_vm86old(struct vm86_struct __user *);
asmlinkage long sys_vm86(unsigned long, unsigned long);

#else /* CONFIG_X86_32 */

/* X86_64 only */
/* kernel/process_64.c */
asmlinkage long sys_arch_prctl(int, unsigned long);

/* kernel/sys_x86_64.c */
asmlinkage long sys_mmap(unsigned long, unsigned long, unsigned long,
			 unsigned long, unsigned long, unsigned long);

asmlinkage long ptregs_sys_rt_sigreturn(struct pt_regs *regs);
asmlinkage long ptregs_sys_fork(struct pt_regs *regs);
asmlinkage long ptregs_sys_vfork(struct pt_regs *regs);
asmlinkage long ptregs_sys_execve(const char __user *filename,
		const char __user *const __user *argv,
		const char __user *const __user *envp);
asmlinkage long ptregs_sys_iopl(unsigned int);
asmlinkage long ptregs_sys_execveat(int dfd, const char __user *filename,
			const char __user *const __user *argv,
			const char __user *const __user *envp, int flags);
asmlinkage long ptregs_sys_clone(unsigned long, unsigned long, int __user *,
	       int __user *, unsigned long);

#ifdef CONFIG_COMPAT
asmlinkage long compat_sys_pwritev64v2(unsigned long fd,
	       const struct compat_iovec __user *vec,
	       compat_ulong_t vlen, u32 pos_low, u32 pos_high);
asmlinkage long compat_sys_preadv64v2(unsigned long fd,
	       const struct compat_iovec __user *vec,
	       unsigned long vlen, loff_t pos, int flags);
asmlinkage long ptregs_compat_sys_execve(unsigned long dfd,
		 const char __user *filename,
		 const compat_uptr_t __user *argv,
		 const compat_uptr_t __user *envp);
asmlinkage long ptregs_compat_sys_execveat(int dfd, const char __user *filename,
		     const compat_uptr_t __user *argv,
		     const compat_uptr_t __user *envp, int flags);
asmlinkage long compat_sys_old_getrlimit(unsigned int resource,
	struct compat_rlimit __user *rlim);
asmlinkage long stub32_clone(unsigned, unsigned, int __user *,
	       compat_uptr_t __user *, unsigned);
#endif

asmlinkage long sys32_x32_rt_sigreturn(void);


#endif /* !CONFIG_X86_32 */
#endif /* _ASM_X86_SYSCALLS_H */
