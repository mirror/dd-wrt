#ifndef _HYPERSTONE_NOMMU_SYSCALLS_H_
#define _HYPERSTONE_NOMMU_SYSCALLS_H_

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/stat.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/utsname.h>
#include <linux/utime.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include <asm/poll.h>

struct mmap_arg_struct {
	unsigned long addr;
	unsigned long len;
	unsigned long prot;
	unsigned long flags;
	unsigned long fd;
	unsigned long offset;
};

struct sel_arg_struct {
	unsigned long n;
	fd_set *inp, *outp, *exp;
	struct timeval *tvp;
};

extern asmlinkage long sys_ni_syscall(void);
extern asmlinkage long sys_exit(int);
extern asmlinkage int sys_fork(struct pt_regs regs);
extern asmlinkage ssize_t sys_read(unsigned int, char *, size_t);
extern asmlinkage ssize_t sys_write(unsigned int, const char *, size_t);
extern asmlinkage long sys_open(const char *, int, int);
extern asmlinkage long sys_close(unsigned int);
extern asmlinkage long sys_waitpid(pid_t, unsigned int *, int);
extern asmlinkage long sys_creat(const char *, int);
extern asmlinkage long sys_link(const char *, const char *);
extern asmlinkage long sys_unlink(const char *);
extern asmlinkage int sys_execve(struct pt_regs regs);
extern asmlinkage long sys_chdir(const char *);
extern asmlinkage long sys_time(int *);
extern asmlinkage long sys_mknod(const char *, int, dev_t);
extern asmlinkage long sys_chmod(const char *, mode_t);
extern asmlinkage long sys_chown(const char *, uid_t,gid_t);
/* sys_break is not used */
/* sys_oldstat is not used */
extern asmlinkage off_t sys_lseek(unsigned int, off_t, unsigned int);
extern asmlinkage long sys_getpid(void);
extern asmlinkage long sys_mount(char *, char *, char *, unsigned long, void *);
extern asmlinkage long sys_umount(char *, int);
extern asmlinkage long sys_setuid(uid_t);
extern asmlinkage long sys_getuid(void);
extern asmlinkage long sys_stime(int *);
extern asmlinkage int sys_ptrace(long request, long pid, long addr, long data);
extern asmlinkage long sys_alarm(unsigned int);
/* sys_oldfstat is not used */
extern asmlinkage int sys_pause(void);
extern asmlinkage long sys_utime(char *, struct utimbuf *);
/* sys_stty is not used */
/* sys_gtty is not used */
extern asmlinkage long sys_access(const char *, int);
extern asmlinkage long sys_nice(int);
/* sys_ftime is not used */
extern asmlinkage long sys_sync(void);
extern asmlinkage long sys_kill(int, int);
extern asmlinkage long sys_rename(const char *, const char *);
extern asmlinkage long sys_mkdir(const char *, int);
extern asmlinkage long sys_rmdir(const char *);
extern asmlinkage long sys_dup(int);
extern asmlinkage int sys_pipe(unsigned long *);
extern asmlinkage long sys_times(struct tms *);
/* sys_prof is not used */
extern asmlinkage long sys_brk(long);
extern asmlinkage long sys_setgid(gid_t);
extern asmlinkage long sys_getgid(void);
extern asmlinkage long sys_signal(int, __sighandler_t);
extern asmlinkage long sys_geteuid(void);
extern asmlinkage long sys_getegid(void);
extern asmlinkage long sys_acct(const char *);
/* sys_umount2 is not used */
/* sys_lock is not used */
extern asmlinkage long sys_ioctl(unsigned int, unsigned int, unsigned long);
extern asmlinkage long sys_fcntl(unsigned int, unsigned int, unsigned long);
/* sys_mpx is not used */
extern asmlinkage long sys_setpgid(pid_t, pid_t);
/* sys_ulimit is not used */
/* sys_oldolduname is not used */
extern asmlinkage long sys_umask(int);
extern asmlinkage long sys_chroot(const char *);
extern asmlinkage long sys_ustat(dev_t, struct ustat *);
extern asmlinkage long sys_dup2(int, int);
extern asmlinkage long sys_getppid(void);
extern asmlinkage long sys_getpgrp(void);
extern asmlinkage long sys_setsid(void);
extern asmlinkage long sys_sigaction(int, const struct sigaction *, struct sigaction *);
extern asmlinkage long sys_sgetmask(void);
extern asmlinkage long sys_ssetmask(int);
extern asmlinkage long sys_setreuid(uid_t, uid_t);
extern asmlinkage long sys_setregid(uid_t, uid_t);
extern asmlinkage long sys_sigsuspend(struct pt_regs);
extern asmlinkage long sys_sigpending(old_sigset_t *);
extern asmlinkage long sys_sethostname(char *, int);
extern asmlinkage long sys_setrlimit(unsigned int, struct rlimit *);
extern asmlinkage long sys_old_getrlimit(unsigned int, struct rlimit *);
extern asmlinkage long sys_getrusage(int, struct rusage *);
extern asmlinkage long sys_gettimeofday(struct timeval *, struct timezone *);
extern asmlinkage long sys_settimeofday(struct timeval *, struct timezone *);
extern asmlinkage long sys_getgroups(int, gid_t *);
extern asmlinkage long sys_setgroups(int, gid_t *);
extern asmlinkage long sys_old_select(struct sel_arg_struct *);
extern asmlinkage long sys_symlink(const char *, const char *);
/* sys_oldlstat is not used */
extern asmlinkage long sys_readlink(const char *, char *, int);
extern asmlinkage long sys_uselib(const char *);
extern asmlinkage long sys_swapon(const char *, int);
extern asmlinkage long sys_reboot(int, int, unsigned int, void *);
/* sys_readdir is not used */
extern asmlinkage int sys_old_mmap(struct mmap_arg_struct *arg);
extern asmlinkage long sys_munmap(unsigned long, size_t);
extern asmlinkage long sys_truncate(const char *, unsigned long);
extern asmlinkage long sys_ftruncate(unsigned int, unsigned long);
extern asmlinkage long sys_fchmod(unsigned int, mode_t);
extern asmlinkage long sys_fchown(unsigned int, uid_t,gid_t);
extern asmlinkage long sys_getpriority(int, int);
extern asmlinkage long sys_setpriority(int, int,int);
/* sys_profil is not used */
extern asmlinkage long sys_statfs(char *, struct statfs *);
extern asmlinkage long sys_fstatfs(unsigned int, struct statfs *);
extern asmlinkage int sys_ioperm(unsigned long, unsigned long, int);
extern asmlinkage long sys_socketcall(int, unsigned long *);
extern asmlinkage long sys_syslog(int, char *, int);
extern asmlinkage long sys_setitimer(int, struct itimerval *, struct itimerval *);
extern asmlinkage long sys_getitimer(int, struct itimerval *);
//extern asmlinkage long sys_stat(char *, struct __old_kernel_stat *);
extern asmlinkage long sys_newstat(char *, struct __old_kernel_stat *);
//extern asmlinkage long sys_lstat(char *, struct __old_kernel_stat *);
extern asmlinkage long sys_newlstat(char *, struct __old_kernel_stat *);
//extern asmlinkage long sys_fstat(unsigned int, struct __old_kernel_stat *);
extern asmlinkage long sys_newfstat(unsigned int, struct __old_kernel_stat *);
extern asmlinkage long sys_olduname(struct old_utsname *);
/* sys_iopl is not available */
extern asmlinkage long sys_vhangup(void);
/* sys_idle is not used */
/* sys_vm86 is not used */
extern asmlinkage long sys_wait4(pid_t, unsigned int *, int, struct rusage *);
extern asmlinkage long sys_swapoff(const char *);
extern asmlinkage long sys_sysinfo(struct sysinfo *);
extern asmlinkage long sys_ipc (uint, int, int, int, void *, long);
extern asmlinkage long sys_fsync(unsigned int);
extern asmlinkage long sys_sigreturn(void);
extern asmlinkage long sys_clone(struct pt_regs);
extern asmlinkage long sys_setdomainname(char *, int);
extern asmlinkage long sys_newuname(struct new_utsname * name);
//extern asmlinkage long sys_uname(struct old_utsname *);
extern asmlinkage long sys_cacheflush (unsigned long, int, int, unsigned long);
extern asmlinkage long sys_adjtimex(struct timex *);
extern asmlinkage long sys_mprotect(unsigned long, size_t, unsigned long);
extern asmlinkage long sys_sigprocmask(int, old_sigset_t *, old_sigset_t *);
extern asmlinkage long sys_create_module(const char *, size_t);
extern asmlinkage long sys_init_module(const char *, struct module *);
extern asmlinkage long sys_delete_module(const char *);
extern asmlinkage long sys_get_kernel_syms(struct kernel_sym *);
extern asmlinkage long sys_quotactl(int, const char *, int, caddr_t);
extern asmlinkage long sys_getpgid(pid_t);
extern asmlinkage long sys_fchdir(unsigned int);
extern asmlinkage long sys_bdflush(int, long);
extern asmlinkage long sys_sysfs(int, unsigned long, unsigned long);
extern asmlinkage long sys_personality(unsigned long);
/* Andrew Filesystem */
extern asmlinkage long sys_setfsuid(uid_t);
extern asmlinkage long sys_setfsgid(gid_t);
extern asmlinkage long sys_llseek(unsigned int, unsigned long, unsigned long, loff_t *, unsigned int);
extern asmlinkage long sys_getdents(unsigned int, void *, unsigned int);
extern asmlinkage long sys_select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
extern asmlinkage long sys_flock(unsigned int, unsigned int);
extern asmlinkage long sys_msync(unsigned long, size_t, int);
extern asmlinkage long sys_readv(unsigned long, const struct iovec *, unsigned long);
extern asmlinkage long sys_writev(unsigned long, const struct iovec *, unsigned long);
extern asmlinkage long sys_getsid(pid_t);
extern asmlinkage long sys_fdatasync(unsigned int);
extern asmlinkage long sys_sysctl(struct __sysctl_args *);
extern asmlinkage long sys_mlock(unsigned long, size_t);
extern asmlinkage long sys_munlock(unsigned long, size_t);
extern asmlinkage long sys_mlockall(int);
extern asmlinkage long sys_munlockall(void);
extern asmlinkage long sys_sched_setparam(pid_t, struct sched_param *);
extern asmlinkage long sys_sched_getparam(pid_t, struct sched_param *);
extern asmlinkage long sys_sched_setscheduler(pid_t, int, struct sched_param *);
extern asmlinkage long sys_sched_getscheduler(pid_t);
extern asmlinkage long sys_sched_yield(void);
extern asmlinkage long sys_sched_get_priority_max(int);
extern asmlinkage long sys_sched_get_priority_min(int);
extern asmlinkage long sys_sched_rr_get_interval(pid_t, struct timespec *);
extern asmlinkage long sys_nanosleep(struct timespec *, struct timespec *);
extern asmlinkage long sys_mremap(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
extern asmlinkage long sys_setresuid(uid_t, uid_t, uid_t);
extern asmlinkage long sys_getresuid(uid_t *, uid_t *, uid_t *);
extern asmlinkage long sys_query_module(const char *, int, char *, size_t, size_t *);
extern asmlinkage long sys_poll(struct pollfd *, unsigned int, long);
extern asmlinkage long sys_nfsservctl(int, void *, void *);
extern asmlinkage long sys_setresgid(gid_t, gid_t, gid_t);
extern asmlinkage long sys_getresgid(gid_t *, gid_t *, gid_t *);
extern asmlinkage long sys_prctl(int, unsigned long, unsigned long, unsigned long, unsigned long);
/* sys_rt_sigreturn is not available */
extern asmlinkage long sys_rt_sigaction(int, const struct sigaction *, struct sigaction *, size_t);
extern asmlinkage long sys_rt_sigprocmask(int, sigset_t *, sigset_t *, size_t);
extern asmlinkage long sys_rt_sigpending(sigset_t *, size_t);
extern asmlinkage long sys_rt_sigtimedwait(const sigset_t *, siginfo_t *, const struct timespec *, size_t);
extern asmlinkage long sys_rt_sigqueueinfo(int, int, siginfo_t *);
extern asmlinkage long sys_rt_sigsuspend(struct pt_regs);
extern asmlinkage long sys_pread(unsigned int, char *, size_t, loff_t);
extern asmlinkage long sys_pwrite(unsigned int, const char *, size_t, loff_t);
extern asmlinkage long sys_lchown(const char *, uid_t,gid_t);
extern asmlinkage long sys_getcwd(char *, unsigned long);
extern asmlinkage long sys_capget(cap_user_header_t, cap_user_data_t);
extern asmlinkage long sys_capset(cap_user_header_t, const cap_user_data_t);
/* sys_sigaltstack is not available */
extern asmlinkage ssize_t sys_sendfile(int, int, off_t *, size_t);
/* sys_getpmsg is not used */
/* sys_putpmsg is not used */
extern asmlinkage int sys_vfork(struct pt_regs regs);
extern asmlinkage long sys_getrlimit(unsigned int resource, struct rlimit *rlim);
extern asmlinkage long sys_mmap2(	unsigned long, unsigned long,
				    				unsigned long, unsigned long,
					    			unsigned long, unsigned long );
extern asmlinkage long sys_truncate64(const char *, loff_t);
extern asmlinkage long sys_ftruncate64(unsigned int, loff_t);
extern asmlinkage long sys_stat64(char *, struct stat64 *, long);
extern asmlinkage long sys_lstat64(char *, struct stat64 *, long);
extern asmlinkage long sys_fstat64(unsigned long, struct stat64 *, long);
/* sys_chown32 is not used */
/* sys_getuid32 is not used */
/* sys_getgid32 is not used */
/* sys_geteuid32 is not used */
/* sys_getegid32 is not used */
/* sys_setreuid32 is not used */
/* sys_setregid32 is not used */
/* sys_getgroups32 is not used  */
/* sys_setgroups32 is not used  */
/* sys_fchown32 is not used  */
/* sys_setresuid32 is not used  */
/* sys_getresuid32 is not used  */
/* sys_setresgid32 is not used  */ /*210*/
/* sys_getresgid32 is not used  */
/* sys_lchown32 is not used  */
/* sys_setuid32 is not used  */
/* sys_setgid32 is not used  */
/* sys_setfsuid32 is not used  */
/* sys_setfsgid32 is not used  */
extern asmlinkage long sys_pivot_root(const char *new_root, const char *put_old);
extern asmlinkage long sys_getdents64(unsigned int fd, void * dirent, unsigned int count);
extern asmlinkage long sys_gettid(void);
extern asmlinkage long sys_tkill(int pid, int sig);
extern asmlinkage long sys_kprintf(const char *msg, int len);
extern asmlinkage long sys_e1newSP(unsigned long);

#endif /* _HYPERSTONE_NOMMU_SYSCALLS_H_ */
