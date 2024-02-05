// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

typedef enum fldt	{
	FLDT_AGBLOCK,
	FLDT_AGBLOCKNZ,
	FLDT_AGF,
	FLDT_AGFL,
	FLDT_AGFL_CRC,
	FLDT_AGI,
	FLDT_AGINO,
	FLDT_AGINONN,
	FLDT_AGNUMBER,

	/* attr fields */
	FLDT_ATTR,
	FLDT_ATTR_BLKINFO,
	FLDT_ATTR_LEAF_ENTRY,
	FLDT_ATTR_LEAF_HDR,
	FLDT_ATTR_LEAF_MAP,
	FLDT_ATTR_LEAF_NAME,
	FLDT_ATTR_NODE_ENTRY,
	FLDT_ATTR_NODE_HDR,
	FLDT_ATTR_SF_ENTRY,
	FLDT_ATTR_SF_HDR,
	FLDT_ATTRBLOCK,
	FLDT_ATTRSHORT,

	/* attr 3 specific fields */
	FLDT_ATTR3,
	FLDT_ATTR3_BLKINFO,
	FLDT_ATTR3_LEAF_HDR,
	FLDT_ATTR3_NODE_HDR,
	FLDT_ATTR3_REMOTE_HDR,

	FLDT_BMAPBTA,
	FLDT_BMAPBTA_CRC,
	FLDT_BMAPBTAKEY,
	FLDT_BMAPBTAPTR,
	FLDT_BMAPBTAREC,
	FLDT_BMAPBTD,
	FLDT_BMAPBTD_CRC,
	FLDT_BMAPBTDKEY,
	FLDT_BMAPBTDPTR,
	FLDT_BMAPBTDREC,
	FLDT_BMROOTA,
	FLDT_BMROOTAKEY,
	FLDT_BMROOTAPTR,
	FLDT_BMROOTD,
	FLDT_BMROOTDKEY,
	FLDT_BMROOTDPTR,
	FLDT_BNOBT,
	FLDT_BNOBT_CRC,
	FLDT_BNOBTKEY,
	FLDT_BNOBTPTR,
	FLDT_BNOBTREC,
	FLDT_CEXTFLG,
	FLDT_CEXTLEN,
	FLDT_CFILEOFFA,
	FLDT_CFILEOFFD,
	FLDT_CFSBLOCK,
	FLDT_CHARNS,
	FLDT_CHARS,
	FLDT_REXTLEN,
	FLDT_RFILEOFFD,
	FLDT_REXTFLG,
	FLDT_RATTRFORKFLG,
	FLDT_RBMBTFLG,
	FLDT_CAGBLOCK,
	FLDT_CCOWFLG,
	FLDT_CNTBT,
	FLDT_CNTBT_CRC,
	FLDT_CNTBTKEY,
	FLDT_CNTBTPTR,
	FLDT_CNTBTREC,
	FLDT_RMAPBT_CRC,
	FLDT_RMAPBTKEY,
	FLDT_RMAPBTPTR,
	FLDT_RMAPBTREC,
	FLDT_REFCBT_CRC,
	FLDT_REFCBTKEY,
	FLDT_REFCBTPTR,
	FLDT_REFCBTREC,

	/* CRC field type */
	FLDT_CRC,

	FLDT_DEV,
	FLDT_DFILOFFA,
	FLDT_DFILOFFD,
	FLDT_DFSBNO,
	FLDT_DINODE_A,
	FLDT_DINODE_CORE,
	FLDT_DINODE_FMT,
	FLDT_DINODE_U,
	FLDT_DINODE_V3,

	/* dir v2 fields */
	FLDT_DIR2,
	FLDT_DIR2_BLOCK_TAIL,
	FLDT_DIR2_DATA_FREE,
	FLDT_DIR2_DATA_HDR,
	FLDT_DIR2_DATA_OFF,
	FLDT_DIR2_DATA_OFFNZ,
	FLDT_DIR2_DATA_UNION,
	FLDT_DIR2_FREE_HDR,
	FLDT_DIR2_INO4,
	FLDT_DIR2_INO8,
	FLDT_DIR2_INOU,
	FLDT_DIR2_LEAF_ENTRY,
	FLDT_DIR2_LEAF_HDR,
	FLDT_DIR2_LEAF_TAIL,
	FLDT_DIR2_SF_ENTRY,
	FLDT_DIR2_SF_HDR,
	FLDT_DIR2_SF_OFF,
	FLDT_DIR2SF,

	/* dir v3 fields */
	FLDT_DIR3,
	FLDT_DIR3_BLKHDR,
	FLDT_DIR3_DATA_HDR,
	FLDT_DIR3_FREE_HDR,
	FLDT_DIR3_LEAF_HDR,
	FLDT_DIR3_DATA_UNION,
	FLDT_DIR3_SF_ENTRY,
	FLDT_DIR3SF,

	/* dir v2/3 node fields */
	FLDT_DA_BLKINFO,
	FLDT_DA_NODE_ENTRY,
	FLDT_DA_NODE_HDR,
	FLDT_DA3_BLKINFO,
	FLDT_DA3_NODE_HDR,

	FLDT_DIRBLOCK,
	FLDT_DISK_DQUOT,
	FLDT_DQBLK,
	FLDT_DQID,
	FLDT_DRFSBNO,
	FLDT_DRTBNO,
	FLDT_EXTLEN,
	FLDT_FSIZE,
	FLDT_INO,
	FLDT_INOBT,
	FLDT_INOBT_CRC,
	FLDT_INOBT_SPCRC,
	FLDT_FINOBT,
	FLDT_FINOBT_CRC,
	FLDT_FINOBT_SPCRC,
	FLDT_INOBTKEY,
	FLDT_INOBTPTR,
	FLDT_FINOBTPTR,
	FLDT_INOBTREC,
	FLDT_INOBTSPREC,
	FLDT_INODE,
	FLDT_INODE_CRC,
	FLDT_INOFREE,
	FLDT_INT16D,
	FLDT_INT32D,
	FLDT_INT64D,
	FLDT_INT8D,
	FLDT_NSEC,
	FLDT_QCNT,
	FLDT_QWARNCNT,
	FLDT_SB,

	/* CRC enabled symlink */
	FLDT_SYMLINK_CRC,

	FLDT_TIME,
	FLDT_TIMESTAMP,
	FLDT_QTIMER,
	FLDT_UINT1,
	FLDT_UINT16D,
	FLDT_UINT16O,
	FLDT_UINT16X,
	FLDT_UINT32D,
	FLDT_UINT32O,
	FLDT_UINT32X,
	FLDT_UINT64D,
	FLDT_UINT64O,
	FLDT_UINT64X,
	FLDT_UINT8D,
	FLDT_UINT8O,
	FLDT_UINT8X,
	FLDT_UUID,
	FLDT_ZZZ			/* mark last entry */
} fldt_t;

