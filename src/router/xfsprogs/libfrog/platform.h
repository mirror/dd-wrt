/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef __LIBFROG_PLATFORM_H__
#define __LIBFROG_PLATFORM_H__

int platform_check_ismounted(char *path, char *block, struct stat *sptr,
		int verbose);
int platform_check_iswritable(char *path, char *block, struct stat *sptr);
int platform_set_blocksize(int fd, char *path, dev_t device, int bsz,
		int fatal);
int platform_flush_device(int fd, dev_t device);
char *platform_findrawpath(char *path);
char *platform_findblockpath(char *path);
int platform_direct_blockdev(void);
int platform_align_blockdev(void);
unsigned long platform_physmem(void);	/* in kilobytes */
void platform_findsizes(char *path, int fd, long long *sz, int *bsz);
int platform_nproc(void);

void platform_findsizes(char *path, int fd, long long *sz, int *bsz);

#endif /* __LIBFROG_PLATFORM_H__ */
