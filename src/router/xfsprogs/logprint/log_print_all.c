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

#include "logprint.h"

/*
 * Start is defined to be the block pointing to the oldest valid log record.
 */
int
xlog_print_find_oldest(
	struct log	*log,
	xfs_daddr_t	*last_blk)
{
	xfs_buf_t	*bp;
	xfs_daddr_t	first_blk;
	uint		first_half_cycle, last_half_cycle;
	int		error;

	if (xlog_find_zeroed(log, &first_blk))
		return 0;

	first_blk = 0;		/* read first block */
	bp = xlog_get_bp(log, 1);
	xlog_bread_noalign(log, 0, 1, bp);
	first_half_cycle = xlog_get_cycle(XFS_BUF_PTR(bp));
	*last_blk = log->l_logBBsize-1;	/* read last block */
	xlog_bread_noalign(log, *last_blk, 1, bp);
	last_half_cycle = xlog_get_cycle(XFS_BUF_PTR(bp));
	ASSERT(last_half_cycle != 0);

	if (first_half_cycle == last_half_cycle) { /* all cycle nos are same */
		*last_blk = 0;
	} else {		/* have 1st and last; look for middle cycle */
		error = xlog_find_cycle_start(log, bp, first_blk,
					      last_blk, last_half_cycle);
		if (error)
			return error;
	}

	xlog_put_bp(bp);
	return 0;
}

void
xlog_recover_print_data(
	xfs_caddr_t	p,
	int		len)
{
	if (print_data) {
		uint *dp  = (uint *)p;
		int  nums = len >> 2;
		int  j = 0;

		while (j < nums) {
			if ((j % 8) == 0)
				printf("%2x ", j);
			printf("%8x ", *dp);
			dp++;
			j++;
			if ((j % 8) == 0)
				printf("\n");
		}
		printf("\n");
	}
}

