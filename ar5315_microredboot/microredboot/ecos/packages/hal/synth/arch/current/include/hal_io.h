#ifndef CYGONCE_HAL_HAL_IO_H
#define CYGONCE_HAL_HAL_IO_H

//=============================================================================
//
//      hal_io.h
//
//      HAL device IO register support.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Author(s):   nickg
// Contributors:nickg, bartv, alunn, jlarmour
// Date:        1998-02-17
// Purpose:     Define IO register support
// Description: The macros defined here provide the HAL APIs for handling
//              device IO control registers.
//
//              For the synthetic target these macros should never
//              actually be used since the application will run as an
//              ordinary user application and should not have
//              permission to access any real hardware. Instead
//              hardware access should go via the auxiliary. Possibly
//              the macros should be #pragma poison'd, but some people
//              may want to run the synthetic target in a way that
//              does involve accessing real hardware.
//              
//              The synthetic target provides some additional I/O
//              facilities in the form of Linux system calls. A useful
//              subset of these are prototyped here, together with
//              associated constants. There are also I/O operations to
//              interact with the auxiliary.
//
// Usage:
//              #include <cyg/hal/hal_io.h>
//              ...
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>

//-----------------------------------------------------------------------------
// IO Register address.
// This type is for recording the address of an IO register.

typedef volatile CYG_ADDRWORD HAL_IO_REGISTER;

//-----------------------------------------------------------------------------
// BYTE Register access.
// Individual and vectorized access to 8 bit registers.

#define HAL_READ_UINT8( _register_, _value_ )           \
    CYG_MACRO_START                                     \
    ((_value_) = *((volatile CYG_BYTE *)(_register_))); \
    CYG_MACRO_END

#define HAL_WRITE_UINT8( _register_, _value_ )          \
    CYG_MACRO_START                                     \
    (*((volatile CYG_BYTE *)(_register_)) = (_value_)); \
    CYG_MACRO_END

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        (_buf_)[_i_] = ((volatile CYG_BYTE *)(_register_))[_j_];        \
    }                                                                   \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        ((volatile CYG_BYTE *)(_register_))[_j_] = (_buf_)[_i_];        \
    }                                                                   \
    CYG_MACRO_END


//-----------------------------------------------------------------------------
// 16 bit access.
// Individual and vectorized access to 16 bit registers.
    
#define HAL_READ_UINT16( _register_, _value_ )                  \
    CYG_MACRO_START                                             \
    ((_value_) = *((volatile CYG_WORD16 *)(_register_)));       \
    CYG_MACRO_END

#define HAL_WRITE_UINT16( _register_, _value_ )                 \
    CYG_MACRO_START                                             \
    (*((volatile CYG_WORD16 *)(_register_)) = (_value_));       \
    CYG_MACRO_END

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        (_buf_)[_i_] = ((volatile CYG_WORD16 *)(_register_))[_j_];      \
    }                                                                   \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        ((volatile CYG_WORD16 *)(_register_))[_j_] = (_buf_)[_i_];      \
    }                                                                   \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// 32 bit access.
// Individual and vectorized access to 32 bit registers.
    
#define HAL_READ_UINT32( _register_, _value_ )                  \
    CYG_MACRO_START                                             \
    ((_value_) = *((volatile CYG_WORD32 *)(_register_)));       \
    CYG_MACRO_END

#define HAL_WRITE_UINT32( _register_, _value_ )                 \
    CYG_MACRO_START                                             \
    (*((volatile CYG_WORD32 *)(_register_)) = (_value_));       \
    CYG_MACRO_END

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        (_buf_)[_i_] = ((volatile CYG_WORD32 *)(_register_))[_j_];      \
    }                                                                   \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_)) {   \
        ((volatile CYG_WORD32 *)(_register_))[_j_] = (_buf_)[_i_];      \
    }                                                                   \
    CYG_MACRO_END

// ----------------------------------------------------------------------------
// Linux system calls and associated structures and constants. This is
// by no means a complete list, but there is enough information for
// the needs of the relevant HAL packages. The information needs to be
// kept in synch with the Linux header files, but in practice
// divergence will be rare because that would imply incompatible
// changes in the Linux kernel API.
//
// It may seem tempting to import the Linux header files directly, but
// that would prevent cross-compilation and introduce all kinds of
// namespace pollution.
//
// The actual implementation lives in variant HAL packages since
// typically they involve direct system calls via bits of assembler.
// Note that only a subset of system calls are actually implemented,
// so the variant HALs may need to be updated if this list needs to
// be extended.

// Error codes.
#define CYG_HAL_SYS_EINTR                4
#define CYG_HAL_SYS_EAGAIN              11

