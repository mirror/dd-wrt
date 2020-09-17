// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef _XR_ATTRREPAIR_H
#define _XR_ATTRREPAIR_H

/*
 * Access Control Lists
 */
#define ACL_USER_OBJ	0x01	/* owner */
#define ACL_USER	0x02	/* additional users */
#define ACL_GROUP_OBJ	0x04	/* group */
#define ACL_GROUP	0x08	/* additional groups */
#define ACL_MASK	0x10	/* mask entry */
#define ACL_OTHER	0x20	/* other entry */

#define ACL_READ	04
#define ACL_WRITE	02
#define ACL_EXECUTE	01

typedef uint16_t	xfs_acl_perm_t;
typedef int32_t		xfs_acl_type_t;
typedef int32_t		xfs_acl_tag_t;
typedef int32_t		xfs_acl_id_t;

/*
 * "icacl" = in-core ACL. There is no equivalent in the XFS kernel code,
 * so they are magic names just for repair. The "acl" types are what the kernel
 * code uses for the on-disk format names, so use them here too for the on-disk
 * ACL format definitions.
 */
struct xfs_icacl_entry {
	xfs_acl_tag_t	ae_tag;
	xfs_acl_id_t	ae_id;
	xfs_acl_perm_t	ae_perm;
};

struct xfs_icacl {
	int32_t			acl_cnt;
	struct xfs_icacl_entry	acl_entry[0];
};

/*
 * Mandatory Access Control Labels (IRIX)
 */
#define XFS_MAC_MAX_SETS	250
typedef struct xfs_mac_label {
	uint8_t       ml_msen_type;	/* MSEN label type */
	uint8_t       ml_mint_type;	/* MINT label type */
	uint8_t       ml_level;	/* Hierarchical level */
	uint8_t       ml_grade;	/* Hierarchical grade */
	uint16_t      ml_catcount;	/* Category count */
	uint16_t      ml_divcount;	/* Division count */
					/* Category set, then Division set */
	uint16_t      ml_list[XFS_MAC_MAX_SETS];
} xfs_mac_label_t;

/* MSEN label type names. Choose an upper case ASCII character.  */
#define XFS_MSEN_ADMIN_LABEL	'A'	/* Admin: low<admin != tcsec<high */
#define XFS_MSEN_EQUAL_LABEL	'E'	/* Wildcard - always equal */
#define XFS_MSEN_HIGH_LABEL	'H'	/* System High - always dominates */
#define XFS_MSEN_MLD_HIGH_LABEL	'I'	/* System High, multi-level dir */
#define XFS_MSEN_LOW_LABEL	'L'	/* System Low - always dominated */
#define XFS_MSEN_MLD_LABEL	'M'	/* TCSEC label on a multi-level dir */
#define XFS_MSEN_MLD_LOW_LABEL	'N'	/* System Low, multi-level dir */
#define XFS_MSEN_TCSEC_LABEL	'T'	/* TCSEC label */
#define XFS_MSEN_UNKNOWN_LABEL	'U'	/* unknown label */

/* MINT label type names. Choose a lower case ASCII character.  */
#define XFS_MINT_BIBA_LABEL	'b'	/* Dual of a TCSEC label */
#define XFS_MINT_EQUAL_LABEL	'e'	/* Wildcard - always equal */
#define XFS_MINT_HIGH_LABEL	'h'	/* High Grade - always dominates */
#define XFS_MINT_LOW_LABEL	'l'	/* Low Grade - always dominated */

#define SGI_MAC_FILE	"SGI_MAC_FILE"
#define SGI_MAC_FILE_SIZE	(sizeof(SGI_MAC_FILE)-1)


/*
 * Capabilities (IRIX)
 */
typedef uint64_t xfs_cap_value_t;

typedef struct xfs_cap_set {
	xfs_cap_value_t	cap_effective;  /* use in capability checks */
	xfs_cap_value_t	cap_permitted;  /* combined with file attrs */
	xfs_cap_value_t	cap_inheritable;/* pass through exec */
} xfs_cap_set_t;

#define SGI_CAP_FILE	"SGI_CAP_FILE"
#define SGI_CAP_FILE_SIZE	(sizeof(SGI_CAP_FILE)-1)


/*
 * External functions
 */
struct blkmap;
extern int process_attributes (xfs_mount_t *, xfs_ino_t, xfs_dinode_t *,
				struct blkmap *, int *);

#endif /* _XR_ATTRREPAIR_H */
