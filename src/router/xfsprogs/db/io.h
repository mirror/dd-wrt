/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

struct typ;

#define	BBMAP_SIZE		(XFS_MAX_BLOCKSIZE / BBSIZE)
typedef struct bbmap {
	__int64_t		b[BBMAP_SIZE];
} bbmap_t;

typedef struct iocur {
	__int64_t		bb;	/* BB number in filesystem of buf */
	int			blen;	/* length of "buf", bb's */
	int			boff;	/* data - buf */
	void			*buf;	/* base address of buffer */
	void			*data;	/* current interesting data */
	xfs_ino_t		dirino;	/* current directory inode number */
	xfs_ino_t		ino;	/* current inode number */
	int			len;	/* length of "data", bytes */
	__uint16_t		mode;	/* current inode's mode */
	xfs_off_t		off;	/* fs offset of "data" in bytes */
	const struct typ	*typ;	/* type of "data" */
	int			use_bbmap; /* set if bbmap is valid */
	bbmap_t			bbmap;	/* map daddr if fragmented */
} iocur_t;

#define DB_RING_ADD 1                   /* add to ring on set_cur */
#define DB_RING_IGN 0                   /* do not add to ring on set_cur */

extern iocur_t	*iocur_base;		/* base of stack */
extern iocur_t	*iocur_top;		/* top element of stack */
extern int	iocur_sp;		/* current top of stack */
extern int	iocur_len;		/* length of stack array */

extern void	io_init(void);
extern void	off_cur(int off, int len);
extern void	pop_cur(void);
extern void	print_iocur(char *tag, iocur_t *ioc);
extern void	push_cur(void);
extern int	read_bbs(__int64_t daddr, int count, void **bufp,
			 bbmap_t *bbmap);
extern int	write_bbs(__int64_t daddr, int count, void *bufp,
			  bbmap_t *bbmap);
extern void     write_cur(void);
extern void	set_cur(const struct typ *t, __int64_t d, int c, int ring_add,
			bbmap_t *bbmap);
extern void     ring_add(void);
