#ifndef __ASM_FRIO_UNISTD_H
#define __ASM_FRIO_UNISTD_H


/*
 * This file contains the system call numbers.
 */

#define __NR_setup			0	/* used only by init, to get system going */
#define __NR_exit			1
#define __NR_fork			2
#define __NR_read			3
#define __NR_write			4
#define __NR_open			5
#define __NR_close			6
#define __NR_waitpid			7
#define __NR_creat			8
#define __NR_link			9
#define __NR_unlink			10
#define __NR_execve			11
#define __NR_chdir			12
#define __NR_time			13
#define __NR_mknod			14
#define __NR_chmod			15
#define __NR_lchown			16
#define __NR_break			17
#define __NR_oldstat			18
#define __NR_lseek			19
#define __NR_getpid			20
#define __NR_mount			21
#define __NR_umount			22
#define __NR_setuid			23
#define __NR_getuid			24
#define __NR_stime			25
#define __NR_ptrace			26
#define __NR_alarm			27
#define __NR_oldfstat			28
#define __NR_pause			29
#define __NR_utime			30
#define __NR_stty			31
#define __NR_gtty			32
#define __NR_access			33
#define __NR_nice			34
#define __NR_ftime			35
#define __NR_sync			36
#define __NR_kill			37
#define __NR_rename			38
#define __NR_mkdir			39
#define __NR_rmdir			40
#define __NR_dup			41
#define __NR_pipe			42
#define __NR_times			43
#define __NR_prof			44
#define __NR_brk			45
#define __NR_setgid			46
#define __NR_getgid			47
#define __NR_signal			48
#define __NR_geteuid			49
#define __NR_getegid			50
#define __NR_acct			51
#define __NR_phys			52
#define __NR_lock			53
#define __NR_ioctl			54
#define __NR_fcntl			55
#define __NR_mpx			56
#define __NR_setpgid			57
#define __NR_ulimit			58
#define __NR_oldolduname		59
#define __NR_umask			60
#define __NR_chroot			61
#define __NR_ustat			62
#define __NR_dup2			63
#define __NR_getppid			64
#define __NR_getpgrp			65
#define __NR_setsid			66
#define __NR_sigaction			67
#define __NR_sgetmask			68
#define __NR_ssetmask			69
#define __NR_setreuid			70
#define __NR_setregid			71
#define __NR_sigsuspend			72
#define __NR_sigpending			73
#define __NR_sethostname		74
#define __NR_setrlimit			75
#define __NR_getrlimit			76
#define __NR_getrusage			77
#define __NR_gettimeofday		78
#define __NR_settimeofday		79
#define __NR_getgroups			80
#define __NR_setgroups			81
#define __NR_select			82
#define __NR_symlink			83
#define __NR_oldlstat			84
#define __NR_readlink			85
#define __NR_uselib			86
#define __NR_swapon			87
#define __NR_reboot			88
#define __NR_readdir			89
#define __NR_mmap			90
#define __NR_munmap			91
#define __NR_truncate			92
#define __NR_ftruncate			93
#define __NR_fchmod			94
#define __NR_fchown			95
#define __NR_getpriority		96
#define __NR_setpriority		97
#define __NR_profil			98
#define __NR_statfs			99
#define __NR_fstatfs			100
#define __NR_ioperm			101
#define __NR_socketcall			102
#define __NR_syslog			103
#define __NR_setitimer			104
#define __NR_getitimer			105
#define __NR_stat			106
#define __NR_lstat			107
#define __NR_fstat			108
#define __NR_olduname			109
#define __NR_iopl			110
#define __NR_vhangup			111
#define __NR_idle			112
#define __NR_syscall			113 /* syscall to call a syscall! */
#define __NR_wait4			114
#define __NR_swapoff			115
#define __NR_sysinfo			116
#define __NR_ipc			117
#define __NR_fsync			118
#define __NR_sigreturn			119
#define __NR_clone			120
#define __NR_setdomainname		121
#define __NR_uname			122
#define __NR_modify_ldt			123
#define __NR_adjtimex			124
#define __NR_mprotect			125
#define __NR_sigprocmask		126
#define __NR_create_module		127
#define __NR_init_module		128
#define __NR_delete_module		129
#define __NR_get_kernel_syms		130
#define __NR_quotactl			131
#define __NR_getpgid			132
#define __NR_fchdir			133
#define __NR_bdflush			134
#define __NR_sysfs			135
#define __NR_personality		136
#define __NR_afs_syscall		137 /* Syscall for Andrew File System */
#define __NR_setfsuid			138
#define __NR_setfsgid			139
#define __NR__llseek			140
#define __NR_getdents			141
#define __NR__newselect			142
#define __NR_flock			143
#define __NR_msync			144
#define __NR_readv			145
#define __NR_writev			146
#define __NR_getsid			147
#define __NR_fdatasync			148
#define __NR__sysctl			149
#define __NR_mlock			150
#define __NR_munlock			151
#define __NR_mlockall			152
#define __NR_munlockall			153
#define __NR_sched_setparam			154
#define __NR_sched_getparam			155
#define __NR_sched_setscheduler			156
#define __NR_sched_getscheduler			157
#define __NR_sched_yield			158
#define __NR_sched_get_priority_max		159
#define __NR_sched_get_priority_min		160
#define __NR_sched_rr_get_interval		161
#define __NR_nanosleep			162
#define __NR_mremap			163
#define __NR_setresuid			164
#define __NR_getresuid			165
#define __NR_vm86			166
#define __NR_query_module		167
#define __NR_poll			168
#define __NR_nfsservctl			169
#define __NR_setresgid			170
#define __NR_getresgid			171
#define __NR_prctl			172
#define __NR_rt_sigreturn		173
#define __NR_rt_sigaction		174
#define __NR_rt_sigprocmask		175
#define __NR_rt_sigpending		176
#define __NR_rt_sigtimedwait		177
#define __NR_rt_sigqueueinfo		178
#define __NR_rt_sigsuspend		179
#define __NR_pread			180
#define __NR_pwrite			181
#define __NR_chown			182
#define __NR_getcwd			183
#define __NR_capget			184
#define __NR_capset			185
#define __NR_sigaltstack		186
#define __NR_sendfile			187
#define __NR_getpmsg			188	/* some people actually want streams */
#define __NR_putpmsg			189	/* some people actually want streams */
#define __NR_vfork			190
#define __NR_ugetrlimit			191
#define __NR_mmap2			192
#define __NR_truncate64			193
#define __NR_ftruncate64		194
#define __NR_stat64			195
#define __NR_lstat64			196
#define __NR_fstat64			197


