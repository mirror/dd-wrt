/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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
#include "libxfs.h"
#include "libxlog.h"

#include "logprint.h"

#define CLEARED_BLKS	(-5)
#define ZEROED_LOG	(-4)
#define FULL_READ	(-3)
#define PARTIAL_READ	(-2)
#define BAD_HEADER	(-1)
#define NO_ERROR	(0)

static int logBBsize;

typedef struct xlog_split_item {
	struct xlog_split_item	*si_next;
	struct xlog_split_item	*si_prev;
	xlog_tid_t		si_xtid;
	int			si_skip;
} xlog_split_item_t;

xlog_split_item_t *split_list = NULL;

void
print_xlog_op_line(void)
{
    printf("--------------------------------------"
	   "--------------------------------------\n");
}	/* print_xlog_op_line */

void
print_xlog_xhdr_line(void)
{
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	   "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}	/* print_xlog_xhdr_line */

void
print_xlog_record_line(void)
{
    printf("======================================"
	   "======================================\n");
}	/* print_xlog_record_line */

void
print_stars(void)
{
    printf("***********************************"
	   "***********************************\n");
}	/* print_stars */

/*
 * Given a pointer to a data segment, print out the data as if it were
 * a log operation header.
 */
void
xlog_print_op_header(xlog_op_header_t	*op_head,
		     int		i,
		     char		**ptr)
{
    xlog_op_header_t hbuf;

    /*
     * memmove because on 64/n32, partial reads can cause the op_head
     * pointer to come in pointing to an odd-numbered byte
     */
    memmove(&hbuf, op_head, sizeof(xlog_op_header_t));
    op_head = &hbuf;
    *ptr += sizeof(xlog_op_header_t);
    printf(_("Oper (%d): tid: %x  len: %d  clientid: %s  "), i,
	    be32_to_cpu(op_head->oh_tid),
	    be32_to_cpu(op_head->oh_len),
	    (op_head->oh_clientid == XFS_TRANSACTION ? "TRANS" :
	    (op_head->oh_clientid == XFS_LOG ? "LOG" : "ERROR")));
    printf(_("flags: "));
    if (op_head->oh_flags) {
	if (op_head->oh_flags & XLOG_START_TRANS)
	    printf("START ");
	if (op_head->oh_flags & XLOG_COMMIT_TRANS)
	    printf("COMMIT ");
	if (op_head->oh_flags & XLOG_WAS_CONT_TRANS)
	    printf("WAS_CONT ");
	if (op_head->oh_flags & XLOG_UNMOUNT_TRANS)
	    printf("UNMOUNT ");
	if (op_head->oh_flags & XLOG_CONTINUE_TRANS)
	    printf("CONTINUE ");
	if (op_head->oh_flags & XLOG_END_TRANS)
	    printf("END ");
    } else {
	printf(_("none"));
    }
    printf("\n");
}	/* xlog_print_op_header */


void
xlog_print_add_to_trans(xlog_tid_t	tid,
			int		skip)
{
    xlog_split_item_t *item;

    item	  = (xlog_split_item_t *)calloc(sizeof(xlog_split_item_t), 1);
    item->si_xtid  = tid;
    item->si_skip = skip;
    item->si_next = split_list;
    item->si_prev = NULL;
    if (split_list)
	split_list->si_prev = item;
    split_list	  = item;
}	/* xlog_print_add_to_trans */


int
xlog_print_find_tid(xlog_tid_t tid, uint was_cont)
{
    xlog_split_item_t *listp = split_list;

    if (!split_list) {
	if (was_cont != 0)	/* Not first time we have used this tid */
	    return 1;
	else
	    return 0;
    }
    while (listp) {
	if (listp->si_xtid == tid)
	    break;
	listp = listp->si_next;
    }
    if (!listp)  {
	return 0;
    }
    if (--listp->si_skip == 0) {
	if (listp == split_list) {		/* delete at head */
	    split_list = listp->si_next;
	    if (split_list)
		split_list->si_prev = NULL;
	} else {
	    if (listp->si_next)
		listp->si_next->si_prev = listp->si_prev;
	    listp->si_prev->si_next = listp->si_next;
	}
	free(listp);
    }
    return 1;
}	/* xlog_print_find_tid */

int
xlog_print_trans_header(char **ptr, int len)
{
    xfs_trans_header_t  *h;
    char		*cptr = *ptr;
    uint32_t          magic;
    char                *magic_c = (char *)&magic;

    *ptr += len;

    magic = *(uint32_t *)cptr; /* XXX be32_to_cpu soon */

    if (len >= 4) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	printf("%c%c%c%c:",
		magic_c[3], magic_c[2], magic_c[1], magic_c[0]);
#else
	printf("%c%c%c%c:",
		magic_c[0], magic_c[1], magic_c[2], magic_c[3]);
#endif
    }
    if (len != sizeof(xfs_trans_header_t)) {
	printf(_("   Not enough data to decode further\n"));
	return 1;
    }
    h = (xfs_trans_header_t *)cptr;
    printf(_("     tid: %x  num_items: %d\n"),
	   h->th_tid, h->th_num_items);
    return 0;
}	/* xlog_print_trans_header */


