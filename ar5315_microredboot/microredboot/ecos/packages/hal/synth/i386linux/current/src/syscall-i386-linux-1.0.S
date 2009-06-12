//=============================================================================
//
//      syscall-i386-linux-1.0.S
//
//      Linux system call interface functions for i386.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   proven
// Contributors:proven,bartv
// Date:        1998-10-06
// Description: Machine dependent syscalls for i386/i486/i586
//
//####DESCRIPTIONEND####
//
//=============================================================================

// The numbers come from <asm/unistd.h>
#define SYS_setup               0 // Used only by init, to get system going.
#define SYS_exit                1
#define SYS_fork                2
#define SYS_read                3
#define SYS_write               4
#define SYS_open                5
#define SYS_close               6
#define SYS_waitpid             7
#define SYS_creat               8
#define SYS_link                9
#define SYS_unlink              10
#define SYS_execve              11
#define SYS_chdir               12
#define SYS_time                13
#define SYS_prev_mknod          14
#define SYS_chmod               15
#define SYS_lchown              16
#define SYS_break               17
#define SYS_oldstat             18
#define SYS_lseek               19
#define SYS_getpid              20
#define SYS_mount               21
#define SYS_umount              22
#define SYS_setuid              23
#define SYS_getuid              24
#define SYS_stime               25
#define SYS_ptrace              26
#define SYS_alarm               27
#define SYS_oldfstat            28
#define SYS_pause               29
#define SYS_utime               30
#define SYS_stty                31
#define SYS_gtty                32
#define SYS_access              33
#define SYS_nice                34
#define SYS_ftime               35
#define SYS_sync                36
#define SYS_kill                37
#define SYS_rename              38
#define SYS_mkdir               39
#define SYS_rmdir               40
#define SYS_dup                 41
#define SYS_pipe                42
#define SYS_times               43
#define SYS_prof                44
#define SYS_brk                 45
#define SYS_setgid              46
#define SYS_getgid              47
#define SYS_signal              48
#define SYS_geteuid             49
#define SYS_getegid             50
#define SYS_acct                51
#define SYS_phys                52
#define SYS_lock                53
#define SYS_ioctl               54
#define SYS_fcntl               55
#define SYS_mpx                 56
#define SYS_setpgid             57
#define SYS_ulimit              58
#define SYS_oldolduname         59
#define SYS_umask               60
#define SYS_chroot              61
#define SYS_prev_ustat          62
#define SYS_dup2                63
#define SYS_getppid             64
#define SYS_getpgrp             65
#define SYS_setsid              66
#define SYS_sigaction           67
#define SYS_siggetmask          68
#define SYS_sigsetmask          69
#define SYS_setreuid            70
#define SYS_setregid            71
#define SYS_sigsuspend          72
#define SYS_sigpending          73
#define SYS_sethostname         74
#define SYS_setrlimit           75
#define SYS_getrlimit           76
#define SYS_getrusage           77
#define SYS_gettimeofday        78
#define SYS_settimeofday        79
#define SYS_getgroups           80
#define SYS_setgroups           81
#define SYS_select              82
#define SYS_symlink             83
#define SYS_oldlstat            84
#define SYS_readlink            85
#define SYS_uselib              86
#define SYS_swapon              87
#define SYS_reboot              88
#define SYS_readdir             89
#define SYS_mmap                90
#define SYS_munmap              91
#define SYS_truncate            92
#define SYS_ftruncate           93
#define SYS_fchmod              94
#define SYS_fchown              95
#define SYS_getpriority         96
#define SYS_setpriority         97
#define SYS_profil              98
#define SYS_statfs              99
#define SYS_fstatfs             100
#define SYS_ioperm              101
#define SYS_socketcall          102
#define SYS_klog                103
#define SYS_setitimer           104
#define SYS_getitimer           105
#define SYS_prev_stat           106
#define SYS_prev_lstat          107
#define SYS_prev_fstat          108
#define SYS_olduname            109
#define SYS_iopl                110
#define SYS_vhangup             111
#define SYS_idle                112
#define SYS_vm86old             113
#define SYS_wait4               114
#define SYS_swapoff             115
#define SYS_sysinfo             116
#define SYS_ipc                 117
#define SYS_fsync               118
#define SYS_sigreturn           119
#define SYS_clone               120
#define SYS_setdomainname       121
#define SYS_uname               122
#define SYS_modify_ldt          123
#define SYS_adjtimex            124
#define SYS_mprotect            125
#define SYS_sigprocmask         126
#define SYS_create_module       127
#define SYS_init_module         128
#define SYS_delete_module       129
#define SYS_get_kernel_syms     130
#define SYS_quotactl            131
#define SYS_getpgid             132
#define SYS_fchdir              133
#define SYS_bdflush             134
#define SYS_sysfs               135
#define SYS_personality         136
#define SYS_afs_syscall         137 // Syscall for Andrew File System
#define SYS_setfsuid            138
#define SYS_setfsgid            139
#define SYS__llseek             140
#define SYS_getdents            141
#define SYS__newselect          142
#define SYS_flock               143
#define SYS_syscall_flock       143
#define SYS_msync               144
#define SYS_readv               145
#define SYS_syscall_readv       145
#define SYS_writev              146
#define SYS_syscall_writev      146
#define SYS_getsid              147
#define SYS_fdatasync           148
#define SYS__sysctl             149
#define SYS_mlock               150
#define SYS_munlock             151
#define SYS_mlockall            152
#define SYS_munlockall          153
#define SYS_sched_setparam      154
#define SYS_sched_getparam      155
#define SYS_sched_setscheduler  156
#define SYS_sched_getscheduler  157
#define SYS_sched_yield         158
#define SYS_sched_get_priority_max      159
#define SYS_sched_get_priority_min      160
#define SYS_sched_rr_get_interval       161
#define SYS_nanosleep           162
#define SYS_mremap              163
#define SYS_setresuid		164
#define SYS_getresuid		165
#define SYS_vm86		166
#define SYS_query_module	167
#define SYS_poll		168
#define SYS_nfsservctl		169
#define SYS_setresgid		170
#define SYS_getresgid		171
#define SYS_prctl               172
#define SYS_rt_sigreturn	173
#define SYS_rt_sigaction	174
#define SYS_rt_sigprocmask	175
#define SYS_rt_sigpending	176
#define SYS_rt_sigtimedwait	177
#define SYS_rt_sigqueueinfo	178
#define SYS_rt_sigsuspend	179
#define SYS_pread		180
#define SYS_pwrite		181
#define SYS_chown		182
#define SYS_getcwd		183
#define SYS_capget		184
#define SYS_capset		185
#define SYS_sigaltstack	        186
#define SYS_sendfile		187
#define SYS_getpmsg		188
#define SYS_putpmsg		189
#define SYS_vfork		190
#define SYS_ugetrlimit		191
#define SYS_mmap2		192
#define SYS_truncate64		193
#define SYS_ftruncate64	        194
#define SYS_stat64		195
#define SYS_lstat64		196
#define SYS_fstat64		197
#define SYS_lchown32		198
#define SYS_getuid32		199
#define SYS_getgid32		200
#define SYS_geteuid32		201
#define SYS_getegid32		202
#define SYS_setreuid32		203
#define SYS_setregid32		204
#define SYS_getgroups32	        205
#define SYS_setgroups32         206
#define SYS_fchown32		207
#define SYS_setresuid32         208
#define SYS_getresuid32         209
#define SYS_setresgid32         210
#define SYS_getresgid32         211
#define SYS_chown32		212
#define SYS_setuid32		213
#define SYS_setgid32		214
#define SYS_setfsuid32		215
#define SYS_setfsgid32		216
#define SYS_pivot_root		217
#define SYS_mincore		218
#define SYS_madvise		219
#define SYS_madvise1		219
#define SYS_getdents64		220
#define SYS_fcntl64		221


