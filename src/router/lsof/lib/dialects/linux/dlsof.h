/*
 * dlsof.h - Linux header file for /proc-based lsof
 */

/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

/*
 * $Id: dlsof.h,v 1.23 2015/07/07 19:46:33 abe Exp $
 */

#if !defined(LINUX_LSOF_H)
#    define LINUX_LSOF_H 1

#    include <dirent.h>
#    define DIRTYPE dirent /* for arg.c's enter_dir() */
#    define __USE_GNU      /* to get all O_* symbols in fcntl.h */
#    include <fcntl.h>
#    include <malloc.h>
#    include <signal.h>
#    include <stdlib.h>
#    include <string.h>
#    include <setjmp.h>
#    include <unistd.h>
#    include <netinet/in.h>

#    if defined(GLIBCV) || defined(__UCLIBC__) || defined(NEEDS_NETINET_TCPH)
#        include <netinet/tcp.h>
#    else /* !defined(GLIBCV) && !defined(__UCLIBC__) &&                       \
             !defined(NEEDS_NETINET_TCPH) */
#        include <linux/tcp.h>
#    endif /* defined(GLIBCV) || defined(__UCLIBC__) ||                        \
              defined(NEEDS_NETINET_TCPH) */

#    if !defined(HASNORPC_H)
#        include <rpc/rpc.h>
#        include <rpc/pmap_prot.h>
#    endif /* !defined(HASNORPC_H) */

#    if defined(HASSELINUX)
#        include <selinux/selinux.h>
#    endif /* defined(HASSELINUX) */

#    include <sys/sysmacros.h>
#    include <sys/socket.h>
#    include <arpa/inet.h>
#    include <linux/if_ether.h>
#    include <linux/netlink.h>

#    include <sys/syscall.h>

/*
 * This definition is needed for the common function prototype definitions
 * in "proto.h", but isn't used in /proc-based lsof.
 */

typedef unsigned long KA_T;

/*
 * Local definitions
 */

#    define COMP_P const void
#    define DEVINCR 1024 /* device table malloc() increment */
#    define FSNAMEL 4
#    define MALLOC_P void
#    define FREE_P MALLOC_P
#    define MALLOC_S size_t
#    define MAXSYSCMDL                                                         \
        15 /* max system command name length                                   \
            *   This value should be obtained from a                           \
            * header file #define, but no consistent one                       \
            * exists.  Some versions of the Linux kernel                       \
            * have a hard-coded "char comm[16]" command                        \
            * name member of the task structured                               \
            * definition in <linux/sched.h>, while others                      \
            * have a "char comm[TASK_COMM_LEN]" member                         \
            * with TASK_COMM_LEN #define'd to be 16.                           \
            *   Hence, a universal, local definition of                        \
            * 16 is #define'd here. */
#    define PROCFS "/proc"
#    define QSORT_P void
#    define READLEN_T size_t

/*
 * Definitions that indicate what values are present in a stat(2) or lstat(2)
 * buffer.
 */

#    define SB_DEV 0x01   /* st_dev */
#    define SB_INO 0x02   /* st_ino */
#    define SB_MODE 0x04  /* st_mode */
#    define SB_NLINK 0x08 /* st_nlink */
#    define SB_RDEV 0x10  /* st_rdev */
#    define SB_SIZE 0x20  /* st_size */
#    define SB_ALL                                                             \
        (SB_DEV | SB_INO | SB_MODE | SB_NLINK | SB_RDEV |                      \
         SB_SIZE) /* all values                                                \
                   */

#    define STRNCPY_L size_t
#    define STRNML 32

#    if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64
/* size and offset internal storage
 * type */
#        define SZOFFTYPE unsigned long long
#        define SZOFFPSPEC                                                     \
            "ll" /* SZOFFTYPE print specification                              \
                  * modifier */
#    endif       /* defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS==64 */

#    define XDR_PMAPLIST (xdrproc_t) xdr_pmaplist
#    define XDR_VOID (xdrproc_t) xdr_void

/*
 * Global storage definitions (including their structure definitions)
 */

struct mounts {
    char *dir;           /* directory name (mounted on) */
    char *fsname;        /* file system
                          * (symbolic links unresolved) */
    char *fsnmres;       /* file system
                          * (symbolic links resolved) */
    size_t dirl;         /* length of directory name */
    dev_t dev;           /* directory st_dev */
    dev_t rdev;          /* directory st_rdev */
    INODETYPE inode;     /* directory st_ino */
    mode_t mode;         /* directory st_mode */
    int ds;              /* directory status -- i.e., SB_*
                          * values */
    mode_t fs_mode;      /* file system st_mode */
    int ty;              /* node type -- e.g., N_REGLR, N_NFS */
    struct mounts *next; /* forward link */
};

struct sfile {
    char *aname;               /* argument file name */
    char *name;                /* file name (after readlink()) */
    char *devnm;               /* device name (optional) */
    dev_t dev;                 /* device */
    dev_t rdev;                /* raw device */
    mode_t mode;               /* S_IFMT mode bits from stat() */
    int type;                  /* file type: 0 = file system
                                *	      1 = regular file */
    INODETYPE i;               /* inode number */
    int f;                     /* file found flag */
    struct mounts *mp;         /* mount structure pointer for file
                                * system type entries */
#    define SAVE_MP_IN_SFILE 1 /* for ck_file_arg() im arg.c */
    struct sfile *next;        /* forward link */
};

#    if defined(HASEPTOPTS)
typedef struct pxinfo {  /* hashed pipe, UNIX socket or pseudo-
                          * terminal inode information */
    INODETYPE ino;       /* file's inode */
    struct lfile *lf;    /* connected peer file */
    int lpx;             /* connected process index */
    struct pxinfo *next; /* next entry for hashed inode */
} pxinfo_t;
#    endif /* defined(HASEPTOPTS) */

extern int HasNFS;
extern dev_t MqueueDev;

/* offset type:
 *     0 == unknown
 *     1 == lstat's st_size
 *     2 == from /proc/<PID>/fdinfo
 */
#    define OFFSET_UNKNOWN 0
#    define OFFSET_LSTAT 1
#    define OFFSET_FDINFO 2
extern int OffType;

struct lsof_context_dialect {};

#endif /* LINUX_LSOF_H	*/