STATIC void
xlog_recover_print_buffer(
	xlog_recover_item_t	*item)
{
	xfs_agi_t		*agi;
	xfs_agf_t		*agf;
	xfs_buf_log_format_t	*f;
	xfs_caddr_t		p;
	int			len, num, i;
	xfs_daddr_t		blkno;
	xfs_disk_dquot_t	*ddq;

	f = (xfs_buf_log_format_t *)item->ri_buf[0].i_addr;
	len = item->ri_buf[0].i_len;
	printf("	");
	ASSERT(f->blf_type == XFS_LI_BUF);
	printf(_("BUF:  #regs:%d   start blkno:0x%llx   len:%d   bmap size:%d   flags:0x%x\n"),
		f->blf_size, (long long)f->blf_blkno, f->blf_len, f->blf_map_size, f->blf_flags);
	blkno = (xfs_daddr_t)f->blf_blkno;
	num = f->blf_size-1;
	i = 1;
	while (num-- > 0) {
		p = item->ri_buf[i].i_addr;
		len = item->ri_buf[i].i_len;
		i++;
		if (blkno == 0) { /* super block */
			printf(_("	SUPER Block Buffer:\n"));
			if (!print_buffer) 
				continue;
		       printf(_("              icount:%llu ifree:%llu  "),
			       (unsigned long long)
				       be64_to_cpu(*(__be64 *)(p)),
			       (unsigned long long)
				       be64_to_cpu(*(__be64 *)(p+8)));
		       printf(_("fdblks:%llu  frext:%llu\n"),
			       (unsigned long long)
				       be64_to_cpu(*(__be64 *)(p+16)),
			       (unsigned long long)
				       be64_to_cpu(*(__be64 *)(p+24)));
			printf(_("		sunit:%u  swidth:%u\n"),
			       be32_to_cpu(*(__be32 *)(p+56)),
			       be32_to_cpu(*(__be32 *)(p+60)));
		} else if (be32_to_cpu(*(__be32 *)p) == XFS_AGI_MAGIC) {
			agi = (xfs_agi_t *)p;
			printf(_("	AGI Buffer: (XAGI)\n"));
			if (!print_buffer) 
				continue;
			printf(_("		ver:%d  "),
				be32_to_cpu(agi->agi_versionnum));
			printf(_("seq#:%d  len:%d  cnt:%d  root:%d\n"),
				be32_to_cpu(agi->agi_seqno),
				be32_to_cpu(agi->agi_length),
				be32_to_cpu(agi->agi_count),
				be32_to_cpu(agi->agi_root));
			printf(_("		level:%d  free#:0x%x  newino:0x%x\n"),
				be32_to_cpu(agi->agi_level),
				be32_to_cpu(agi->agi_freecount),
				be32_to_cpu(agi->agi_newino));
		} else if (be32_to_cpu(*(__be32 *)p) == XFS_AGF_MAGIC) {
			agf = (xfs_agf_t *)p;
			printf(_("	AGF Buffer: (XAGF)\n"));
			if (!print_buffer) 
				continue;
			printf(_("		ver:%d  seq#:%d  len:%d  \n"),
				be32_to_cpu(agf->agf_versionnum),
				be32_to_cpu(agf->agf_seqno),
				be32_to_cpu(agf->agf_length));
			printf(_("		root BNO:%d  CNT:%d\n"),
				be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]),
				be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));
			printf(_("		level BNO:%d  CNT:%d\n"),
				be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNOi]),
				be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNTi]));
			printf(_("		1st:%d  last:%d  cnt:%d  "
				"freeblks:%d  longest:%d\n"),
				be32_to_cpu(agf->agf_flfirst),
				be32_to_cpu(agf->agf_fllast),
				be32_to_cpu(agf->agf_flcount),
				be32_to_cpu(agf->agf_freeblks),
				be32_to_cpu(agf->agf_longest));
		} else if (*(uint *)p == XFS_DQUOT_MAGIC) {
			ddq = (xfs_disk_dquot_t *)p;
			printf(_("	DQUOT Buffer:\n"));
			if (!print_buffer) 
				continue;
			printf(_("		UIDs 0x%lx-0x%lx\n"),
			       (unsigned long)be32_to_cpu(ddq->d_id),
			       (unsigned long)be32_to_cpu(ddq->d_id) +
			       (BBTOB(f->blf_len) / sizeof(xfs_dqblk_t)) - 1);
		} else {
			printf(_("	BUF DATA\n"));
			if (!print_buffer) continue;
			xlog_recover_print_data(p, len);
		}
	}
}

STATIC void
xlog_recover_print_quotaoff(
	xlog_recover_item_t	*item)
{
	xfs_qoff_logformat_t	*qoff_f;
	char			str[32] = { 0 };

	qoff_f = (xfs_qoff_logformat_t *)item->ri_buf[0].i_addr;
	ASSERT(qoff_f);
	if (qoff_f->qf_flags & XFS_UQUOTA_ACCT)
		strcat(str, "USER QUOTA");
	if (qoff_f->qf_flags & XFS_GQUOTA_ACCT)
		strcat(str, "GROUP QUOTA");
	if (qoff_f->qf_flags & XFS_PQUOTA_ACCT)
		strcat(str, "PROJECT QUOTA");
	printf(_("\tQUOTAOFF: #regs:%d   type:%s\n"),
	       qoff_f->qf_size, str);
}