int
xlog_print_trans_buffer(char **ptr, int len, int *i, int num_ops)
{
    xfs_buf_log_format_t *f;
    xlog_op_header_t	 *head = NULL;
    int			 num, skip;
    int			 super_block = 0;
    int			 bucket, col, buckets;
    int64_t			 blkno;
    xfs_buf_log_format_t lbuf;
    int			 size, blen, map_size, struct_size;
    __be64		 x, y;
    unsigned short	 flags;

    /*
     * memmove to ensure 8-byte alignment for the long longs in
     * buf_log_format_t structure
     */
    memmove(&lbuf, *ptr, MIN(sizeof(xfs_buf_log_format_t), len));
    f = &lbuf;
    *ptr += len;

    ASSERT(f->blf_type == XFS_LI_BUF);
    printf("BUF:  ");
    blkno = f->blf_blkno;
    size = f->blf_size;
    blen = f->blf_len;
    map_size = f->blf_map_size;
    flags = f->blf_flags;

    /*
     * size of the format header is dependent on the size of the bitmap, not
     * the size of the in-memory structure. Hence the slightly obtuse
     * calculation.
     */
    struct_size = offsetof(struct xfs_buf_log_format, blf_map_size) + map_size;

    if (len >= struct_size) {
	ASSERT((len - sizeof(struct_size)) % sizeof(int) == 0);
	printf(_("#regs: %d   start blkno: %lld (0x%llx)  len: %d  bmap size: %d  flags: 0x%x\n"),
	       size, (long long)blkno, (unsigned long long)blkno, blen, map_size, flags);
	if (blkno == 0)
	    super_block = 1;
    } else {
	ASSERT(len >= 4);	/* must have at least 4 bytes if != 0 */
	printf(_("#regs: %d   Not printing rest of data\n"), f->blf_size);
	return size;
    }
    num = size-1;

    /* Check if all regions in this log item were in the given LR ptr */
    if (*i+num > num_ops-1) {
	skip = num - (num_ops-1-*i);
	num = num_ops-1-*i;
    } else {
	skip = 0;
    }
    while (num-- > 0) {
	(*i)++;
	head = (xlog_op_header_t *)*ptr;
	xlog_print_op_header(head, *i, ptr);
	if (super_block) {
		printf(_("SUPER BLOCK Buffer: "));
		if (be32_to_cpu(head->oh_len) < 4*8) {
			printf(_("Out of space\n"));
		} else {
			printf("\n");
			/*
			 * memmove because *ptr may not be 8-byte aligned
			 */
			memmove(&x, *ptr, sizeof(__be64));
			memmove(&y, *ptr+8, sizeof(__be64));
		       printf(_("icount: %llu  ifree: %llu  "),
			       (unsigned long long) be64_to_cpu(x),
			       (unsigned long long) be64_to_cpu(y));
			memmove(&x, *ptr+16, sizeof(__be64));
			memmove(&y, *ptr+24, sizeof(__be64));
		       printf(_("fdblks: %llu  frext: %llu\n"),
			       (unsigned long long) be64_to_cpu(x),
			       (unsigned long long) be64_to_cpu(y));
		}
		super_block = 0;
	} else if (be32_to_cpu(*(__be32 *)(*ptr)) == XFS_AGI_MAGIC) {
  	  	struct xfs_agi	*agi, agi_s;

		/* memmove because *ptr may not be 8-byte aligned */
		agi = &agi_s;
		memmove(agi, *ptr, sizeof(struct xfs_agi));
		printf(_("AGI Buffer: XAGI  "));
		/*
		 * v4 filesystems only contain the fields before the uuid.
		 * Even v5 filesystems don't log any field beneath it. That
		 * means that the size that is logged is almost always going to
		 * be smaller than the structure itself. Hence we need to make
		 * sure that the buffer contains all the data we want to print
		 * rather than just check against the structure size.
		 */
		if (be32_to_cpu(head->oh_len) < offsetof(xfs_agi_t, agi_uuid) -
				XFS_AGI_UNLINKED_BUCKETS*sizeof(xfs_agino_t)) {
			printf(_("out of space\n"));
		} else {
			printf("\n");
			printf(_("ver: %d  "),
				be32_to_cpu(agi->agi_versionnum));
			printf(_("seq#: %d  len: %d  cnt: %d  root: %d\n"),
				be32_to_cpu(agi->agi_seqno),
				be32_to_cpu(agi->agi_length),
				be32_to_cpu(agi->agi_count),
				be32_to_cpu(agi->agi_root));
			printf(_("level: %d  free#: 0x%x  newino: 0x%x\n"),
				be32_to_cpu(agi->agi_level),
				be32_to_cpu(agi->agi_freecount),
				be32_to_cpu(agi->agi_newino));
			if (be32_to_cpu(head->oh_len) == 128) {
				buckets = 17;
			} else if (be32_to_cpu(head->oh_len) == 256) {
				buckets = 32 + 17;
			} else {
				if (head->oh_flags & XLOG_CONTINUE_TRANS) {
					printf(_("AGI unlinked data skipped "));
					printf(_("(CONTINUE set, no space)\n"));
					continue;
				}
				buckets = XFS_AGI_UNLINKED_BUCKETS;
			}
			for (bucket = 0; bucket < buckets;) {
				printf(_("bucket[%d - %d]: "), bucket, bucket+3);
				for (col = 0; col < 4; col++, bucket++) {
					if (bucket < buckets) {
						printf("0x%x ",
			be32_to_cpu(agi->agi_unlinked[bucket]));
					}
				}
				printf("\n");
			}
		}
	} else if (be32_to_cpu(*(__be32 *)(*ptr)) == XFS_AGF_MAGIC) {
    		struct xfs_agf	*agf, agf_s;

		/* memmove because *ptr may not be 8-byte aligned */
		agf = &agf_s;
		memmove(agf, *ptr, sizeof(struct xfs_agf));
		printf(_("AGF Buffer: XAGF  "));
		/*
		 * v4 filesystems only contain the fields before the uuid.
		 * Even v5 filesystems don't log any field beneath it. That
		 * means that the size that is logged is almost always going to
		 * be smaller than the structure itself. Hence we need to make
		 * sure that the buffer contains all the data we want to print
		 * rather than just check against the structure size.
		 */
		if (be32_to_cpu(head->oh_len) < offsetof(xfs_agf_t, agf_uuid)) {
			printf(_("Out of space\n"));
		} else {
			printf("\n");
			printf(_("ver: %d  seq#: %d  len: %d  \n"),
				be32_to_cpu(agf->agf_versionnum),
				be32_to_cpu(agf->agf_seqno),
				be32_to_cpu(agf->agf_length));
			printf(_("root BNO: %d  CNT: %d\n"),
				be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]),
				be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));
			printf(_("level BNO: %d  CNT: %d\n"),
				be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNOi]),
				be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNTi]));
			printf(_("1st: %d  last: %d  cnt: %d  "
			       "freeblks: %d  longest: %d\n"),
				be32_to_cpu(agf->agf_flfirst),
				be32_to_cpu(agf->agf_fllast),
				be32_to_cpu(agf->agf_flcount),
				be32_to_cpu(agf->agf_freeblks),
				be32_to_cpu(agf->agf_longest));
		}
	} else if (be32_to_cpu(*(__be32 *)(*ptr)) == XFS_DQUOT_MAGIC) {
		struct xfs_disk_dquot *dq, dq_s;

		/* memmove because *ptr may not be 8-byte aligned */
		dq = &dq_s;
		memmove(dq, *ptr, sizeof(struct xfs_disk_dquot));
		printf(_("DQUOT Buffer: DQ  "));
		if (be32_to_cpu(head->oh_len) <
				sizeof(xfs_disk_dquot_t)) {
			printf(_("Out of space\n"));
		}
		else {
			printf("\n");
			printf(_("ver: %d  flags: 0x%x  id: %d  \n"),
				dq->d_version, dq->d_flags,
				be32_to_cpu(dq->d_id));
			printf(_("blk limits  hard: %llu  soft: %llu\n"),
			       (unsigned long long)
				       be64_to_cpu(dq->d_blk_hardlimit),
			       (unsigned long long)
				       be64_to_cpu(dq->d_blk_softlimit));
			printf(_("blk  count: %llu  warns: %d  timer: %d\n"),
			       (unsigned long long) be64_to_cpu(dq->d_bcount),
			       (int) be16_to_cpu(dq->d_bwarns),
				be32_to_cpu(dq->d_btimer));
			printf(_("ino limits  hard: %llu  soft: %llu\n"),
			       (unsigned long long)
				       be64_to_cpu(dq->d_ino_hardlimit),
			       (unsigned long long)
				       be64_to_cpu(dq->d_ino_softlimit));
			printf(_("ino  count: %llu  warns: %d  timer: %d\n"),
			       (unsigned long long) be64_to_cpu(dq->d_icount),
			       (int) be16_to_cpu(dq->d_iwarns),
				be32_to_cpu(dq->d_itimer));
		}
	} else {
		printf(_("BUF DATA\n"));
		if (print_data) {
			uint *dp  = (uint *)*ptr;
			int  nums = be32_to_cpu(head->oh_len) >> 2;
			int  i = 0;

			while (i < nums) {
				if ((i % 8) == 0)
					printf("%2x ", i);
				printf("%8x ", *dp);
				dp++;
				i++;
				if ((i % 8) == 0)
					printf("\n");
			}
			printf("\n");
		}
	}
	*ptr += be32_to_cpu(head->oh_len);
    }
    if (head && head->oh_flags & XLOG_CONTINUE_TRANS)
	skip++;
    return skip;
}	/* xlog_print_trans_buffer */


