/* Copyright (C) 1995, 1996, 1997, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_SHM_H
# error "Never include <bits/shm.h> directly; use <sys/shm.h> instead."
#endif

#include <bits/types.h>

/* Permission flag for shmget.  */
#define SHM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SHM_W		0200		/* or S_IWUGO from <linux/stat.h> */

/* Flags for `shmat'.  */
#define SHM_RDONLY	010000		/* attach read-only else read-write */
#define SHM_RND		020000		/* round attach address to SHMLBA */
#define SHM_REMAP	040000		/* take-over region on attach */

/* Commands for `shmctl'.  */
#define SHM_LOCK	11		/* lock segment (root only) */
#define SHM_UNLOCK	12		/* unlock segment (root only) */

__BEGIN_DECLS

/* Segment low boundary address multiple.  */
#define SHMLBA		(__getpagesize ())
extern int __getpagesize (void) __THROW __attribute__ ((__const__));

__END_DECLS


/* Type to count number of attaches.  */
typedef unsigned short shmatt_t;

/* Data structure describing a set of semaphores.  */
struct shmid_ds {
    struct ipc_perm	shm_perm;	/* operation perms */
    int			shm_segsz;	/* size of segment (bytes) */
    __kernel_time_t	shm_atime;	/* last attach time */
    __kernel_time_t	shm_dtime;	/* last detach time */
    __kernel_time_t	shm_ctime;	/* last change time */
    __kernel_ipc_pid_t	shm_cpid;	/* pid of creator */
    __kernel_ipc_pid_t	shm_lpid;	/* pid of last operator */
    unsigned short	shm_nattch;	/* no. of current attaches */
    unsigned short 	shm_unused;	/* compatibility */
    void 		*shm_unused2;	/* ditto - used by DIPC */
    void		*shm_unused3;	/* unused */
};

#ifdef __USE_MISC

/* ipcs ctl commands */
# define SHM_STAT 	13
# define SHM_INFO 	14

/* shm_mode upper byte flags */
# define SHM_DEST	01000	/* segment will be destroyed on last detach */
# define SHM_LOCKED	02000   /* segment will not be swapped */

struct  shminfo {
    int shmmax;
    int shmmin;
    int shmmni;
    int shmseg;
    int shmall;
};

struct shm_info {
    int used_ids;
    unsigned long shm_tot;  /* total allocated shm */
    unsigned long shm_rss;  /* total resident shm */
    unsigned long shm_swp;  /* total swapped shm */
    unsigned long swap_attempts;
    unsigned long swap_successes;
};

#endif /* __USE_MISC */
