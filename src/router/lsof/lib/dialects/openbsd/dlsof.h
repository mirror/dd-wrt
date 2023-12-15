/*
 * dlsof.h - OpenBSD header file for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dlsof.h,v 1.38 2006/03/28 21:54:08 abe Exp $
 */

#if !defined(OPENBSD_LSOF_H)
#    define OPENBSD_LSOF_H 1

#    include <stdlib.h>
#    include <inttypes.h>
#    include <dirent.h>
#    include <nlist.h>
#    include <paths.h>
#    include <setjmp.h>
#    include <signal.h>
#    include <string.h>
#    include <unistd.h>
#    include <fcntl.h>

#    include <arpa/inet.h>
#    include <sys/queue.h>
#    include <sys/filedesc.h>
#    include <sys/mbuf.h>
#    include <sys/mount.h>
#    include <rpc/types.h>
#    include <sys/protosw.h>
#    include <sys/socket.h>
#    include <sys/socketvar.h>
#    include <sys/un.h>
#    include <sys/unpcb.h>
#    include <net/route.h>
#    include <netinet/in.h>
#    include <netinet/in_systm.h>
#    include <netinet/ip.h>

#    include <netinet/in_pcb.h>
#    include <netinet/ip_var.h>
#    include <netinet/tcp.h>
#    include <netinet/tcp_fsm.h>
#    include <netinet/tcp_timer.h>
#    include <netinet/tcp_var.h>

#    include <sys/ucred.h>

#    include <sys/vnode.h>
#    include <sys/domain.h>

#    define pmap RPC_pmap
#    include <rpc/rpc.h>
#    include <rpc/pmap_prot.h>
#    undef pmap

#    include <sys/proc.h>
#    include <kvm.h>
#    include <sys/sysctl.h>

#    include <sys/file.h>
#    include <sys/fcntl.h>
#    include <sys/lockf.h>

#    define COMP_P const void
#    define DEVINCR 1024 /* device table malloc() increment */
typedef u_long KA_T;
#    define KMEM "/dev/kmem"
#    define MALLOC_P void
#    define FREE_P MALLOC_P
#    define MALLOC_S size_t

#    if !defined(MAXSYSCMDL)
#        define MAXSYSCMDL MAXCOMLEN /* max system command name length */
#    endif                           /* !defined(MAXSYSCMDL) */

#    define QSORT_P void
#    define READLEN_T int
#    define STRNCPY_L size_t
#    define SWAP "/dev/drum"
#    define SZOFFTYPE unsigned long long
/* size and offset internal storage
 * type */
#    define SZOFFPSPEC                                                         \
        "ll" /* SZOFFTYPE print specification                                  \
              * modifier */

/*
 * Global storage definitions (including their structure definitions)
 */

struct mounts {
    char *dir;           /* directory (mounted on) */
    char *fsname;        /* file system
                          * (symbolic links unresolved) */
    char *fsnmres;       /* file system
                          * (symbolic links resolved) */
    dev_t dev;           /* directory st_dev */
    dev_t rdev;          /* directory st_rdev */
    INODETYPE inode;     /* directory st_ino */
    mode_t mode;         /* directory st_mode */
    mode_t fs_mode;      /* file_system st_mode */
    struct mounts *next; /* forward link */
};

#    define X_NCACHE "ncache"
#    define X_NCSIZE "ncsize"
#    define NL_NAME n_name

struct sfile {
    char *aname;        /* argument file name */
    char *name;         /* file name (after readlink()) */
    char *devnm;        /* device name (optional) */
    dev_t dev;          /* device */
    dev_t rdev;         /* raw device */
    u_short mode;       /* S_IFMT mode bits from stat() */
    int type;           /* file type: 0 = file system
                         *	      1 = regular file */
    INODETYPE i;        /* inode number */
    int f;              /* file found flag */
    struct sfile *next; /* forward link */
};

/*
 * Definitions for rdev.c
 */

#    define DIRTYPE dirent
#    define HASDNAMLEN 1 /* struct DIRTYPE has d_namlen element */

struct lsof_context_dialect {};

#endif /* OPENBSD_LSOF_H */