int
xlog_print_trans_qoff(char **ptr, uint len)
{
    xfs_qoff_logformat_t *f;
    xfs_qoff_logformat_t lbuf;

    memmove(&lbuf, *ptr, MIN(sizeof(xfs_qoff_logformat_t), len));
    f = &lbuf;
    *ptr += len;
    if (len >= sizeof(xfs_qoff_logformat_t)) {
	printf(_("QOFF:  #regs: %d    flags: 0x%x\n"), f->qf_size, f->qf_flags);
	return 0;
    } else {
	printf(_("QOFF: Not enough data to decode further\n"));
	return 1;
    }
}	/* xlog_print_trans_qoff */


void
xlog_print_trans_inode_core(
	struct xfs_log_dinode	*ip)
{
    printf(_("INODE CORE\n"));
    printf(_("magic 0x%hx mode 0%ho version %d format %d\n"),
	   ip->di_magic, ip->di_mode, (int)ip->di_version,
	   (int)ip->di_format);
    printf(_("nlink %hd uid %d gid %d\n"),
	   ip->di_nlink, ip->di_uid, ip->di_gid);
    printf(_("atime 0x%x mtime 0x%x ctime 0x%x\n"),
	   ip->di_atime.t_sec, ip->di_mtime.t_sec, ip->di_ctime.t_sec);
    printf(_("size 0x%llx nblocks 0x%llx extsize 0x%x nextents 0x%x\n"),
	   (unsigned long long)ip->di_size, (unsigned long long)ip->di_nblocks,
	   ip->di_extsize, ip->di_nextents);
    printf(_("naextents 0x%x forkoff %d dmevmask 0x%x dmstate 0x%hx\n"),
	   ip->di_anextents, (int)ip->di_forkoff, ip->di_dmevmask,
	   ip->di_dmstate);
    printf(_("flags 0x%x gen 0x%x\n"),
	   ip->di_flags, ip->di_gen);
    if (ip->di_version == 3) {
        printf(_("flags2 0x%llx cowextsize 0x%x\n"),
            (unsigned long long)ip->di_flags2, ip->di_cowextsize);
    }
}

void
xlog_print_dir2_sf(
	struct xlog	*log,
	xfs_dir2_sf_hdr_t *sfp,
	int		size)
{
	xfs_ino_t	ino;
	int		count;
	int		i;
	char		namebuf[257];
	xfs_dir2_sf_entry_t	*sfep;

	printf(_("SHORTFORM DIRECTORY size %d\n"),
		size);
	/* bail out for now */

	return;

	printf(_("SHORTFORM DIRECTORY size %d count %d\n"),
	       size, sfp->count);
	memmove(&ino, &(sfp->parent), sizeof(ino));
	printf(_(".. ino 0x%llx\n"), (unsigned long long) be64_to_cpu(ino));

	count = sfp->count;
	sfep = xfs_dir2_sf_firstentry(sfp);
	for (i = 0; i < count; i++) {
		ino = M_DIROPS(log->l_mp)->sf_get_ino(sfp, sfep);
		memmove(namebuf, (sfep->name), sfep->namelen);
		namebuf[sfep->namelen] = '\0';
		printf(_("%s ino 0x%llx namelen %d\n"),
		       namebuf, (unsigned long long)ino, sfep->namelen);
		sfep = M_DIROPS(log->l_mp)->sf_nextentry(sfp, sfep);
	}
}