#define __NR_chown32		198
#define __NR_getuid32		199
#define __NR_getgid32		200
#define __NR_geteuid32		201
#define __NR_getegid32		202
#define __NR_setreuid32		203
#define __NR_setregid32		204
#define __NR_getgroups32	205
#define __NR_setgroups32	206
#define __NR_fchown32		207
#define __NR_setresuid32	208
#define __NR_getresuid32	209
#define __NR_setresgid32	210
#define __NR_getresgid32	211
#define __NR_lchown32		212
#define __NR_setuid32		213
#define __NR_setgid32		214
#define __NR_setfsuid32		215
#define __NR_setfsgid32		216

#define __syscall_return(type, res)					\
do {									\
	if ((unsigned long)(res) >= (unsigned long)(-125)) 		\
	{	errno = -(res);						\
		res = -1;						\
	}								\
	return (type) (res);						\
} while (0)

#define _syscall0(type,name)						\
type name(void) {							\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = r5;\n\t"							\
  "r5 = %1;\n\t"							\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "r5 = [sp++];\n\t"							\
  : "=da" (__res) 							\
  : "i" (__NR_##name)							\
  : "CC", "R5");							\
  __syscall_return(type,__res);						\
}

#define _syscall1(type,name,type1,arg1)					\
type name(type1 arg1) {							\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = r5;\n\t"							\
  "r0=%2;\n\t"								\
  "r5=%1;\n\t"								\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "r5 = [sp++];\n\t"							\
        : "=da" (__res)							\
        : "i" (__NR_##name),						\
	  "a" ((long)(arg1))						\
	: "CC", "R0", "R5");						\
  __syscall_return(type,__res);						\
}

#define _syscall2(type,name,type1,arg1,type2,arg2)			\
type name(type1 arg1,type2 arg2) {					\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = r5;\n\t"							\
  "r1=%3;\n\t"								\
  "r0=%2;\n\t"								\
  "r5=%1;\n\t"								\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "r5 = [sp++];\n\t"							\
        : "=da" (__res)							\
        : "i" (__NR_##name),						\
	  "a" ((long)(arg1)),						\
	  "a" ((long)(arg2))						\
	: "CC", "R0","R1", "R5");					\
  __syscall_return(type,__res);						\
}


#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3)		\
type name(type1 arg1,type2 arg2,type3 arg3) {				\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = r5;\n\t"							\
  "r2=%4;\n\t"								\
  "r1=%3;\n\t"								\
  "r0=%2;\n\t"								\
  "r5=%1;\n\t"								\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "r5 = [sp++];\n\t"							\
        : "=da" (__res)							\
        : "i"   (__NR_##name),						\
	  "a"   ((long)(arg1)),						\
	  "a"   ((long)(arg2)),						\
	  "a"   ((long)(arg3))						\
        : "CC", "R0","R1","R2", "R5");					\
  __syscall_return(type,__res);						\
}


#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4)\
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4) {		\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = r5;\n\t"							\
  "[--sp] = r3;\n\t"							\
  "r3=%5;\n\t"								\
  "r2=%4;\n\t"								\
  "r1=%3;\n\t"								\
  "r0=%2;\n\t"								\
  "r5=%1;\n\t"								\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "r3 = [sp++];\n\t"							\
  "r5 = [sp++];\n\t"							\
  	: "=da" (__res)							\
  	: "i"  (__NR_##name),						\
	  "a"  ((long)(arg1)),						\
	  "a"  ((long)(arg2)),						\
	  "a"  ((long)(arg3)),						\
	  "a"  ((long)(arg4))						\
  	: "CC", "R0","R1","R2","R3", "R5");				\
  __syscall_return(type,__res);						\
}
  

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5)	\
type name(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) {	\
  long __res;								\
  __asm__ __volatile__ (						\
  "[--sp] = ( r7:3 );\n\t"						\
  "r4=%6;\n\t"								\
  "r3=%5;\n\t"								\
  "r2=%4;\n\t"								\
  "r1=%3;\n\t"								\
  "r0=%2;\n\t"								\
  "r5=%1;\n\t"								\
  "excpt 0;\n\t" 							\
  "%0=r0;\n\t"								\
  "( r7:3 ) = [sp++];\n\t"						\
  	: "=da" (__res)							\
  	: "i"  (__NR_##name),						\
	  "a"  ((long)(arg1)),						\
	  "a"  ((long)(arg2)),						\
	  "a"  ((long)(arg3)),						\
	  "a"  ((long)(arg4)),						\
	  "a"  ((long)(arg5))						\
	: "CC","R0","R1","R2","R3","R4","R5");				\
  __syscall_return(type,__res);						\
}

#ifdef __KERNEL_SYSCALLS__

static inline int idle(void)
{
	extern int sys_idle(void);
	return sys_idle();
}

static inline int pause(void)
{
	extern int sys_pause(void);
	return sys_pause();
}

static inline int sync(void)
{
	extern int sys_sync(void);
	return sys_sync();
}

static inline pid_t setsid(void)
{
	extern int sys_setsid(void);
	return sys_setsid();
}

static inline int write(int fd, const char *buf, off_t count)
{
	extern int sys_write(int, const char *, int);
	return sys_write(fd, buf, count);
}

static inline int read(int fd, char *buf, off_t count)
{
	extern int sys_read(int, char *, int);
	return sys_read(fd, buf, count);
}

static inline off_t lseek(int fd, off_t offset, int count)
{
	extern off_t sys_lseek(int, off_t, int);
	return sys_lseek(fd, offset, count);
}

static inline int dup(int fd)
{
	extern int sys_dup(int);
	return sys_dup(fd);
}

static inline int open(const char *file, int flag, int mode)
{
	extern long sys_open(const char *, int, int);
	return sys_open(file, flag, mode);
}

static inline int close(int fd)
{
	return sys_close(fd);
}

static inline int _exit(int exitcode)
{
	extern int sys_exit(int) __attribute__((noreturn));
	return sys_exit(exitcode);
}

static inline pid_t waitpid(pid_t pid, int *wait_stat, int options)
{
	extern long sys_wait4(pid_t pid,unsigned int * stat_addr, int options, struct rusage * ru);
	return sys_wait4(pid, wait_stat, options, NULL);
}

static inline int delete_module(const char *name)
{
	extern int sys_delete_module(const char *name);
	return sys_delete_module(name);
}

static inline pid_t wait(int * wait_stat)
{
	extern long sys_wait4(pid_t pid,unsigned int * stat_addr, int options, struct rusage * ru);
	return sys_wait4(-1, wait_stat, 0, NULL);
}

/*
 * The following two can't be eliminated yet - they rely on
 * specific conditions.
 */
static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp);

#endif	/* __KERNEL_SYSCALLS__ */
#endif  /* __ASM_FRIO_UNISTD_H */