typedef int (*offset_fnc_t)(void *obj, int startoff, int idx);
#define	OI(o)	((offset_fnc_t)(intptr_t)(o))

typedef int (*count_fnc_t)(void *obj, int startoff);
#define	CI(c)	((count_fnc_t)(intptr_t)(c))
#define	C1	CI(1)

typedef struct field
{
	char		*name;
	fldt_t		ftyp;
	offset_fnc_t	offset;
	count_fnc_t	count;
	int		flags;
	typnm_t		next;
} field_t;

/*
 * flag values
 */
#define	FLD_ABASE1	1	/* field array base is 1 not 0 */
#define	FLD_SKIPALL	2	/* skip this field in an all-fields print */
#define	FLD_ARRAY	4	/* this field is an array */
#define	FLD_OFFSET	8	/* offset value is a function pointer */
#define	FLD_COUNT	16	/* count value is a function pointer */

typedef int (*size_fnc_t)(void *obj, int startoff, int idx);
#define	SI(s)	((size_fnc_t)(intptr_t)(s))

typedef struct ftattr
{
	fldt_t		ftyp;
	char		*name;
	prfnc_t		prfunc;
	char		*fmtstr;
	size_fnc_t	size;
	int		arg;
	adfnc_t		adfunc;
	const field_t	*subfld;
} ftattr_t;
extern const ftattr_t	ftattrtab[];

/*
 * arg values
 */
#define	FTARG_SKIPZERO	1	/* skip 0 words */
#define	FTARG_DONULL	2	/* make -1 words be "null" */
#define	FTARG_SKIPNULL	4	/* skip -1 words */
#define	FTARG_SIGNED	8	/* field value is signed */
#define	FTARG_SIZE	16	/* size field is a function */
#define	FTARG_SKIPNMS	32	/* skip printing names this time */
#define	FTARG_OKEMPTY	64	/* ok if this (union type) is empty */

extern int		bitoffset(const field_t *f, void *obj, int startoff,
				  int idx);
extern int		fcount(const field_t *f, void *obj, int startoff);
extern const field_t	*findfield(char *name, const field_t *fields,
				  void *obj, int startoff);
extern int		fsize(const field_t *f, void *obj, int startoff,
				  int idx);