#define NAME(X) cyg_hal_sys_##X 
#define END(X)  1: ; .type NAME(X),@function ; .size NAME(X),1b - NAME(X)

#define SYSCALL0(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %ebx;                               \
        ret;                                    \
                                                \
        END(x)

#define SYSCALL1(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        mov 8(%esp), %ebx;                      \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %ebx;                               \
        ret;                                    \
                                                \
    END(x)

#define SYSCALL2(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        mov 8(%esp), %ebx;                      \
        mov 12(%esp), %ecx;                     \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %ebx;                               \
        ret;                                    \
                                                \
    END(x)

#define STATCALL2(x)                            \
   .globl NAME(x) ;                             \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        mov 8(%esp), %ebx;                      \
        mov 12(%esp), %ecx;                     \
        lea SYS_prev_##x, %eax;                 \
        int $0x80;                              \
        pop %ebx;                               \
        ret;                                    \
        END(x)

#define SYSCALL3(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        mov 8(%esp), %ebx;                      \
        mov 12(%esp), %ecx;                     \
        mov 16(%esp), %edx;                     \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %ebx;                               \
        ret;                                    \
        END(x)


#define SYSCALL4(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        push %esi;                              \
        mov 12(%esp), %ebx;                     \
        mov 16(%esp), %ecx;                     \
        mov 20(%esp), %edx;                     \
        mov 24(%esp), %esi;                     \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %esi;                               \
        pop %ebx;                               \
        ret;                                    \
        END(x)

#define SYSCALL5(x)                             \
        .globl NAME(x) ;                        \
                                                \
NAME(x):                                        \
                                                \
        push %ebx;                              \
        push %esi;                              \
        push %edi;                              \
        mov 16(%esp), %ebx;                     \
        mov 20(%esp), %ecx;                     \
        mov 24(%esp), %edx;                     \
        mov 28(%esp), %esi;                     \
        mov 32(%esp), %edi;                     \
        lea     SYS_##x, %eax;                  \
        int $0x80;                              \
        pop %edi;                               \
        pop %esi;                               \
        pop %ebx;                               \
        ret;                                    \
        END(x)


//==========================================================================
// Initial asm stuff for all functions.
        .text
        .align  2

SYSCALL1(exit)
SYSCALL0(fork)
SYSCALL3(read)
SYSCALL3(write)
SYSCALL3(open)
SYSCALL1(close)
SYSCALL3(waitpid)
SYSCALL2(creat)
SYSCALL2(link)
SYSCALL1(unlink)
SYSCALL3(execve)
SYSCALL1(chdir)
SYSCALL1(time)
SYSCALL2(chmod)
SYSCALL3(lseek)
SYSCALL0(getpid)
SYSCALL0(getuid)
SYSCALL1(nice)
SYSCALL2(kill)
SYSCALL1(dup)
SYSCALL1(pipe)
SYSCALL1(brk)
SYSCALL3(ioctl)
SYSCALL3(fcntl)
SYSCALL1(chroot)
SYSCALL2(dup2)
SYSCALL3(sigaction)
SYSCALL2(gettimeofday)
SYSCALL1(mmap)
SYSCALL2(socketcall)
SYSCALL3(setitimer)
SYSCALL3(sigprocmask)
SYSCALL5(_newselect)
SYSCALL3(readv)
SYSCALL3(writev)
SYSCALL1(fdatasync)
SYSCALL2(getcwd)
SYSCALL2(access)
SYSCALL3(readdir)
STATCALL2(lstat)
STATCALL2(fstat)
SYSCALL2(mkdir)