int
xlog_print_trans_inode(
	struct xlog		*log,
	char			**ptr,
	int			len,
	int			*i,
	int			num_ops,
	int			continued)
{
    struct xfs_log_dinode	dino;
    struct xlog_op_header	*op_head;
    struct xfs_inode_log_format	dst_lbuf;
    struct xfs_inode_log_format	src_lbuf;
    struct xfs_inode_log_format *f;
    int				mode;
    int				size;
    int				skip_count;

    /*
     * print inode type header region
     *
     * memmove to ensure 8-byte alignment for the long longs in
     * xfs_inode_log_format_t structure
     *
     * len can be smaller than xfs_inode_log_format_t
     * if format data is split over operations
     */
    memmove(&src_lbuf, *ptr, MIN(sizeof(src_lbuf), len));
    (*i)++;					/* bump index */
    *ptr += len;
    if (!continued &&
	(len == sizeof(struct xfs_inode_log_format_32) ||
	 len == sizeof(struct xfs_inode_log_format))) {
	f = xfs_inode_item_format_convert((char*)&src_lbuf, len, &dst_lbuf);
	printf(_("INODE: "));
	printf(_("#regs: %d   ino: 0x%llx  flags: 0x%x   dsize: %d\n"),
	       f->ilf_size, (unsigned long long)f->ilf_ino,
	       f->ilf_fields, f->ilf_dsize);
	printf(_("        blkno: %lld  len: %d  boff: %d\n"),
	       (long long)f->ilf_blkno, f->ilf_len, f->ilf_boffset);
    } else {
	ASSERT(len >= 4);	/* must have at least 4 bytes if != 0 */
	f = (xfs_inode_log_format_t *)&src_lbuf;
	printf(_("INODE: #regs: %d   Not printing rest of data\n"),
	       f->ilf_size);
	return f->ilf_size;
    }

    skip_count = f->ilf_size-1;

    if (*i >= num_ops)			/* end of LR */
	    return skip_count;

    /* core inode comes 2nd */
    op_head = (xlog_op_header_t *)*ptr;
    xlog_print_op_header(op_head, *i, ptr);

    if (op_head->oh_flags & XLOG_CONTINUE_TRANS)  {
        return skip_count;
    }

    memmove(&dino, *ptr, sizeof(dino));
    mode = dino.di_mode & S_IFMT;
    size = (int)dino.di_size;
    xlog_print_trans_inode_core(&dino);
    *ptr += xfs_log_dinode_size(dino.di_version);
    skip_count--;

    switch (f->ilf_fields & (XFS_ILOG_DEV | XFS_ILOG_UUID)) {
    case XFS_ILOG_DEV:
	printf(_("DEV inode: no extra region\n"));
	break;
    case XFS_ILOG_UUID:
	printf(_("UUID inode: no extra region\n"));
	break;
    }

    /* Only the inode core is logged */
    if (f->ilf_size == 2)
	return 0;

    ASSERT(f->ilf_size <= 4);
    ASSERT((f->ilf_size == 3) || (f->ilf_fields & XFS_ILOG_AFORK));

    /* does anything come next */
    op_head = (xlog_op_header_t *)*ptr;

    if (f->ilf_fields & XFS_ILOG_DFORK) {
	    if (*i == num_ops-1)
	        return skip_count;
	    (*i)++;
	    xlog_print_op_header(op_head, *i, ptr);

	    switch (f->ilf_fields & XFS_ILOG_DFORK) {
	    case XFS_ILOG_DEXT:
		printf(_("EXTENTS inode data\n"));
		break;
	    case XFS_ILOG_DBROOT:
		printf(_("BTREE inode data\n"));
		break;
	    case XFS_ILOG_DDATA:
		printf(_("LOCAL inode data\n"));
		if (mode == S_IFDIR)
		    xlog_print_dir2_sf(log, (xfs_dir2_sf_hdr_t *)*ptr, size);
		break;
	    default:
		ASSERT((f->ilf_fields & XFS_ILOG_DFORK) == 0);
		break;
	    }

	    *ptr += be32_to_cpu(op_head->oh_len);
	    if (op_head->oh_flags & XLOG_CONTINUE_TRANS)
	        return skip_count;
	    op_head = (xlog_op_header_t *)*ptr;
	    skip_count--;
    }

    if (f->ilf_fields & XFS_ILOG_AFORK) {
	    if (*i == num_ops-1)
	        return skip_count;
	    (*i)++;
	    xlog_print_op_header(op_head, *i, ptr);

	    switch (f->ilf_fields & XFS_ILOG_AFORK) {
	    case XFS_ILOG_AEXT:
		printf(_("EXTENTS attr data\n"));
		break;
	    case XFS_ILOG_ABROOT:
		printf(_("BTREE attr data\n"));
		break;
	    case XFS_ILOG_ADATA:
		printf(_("LOCAL attr data\n"));
		if (mode == S_IFDIR)
		    xlog_print_dir2_sf(log, (xfs_dir2_sf_hdr_t *)*ptr, size);
		break;
	    default:
		ASSERT((f->ilf_fields & XFS_ILOG_AFORK) == 0);
		break;
	    }
	    *ptr += be32_to_cpu(op_head->oh_len);
	    if (op_head->oh_flags & XLOG_CONTINUE_TRANS)
	        return skip_count;
	    skip_count--;
    }

    ASSERT(skip_count == 0);

    return 0;
}	/* xlog_print_trans_inode */


int
xlog_print_trans_dquot(char **ptr, int len, int *i, int num_ops)
{
    xfs_dq_logformat_t	*f;
    xfs_dq_logformat_t	lbuf = {0};
    xfs_disk_dquot_t	ddq;
    xlog_op_header_t	*head = NULL;
    int			num, skip;

    /*
     * print dquot header region
     *
     * memmove to ensure 8-byte alignment for the long longs in
     * xfs_dq_logformat_t structure
     */
    memmove(&lbuf, *ptr, MIN(sizeof(xfs_dq_logformat_t), len));
    f = &lbuf;
    (*i)++;					/* bump index */
    *ptr += len;

    if (len == sizeof(xfs_dq_logformat_t)) {
	printf(_("#regs: %d   id: 0x%x"), f->qlf_size, f->qlf_id);
	printf(_("  blkno: %lld  len: %d  boff: %d\n"),
		(long long)f->qlf_blkno, f->qlf_len, f->qlf_boffset);
    } else {
	ASSERT(len >= 4);	/* must have at least 4 bytes if != 0 */
	printf(_("DQUOT: #regs: %d   Not printing rest of data\n"),
		f->qlf_size);
	return f->qlf_size;
    }
    num = f->qlf_size-1;

    /* Check if all regions in this log item were in the given LR ptr */
    if (*i+num > num_ops-1) {
	skip = num - (num_ops-1-*i);
	num = num_ops-1-*i;
    } else {
	skip = 0;
    }

    while (num-- > 0) {
	head = (xlog_op_header_t *)*ptr;
	xlog_print_op_header(head, *i, ptr);
	ASSERT(be32_to_cpu(head->oh_len) == sizeof(xfs_disk_dquot_t));
	memmove(&ddq, *ptr, sizeof(xfs_disk_dquot_t));
	printf(_("DQUOT: magic 0x%hx flags 0%ho\n"),
	       be16_to_cpu(ddq.d_magic), ddq.d_flags);
	*ptr += be32_to_cpu(head->oh_len);
    }
    if (head && head->oh_flags & XLOG_CONTINUE_TRANS)
	skip++;
    return skip;
}	/* xlog_print_trans_dquot */


STATIC int
xlog_print_trans_icreate(
	char		**ptr,
	int		len,
	int		*i,
	int		num_ops)
{
	struct xfs_icreate_log	icl_buf = {0};
	struct xfs_icreate_log	*icl;

	memmove(&icl_buf, *ptr, MIN(sizeof(struct xfs_icreate_log), len));
	icl = &icl_buf;
	*ptr += len;

	/* handle complete header only */
	if (len != sizeof(struct xfs_icreate_log)) {
		printf(_("ICR: split header, not printing\n"));
		return 1; /* to skip leftover in next region */
	}

	printf(_("ICR:  #ag: %d  agbno: 0x%x  len: %d\n"
		 "      cnt: %d  isize: %d    gen: 0x%x\n"),
		be32_to_cpu(icl->icl_ag), be32_to_cpu(icl->icl_agbno),
		be32_to_cpu(icl->icl_length), be32_to_cpu(icl->icl_count),
		be32_to_cpu(icl->icl_isize), be32_to_cpu(icl->icl_gen));
	return 0;
}

