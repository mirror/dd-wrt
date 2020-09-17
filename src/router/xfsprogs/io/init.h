// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#define CMD_NOFILE_OK	(1<<0)	/* command doesn't need an open file	*/
#define CMD_NOMAP_OK	(1<<1)	/* command doesn't need a mapped region	*/
#define CMD_FOREIGN_OK	CMD_FLAG_FOREIGN_OK

extern char	*progname;
extern int	exitcode;
extern int	expert;
extern size_t	pagesize;
extern struct timeval stopwatch;

extern void init_cvtnum(size_t *blocksize, size_t *sectsize);
