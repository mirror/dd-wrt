#ifndef _ASM_IA64_STATFS_H
#define _ASM_IA64_STATFS_H

/*
 * Copyright (C) 1998, 1999 Hewlett-Packard Co
 * Copyright (C) 1998, 1999 David Mosberger-Tang <davidm@hpl.hp.com>
 */

# ifndef __KERNEL_STRICT_NAMES
#  include <linux/types.h>
   typedef __kernel_fsid_t	fsid_t;
# endif

struct statfs {
	long f_type;
	long f_bsize;
	long f_blocks;
	long f_bfree;
	long f_bavail;
	long f_files;
	long f_ffree;
	__kernel_fsid_t f_fsid;
	long f_namelen;
	long f_spare[6];
};

#endif /* _ASM_IA64_STATFS_H */