/******************************************************************************
 *
 *		Log print routines
 *
 ******************************************************************************
 */

void
xlog_print_lseek(struct xlog *log, int fd, xfs_daddr_t blkno, int whence)
{
#define BBTOOFF64(bbs)	(((xfs_off_t)(bbs)) << BBSHIFT)
	xfs_off_t offset;

	if (whence == SEEK_SET)
		offset = BBTOOFF64(blkno+log->l_logBBstart);
	else
		offset = BBTOOFF64(blkno);
	if (lseek(fd, offset, whence) < 0) {
		fprintf(stderr, _("%s: lseek to %lld failed: %s\n"),
			progname, (long long)offset, strerror(errno));
		exit(1);
	}
}	/* xlog_print_lseek */


void
print_lsn(char		*string,
	  __be64	*lsn)
{
    printf("%s: %u,%u", string,
	    CYCLE_LSN(be64_to_cpu(*lsn)), BLOCK_LSN(be64_to_cpu(*lsn)));
}


int
xlog_print_record(
	struct xlog		*log,
	int			fd,
	int			num_ops,
	int			len,
	int			*read_type,
	char			**partial_buf,
	xlog_rec_header_t	*rhead,
	xlog_rec_ext_header_t	*xhdrs,
	int			bad_hdr_warn)
{
    char		*buf, *ptr;
    int			read_len, skip, lost_context = 0;
    int			ret, n, i, j, k;

    if (print_no_print)
	    return NO_ERROR;

    if (!len) {
	printf("\n");
	return NO_ERROR;
    }

    /* read_len must read up to some block boundary */
    read_len = (int) BBTOB(BTOBB(len));

    /* read_type => don't malloc() new buffer, use old one */
    if (*read_type == FULL_READ) {
	if ((ptr = buf = malloc(read_len)) == NULL) {
	    fprintf(stderr, _("%s: xlog_print_record: malloc failed\n"), progname);
	    exit(1);
	}
    } else {
	read_len -= *read_type;
	buf = (char *)((intptr_t)(*partial_buf) + (intptr_t)(*read_type));
	ptr = *partial_buf;
    }
    if ((ret = (int) read(fd, buf, read_len)) == -1) {
	fprintf(stderr, _("%s: xlog_print_record: read error\n"), progname);
	exit(1);
    }
    /* Did we overflow the end? */
    if (*read_type == FULL_READ &&
	BLOCK_LSN(be64_to_cpu(rhead->h_lsn)) + BTOBB(read_len) >=
		logBBsize) {
	*read_type = BBTOB(logBBsize - BLOCK_LSN(be64_to_cpu(rhead->h_lsn))-1);
	*partial_buf = buf;
	return PARTIAL_READ;
    }

    /* Did we read everything? */
    if ((ret == 0 && read_len != 0) || ret != read_len) {
	*read_type = ret;
	*partial_buf = buf;
	return PARTIAL_READ;
    }
    if (*read_type != FULL_READ)
	read_len += *read_type;

    /* Everything read in.  Start from beginning of buffer
     * Unpack the data, by putting the saved cycle-data back
     * into the first word of each BB.
     * Do some checks.
     */
    buf = ptr;
    for (i = 0; ptr < buf + read_len; ptr += BBSIZE, i++) {
	xlog_rec_header_t *rechead = (xlog_rec_header_t *)ptr;

	/* sanity checks */
	if (be32_to_cpu(rechead->h_magicno) == XLOG_HEADER_MAGIC_NUM) {
	    /* data should not have magicno as first word
	     * as it should by cycle#
	     */
	    free(buf);
	    return -1;
	} else {
	    /* verify cycle#
	     * FIXME: cycle+1 should be a macro pv#900369
	     */
	    if (be32_to_cpu(rhead->h_cycle) !=
			be32_to_cpu(*(__be32 *)ptr)) {
		if ((*read_type == FULL_READ) ||
		    (be32_to_cpu(rhead->h_cycle) + 1 !=
				be32_to_cpu(*(__be32 *)ptr))) {
			free(buf);
			return -1;
		}
	    }
	}

	/* copy back the data from the header */
	if (i < XLOG_HEADER_CYCLE_SIZE / BBSIZE) {
		/* from 1st header */
		*(__be32 *)ptr = rhead->h_cycle_data[i];
	}
	else {
		ASSERT(xhdrs != NULL);
		/* from extra headers */
		j = i / (XLOG_HEADER_CYCLE_SIZE / BBSIZE);
		k = i % (XLOG_HEADER_CYCLE_SIZE / BBSIZE);
		*(__be32 *)ptr = xhdrs[j-1].xh_cycle_data[k];
	}

    }

    ptr = buf;
    for (i=0; i<num_ops; i++) {
	int continued;

	xlog_op_header_t *op_head = (xlog_op_header_t *)ptr;

	print_xlog_op_line();
	xlog_print_op_header(op_head, i, &ptr);
	continued = ((op_head->oh_flags & XLOG_WAS_CONT_TRANS) ||
		     (op_head->oh_flags & XLOG_CONTINUE_TRANS));

	if (continued && be32_to_cpu(op_head->oh_len) == 0)
		continue;

	if (print_no_data) {
	    for (n = 0; n < be32_to_cpu(op_head->oh_len); n++) {
		printf("0x%02x ", (unsigned int)*ptr);
		if (n % 16 == 15)
			printf("\n");
		ptr++;
	    }
	    printf("\n");
	    continue;
	}

	/* print transaction data */
	if (xlog_print_find_tid(be32_to_cpu(op_head->oh_tid),
				op_head->oh_flags & XLOG_WAS_CONT_TRANS)) {
	    printf(_("Left over region from split log item\n"));
	    /* Skip this leftover bit */
	    ptr += be32_to_cpu(op_head->oh_len);
	    /* We've lost context; don't complain if next one looks bad too */
	    lost_context = 1;
	    continue;
	}

	if (be32_to_cpu(op_head->oh_len) != 0) {
	    if (*(uint *)ptr == XFS_TRANS_HEADER_MAGIC) {
		skip = xlog_print_trans_header(&ptr,
					be32_to_cpu(op_head->oh_len));
	    } else {
		switch (*(unsigned short *)ptr) {
		    case XFS_LI_BUF: {
			skip = xlog_print_trans_buffer(&ptr,
					be32_to_cpu(op_head->oh_len),
					&i, num_ops);
			break;
		    }
		    case XFS_LI_ICREATE: {
			skip = xlog_print_trans_icreate(&ptr,
					be32_to_cpu(op_head->oh_len),
					&i, num_ops);
			break;
		    }
		    case XFS_LI_INODE: {
			skip = xlog_print_trans_inode(log, &ptr,
					be32_to_cpu(op_head->oh_len),
					&i, num_ops, continued);
			break;
		    }
		    case XFS_LI_DQUOT: {
			skip = xlog_print_trans_dquot(&ptr,
					be32_to_cpu(op_head->oh_len),
					&i, num_ops);
			break;
		    }
		    case XFS_LI_EFI: {
			skip = xlog_print_trans_efi(&ptr,
					be32_to_cpu(op_head->oh_len),
					continued);
			break;
		    }
		    case XFS_LI_EFD: {
			skip = xlog_print_trans_efd(&ptr,
					be32_to_cpu(op_head->oh_len));
			break;
		    }
		    case XFS_LI_RUI: {
			skip = xlog_print_trans_rui(&ptr,
					be32_to_cpu(op_head->oh_len),
					continued);
			break;
		    }
		    case XFS_LI_RUD: {
			skip = xlog_print_trans_rud(&ptr,
					be32_to_cpu(op_head->oh_len));
			break;
		    }
		    case XFS_LI_CUI: {
			skip = xlog_print_trans_cui(&ptr,
					be32_to_cpu(op_head->oh_len),
					continued);
			break;
		    }
		    case XFS_LI_CUD: {
			skip = xlog_print_trans_cud(&ptr,
					be32_to_cpu(op_head->oh_len));
			break;
		    }
		    case XFS_LI_BUI: {
			skip = xlog_print_trans_bui(&ptr,
					be32_to_cpu(op_head->oh_len),
					continued);
			break;
		    }
		    case XFS_LI_BUD: {
			skip = xlog_print_trans_bud(&ptr,
					be32_to_cpu(op_head->oh_len));
			break;
		    }
		    case XFS_LI_QUOTAOFF: {
			skip = xlog_print_trans_qoff(&ptr,
					be32_to_cpu(op_head->oh_len));
			break;
		    }
		    case XLOG_UNMOUNT_TYPE: {
			printf(_("Unmount filesystem\n"));
			skip = 0;
			break;
		    }
		    default: {
			if (bad_hdr_warn && !lost_context) {
				fprintf(stderr,
			_("%s: unknown log operation type (%x)\n"),
					progname, *(unsigned short *)ptr);
				if (print_exit) {
					free(buf);
					return BAD_HEADER;
				}
			} else {
				printf(
			_("Left over region from split log item\n"));
			}
			skip = 0;
			ptr += be32_to_cpu(op_head->oh_len);
			lost_context = 0;
		    }
		} /* switch */
	    } /* else */
	    if (skip != 0)
		xlog_print_add_to_trans(be32_to_cpu(op_head->oh_tid), skip);
	}
    }
    printf("\n");
    free(buf);
    return NO_ERROR;
}	/* xlog_print_record */