STATIC void
xlog_recover_print_dquot(
	xlog_recover_item_t	*item)
{
	xfs_dq_logformat_t	*f;
	xfs_disk_dquot_t	*d;

	f = (xfs_dq_logformat_t *)item->ri_buf[0].i_addr;
	ASSERT(f);
	ASSERT(f->qlf_len == 1);
	d = (xfs_disk_dquot_t *)item->ri_buf[1].i_addr;
	printf(_("\tDQUOT: #regs:%d  blkno:%lld  boffset:%u id: %d\n"),
	       f->qlf_size, (long long)f->qlf_blkno, f->qlf_boffset, f->qlf_id);
	if (!print_quota)
		return;
	printf(_("\t\tmagic 0x%x\tversion 0x%x\tID 0x%x (%d)\t\n"),
	       be16_to_cpu(d->d_magic),
	       d->d_version,
	       be32_to_cpu(d->d_id),
	       be32_to_cpu(d->d_id));
	printf(_("\t\tblk_hard 0x%x\tblk_soft 0x%x\tino_hard 0x%x"
	       "\tino_soft 0x%x\n"),
	       (int)be64_to_cpu(d->d_blk_hardlimit),
	       (int)be64_to_cpu(d->d_blk_softlimit),
	       (int)be64_to_cpu(d->d_ino_hardlimit),
	       (int)be64_to_cpu(d->d_ino_softlimit));
	printf(_("\t\tbcount 0x%x (%d) icount 0x%x (%d)\n"),
	       (int)be64_to_cpu(d->d_bcount),
	       (int)be64_to_cpu(d->d_bcount),
	       (int)be64_to_cpu(d->d_icount),
	       (int)be64_to_cpu(d->d_icount));
	printf(_("\t\tbtimer 0x%x itimer 0x%x \n"),
	       (int)be32_to_cpu(d->d_btimer),
	       (int)be32_to_cpu(d->d_itimer));
}

STATIC void
xlog_recover_print_inode_core(
	xfs_icdinode_t		*di)
{
	printf(_("	CORE inode:\n"));
	if (!print_inode)
		return;
	printf(_("		magic:%c%c  mode:0x%x  ver:%d  format:%d  "
	     "onlink:%d\n"),
	       (di->di_magic>>8) & 0xff, di->di_magic & 0xff,
	       di->di_mode, di->di_version, di->di_format, di->di_onlink);
	printf(_("		uid:%d  gid:%d  nlink:%d projid:%u\n"),
	       di->di_uid, di->di_gid, di->di_nlink, xfs_get_projid(*di));
	printf(_("		atime:%d  mtime:%d  ctime:%d\n"),
	       di->di_atime.t_sec, di->di_mtime.t_sec, di->di_ctime.t_sec);
	printf(_("		flushiter:%d\n"), di->di_flushiter);
	printf(_("		size:0x%llx  nblks:0x%llx  exsize:%d  "
	     "nextents:%d  anextents:%d\n"), (unsigned long long)
	       di->di_size, (unsigned long long)di->di_nblocks,
	       di->di_extsize, di->di_nextents, (int)di->di_anextents);
	printf(_("		forkoff:%d  dmevmask:0x%x  dmstate:%d  flags:0x%x  "
	     "gen:%d\n"),
	       (int)di->di_forkoff, di->di_dmevmask, (int)di->di_dmstate,
	       (int)di->di_flags, di->di_gen);
}