// Signal-related information
#define CYG_HAL_SYS_SIGHUP               1
#define CYG_HAL_SYS_SIGINT               2
#define CYG_HAL_SYS_SIGQUIT              3
#define CYG_HAL_SYS_SIGILL               4
#define CYG_HAL_SYS_SIGTRAP              5
#define CYG_HAL_SYS_SIGABRT              6
#define CYG_HAL_SYS_SIGBUS               7
#define CYG_HAL_SYS_SIGFPE               8
#define CYG_HAL_SYS_SIGKILL              9
#define CYG_HAL_SYS_SIGUSR1             10
#define CYG_HAL_SYS_SIGSEGV             11
#define CYG_HAL_SYS_SIGUSR2             12
#define CYG_HAL_SYS_SIGPIPE             13
#define CYG_HAL_SYS_SIGALRM             14
#define CYG_HAL_SYS_SIGTERM             15
#define CYG_HAL_SYS_SIGSTKFLT           16
#define CYG_HAL_SYS_SIGCHLD             17
#define CYG_HAL_SYS_SIGCONT             18
#define CYG_HAL_SYS_SIGSTOP             19
#define CYG_HAL_SYS_SIGTSTP             20
#define CYG_HAL_SYS_SIGTTIN             21
#define CYG_HAL_SYS_SIGTTOU             22
#define CYG_HAL_SYS_SIGURG              23
#define CYG_HAL_SYS_SIGXCPU             24
#define CYG_HAL_SYS_SIGXFSZ             25
#define CYG_HAL_SYS_SIGVTALRM           26
#define CYG_HAL_SYS_SIGPROF             27
#define CYG_HAL_SYS_SIGWINCH            28
#define CYG_HAL_SYS_SIGIO               29
#define CYG_HAL_SYS_SIGPWR              30
#define CYG_HAL_SYS_SIGSYS              31

#define CYG_HAL_SYS_SA_NOCLDSTOP        0x00000001
#define CYG_HAL_SYS_SA_NOCLDWAIT        0x00000002
#define CYG_HAL_SYS_SA_SIGINFO          0x00000004
#define CYG_HAL_SYS_SA_RESTART          0x10000000
#define CYG_HAL_SYS_SA_NODEFER          0x40000000
#define CYG_HAL_SYS_SIG_BLOCK           0
#define CYG_HAL_SYS_SIG_UNBLOCK         1
#define CYG_HAL_SYS_SIG_SETMASK         2

#define CYG_HAL_SYS__NSIG               64
#define CYG_HAL_SYS__SIGBITS            (8 * sizeof(unsigned long))
#define CYG_HAL_SYS__SIGELT(_d_)        ((_d_) / CYG_HAL_SYS__SIGBITS)
#define CYG_HAL_SYS__SIGMASK(_d_)       ((unsigned long)1 << ((_d_) % CYG_HAL_SYS__SIGBITS))

typedef struct cyg_hal_sys_sigset_t {
    unsigned long hal_sig_bits[CYG_HAL_SYS__NSIG / CYG_HAL_SYS__SIGBITS];
} cyg_hal_sys_sigset_t;

#define CYG_HAL_SYS_SIGFILLSET(_set_)                                                   \
    CYG_MACRO_START                                                                     \
        unsigned int __i;                                                               \
        for (__i = 0; __i < (CYG_HAL_SYS__NSIG / CYG_HAL_SYS__SIGBITS); __i++) {        \
            (_set_)->hal_sig_bits[__i] = ~0;                                            \
        }                                                                               \
    CYG_MACRO_END

#define CYG_HAL_SYS_SIGEMPTYSET(_set_)                                                  \
    CYG_MACRO_START                                                                     \
        unsigned int __i;                                                               \
        for (__i = 0; __i < (CYG_HAL_SYS__NSIG / CYG_HAL_SYS__SIGBITS); __i++) {        \
            (_set_)->hal_sig_bits[__i] = 0;                                             \
        }                                                                               \
    CYG_MACRO_END

#define CYG_HAL_SYS_SIGADDSET(_set_, _bit_)                                                     \
    CYG_MACRO_START                                                                             \
    (_set_)->hal_sig_bits[CYG_HAL_SYS__SIGELT(_bit_ - 1)] |= CYG_HAL_SYS__SIGMASK(_bit_ - 1);   \
    CYG_MACRO_END

#define CYG_HAL_SYS_SIGDELSET(_set_, _bit_)                                                     \
    CYG_MACRO_START                                                                             \
    (_set_)->hal_sig_bits[CYG_HAL_SYS__SIGELT(_bit_ - 1)] &= ~CYG_HAL_SYS__SIGMASK(_bit_ - 1);  \
    CYG_MACRO_END
               