int
xlog_print_rec_head(xlog_rec_header_t *head, int *len, int bad_hdr_warn)
{
    int i;
    char uub[64];
    int datalen,bbs;

    if (print_no_print)
	    return be32_to_cpu(head->h_num_logops);

    if (!head->h_magicno)
	return ZEROED_LOG;

    if (be32_to_cpu(head->h_magicno) != XLOG_HEADER_MAGIC_NUM) {
	if (bad_hdr_warn)
		printf(_("Header 0x%x wanted 0x%x\n"),
			be32_to_cpu(head->h_magicno),
			XLOG_HEADER_MAGIC_NUM);
	return BAD_HEADER;
    }

    /* check for cleared blocks written by xlog_clear_stale_blocks() */
    if (!head->h_len && !head->h_crc && !head->h_prev_block &&
	!head->h_num_logops && !head->h_size)
	return CLEARED_BLKS;

    datalen=be32_to_cpu(head->h_len);
    bbs=BTOBB(datalen);

    printf(_("cycle: %d	version: %d	"),
	    be32_to_cpu(head->h_cycle),
	    be32_to_cpu(head->h_version));
    print_lsn("	lsn", &head->h_lsn);
    print_lsn("	tail_lsn", &head->h_tail_lsn);
    printf("\n");
    printf(_("length of Log Record: %d	prev offset: %d		num ops: %d\n"),
	   datalen,
	    be32_to_cpu(head->h_prev_block),
	    be32_to_cpu(head->h_num_logops));

    if (print_overwrite) {
	printf(_("cycle num overwrites: "));
	for (i=0; i< MIN(bbs, XLOG_HEADER_CYCLE_SIZE / BBSIZE); i++)
	    printf("%d - 0x%x  ",
		    i,
		    be32_to_cpu(head->h_cycle_data[i]));
	printf("\n");
    }

    platform_uuid_unparse(&head->h_fs_uuid, uub);
    printf(_("uuid: %s   format: "), uub);
    switch (be32_to_cpu(head->h_fmt)) {
	case XLOG_FMT_UNKNOWN:
	    printf(_("unknown\n"));
	    break;
	case XLOG_FMT_LINUX_LE:
	    printf(_("little endian linux\n"));
	    break;
	case XLOG_FMT_LINUX_BE:
	    printf(_("big endian linux\n"));
	    break;
	case XLOG_FMT_IRIX_BE:
	    printf(_("big endian irix\n"));
	    break;
	default:
	    printf("? (%d)\n", be32_to_cpu(head->h_fmt));
	    break;
    }
    printf(_("h_size: %d\n"), be32_to_cpu(head->h_size));

    *len = be32_to_cpu(head->h_len);
    return(be32_to_cpu(head->h_num_logops));
}	/* xlog_print_rec_head */

void
xlog_print_rec_xhead(xlog_rec_ext_header_t *head, int coverage)
{
    int i;

    print_xlog_xhdr_line();
    printf(_("extended-header: cycle: %d\n"), be32_to_cpu(head->xh_cycle));

    if (print_overwrite) {
	printf(_("cycle num overwrites: "));
	for (i = 0; i < coverage; i++)
	    printf("%d - 0x%x  ",
		    i,
		    be32_to_cpu(head->xh_cycle_data[i]));
	printf("\n");
    }
}	/* xlog_print_rec_xhead */

static void
print_xlog_bad_zeroed(xfs_daddr_t blkno)
{
	print_stars();
	printf(_("* ERROR: found data after zeroed blocks block=%-21lld  *\n"),
		(long long)blkno);
	print_stars();
	if (print_exit)
	    xlog_exit("Bad log - data after zeroed blocks");
}	/* print_xlog_bad_zeroed */

