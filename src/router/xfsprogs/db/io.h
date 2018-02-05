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
	int			nmaps;
	struct xfs_buf_map	b[BBMAP_SIZE];
} bbmap_t;

typedef struct iocur {
	int64_t			bb;	/* BB number in filesystem of buf */
	int			blen;	/* length of "buf", bb's */
	int			boff;	/* data - buf */
	void			*buf;	/* base address of buffer */
	void			*data;	/* current interesting data */
	xfs_ino_t		dirino;	/* current directory inode number */
	xfs_ino_t		ino;	/* current inode number */
	int			len;	/* length of "data", bytes */
	uint16_t		mode;	/* current inode's mode */
	xfs_off_t		off;	/* fs offset of "data" in bytes */
	const struct typ	*typ;	/* type of "data" */
	bbmap_t			*bbmap;	/* map daddr if fragmented */
	struct xfs_buf		*bp;	/* underlying buffer */
	int			ino_crc_ok:1;
	int			ino_buf:1;
	int			dquot_buf:1;
	int			need_crc:1;
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
extern void	push_cur_and_set_type(void);
extern int	read_buf(int64_t daddr, int count, void *bufp);
extern void     write_cur(void);
extern void	set_cur(const struct typ *type, xfs_daddr_t blknum,
			int len, int ring_add, bbmap_t *bbmap);
extern void     ring_add(void);
extern void	set_iocur_type(const struct typ *type);
extern void	xfs_dummy_verify(struct xfs_buf *bp);
extern void	xfs_verify_recalc_crc(struct xfs_buf *bp);

/*
 * returns -1 for unchecked, 0 for bad and 1 for good
 */
static inline int
iocur_crc_valid()
{
	if (!iocur_top->bp)
		return -1;
	if (iocur_top->bp->b_flags & LIBXFS_B_UNCHECKED)
		return -1;
	return (iocur_top->bp->b_error != -EFSBADCRC &&
		(!iocur_top->ino_buf || iocur_top->ino_crc_ok));
}