#define CYG_HAL_SYS_SIGISMEMBER(_set_, _bit_)                                                   \
    (0 != ((_set_)->hal_sig_bits[CYG_HAL_SYS__SIGELT(_bit_ - 1)] & CYG_HAL_SYS__SIGMASK(_bit_ - 1)))

struct cyg_hal_sys_sigaction {
    void        (*hal_handler)(int);
    long        hal_mask;
    int         hal_flags;
    void        (*hal_bogus)(int);
};

// Time support.
struct cyg_hal_sys_timeval {
    long        hal_tv_sec;
    long        hal_tv_usec;
};

struct cyg_hal_sys_timezone {
    int         hal_tz_minuteswest;
    int         hal_tz_dsttime;
};

// Select support. Initially this is used only by the idle handler.
#define CYG_HAL_SYS_FD_SETSIZE          1024
#define CYG_HAL_SYS__NFDBITS            (8 * sizeof(unsigned long))
#define CYG_HAL_SYS__FDELT(_d_)         ((_d_) / CYG_HAL_SYS__NFDBITS)
#define CYG_HAL_SYS__FDMASK(_d_)        ((unsigned long)1 << ((_d_) % CYG_HAL_SYS__NFDBITS))

struct cyg_hal_sys_fd_set {
    unsigned long hal_fds_bits[CYG_HAL_SYS_FD_SETSIZE / CYG_HAL_SYS__NFDBITS];
};
#define CYG_HAL_SYS_FD_ZERO(_fdsp_)                                     \
    do {                                                                \
        unsigned int __i;                                               \
        for (__i = 0;                                                   \
             __i < (CYG_HAL_SYS_FD_SETSIZE / CYG_HAL_SYS__NFDBITS);     \
             __i++) {                                                   \
           (_fdsp_)->hal_fds_bits[__i] = 0;                             \
        }                                                               \
     } while (0);
    
#define CYG_HAL_SYS_FD_SET(_fd_, _fdsp_)                                \
    CYG_MACRO_START                                                     \
    (_fdsp_)->hal_fds_bits[CYG_HAL_SYS__FDELT(_fd_)] |= CYG_HAL_SYS__FDMASK(_fd_); \
    CYG_MACRO_END
    
#define CYG_HAL_SYS_FD_CLR(_fd_, _fdsp_)                                \
    CYG_MACRO_START                                                     \
    (_fdsp_)->hal_fds_bits[CYG_HAL_SYS__FDELT(_fd_)] &= ~CYG_HAL_SYS__FDMASK(_fd_); \
    CYG_MACRO_END

#define CYG_HAL_SYS_FD_ISSET(_fd_, _fdsp_) \
    (0 != ((_fdsp_)->hal_fds_bits[CYG_HAL_SYS__FDELT(_fd_)] & CYG_HAL_SYS__FDMASK(_fd_)))

// Interval timer support, needed for the clock.
#define CYG_HAL_SYS_ITIMER_REAL     0
#define CYG_HAL_SYS_ITIMER_VIRTUAL  1
#define CYG_HAL_SYS_ITIMER_PROF     2
 
struct cyg_hal_sys_itimerval {
    struct cyg_hal_sys_timeval  hal_it_interval;
    struct cyg_hal_sys_timeval  hal_it_value;
};

// System calls and related constants, or rather the subset that is
// needed internally.
#define CYG_HAL_SYS_R_OK    0x04
#define CYG_HAL_SYS_W_OK    0x02
#define CYG_HAL_SYS_X_OK    0x01
#define CYG_HAL_SYS_F_OK    0x00

/* lseek whence flags */
#define CYG_HAL_SYS_SEEK_SET        0       /* Seek from beginning of file.  */
#define CYG_HAL_SYS_SEEK_CUR        1       /* Seek from current position.  */
#define CYG_HAL_SYS_SEEK_END        2       /* Seek from end of file.  */

/* open/fcntl flags */

#define CYG_HAL_SYS_O_RDONLY        0
#define CYG_HAL_SYS_O_WRONLY       01
#define CYG_HAL_SYS_O_RDWR         02
#define CYG_HAL_SYS_O_CREAT      0100
#define CYG_HAL_SYS_O_EXCL       0200
#define CYG_HAL_SYS_O_NOCTTY     0400
#define CYG_HAL_SYS_O_TRUNC     01000
#define CYG_HAL_SYS_O_APPEND    02000
#define CYG_HAL_SYS_O_NONBLOCK  04000
#define CYG_HAL_SYS_O_NDELAY     CYG_HAL_SYS_O_NONBLOCK
#define CYG_HAL_SYS_O_SYNC     010000
#define CYG_HAL_SYS_O_FSYNC     CYG_HAL_SYS_O_SYNC
#define CYG_HAL_SYS_O_ASYNC    020000