static void
print_xlog_bad_header(xfs_daddr_t blkno, char *buf)
{
	print_stars();
	printf(_("* ERROR: header cycle=%-11d block=%-21lld        *\n"),
		xlog_get_cycle(buf), (long long)blkno);
	print_stars();
	if (print_exit)
	    xlog_exit("Bad log record header");
}	/* print_xlog_bad_header */

void
print_xlog_bad_data(xfs_daddr_t blkno)
{
	print_stars();
	printf(_("* ERROR: data block=%-21lld                             *\n"),
		(long long)blkno);
	print_stars();
	if (print_exit)
	    xlog_exit("Bad data in log");
}	/* print_xlog_bad_data */

static void
print_xlog_bad_reqd_hdrs(xfs_daddr_t blkno, int num_reqd, int num_hdrs)
{
	print_stars();
	printf(_("* ERROR: for header block=%lld\n"
	       "*        not enough hdrs for data length, "
		"required num = %d, hdr num = %d\n"),
		(long long)blkno, num_reqd, num_hdrs);
	print_stars();
	if (print_exit)
	    xlog_exit(_("Not enough headers for data length."));
}	/* print_xlog_bad_reqd_hdrs */

static void
xlog_reallocate_xhdrs(int num_hdrs, xlog_rec_ext_header_t **ret_xhdrs)
{
	int len = (num_hdrs-1) * sizeof(xlog_rec_ext_header_t);

	*ret_xhdrs = (xlog_rec_ext_header_t *)realloc(*ret_xhdrs, len);
	if (*ret_xhdrs == NULL) {
		fprintf(stderr, _("%s: xlog_print: malloc failed for ext hdrs\n"), progname);
		exit(1);
	}
}

/* for V2 logs read each extra hdr and print it out */
static int
xlog_print_extended_headers(
	int			fd,
	int			len,
	xfs_daddr_t		*blkno,
	xlog_rec_header_t	*hdr,
	int 			*ret_num_hdrs,
	xlog_rec_ext_header_t	**ret_xhdrs)
{
	int			i, j;
	int			coverage_bb;
	int 			num_hdrs;
	int 			num_required;
	char			xhbuf[XLOG_HEADER_SIZE];
	xlog_rec_ext_header_t	*x;

	num_required = howmany(len, XLOG_HEADER_CYCLE_SIZE);
	num_hdrs = be32_to_cpu(hdr->h_size) / XLOG_HEADER_CYCLE_SIZE;
	if (be32_to_cpu(hdr->h_size) % XLOG_HEADER_CYCLE_SIZE)
		num_hdrs++;

	if (num_required > num_hdrs) {
	    print_xlog_bad_reqd_hdrs((*blkno)-1, num_required, num_hdrs);
	}

	if (num_hdrs == 1) {
	    free(*ret_xhdrs);
	    *ret_xhdrs = NULL;
	    *ret_num_hdrs = 1;
	    return 0;
	}

	if (*ret_xhdrs == NULL || num_hdrs > *ret_num_hdrs) {
	    xlog_reallocate_xhdrs(num_hdrs, ret_xhdrs);
	}

	*ret_num_hdrs = num_hdrs;

	/* don't include 1st header */
	for (i = 1, x = *ret_xhdrs; i < num_hdrs; i++, (*blkno)++, x++) {
	    /* read one extra header blk */
	    if (read(fd, xhbuf, 512) == 0) {
		printf(_("%s: physical end of log\n"), progname);
		print_xlog_record_line();
		/* reached the end so return 1 */
		return 1;
	    }
	    if (print_only_data) {
		printf(_("BLKNO: %lld\n"), (long long)*blkno);
		xlog_recover_print_data(xhbuf, 512);
	    }
	    else {
		if (i == num_hdrs - 1) {
		    /* last header */
		    coverage_bb = BTOBB(len) %
				    (XLOG_HEADER_CYCLE_SIZE / BBSIZE);
		}
		else {
		    /* earliear header */
		    coverage_bb = XLOG_HEADER_CYCLE_SIZE / BBSIZE;
		}
		xlog_print_rec_xhead((xlog_rec_ext_header_t*)xhbuf, coverage_bb);
	    }

	    /* Copy from buffer into xhdrs array for later.
	     * Could endian convert here but then code later on
	     * will look asymmetric with the 1 hdr normal case
	     * which does endian coversion on access.
	     */
	    x->xh_cycle = ((xlog_rec_ext_header_t*)xhbuf)->xh_cycle;
	    for (j = 0; j < XLOG_HEADER_CYCLE_SIZE / BBSIZE; j++) {
		x->xh_cycle_data[j] =
		    ((xlog_rec_ext_header_t*)xhbuf)->xh_cycle_data[j];
	    }
	}
	return 0;
}


/*
 * This code is gross and needs to be rewritten.
 */