STATIC void
xlog_recover_print_inode(
	xlog_recover_item_t	*item)
{
	xfs_inode_log_format_t	f_buf;
	xfs_inode_log_format_t	*f;
	int			attr_index;
	int			hasdata;
	int			hasattr;

	ASSERT(item->ri_buf[0].i_len == sizeof(xfs_inode_log_format_32_t) ||
	       item->ri_buf[0].i_len == sizeof(xfs_inode_log_format_64_t));
	f = xfs_inode_item_format_convert(item->ri_buf[0].i_addr, item->ri_buf[0].i_len, &f_buf);

	printf(_("	INODE: #regs:%d   ino:0x%llx  flags:0x%x   dsize:%d\n"),
	       f->ilf_size, (unsigned long long)f->ilf_ino, f->ilf_fields,
	       f->ilf_dsize);

	/* core inode comes 2nd */
	ASSERT(item->ri_buf[1].i_len == sizeof(xfs_icdinode_t));
	xlog_recover_print_inode_core((xfs_icdinode_t *)
				      item->ri_buf[1].i_addr);

	hasdata = (f->ilf_fields & XFS_ILOG_DFORK) != 0;
	hasattr = (f->ilf_fields & XFS_ILOG_AFORK) != 0;
	/* does anything come next */
	switch (f->ilf_fields & (XFS_ILOG_DFORK|XFS_ILOG_DEV|XFS_ILOG_UUID)) {
	case XFS_ILOG_DEXT:
		ASSERT(f->ilf_size == 3 + hasattr);
		printf(_("		DATA FORK EXTENTS inode data:\n"));
		if (print_inode && print_data)
			xlog_recover_print_data(item->ri_buf[2].i_addr,
						item->ri_buf[2].i_len);
		break;
	case XFS_ILOG_DBROOT:
		ASSERT(f->ilf_size == 3 + hasattr);
		printf(_("		DATA FORK BTREE inode data:\n"));
		if (print_inode && print_data)
			xlog_recover_print_data(item->ri_buf[2].i_addr,
						item->ri_buf[2].i_len);
		break;
	case XFS_ILOG_DDATA:
		ASSERT(f->ilf_size == 3 + hasattr);
		printf(_("		DATA FORK LOCAL inode data:\n"));
		if (print_inode && print_data)
			xlog_recover_print_data(item->ri_buf[2].i_addr,
						item->ri_buf[2].i_len);
		break;
	case XFS_ILOG_DEV:
		ASSERT(f->ilf_size == 2 + hasattr);
		printf(_("		DEV inode: no extra region\n"));
		break;
	case XFS_ILOG_UUID:
		ASSERT(f->ilf_size == 2 + hasattr);
		printf(_("		UUID inode: no extra region\n"));
		break;

	case 0:
		ASSERT(f->ilf_size == 2 + hasattr);
		break;
	default:
		xlog_panic("xlog_print_trans_inode: illegal inode type");
	}

	if (hasattr) {
		attr_index = 2 + hasdata;
		switch (f->ilf_fields & XFS_ILOG_AFORK) {
		case XFS_ILOG_AEXT:
			ASSERT(f->ilf_size == 3 + hasdata);
			printf(_("		ATTR FORK EXTENTS inode data:\n"));
			if (print_inode && print_data)
				xlog_recover_print_data(
					item->ri_buf[attr_index].i_addr,
					item->ri_buf[attr_index].i_len);
			break;
		case XFS_ILOG_ABROOT:
			ASSERT(f->ilf_size == 3 + hasdata);
			printf(_("		ATTR FORK BTREE inode data:\n"));
			if (print_inode && print_data)
				xlog_recover_print_data(
					item->ri_buf[attr_index].i_addr,
					item->ri_buf[attr_index].i_len);
			break;
		case XFS_ILOG_ADATA:
			ASSERT(f->ilf_size == 3 + hasdata);
			printf(_("		ATTR FORK LOCAL inode data:\n"));
			if (print_inode && print_data)
				xlog_recover_print_data(
					item->ri_buf[attr_index].i_addr,
					item->ri_buf[attr_index].i_len);
			break;
		default:
			xlog_panic("%s: illegal inode log flag", __FUNCTION__);
		}
	}
}

STATIC void
xlog_recover_print_efd(
	xlog_recover_item_t	*item)
{
	xfs_efd_log_format_t	*f;

	f = (xfs_efd_log_format_t *)item->ri_buf[0].i_addr;
	/*
	 * An xfs_efd_log_format structure contains a variable length array
	 * as the last field.
	 * Each element is of size xfs_extent_32_t or xfs_extent_64_t.
	 * However, the extents are never used and won't be printed.
	 */
	printf(_("	EFD:  #regs: %d    num_extents: %d  id: 0x%llx\n"),
	       f->efd_size, f->efd_nextents, (unsigned long long)f->efd_efi_id);
}