/* open mode flags */
#define CYG_HAL_SYS_S_IRUSR 0400
#define CYG_HAL_SYS_S_IREAD CYG_HAL_SYS_S_IRUSR
#define CYG_HAL_SYS_S_IWUSR 0200
#define CYG_HAL_SYS_S_IWRITE CYG_HAL_SYS_S_IWUSR
#define CYG_HAL_SYS_S_IXUSR 0100
#define CYG_HAL_SYS_S_IEXEC CYG_HAL_SYS_S_IXUSR
#define CYG_HAL_SYS_S_IRWXU \
  (CYG_HAL_SYS_S_IREAD|CYG_HAL_SYS_S_IWRITE|CYG_HAL_SYS_S_IEXEC)
#define CYG_HAL_SYS_S_IRWXG (CYG_HAL_SYS_S_IRWXU>>3)
#define CYG_HAL_SYS_S_IRGRP (CYG_HAL_SYS_S_IRUSR>>3)
#define CYG_HAL_SYS_S_IWGRP (CYG_HAL_SYS_S_IWUSR>>3)
#define CYG_HAL_SYS_S_IXGRP (CYG_HAL_SYS_S_IXUSR>>3)
#define CYG_HAL_SYS_S_IRWXO (CYG_HAL_SYS_S_IRWXG>>3)
#define CYG_HAL_SYS_S_IROTH (CYG_HAL_SYS_S_IRGRP>>3)
#define CYG_HAL_SYS_S_IWOTH (CYG_HAL_SYS_S_IWGRP>>3)
#define CYG_HAL_SYS_S_IXOTH (CYG_HAL_SYS_S_IXGRP>>3)

/* stat flags */
#define CYG_HAL_SYS_S_IFMT   0170000 /*bitmask for the file type bitfields*/
#define CYG_HAL_SYS_S_IFSOCK 0140000 /*socket*/
#define CYG_HAL_SYS_S_IFLNK  0120000 /*symbolic link*/
#define CYG_HAL_SYS_S_IFREG  0100000 /*regular file*/
#define CYG_HAL_SYS_S_IFBLK  0060000 /*block device*/
#define CYG_HAL_SYS_S_IFDIR  0040000 /*directory*/
#define CYG_HAL_SYS_S_IFCHR  0020000 /*character device*/
#define CYG_HAL_SYS_S_IFIFO  0010000 /*fifo*/
#define CYG_HAL_SYS_S_ISUID  0004000 /*set UID bit*/
#define CYG_HAL_SYS_S_ISGID  0002000 /*set GID bit (see below)*/
#define CYG_HAL_SYS_S_ISVTX  0001000 /*sticky bit (see below)*/

struct cyg_hal_sys_mmap_args {
        unsigned long addr;
        unsigned long len;
        unsigned long prot;
        unsigned long flags;
        unsigned long fd;
        unsigned long offset;
};

/* Protection flags for mmap */
#define CYG_HAL_SYS_PROT_READ       0x1     /* page can be read */
#define CYG_HAL_SYS_PROT_WRITE      0x2     /* page can be written */
#define CYG_HAL_SYS_PROT_EXEC       0x4     /* page can be executed */
#define CYG_HAL_SYS_PROT_NONE       0x0     /* page can not be accessed */

/* Sharing types and other flags */
#define CYG_HAL_SYS_MAP_SHARED      0x01     /* Share changes.  */
#define CYG_HAL_SYS_MAP_PRIVATE     0x02     /* Changes are private.  */
#define CYG_HAL_SYS_MAP_FIXED       0x10     /* Interpret addr exactly.  */
 
struct cyg_hal_sys_dirent
{
   unsigned long d_ino;
   unsigned long d_off;
   unsigned short int d_reclen;
   char d_name[256];
};

struct cyg_hal_sys_timespec
{
   unsigned int tv_sec;
   unsigned int tv_nsec;
};

struct cyg_hal_sys_stat
{
	unsigned int  dev;          /* inode */
	unsigned long ino;          /* device */
	unsigned short mode;        /* protection */
	unsigned short	nlink;       /* number of hard links */
	unsigned short	uid;         /* user ID of owner */
	unsigned short	gid;         /* group ID of owner */
	unsigned long rdev;         /* device type (if inode device) */
	unsigned long size;         /* total size, in bytes */
	unsigned int blksize;       /* blocksize for filesystem I/O */
	unsigned int blocks;        /* number of blocks allocated */
	struct cyg_hal_sys_timespec atime; /* time of last access */
	struct cyg_hal_sys_timespec mtime; /* time of last modification */
	struct cyg_hal_sys_timespec ctime; /* time of last change */
};