void xfs_log_print(struct xlog  *log,
		   int          fd,
		   int		print_block_start)
{
    char			hbuf[XLOG_HEADER_SIZE];
    xlog_rec_header_t		*hdr = (xlog_rec_header_t *)&hbuf[0];
    xlog_rec_ext_header_t 	*xhdrs = NULL;
    int				num_ops, len, num_hdrs = 1;
    xfs_daddr_t			block_end = 0, block_start, blkno, error;
    xfs_daddr_t			zeroed_blkno = 0, cleared_blkno = 0;
    int				read_type = FULL_READ;
    char			*partial_buf;
    int				zeroed = 0;
    int				cleared = 0;
    int				first_hdr_found = 0;

    logBBsize = log->l_logBBsize;

    /*
     * Normally, block_start and block_end are the same value since we
     * are printing the entire log.  However, if the start block is given,
     * we still end at the end of the logical log.
     */
    if ((error = xlog_print_find_oldest(log, &block_end))) {
	fprintf(stderr, _("%s: problem finding oldest LR\n"), progname);
	return;
    }
    if (print_block_start == -1)
	block_start = block_end;
    else
	block_start = print_block_start;
    xlog_print_lseek(log, fd, block_start, SEEK_SET);
    blkno = block_start;

    for (;;) {
	if (read(fd, hbuf, 512) == 0) {
	    printf(_("%s: physical end of log\n"), progname);
	    print_xlog_record_line();
	    break;
	}
	if (print_only_data) {
	    printf(_("BLKNO: %lld\n"), (long long)blkno);
	    xlog_recover_print_data(hbuf, 512);
	    blkno++;
	    goto loop;
	}
	num_ops = xlog_print_rec_head(hdr, &len, first_hdr_found);
	blkno++;

	if (zeroed && num_ops != ZEROED_LOG) {
	    printf(_("%s: after %d zeroed blocks\n"), progname, zeroed);
	    /* once we find zeroed blocks - that's all we expect */
	    print_xlog_bad_zeroed(blkno-1);
	    /* reset count since we're assuming previous zeroed blocks
	     * were bad
	     */
	    zeroed = 0;
	}

	if (num_ops == ZEROED_LOG ||
	    num_ops == CLEARED_BLKS ||
	    num_ops == BAD_HEADER) {
	    if (num_ops == ZEROED_LOG) {
		if (zeroed == 0)
		    zeroed_blkno = blkno-1;
		zeroed++;
	    }
	    else if (num_ops == CLEARED_BLKS) {
		if (cleared == 0)
		    cleared_blkno = blkno-1;
		cleared++;
	    } else {
		if (!first_hdr_found)
			block_start = blkno;
		else
			print_xlog_bad_header(blkno-1, hbuf);
	    }

	    goto loop;
	}

	if (be32_to_cpu(hdr->h_version) == 2) {
	    if (xlog_print_extended_headers(fd, len, &blkno, hdr, &num_hdrs, &xhdrs) != 0)
		break;
	}

	error =	xlog_print_record(log, fd, num_ops, len, &read_type, &partial_buf,
				  hdr, xhdrs, first_hdr_found);
	first_hdr_found++;
	switch (error) {
	    case 0: {
		blkno += BTOBB(len);
		if (print_block_start != -1 &&
		    blkno >= block_end)		/* If start specified, we */
			goto end;		/* end early */
		break;
	    }
	    case -1: {
		print_xlog_bad_data(blkno-1);
		if (print_block_start != -1 &&
		    blkno >= block_end)		/* If start specified, */
			goto end;		/* we end early */
		xlog_print_lseek(log, fd, blkno, SEEK_SET);
		goto loop;
	    }
	    case PARTIAL_READ: {
		print_xlog_record_line();
		printf(_("%s: physical end of log\n"), progname);
		print_xlog_record_line();
		blkno = 0;
		xlog_print_lseek(log, fd, 0, SEEK_SET);
		/*
		 * We may have hit the end of the log when we started at 0.
		 * In this case, just end.
		 */
		if (block_start == 0)
			goto end;
		goto partial_log_read;
	    }
	    default: xlog_panic(_("illegal value"));
	}
	print_xlog_record_line();
loop:
	if (blkno >= logBBsize) {
	    if (cleared) {
		printf(_("%s: skipped %d cleared blocks in range: %lld - %lld\n"),
			progname, cleared,
			(long long)(cleared_blkno),
			(long long)(cleared + cleared_blkno - 1));
		if (cleared == logBBsize)
		    printf(_("%s: totally cleared log\n"), progname);

		cleared=0;
	    }
	    if (zeroed) {
		printf(_("%s: skipped %d zeroed blocks in range: %lld - %lld\n"),
			progname, zeroed,
			(long long)(zeroed_blkno),
			(long long)(zeroed + zeroed_blkno - 1));
		if (zeroed == logBBsize)
		    printf(_("%s: totally zeroed log\n"), progname);

		zeroed=0;
	    }
	    printf(_("%s: physical end of log\n"), progname);
	    print_xlog_record_line();
	    break;
	}
    }

    /* Do we need to print the first part of physical log? */
    if (block_start != 0) {
	blkno = 0;
	xlog_print_lseek(log, fd, 0, SEEK_SET);
	for (;;) {
	    if (read(fd, hbuf, 512) == 0) {
		xlog_panic(_("xlog_find_head: bad read"));
	    }
	    if (print_only_data) {
		printf(_("BLKNO: %lld\n"), (long long)blkno);
		xlog_recover_print_data(hbuf, 512);
		blkno++;
		goto loop2;
	    }
	    num_ops = xlog_print_rec_head(hdr, &len, first_hdr_found);
	    blkno++;

	    if (num_ops == ZEROED_LOG ||
		num_ops == CLEARED_BLKS ||
		num_ops == BAD_HEADER) {
		/* we only expect zeroed log entries  or cleared log
		 * entries at the end of the _physical_ log,
		 * so treat them the same as bad blocks here
		 */
		print_xlog_bad_header(blkno-1, hbuf);

		if (blkno >= block_end)
		    break;
		continue;
	    }

	    if (be32_to_cpu(hdr->h_version) == 2) {
		if (xlog_print_extended_headers(fd, len, &blkno, hdr, &num_hdrs, &xhdrs) != 0)
		    break;
	    }

partial_log_read:
	    error= xlog_print_record(log, fd, num_ops, len, &read_type,
				    &partial_buf, (xlog_rec_header_t *)hbuf,
				    xhdrs, first_hdr_found);
	    if (read_type != FULL_READ)
		len -= read_type;
	    read_type = FULL_READ;
	    if (!error)
		blkno += BTOBB(len);
	    else {
		print_xlog_bad_data(blkno-1);
		xlog_print_lseek(log, fd, blkno, SEEK_SET);
		goto loop2;
	    }
	    print_xlog_record_line();
loop2:
	    if (blkno >= block_end)
		break;
	}
    }

end:
    printf(_("%s: logical end of log\n"), progname);
    print_xlog_record_line();
}

/*
 * if necessary, convert an xfs_inode_log_format struct from the old 32bit version
 * (which can have different field alignments) to the native 64 bit version
 */
xfs_inode_log_format_t *
xfs_inode_item_format_convert(char *src_buf, uint len, xfs_inode_log_format_t *in_f)
{
	struct xfs_inode_log_format_32	*in_f32;

	/* if we have native format then just return buf without copying data */
	if (len == sizeof(xfs_inode_log_format_t)) {
		return (xfs_inode_log_format_t *)src_buf;
	}

	in_f32 = (struct xfs_inode_log_format_32 *)src_buf;
	in_f->ilf_type = in_f32->ilf_type;
	in_f->ilf_size = in_f32->ilf_size;
	in_f->ilf_fields = in_f32->ilf_fields;
	in_f->ilf_asize = in_f32->ilf_asize;
	in_f->ilf_dsize = in_f32->ilf_dsize;
	in_f->ilf_ino = in_f32->ilf_ino;
	/* copy biggest field of ilf_u */
	memcpy(&in_f->ilf_u.ilfu_uuid, &in_f32->ilf_u.ilfu_uuid, sizeof(uuid_t));
	in_f->ilf_blkno = in_f32->ilf_blkno;
	in_f->ilf_len = in_f32->ilf_len;
	in_f->ilf_boffset = in_f32->ilf_boffset;

	return in_f;
}