STATIC void
xlog_recover_print_efi(
	xlog_recover_item_t	*item)
{
	xfs_efi_log_format_t	*f, *src_f;
	xfs_extent_t		*ex;
	int			i;
	uint			src_len, dst_len;

	src_f = (xfs_efi_log_format_t *)item->ri_buf[0].i_addr;
	src_len = item->ri_buf[0].i_len;
	/*
	 * An xfs_efi_log_format structure contains a variable length array
	 * as the last field.
	 * Each element is of size xfs_extent_32_t or xfs_extent_64_t.
	 * Need to convert to native format.
	 */
	dst_len = sizeof(xfs_efi_log_format_t) + (src_f->efi_nextents - 1) * sizeof(xfs_extent_t);
	if ((f = (xfs_efi_log_format_t *)malloc(dst_len)) == NULL) {
	    fprintf(stderr, _("%s: xlog_recover_print_efi: malloc failed\n"), progname);
	    exit(1);
	}
	if (xfs_efi_copy_format((char*)src_f, src_len, f)) {
	    free(f);
	    return;
	}

	printf(_("	EFI:  #regs:%d    num_extents:%d  id:0x%llx\n"),
	       f->efi_size, f->efi_nextents, (unsigned long long)f->efi_id);
	ex = f->efi_extents;
	printf("	");
	for (i=0; i< f->efi_nextents; i++) {
		printf("(s: 0x%llx, l: %d) ",
			(unsigned long long)ex->ext_start, ex->ext_len);
		if (i % 4 == 3)
			printf("\n");
		ex++;
	}
	if (i % 4 != 0)
		printf("\n");
	free(f);
}

void
xlog_recover_print_logitem(
	xlog_recover_item_t	*item)
{
	switch (ITEM_TYPE(item)) {
	case XFS_LI_BUF:
		xlog_recover_print_buffer(item);
		break;
	case XFS_LI_INODE:
		xlog_recover_print_inode(item);
		break;
	case XFS_LI_EFD:
		xlog_recover_print_efd(item);
		break;
	case XFS_LI_EFI:
		xlog_recover_print_efi(item);
		break;
	case XFS_LI_DQUOT:
		xlog_recover_print_dquot(item);
		break;
	case XFS_LI_QUOTAOFF:
		xlog_recover_print_quotaoff(item);
		break;
	default:
		printf(_("xlog_recover_print_logitem: illegal type\n"));
		break;
	}
}

void
xlog_recover_print_item(
	xlog_recover_item_t	*item)
{
	int			i;

	switch (ITEM_TYPE(item)) {
	case XFS_LI_BUF:
		printf("BUF");
		break;
	case XFS_LI_INODE:
		printf("INO");
		break;
	case XFS_LI_EFD:
		printf("EFD");
		break;
	case XFS_LI_EFI:
		printf("EFI");
		break;
	case XFS_LI_DQUOT:
		printf("DQ ");
		break;
	case XFS_LI_QUOTAOFF:
		printf("QOFF");
		break;
	default:
		cmn_err(CE_PANIC, _("%s: illegal type"), __FUNCTION__);
		break;
	}

/*	type isn't filled in yet
	printf(_("ITEM: type: %d cnt: %d total: %d "),
	       item->ri_type, item->ri_cnt, item->ri_total);
*/
	printf(_(": cnt:%d total:%d "), item->ri_cnt, item->ri_total);
	for (i=0; i<item->ri_cnt; i++) {
		printf(_("a:0x%lx len:%d "),
		       (long)item->ri_buf[i].i_addr, item->ri_buf[i].i_len);
	}
	printf("\n");
	xlog_recover_print_logitem(item);
}

void
xlog_recover_print_trans(
	xlog_recover_t		*trans,
	struct list_head	*itemq,
	int			print)
{
	xlog_recover_item_t	*item;

	if (print < 3)
		return;

	print_xlog_record_line();
	xlog_recover_print_trans_head(trans);
	list_for_each_entry(item, itemq, ri_list)
		xlog_recover_print_item(item);
}