// System calls, or rather the subset that is needed internally or by
// applications which want to access the host OS.

externC unsigned long   cyg_hal_sys_write(int, const void*, long);
externC unsigned long   cyg_hal_sys_read(int, void*, long);
externC int             cyg_hal_sys_lseek(int, int, int);
externC int             cyg_hal_sys_open(const char *,int,int); 
externC int             cyg_hal_sys_fdatasync(int); 
externC int             cyg_hal_sys_sigaction(int, 
                                              const struct cyg_hal_sys_sigaction*,
                                              struct cyg_hal_sys_sigaction*);
externC int             cyg_hal_sys_sigprocmask(int,
                                                const cyg_hal_sys_sigset_t*,
                                                cyg_hal_sys_sigset_t*);
externC int             cyg_hal_sys__newselect(int,
                                               struct cyg_hal_sys_fd_set*,
                                               struct cyg_hal_sys_fd_set*,
                                               struct cyg_hal_sys_fd_set*,
                                               struct cyg_hal_sys_timeval*);
externC int             cyg_hal_sys_setitimer(int,
                                              const struct cyg_hal_sys_itimerval*,
                                              struct cyg_hal_sys_itimerval*);
externC int             cyg_hal_sys_gettimeofday(struct cyg_hal_sys_timeval*,
                                                 struct cyg_hal_sys_timezone*);

externC int             cyg_hal_sys_access(const char*, int);
externC int             cyg_hal_sys_fork(void);
externC int             cyg_hal_sys_execve(const char*, const char* [], const char* []);
externC int             cyg_hal_sys_pipe(int []);
externC int             cyg_hal_sys_close(int);
externC int             cyg_hal_sys_dup2(int, int);
 
// The actual implementation appears to return the new brk() value.
externC void*           cyg_hal_sys_brk(void*);

// Returns the number of characters placed in the buffer or <0 for error,
// not a char*. 
externC int             cyg_hal_sys_getcwd(char*, int);

// mmap on the "host" system - this may be unportable.
externC int             cyg_hal_sys_mmap(struct cyg_hal_sys_mmap_args *);

externC int cyg_hal_sys_readdir(unsigned int fd, 
                                struct cyg_hal_sys_dirent *dp, 
                                unsigned int count);
externC int cyg_hal_sys_lstat(const char* name, struct cyg_hal_sys_stat *buf);
externC int cyg_hal_sys_fstat(int fd, struct cyg_hal_sys_stat *buf);
externC int cyg_hal_sys_mkdir(const char* path, int mode);

// Access to environmental data
extern int              cyg_hal_sys_argc;
extern const char**     cyg_hal_sys_argv;
extern const char**     cyg_hal_sys_environ;

// ----------------------------------------------------------------------------
// Interaction between the application and the auxiliary.

// Is the  auxiliary actually in use/available? This flag should be tested by
// device drivers prior to attempting any communication with the auxiliary.
extern cyg_bool synth_auxiliary_running;
 
// The fundamental I/O operation: sending a request to the auxiliary and
// optionally getting back a reply. A null pointer for the response field
// indicates that no reply is expected. 
externC void synth_auxiliary_xchgmsg(int /* devid */, int /* request */,
                                     int /* arg1  */, int /* arg2 */,
                                     const unsigned char* /* txdata */, int /* txlen */,
                                     int* /* response */,
                                     unsigned char* /* rxdata */,  int* /* actual_rxlen */,
                                     int /* rxlen */);

// Request that the auxiliary instantiates a given device, loading appropriate
// support code as required. This function takes the following arguments:
// 1) the location of the package that should provide this device, e.g.
//    devs/eth/synth
// 2) the version of that package currently being used, e.g. "current"
// 3) the name of the device, e.g. "ethernet". This identifies the
//    Tcl script that should be loaded to handle requests for this device.
// 4) the name of the device instance, e.g. "eth0".
// 5) device-specific initialization data. 
externC int  synth_auxiliary_instantiate(const char*, const char*, const char*, const char*, const char*);

// Support for generating strings
#define SYNTH_MAKESTRING1(a) #a
#define SYNTH_MAKESTRING(a)  SYNTH_MAKESTRING1(a)
 
//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_IO_H
// End of hal_io.h
