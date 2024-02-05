// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.All Rights Reserved.
 */
#ifndef LIBXLOG_H
#define LIBXLOG_H

/*
 * define the userlevel xlog_t to be the subset of the kernel's
 * xlog_t that we actually need to get our work done, avoiding
 * the need to define any exotic kernel types in userland.
 */
struct xlog {
	atomic64_t	l_tail_lsn;     /* lsn of 1st LR w/ unflush buffers */
	atomic64_t	l_last_sync_lsn;/* lsn of last LR on disk */
	xfs_mount_t	*l_mp;	        /* mount point */
	struct xfs_buftarg *l_dev;	        /* dev_t of log */
	xfs_daddr_t	l_logBBstart;   /* start block of log */
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
};

#include "xfs_log_recover.h"

/*
 * macros mapping kernel code to user code
 *
 * XXX: this is duplicated stuff - should be shared with libxfs.
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
#define xfs_alert(mp,fmt,args...)	cmn_err(CE_ALERT,fmt, ## args)
#define xfs_warn(mp,fmt,args...)	cmn_err(CE_WARN,fmt, ## args)
#define xfs_hex_dump(d,n)		((void) 0)
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

extern void xlog_warn(char *fmt,...);
extern void xlog_exit(char *fmt,...);
extern void xlog_panic(char *fmt,...);

/* exports */
extern int	print_exit;
extern int	print_skip_uuid;
extern int	print_record_header;

/* libxfs parameters */
extern libxfs_init_t	x;


extern int xlog_is_dirty(struct xfs_mount *, struct xlog *, libxfs_init_t *,
			 int);
extern struct xfs_buf *xlog_get_bp(struct xlog *, int);
extern int	xlog_bread(struct xlog *log, xfs_daddr_t blk_no, int nbblks,
				struct xfs_buf *bp, char **offset);
extern int	xlog_bread_noalign(struct xlog *log, xfs_daddr_t blk_no,
				int nbblks, struct xfs_buf *bp);

extern int	xlog_find_zeroed(struct xlog *log, xfs_daddr_t *blk_no);
extern int	xlog_find_cycle_start(struct xlog *log, struct xfs_buf *bp,
				xfs_daddr_t first_blk, xfs_daddr_t *last_blk,
				uint cycle);
extern int	xlog_find_tail(struct xlog *log, xfs_daddr_t *head_blk,
				xfs_daddr_t *tail_blk);

extern int	xlog_recover(struct xlog *log, int readonly);
extern void	xlog_recover_print_data(char *p, int len);
extern void	xlog_recover_print_logitem(struct xlog_recover_item *item);
extern void	xlog_recover_print_trans_head(struct xlog_recover *tr);
extern int	xlog_print_find_oldest(struct xlog *log, xfs_daddr_t *last_blk);

/* for transactional view */
extern void	xlog_recover_print_trans_head(struct xlog_recover *tr);
extern void	xlog_recover_print_trans(struct xlog_recover *trans,
				struct list_head *itemq, int print);
extern int	xlog_do_recovery_pass(struct xlog *log, xfs_daddr_t head_blk,
				xfs_daddr_t tail_blk, int pass);
extern int	xlog_recover_do_trans(struct xlog *log, struct xlog_recover *trans,
				int pass);
extern int	xlog_header_check_recover(xfs_mount_t *mp,
				xlog_rec_header_t *head);
extern int	xlog_header_check_mount(xfs_mount_t *mp,
				xlog_rec_header_t *head);

#define xlog_assign_atomic_lsn(l,a,b) ((void) 0)
#define xlog_assign_grant_head(l,a,b) ((void) 0)
#endif	/* LIBXLOG_H */
