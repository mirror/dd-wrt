// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "btblock.h"
#include "bmroot.h"
#include "bit.h"
#include "agf.h"
#include "agfl.h"
#include "agi.h"
#include "sb.h"
#include "attr.h"
#include "attrshort.h"
#include "dquot.h"
#include "dir2.h"
#include "dir2sf.h"
#include "symlink.h"

const ftattr_t	ftattrtab[] = {
	{ FLDT_AEXTNUM, "aextnum", fp_num, "%d", SI(bitsz(xfs_aextnum_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_AGBLOCK, "agblock", fp_num, "%u", SI(bitsz(xfs_agblock_t)),
	  FTARG_DONULL, fa_agblock, NULL },
	{ FLDT_AGBLOCKNZ, "agblocknz", fp_num, "%u", SI(bitsz(xfs_agblock_t)),
	  FTARG_SKIPZERO|FTARG_DONULL, fa_agblock, NULL },
	{ FLDT_AGF, "agf", NULL, (char *)agf_flds, agf_size, FTARG_SIZE, NULL,
	  agf_flds },
	{ FLDT_AGFL, "agfl", NULL, (char *)agfl_flds, agfl_size, FTARG_SIZE,
	  NULL, agfl_flds },
	{ FLDT_AGFL_CRC, "agfl", NULL, (char *)agfl_crc_flds, agfl_size,
	  FTARG_SIZE, NULL, agfl_crc_flds },
	{ FLDT_AGI, "agi", NULL, (char *)agi_flds, agi_size, FTARG_SIZE, NULL,
	  agi_flds },
	{ FLDT_AGINO, "agino", fp_num, "%u", SI(bitsz(xfs_agino_t)),
	  FTARG_DONULL, fa_agino, NULL },
	{ FLDT_AGINONN, "aginonn", fp_num, "%u", SI(bitsz(xfs_agino_t)),
	  FTARG_SKIPNULL, fa_agino, NULL },
	{ FLDT_AGNUMBER, "agnumber", fp_num, "%u", SI(bitsz(xfs_agnumber_t)),
	  FTARG_DONULL, NULL, NULL },

/* attr fields */
	{ FLDT_ATTR, "attr", NULL, (char *)attr_flds, attr_size, FTARG_SIZE,
	  NULL, attr_flds },
	{ FLDT_ATTR_BLKINFO, "attr_blkinfo", NULL, (char *)attr_blkinfo_flds,
	  SI(bitsz(struct xfs_da_blkinfo)), 0, NULL, attr_blkinfo_flds },
	{ FLDT_ATTR_LEAF_ENTRY, "attr_leaf_entry", fp_sarray,
	  (char *)attr_leaf_entry_flds, SI(bitsz(struct xfs_attr_leaf_entry)),
	  0, NULL, attr_leaf_entry_flds },
	{ FLDT_ATTR_LEAF_HDR, "attr_leaf_hdr", NULL, (char *)attr_leaf_hdr_flds,
	  SI(bitsz(struct xfs_attr_leaf_hdr)), 0, NULL, attr_leaf_hdr_flds },
	{ FLDT_ATTR_LEAF_MAP, "attr_leaf_map", fp_sarray,
	  (char *)attr_leaf_map_flds, SI(bitsz(struct xfs_attr_leaf_map)), 0,
	  NULL, attr_leaf_map_flds },
	{ FLDT_ATTR_LEAF_NAME, "attr_leaf_name", NULL,
	  (char *)attr_leaf_name_flds, attr_leaf_name_size, FTARG_SIZE, NULL,
	  attr_leaf_name_flds },
	{ FLDT_ATTR_NODE_ENTRY, "attr_node_entry", fp_sarray,
	  (char *)attr_node_entry_flds, SI(bitsz(struct xfs_da_node_entry)), 0,
	  NULL, attr_node_entry_flds },
	{ FLDT_ATTR_NODE_HDR, "attr_node_hdr", NULL, (char *)attr_node_hdr_flds,
	  SI(bitsz(struct xfs_da_node_hdr)), 0, NULL, attr_node_hdr_flds },
	{ FLDT_ATTR_SF_ENTRY, "attr_sf_entry", NULL, (char *)attr_sf_entry_flds,
	  attr_sf_entry_size, FTARG_SIZE, NULL, attr_sf_entry_flds },
	{ FLDT_ATTR_SF_HDR, "attr_sf_hdr", NULL, (char *)attr_sf_hdr_flds,
	  SI(bitsz(struct xfs_attr_sf_hdr)), 0, NULL, attr_sf_hdr_flds },
	{ FLDT_ATTRBLOCK, "attrblock", fp_num, "%u", SI(bitsz(uint32_t)), 0,
	  fa_attrblock, NULL },
	{ FLDT_ATTRSHORT, "attrshort", NULL, (char *)attr_shortform_flds,
	  attrshort_size, FTARG_SIZE, NULL, attr_shortform_flds },

/* attr3 specific fields */
	{ FLDT_ATTR3, "attr3", NULL, (char *)attr3_flds, attr_size, FTARG_SIZE,
	  NULL, attr3_flds },
	{ FLDT_ATTR3_BLKINFO, "attr3_blkinfo", NULL, (char *)attr3_blkinfo_flds,
	  SI(bitsz(struct xfs_da3_blkinfo)), 0, NULL, attr3_blkinfo_flds },
	{ FLDT_ATTR3_LEAF_HDR, "attr3_leaf_hdr", NULL,
	  (char *)attr3_leaf_hdr_flds, SI(bitsz(struct xfs_attr3_leaf_hdr)),
	  0, NULL, attr3_leaf_hdr_flds },
	{ FLDT_ATTR3_NODE_HDR, "attr3_node_hdr", NULL,
	  (char *)attr3_node_hdr_flds, SI(bitsz(struct xfs_da3_node_hdr)),
	  0, NULL, attr3_node_hdr_flds },
	{ FLDT_ATTR3_REMOTE_HDR, "attr3_remote_hdr", NULL,
	  (char *)attr3_remote_crc_flds, attr_size, FTARG_SIZE, NULL,
	  attr3_remote_crc_flds },

	{ FLDT_BMAPBTA, "bmapbta", NULL, (char *)bmapbta_flds, btblock_size,
	  FTARG_SIZE, NULL, bmapbta_flds },
	{ FLDT_BMAPBTA_CRC, "bmapbta", NULL, (char *)bmapbta_crc_flds,
	  btblock_size, FTARG_SIZE, NULL, bmapbta_crc_flds },
	{ FLDT_BMAPBTAKEY, "bmapbtakey", fp_sarray, (char *)bmapbta_key_flds,
	  SI(bitsz(xfs_bmbt_key_t)), 0, NULL, bmapbta_key_flds },
	{ FLDT_BMAPBTAPTR, "bmapbtaptr", fp_num, "%llu",
	  SI(bitsz(xfs_bmbt_ptr_t)), 0, fa_dfsbno, NULL },
	{ FLDT_BMAPBTAREC, "bmapbtarec", fp_sarray, (char *)bmapbta_rec_flds,
	  SI(bitsz(xfs_bmbt_rec_t)), 0, NULL, bmapbta_rec_flds },
	{ FLDT_BMAPBTD, "bmapbtd", NULL, (char *)bmapbtd_flds, btblock_size,
	  FTARG_SIZE, NULL, bmapbtd_flds },
	{ FLDT_BMAPBTD_CRC, "bmapbtd", NULL, (char *)bmapbtd_crc_flds,
	  btblock_size, FTARG_SIZE, NULL, bmapbtd_crc_flds },
	{ FLDT_BMAPBTDKEY, "bmapbtdkey", fp_sarray, (char *)bmapbtd_key_flds,
	  SI(bitsz(xfs_bmbt_key_t)), 0, NULL, bmapbtd_key_flds },
	{ FLDT_BMAPBTDPTR, "bmapbtdptr", fp_num, "%llu",
	  SI(bitsz(xfs_bmbt_ptr_t)), 0, fa_dfsbno, NULL },
	{ FLDT_BMAPBTDREC, "bmapbtdrec", fp_sarray, (char *)bmapbtd_rec_flds,
	  SI(bitsz(xfs_bmbt_rec_t)), 0, NULL, bmapbtd_rec_flds },
	{ FLDT_BMROOTA, "bmroota", NULL, (char *)bmroota_flds, bmroota_size,
	  FTARG_SIZE, NULL, bmroota_flds },
	{ FLDT_BMROOTAKEY, "bmrootakey", fp_sarray, (char *)bmroota_key_flds,
	  SI(bitsz(xfs_bmdr_key_t)), 0, NULL, bmroota_key_flds },
	{ FLDT_BMROOTAPTR, "bmrootaptr", fp_num, "%llu",
	  SI(bitsz(xfs_bmdr_ptr_t)), 0, fa_dfsbno, NULL },
	{ FLDT_BMROOTD, "bmrootd", NULL, (char *)bmrootd_flds, bmrootd_size,
	  FTARG_SIZE, NULL, bmrootd_flds },
	{ FLDT_BMROOTDKEY, "bmrootdkey", fp_sarray, (char *)bmrootd_key_flds,
	  SI(bitsz(xfs_bmdr_key_t)), 0, NULL, bmrootd_key_flds },
	{ FLDT_BMROOTDPTR, "bmrootdptr", fp_num, "%llu",
	  SI(bitsz(xfs_bmdr_ptr_t)), 0, fa_dfsbno, NULL },
	{ FLDT_BNOBT, "bnobt", NULL, (char *)bnobt_flds, btblock_size, FTARG_SIZE,
	  NULL, bnobt_flds },
	{ FLDT_BNOBT_CRC, "bnobt", NULL, (char *)bnobt_crc_flds, btblock_size,
	  FTARG_SIZE, NULL, bnobt_crc_flds },
	{ FLDT_BNOBTKEY, "bnobtkey", fp_sarray, (char *)bnobt_key_flds,
	  SI(bitsz(xfs_alloc_key_t)), 0, NULL, bnobt_key_flds },
	{ FLDT_BNOBTPTR, "bnobtptr", fp_num, "%u", SI(bitsz(xfs_alloc_ptr_t)),
	  0, fa_agblock, NULL },
	{ FLDT_BNOBTREC, "bnobtrec", fp_sarray, (char *)bnobt_rec_flds,
	  SI(bitsz(xfs_alloc_rec_t)), 0, NULL, bnobt_rec_flds },
	{ FLDT_CEXTFLG, "cextflag", fp_num, "%u", SI(BMBT_EXNTFLAG_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_CEXTLEN, "cextlen", fp_num, "%u", SI(BMBT_BLOCKCOUNT_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_CFILEOFFA, "cfileoffa", fp_num, "%llu", SI(BMBT_STARTOFF_BITLEN),
	  0, fa_cfileoffa, NULL },
	{ FLDT_CFILEOFFD, "cfileoffd", fp_num, "%llu", SI(BMBT_STARTOFF_BITLEN),
	  0, fa_cfileoffd, NULL },
	{ FLDT_CFSBLOCK, "cfsblock", fp_num, "%llu", SI(BMBT_STARTBLOCK_BITLEN),
	  0, fa_cfsblock, NULL },
	{ FLDT_CHARNS, "charns", fp_charns, NULL, SI(bitsz(char)), 0, NULL,
	  NULL },
	{ FLDT_CHARS, "chars", fp_num, "%c", SI(bitsz(char)), 0, NULL, NULL },
	{ FLDT_REXTLEN, "rextlen", fp_num, "%u", SI(RMAPBT_BLOCKCOUNT_BITLEN),
	  0, NULL, NULL },
	{ FLDT_RFILEOFFD, "rfileoffd", fp_num, "%llu", SI(RMAPBT_OFFSET_BITLEN),
	  0, NULL, NULL },
	{ FLDT_REXTFLG, "rextflag", fp_num, "%u", SI(RMAPBT_EXNTFLAG_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_RATTRFORKFLG, "rattrforkflag", fp_num, "%u", SI(RMAPBT_ATTRFLAG_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_RBMBTFLG, "rbmbtflag", fp_num, "%u", SI(RMAPBT_BMBTFLAG_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_CAGBLOCK, "cagblock", fp_num, "%u", SI(REFCNTBT_AGBLOCK_BITLEN),
	  FTARG_DONULL, fa_agblock, NULL },
	{ FLDT_CCOWFLG, "ccowflag", fp_num, "%u", SI(REFCNTBT_COWFLAG_BITLEN), 0,
	  NULL, NULL },
	{ FLDT_CNTBT, "cntbt", NULL, (char *)cntbt_flds, btblock_size, FTARG_SIZE,
	  NULL, cntbt_flds },
	{ FLDT_CNTBT_CRC, "cntbt", NULL, (char *)cntbt_crc_flds, btblock_size,
	  FTARG_SIZE, NULL, cntbt_crc_flds },
	{ FLDT_CNTBTKEY, "cntbtkey", fp_sarray, (char *)cntbt_key_flds,
	  SI(bitsz(xfs_alloc_key_t)), 0, NULL, cntbt_key_flds },
	{ FLDT_CNTBTPTR, "cntbtptr", fp_num, "%u", SI(bitsz(xfs_alloc_ptr_t)),
	  0, fa_agblock, NULL },
	{ FLDT_CNTBTREC, "cntbtrec", fp_sarray, (char *)cntbt_rec_flds,
	  SI(bitsz(xfs_alloc_rec_t)), 0, NULL, cntbt_rec_flds },

	{ FLDT_RMAPBT_CRC, "rmapbt", NULL, (char *)rmapbt_crc_flds, btblock_size,
	  FTARG_SIZE, NULL, rmapbt_crc_flds },
	{ FLDT_RMAPBTKEY, "rmapbtkey", fp_sarray, (char *)rmapbt_key_flds,
	  SI(bitize(2 * sizeof(struct xfs_rmap_key))), 0, NULL, rmapbt_key_flds },
	{ FLDT_RMAPBTPTR, "rmapbtptr", fp_num, "%u",
	  SI(bitsz(xfs_rmap_ptr_t)), 0, fa_agblock, NULL },
	{ FLDT_RMAPBTREC, "rmapbtrec", fp_sarray, (char *)rmapbt_rec_flds,
	  SI(bitsz(struct xfs_rmap_rec)), 0, NULL, rmapbt_rec_flds },

	{ FLDT_REFCBT_CRC, "refcntbt", NULL, (char *)refcbt_crc_flds, btblock_size,
	  FTARG_SIZE, NULL, refcbt_crc_flds },
	{ FLDT_REFCBTKEY, "refcntbtkey", fp_sarray, (char *)refcbt_key_flds,
	  SI(bitsz(struct xfs_refcount_key)), 0, NULL, refcbt_key_flds },
	{ FLDT_REFCBTPTR, "refcntbtptr", fp_num, "%u", SI(bitsz(xfs_refcount_ptr_t)),
	  0, fa_agblock, NULL },
	{ FLDT_REFCBTREC, "refcntbtrec", fp_sarray, (char *)refcbt_rec_flds,
	  SI(bitsz(struct xfs_refcount_rec)), 0, NULL, refcbt_rec_flds },

/* CRC field */
	{ FLDT_CRC, "crc", fp_crc, "%#x (%s)", SI(bitsz(uint32_t)),
	  0, NULL, NULL },

	{ FLDT_DEV, "dev", fp_num, "%#x", SI(bitsz(xfs_dev_t)), 0, NULL, NULL },
	{ FLDT_DFILOFFA, "dfiloffa", fp_num, "%llu", SI(bitsz(xfs_fileoff_t)),
	  0, fa_dfiloffa, NULL },
	{ FLDT_DFILOFFD, "dfiloffd", fp_num, "%llu", SI(bitsz(xfs_fileoff_t)),
	  0, fa_dfiloffd, NULL },
	{ FLDT_DFSBNO, "dfsbno", fp_num, "%llu", SI(bitsz(xfs_fsblock_t)),
	  FTARG_DONULL, fa_dfsbno, NULL },
	{ FLDT_DINODE_A, "dinode_a", NULL, (char *)inode_a_flds, inode_a_size,
	  FTARG_SIZE|FTARG_OKEMPTY, NULL, inode_a_flds },
	{ FLDT_DINODE_CORE, "dinode_core", NULL, (char *)inode_core_flds,
	  SI(bitsz(xfs_dinode_t)), 0, NULL, inode_core_flds },
	{ FLDT_DINODE_FMT, "dinode_fmt", fp_dinode_fmt, NULL,
	  SI(bitsz(int8_t)), 0, NULL, NULL },
	{ FLDT_DINODE_U, "dinode_u", NULL, (char *)inode_u_flds, inode_u_size,
	  FTARG_SIZE|FTARG_OKEMPTY, NULL, inode_u_flds },
	{ FLDT_DINODE_V3, "dinode_v3", NULL, (char *)inode_v3_flds,
	  SI(bitsz(xfs_dinode_t)), 0, NULL, inode_v3_flds },

/* dir v2 fields */
	{ FLDT_DIR2, "dir2", NULL, (char *)dir2_flds, dir2_size, FTARG_SIZE,
	  NULL, dir2_flds },
	{ FLDT_DIR2_BLOCK_TAIL, "dir2_block_tail", NULL,
	  (char *)dir2_block_tail_flds, SI(bitsz(xfs_dir2_block_tail_t)), 0,
	  NULL, dir2_block_tail_flds },
	{ FLDT_DIR2_DATA_FREE, "dir2_data_free", NULL,
	  (char *)dir2_data_free_flds, SI(bitsz(xfs_dir2_data_free_t)), 0, NULL,
	  dir2_data_free_flds },
	{ FLDT_DIR2_DATA_HDR, "dir2_data_hdr", NULL, (char *)dir2_data_hdr_flds,
	  SI(bitsz(xfs_dir2_data_hdr_t)), 0, NULL, dir2_data_hdr_flds },
	{ FLDT_DIR2_DATA_OFF, "dir2_data_off", fp_num, "%#x",
	  SI(bitsz(xfs_dir2_data_off_t)), 0, NULL, NULL },
	{ FLDT_DIR2_DATA_OFFNZ, "dir2_data_offnz", fp_num, "%#x",
	  SI(bitsz(xfs_dir2_data_off_t)), FTARG_SKIPZERO, NULL, NULL },
	{ FLDT_DIR2_DATA_UNION, "dir2_data_union", NULL,
	  (char *)dir2_data_union_flds, dir2_data_union_size, FTARG_SIZE, NULL,
	  dir2_data_union_flds },
	{ FLDT_DIR2_FREE_HDR, "dir2_free_hdr", NULL, (char *)dir2_free_hdr_flds,
	  SI(bitsz(xfs_dir2_free_hdr_t)), 0, NULL, dir2_free_hdr_flds },
	{ FLDT_DIR2_INO4, "dir2_ino4", fp_num, "%u",
	  SI(bitize(XFS_INO32_SIZE)), 0, fa_ino4, NULL },
	{ FLDT_DIR2_INO8, "dir2_ino8", fp_num, "%llu",
	  SI(bitize(XFS_INO64_SIZE)), 0, fa_ino8, NULL },
	{ FLDT_DIR2_INOU, "dir2_inou", NULL, (char *)dir2_inou_flds,
	  dir2_inou_size, FTARG_SIZE, NULL, dir2_inou_flds },
	{ FLDT_DIR2_LEAF_ENTRY, "dir2_leaf_entry", NULL,
	  (char *)dir2_leaf_entry_flds, SI(bitsz(xfs_dir2_leaf_entry_t)), 0,
	  NULL, dir2_leaf_entry_flds },
	{ FLDT_DIR2_LEAF_HDR, "dir2_leaf_hdr", NULL, (char *)dir2_leaf_hdr_flds,
	  SI(bitsz(xfs_dir2_leaf_hdr_t)), 0, NULL, dir2_leaf_hdr_flds },
	{ FLDT_DIR2_LEAF_TAIL, "dir2_leaf_tail", NULL,
	  (char *)dir2_leaf_tail_flds, SI(bitsz(xfs_dir2_leaf_tail_t)), 0, NULL,
	  dir2_leaf_tail_flds },
	{ FLDT_DIR2_SF_ENTRY, "dir2_sf_entry", NULL, (char *)dir2_sf_entry_flds,
	  dir2_sf_entry_size, FTARG_SIZE, NULL, dir2_sf_entry_flds },
	{ FLDT_DIR2_SF_HDR, "dir2_sf_hdr", NULL, (char *)dir2_sf_hdr_flds,
	  dir2_sf_hdr_size, FTARG_SIZE, NULL, dir2_sf_hdr_flds },
	{ FLDT_DIR2_SF_OFF, "dir2_sf_off", fp_num, "%#x",
	  SI(bitize(2*sizeof(__u8))), 0, NULL, NULL },
	{ FLDT_DIR2SF, "dir2sf", NULL, (char *)dir2sf_flds, dir2sf_size,
	  FTARG_SIZE, NULL, dir2sf_flds },

/* dir v3 fields */
	{ FLDT_DIR3, "dir3", NULL, (char *)dir3_flds, dir2_size, FTARG_SIZE,
	  NULL, dir3_flds },
	{ FLDT_DIR3_BLKHDR, "dir3_blk_hdr", NULL, (char *)dir3_blkhdr_flds,
	  SI(bitsz(struct xfs_dir3_blk_hdr)), 0, NULL, dir3_blkhdr_flds },
	{ FLDT_DIR3_DATA_HDR, "dir3_data_hdr", NULL, (char *)dir3_data_hdr_flds,
	  SI(bitsz(struct xfs_dir3_data_hdr)), 0, NULL, dir3_data_hdr_flds },
	{ FLDT_DIR3_FREE_HDR, "dir3_free_hdr", NULL, (char *)dir3_free_hdr_flds,
	  SI(bitsz(struct xfs_dir3_free_hdr)), 0, NULL, dir3_free_hdr_flds },
	{ FLDT_DIR3_LEAF_HDR, "dir3_leaf_hdr", NULL, (char *)dir3_leaf_hdr_flds,
	  SI(bitsz(struct xfs_dir3_leaf_hdr)), 0, NULL, dir3_leaf_hdr_flds },
	{ FLDT_DIR3_DATA_UNION, "dir3_data_union", NULL,
	  (char *)dir3_data_union_flds, dir2_data_union_size, FTARG_SIZE, NULL,
	  dir3_data_union_flds },
	{ FLDT_DIR3_SF_ENTRY, "dir3_sf_entry", NULL, (char *)dir3_sf_entry_flds,
	  dir2_sf_entry_size, FTARG_SIZE, NULL, dir3_sf_entry_flds },
	{ FLDT_DIR3SF, "dir3sf", NULL, (char *)dir3sf_flds, dir2sf_size,
	  FTARG_SIZE, NULL, dir3sf_flds },

/* dir v2/3 node fields */
	{ FLDT_DA_BLKINFO, "dir_blkinfo", NULL, (char *)da_blkinfo_flds,
	  SI(bitsz(struct xfs_da_blkinfo)), 0, NULL, da_blkinfo_flds },
	{ FLDT_DA_NODE_ENTRY, "dir_node_entry", fp_sarray,
	  (char *)da_node_entry_flds, SI(bitsz(struct xfs_da_node_entry)), 0,
	  NULL, da_node_entry_flds },
	{ FLDT_DA_NODE_HDR, "dir_node_hdr", NULL, (char *)da_node_hdr_flds,
	  SI(bitsz(struct xfs_da_node_hdr)), 0, NULL, da_node_hdr_flds },
	{ FLDT_DA3_BLKINFO, "dir_blkinfo", NULL, (char *)da3_blkinfo_flds,
	  SI(bitsz(struct xfs_da3_blkinfo)), 0, NULL, da3_blkinfo_flds },
	{ FLDT_DA3_NODE_HDR, "dir_node_hdr", NULL, (char *)da3_node_hdr_flds,
	  SI(bitsz(struct xfs_da3_node_hdr)), 0, NULL, da3_node_hdr_flds },

	{ FLDT_DIRBLOCK, "dirblock", fp_num, "%u", SI(bitsz(uint32_t)), 0,
	  fa_dirblock, NULL },
	{ FLDT_DISK_DQUOT, "disk_dquot", NULL, (char *)disk_dquot_flds,
	  SI(bitsz(struct xfs_disk_dquot)), 0, NULL, disk_dquot_flds },
	{ FLDT_DQBLK, "dqblk", NULL, (char *)dqblk_flds, SI(bitsz(xfs_dqblk_t)),
	  0, NULL, dqblk_flds },
	{ FLDT_DQID, "dqid", fp_num, "%d", SI(bitsz(xfs_dqid_t)), 0, NULL,
	  NULL },
	{ FLDT_DRFSBNO, "drfsbno", fp_num, "%llu", SI(bitsz(xfs_rfsblock_t)),
	  FTARG_DONULL, fa_drfsbno, NULL },
	{ FLDT_DRTBNO, "drtbno", fp_num, "%llu", SI(bitsz(xfs_rtblock_t)),
	  FTARG_DONULL, fa_drtbno, NULL },
	{ FLDT_EXTLEN, "extlen", fp_num, "%u", SI(bitsz(xfs_extlen_t)), 0, NULL,
	  NULL },
	{ FLDT_EXTNUM, "extnum", fp_num, "%d", SI(bitsz(xfs_extnum_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_FSIZE, "fsize", fp_num, "%lld", SI(bitsz(xfs_fsize_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_INO, "ino", fp_num, "%llu", SI(bitsz(xfs_ino_t)), FTARG_DONULL,
	  fa_ino, NULL },
	{ FLDT_INOBT, "inobt",  NULL, (char *)inobt_flds, btblock_size,
	  FTARG_SIZE, NULL, inobt_flds },
	{ FLDT_INOBT_CRC, "inobt",  NULL, (char *)inobt_crc_flds, btblock_size,
	  FTARG_SIZE, NULL, inobt_crc_flds },
	{ FLDT_INOBT_SPCRC, "inobt",  NULL, (char *)inobt_spcrc_flds,
	  btblock_size, FTARG_SIZE, NULL, inobt_spcrc_flds },
	{ FLDT_INOBTKEY, "inobtkey", fp_sarray, (char *)inobt_key_flds,
	  SI(bitsz(xfs_inobt_key_t)), 0, NULL, inobt_key_flds },
	{ FLDT_INOBTPTR, "inobtptr", fp_num, "%u", SI(bitsz(xfs_inobt_ptr_t)),
	  0, fa_agblock, NULL },
	{ FLDT_INOBTREC, "inobtrec", fp_sarray, (char *)inobt_rec_flds,
	  SI(bitsz(xfs_inobt_rec_t)), 0, NULL, inobt_rec_flds },
	{ FLDT_INOBTSPREC, "inobtsprec", fp_sarray, (char *) inobt_sprec_flds,
	  SI(bitsz(xfs_inobt_rec_t)), 0, NULL, inobt_sprec_flds },
	{ FLDT_INODE, "inode", NULL, (char *)inode_flds, inode_size, FTARG_SIZE,
	  NULL, inode_flds },
	{ FLDT_INODE_CRC, "inode", NULL, (char *)inode_crc_flds, inode_size,
	  FTARG_SIZE, NULL, inode_crc_flds },
	{ FLDT_INOFREE, "inofree", fp_num, "%#llx", SI(bitsz(xfs_inofree_t)), 0,
	  NULL, NULL },
	{ FLDT_INT16D, "int16d", fp_num, "%d", SI(bitsz(int16_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_INT32D, "int32d", fp_num, "%d", SI(bitsz(int32_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_INT64D, "int64d", fp_num, "%lld", SI(bitsz(int64_t)),
	  FTARG_SIGNED, NULL, NULL },
	{ FLDT_INT8D, "int8d", fp_num, "%d", SI(bitsz(int8_t)), FTARG_SIGNED,
	  NULL, NULL },
	{ FLDT_NSEC, "nsec", fp_num, "%09d", SI(bitsz(int32_t)), FTARG_SIGNED,
	  NULL, NULL },
	{ FLDT_QCNT, "qcnt", fp_num, "%llu", SI(bitsz(xfs_qcnt_t)), 0, NULL,
	  NULL },
	{ FLDT_QWARNCNT, "qwarncnt", fp_num, "%u", SI(bitsz(xfs_qwarncnt_t)), 0,
	  NULL, NULL },
	{ FLDT_SB, "sb", NULL, (char *)sb_flds, sb_size, FTARG_SIZE, NULL,
	  sb_flds },

/* CRC enabled symlink */
	{ FLDT_SYMLINK_CRC, "symlink", NULL, (char *)symlink_crc_flds,
	  symlink_size, FTARG_SIZE, NULL, symlink_crc_flds },

	{ FLDT_TIME, "time", fp_time, NULL, SI(bitsz(int32_t)), FTARG_SIGNED,
	  NULL, NULL },
	{ FLDT_TIMESTAMP, "timestamp", NULL, (char *)timestamp_flds,
	  SI(bitsz(xfs_timestamp_t)), 0, NULL, timestamp_flds },
	{ FLDT_UINT1, "uint1", fp_num, "%u", SI(1), 0, NULL, NULL },
	{ FLDT_UINT16D, "uint16d", fp_num, "%u", SI(bitsz(uint16_t)), 0, NULL,
	  NULL },
	{ FLDT_UINT16O, "uint16o", fp_num, "%#o", SI(bitsz(uint16_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT16X, "uint16x", fp_num, "%#x", SI(bitsz(uint16_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT32D, "uint32d", fp_num, "%u", SI(bitsz(uint32_t)), 0, NULL,
	  NULL },
	{ FLDT_UINT32O, "uint32o", fp_num, "%#o", SI(bitsz(uint32_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT32X, "uint32x", fp_num, "%#x", SI(bitsz(uint32_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT64D, "uint64d", fp_num, "%llu", SI(bitsz(uint64_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT64O, "uint64o", fp_num, "%#llo", SI(bitsz(uint64_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT64X, "uint64x", fp_num, "%#llx", SI(bitsz(uint64_t)), 0,
	  NULL, NULL },
	{ FLDT_UINT8D, "uint8d", fp_num, "%u", SI(bitsz(uint8_t)), 0, NULL,
	  NULL },
	{ FLDT_UINT8O, "uint8o", fp_num, "%#o", SI(bitsz(uint8_t)), 0, NULL,
	  NULL },
	{ FLDT_UINT8X, "uint8x", fp_num, "%#x", SI(bitsz(uint8_t)), 0, NULL,
	  NULL },
	{ FLDT_UUID, "uuid", fp_uuid, NULL, SI(bitsz(uuid_t)), 0, NULL, NULL },
	{ FLDT_ZZZ, NULL }
};

int
bitoffset(
	const field_t	*f,
	void		*obj,
	int		startoff,
	int		idx)
{

	if (!(f->flags & FLD_OFFSET)) {
		if (f->flags & FLD_ARRAY) {
			int		abase;
#ifdef DEBUG
			const ftattr_t	*fa = &ftattrtab[f->ftyp];
#endif

			abase = (f->flags & FLD_ABASE1) != 0;
			ASSERT(fa->ftyp == f->ftyp);
			ASSERT((fa->arg & FTARG_SIZE) == 0);
			return (int)(intptr_t)f->offset +
				(idx - abase) * fsize(f, obj, startoff, idx);
		} else
			return (int)(intptr_t)f->offset;
	} else
		return (*f->offset)(obj, startoff, idx);
}

int
fcount(
	const field_t	*f,
	void		*obj,
	int		startoff)
{
	if (!(f->flags & FLD_COUNT))
		return (int)(intptr_t)f->count;
	else
		return (*f->count)(obj, startoff);
}

const field_t *
findfield(
	char		*name,
	const field_t	*fields,
	void            *obj,
	int             startoff)
{
	const field_t	*f;

	/* we only match if this field name matches and has a non-zero count */
	for (f = fields; f->name; f++)
		if (strcmp(f->name, name) == 0 && fcount(f, obj, startoff))
			return f;
	return NULL;
}

int
fsize(
	const field_t	*f,
	void		*obj,
	int		startoff,
	int		idx)
{
	const ftattr_t	*fa;

	fa = &ftattrtab[f->ftyp];
	ASSERT(fa->ftyp == f->ftyp);
	if (!(fa->arg & FTARG_SIZE))
		return (int)(intptr_t)fa->size;
	else
		return (*fa->size)(obj, startoff, idx);
}
