/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.All Rights Reserved.
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
#ifndef LIBXLOG_H
#define LIBXLOG_H

#include <xfs/libxfs.h>

/*
 * define the userlevel xlog_t to be the subset of the kernel's
 * xlog_t that we actually need to get our work done, avoiding
 * the need to define any exotic kernel types in userland.
 */
typedef struct log {
	xfs_lsn_t	l_tail_lsn;     /* lsn of 1st LR w/ unflush buffers */
	xfs_lsn_t	l_last_sync_lsn;/* lsn of last LR on disk */
	xfs_mount_t	*l_mp;	        /* mount point */
	dev_t		l_dev;	        /* dev_t of log */
	xfs_daddr_t	l_logBBstart;   /* start block of log */
	int		l_logsize;      /* size of log in bytes */
	int		l_logBBsize;    /* size of log in 512 byte chunks */
	int		l_curr_cycle;   /* Cycle number of log writes */
	int		l_prev_cycle;   /* Cycle # b4 last block increment */
	int		l_curr_block;   /* current logical block of log */
	int		l_prev_block;   /* previous logical block of log */
	int		l_iclog_size;	 /* size of log in bytes */
	int		l_iclog_size_log;/* log power size of log */
	int		l_iclog_bufs;	 /* number of iclog buffers */
	atomic64_t	l_grant_reserve_head;
	atomic64_t	l_grant_write_head;
	uint		l_sectbb_log;   /* log2 of sector size in bbs */
	uint		l_sectbb_mask;  /* sector size (in BBs)
					 * alignment mask */
	int		l_sectBBsize;   /* size of log sector in 512 byte chunks */
} xlog_t;

#include <xfs/xfs_log_recover.h>
#include <xfs/xfs_buf_item.h>
#include <xfs/xfs_inode_item.h>
#include <xfs/xfs_extfree_item.h>

typedef union {
	xlog_rec_header_t       hic_header;
	xlog_rec_ext_header_t   hic_xheader;
	char                    hic_sector[XLOG_HEADER_SIZE];
} xlog_in_core_2_t;

/*
 * macros mapping kernel code to user code
 */
#ifndef EFSCORRUPTED
#define EFSCORRUPTED			 990
#endif
#define STATIC				static
#define XFS_ERROR(e)			(e)
#ifdef DEBUG
#define XFS_ERROR_REPORT(e,l,mp)	fprintf(stderr, "ERROR: %s\n", e)
#else
#define XFS_ERROR_REPORT(e,l,mp)	((void) 0)
#endif
#define XFS_CORRUPTION_ERROR(e,l,mp,m)	((void) 0)
#define XFS_MOUNT_WAS_CLEAN		0x1
#define unlikely(x)			(x)
#define min(a,b)			((a) < (b) ? (a) : (b))

extern void xlog_warn(char *fmt,...);
extern void xlog_exit(char *fmt,...);
extern void xlog_panic(char *fmt,...);

/* exports */
extern int	print_exit;
extern int	print_skip_uuid;
extern int	print_record_header;

/* libxfs parameters */
extern libxfs_init_t	x;

extern struct xfs_buf *xlog_get_bp(xlog_t *, int);
extern void	xlog_put_bp(struct xfs_buf *);
extern int	xlog_bread(xlog_t *log, xfs_daddr_t blk_no, int nbblks,
				xfs_buf_t *bp, xfs_caddr_t *offset);
extern int	xlog_bread_noalign(xlog_t *log, xfs_daddr_t blk_no, int nbblks,
				xfs_buf_t *bp);

extern int	xlog_find_zeroed(xlog_t *log, xfs_daddr_t *blk_no);
extern int	xlog_find_cycle_start(xlog_t *log, xfs_buf_t *bp,
				xfs_daddr_t first_blk, xfs_daddr_t *last_blk, 
				uint cycle);
extern int	xlog_find_tail(xlog_t *log, xfs_daddr_t *head_blk,
				xfs_daddr_t *tail_blk);

extern int	xlog_test_footer(xlog_t *log);
extern int	xlog_recover(xlog_t *log, int readonly);
extern void	xlog_recover_print_data(xfs_caddr_t p, int len);
extern void	xlog_recover_print_logitem(xlog_recover_item_t *item);
extern void	xlog_recover_print_trans_head(xlog_recover_t *tr);
extern int	xlog_print_find_oldest(xlog_t *log, xfs_daddr_t *last_blk);

/* for transactional view */
extern void	xlog_recover_print_trans_head(xlog_recover_t *tr);
extern void	xlog_recover_print_trans(xlog_recover_t *trans,
				struct list_head *itemq, int print);
extern int	xlog_do_recovery_pass(xlog_t *log, xfs_daddr_t head_blk,
				xfs_daddr_t tail_blk, int pass);
extern int	xlog_recover_do_trans(xlog_t *log, xlog_recover_t *trans,
				int pass);
extern int	xlog_header_check_recover(xfs_mount_t *mp, 
				xlog_rec_header_t *head);
extern int	xlog_header_check_mount(xfs_mount_t *mp,
				xlog_rec_header_t *head);

#define xlog_assign_atomic_lsn(l,a,b) ((void) 0)
#define xlog_assign_grant_head(l,a,b) ((void) 0)
#endif	/* LIBXLOG_H */
