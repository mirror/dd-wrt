/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define CMD_NOFILE_OK	(1<<0)	/* command doesn't need an open file	*/
#define CMD_NOMAP_OK	(1<<1)	/* command doesn't need a mapped region	*/
#define CMD_FOREIGN_OK	(1<<2)	/* command not restricted to XFS files	*/

extern char	*progname;
extern int	exitcode;
extern int	expert;
extern size_t	pagesize;
extern struct timeval stopwatch;

#define min(a,b)	(((a)<(b))?(a):(b))
#define max(a,b)	(((a)>(b))?(a):(b))

extern void init_cvtnum(size_t *blocksize, size_t *sectsize);
