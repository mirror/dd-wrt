/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "apfsck.h"
#include "compress.h"
#include "dir.h"
#include "extents.h"
#include "htable.h"
#include "inode.h"
#include "key.h"
#include "super.h"

/**
 * xbmap_set - Set an xfield type in the xfield bitmap
 * @bmap: the xfield bitmap
 * @type: the extended field type
 */
static inline void xbmap_set(u16 *bmap, u8 type)
{
	*bmap |= 1 << type;
}

/**
 * xbmap_test - Test if an xfield type is present in the xfield bitmap
 * @bmap: the xfield bitmap
 * @type: the extended field type
 */
static inline bool xbmap_test(u16 bmap, u8 type)
{
	return bmap & (1 << type);
}

/**
 * check_finder_info - Check the inode flag that reports finder info
 * @inode: the inode to check
 *
 * I've encountered some old images that put the finder info in a xattr instead
 * of an xfield, but they still set the inode flag, so this check has to be done
 * after all xattrs have been parsed.
 */
static void check_finder_info(struct inode *inode)
{
	bool xfield = xbmap_test(inode->i_xfield_bmap, APFS_INO_EXT_TYPE_FINDER_INFO);
	bool xattr = inode->i_xattr_bmap & XATTR_BMAP_FINDER_INFO;
	bool flag = inode->i_flags & APFS_INODE_HAS_FINDER_INFO;

	if (xfield && xattr)
		report("Inode record", "duplicated finder info.");
	if (flag != (xfield || xattr))
		report("Inode record", "wrong setting for finder info flag.");
}

/**
 * check_inode_stats - Verify the stats gathered by the fsck vs the metadata
 * @inode: inode structure to check
 */
static void check_inode_stats(struct inode *inode)
{
	struct dstream *dstream;

	/* The inodes must be freed before the dstreams */
	assert(vsb->v_dstream_table);

	if ((inode->i_mode & S_IFMT) == S_IFDIR) {
		if (inode->i_link_count != 1)
			report("Inode record", "directory has hard links.");
		if (inode->i_nchildren != inode->i_child_count)
			report("Inode record", "wrong directory child count.");
		if (inode->i_first_parent == APFS_PRIV_DIR_INO_NUM && inode->i_nchildren != 0)
			report("Orphan directory", "has children of its own.");
	} else if (inode->i_first_parent == APFS_PRIV_DIR_INO_NUM) {
		if (inode->i_link_count != 1)
			report("Orphan inode", "not really orphaned.");
		if (inode->i_nlink != 0)
			report("Orphan inode", "has a link count.");
	} else {
		if (inode->i_nlink != inode->i_link_count)
			report("Inode record", "wrong link count.");
	}

	dstream = inode->i_dstream;
	if (dstream) {
		if (dstream->d_refcnt > 1 && !(inode->i_flags & (APFS_INODE_WAS_CLONED | APFS_INODE_WAS_EVER_CLONED)))
			report("Inode record", "wrong flags for cloned inode.");
		if (inode->i_first_parent == APFS_PRIV_DIR_INO_NUM) {
			dstream->d_orphan = true;
		} else if (dstream->d_sparse_bytes != inode->i_sparse_bytes) {
			/* The official fsck ignores this field for orphans */
			report("Inode record", "wrong count of sparse bytes.");
		}
	} else {
		if (inode->i_sparse_bytes)
			report("Inode record", "sparse bytes without dstream.");
	}

	if ((bool)(inode->i_xattr_bmap & XATTR_BMAP_SYMLINK) !=
	    (bool)((inode->i_mode & S_IFMT) == S_IFLNK))
		report("Inode record",
		       "symlink inode should come with target xattr.");
	if ((bool)(inode->i_xattr_bmap & XATTR_BMAP_RSRC_FORK) !=
	    (bool)(inode->i_flags & APFS_INODE_HAS_RSRC_FORK))
		report("Inode record", "wrong flag for resource fork.");
	if ((bool)(inode->i_xattr_bmap & XATTR_BMAP_SECURITY) !=
	    (bool)(inode->i_flags & APFS_INODE_HAS_SECURITY_EA))
		report("Inode record", "wrong flag for access control list.");

	if ((bool)(inode->i_purg_name) != (bool)(inode->i_flags & APFS_INODE_IS_PURGEABLE))
		report("Inode record", "wrong purgeability flag.");

	check_finder_info(inode);
}

static void check_purgeable_name(struct inode *inode)
{
	const char *format = "0x%llx-0x%llx:%u";
	char *buf;
	int buflen;
	struct dstream *dstream = NULL;
	unsigned long long filesize;

	dstream = inode->i_dstream;
	filesize = dstream ? dstream->d_bytes : 0;

	buflen = snprintf(NULL, 0, format, filesize, (unsigned long long)inode->i_ino, (unsigned int)inode->i_owner);
	if (buflen < 0)
		system_error();
	buf = calloc(1, ++buflen);
	if (!buf)
		system_error();
	buflen = snprintf(buf, buflen, format, filesize, (unsigned long long)inode->i_ino, (unsigned int)inode->i_owner);
	if (buflen < 0)
		system_error();

	if (strcmp(inode->i_purg_name, buf) != 0)
		report("Purgeable inode", "wrong name for purgeable dentry.");

	free(buf);
}

/**
 * free_inode_names - Free all data on an inode's names
 * @inode: inode to free
 *
 * Frees the primary name and all sibling links, but not before running a few
 * remaining consistency checks.
 */
static void free_inode_names(struct inode *inode)
{
	struct sibling *current = inode->i_siblings;
	struct sibling *next;
	u32 count = 0;

	if (!inode->i_name) /* Oddly, this seems to be always required */
		report("Inode record", "no name for primary link.");
	if (!inode->i_first_name)
		report("Catalog", "inode with no dentries.");

	if (inode->i_flags & APFS_INODE_ACTIVE_FILE_TRIMMED) {
		/* No idea if any of this is actually required */
		if (strcmp(inode->i_name, ".overprovisioning_file"))
			report("Overprovisioning file", "wrong name.");
		if (inode->i_link_count != 1)
			report("Overprovisioning file", "has hard links.");
	}

	if (inode->i_purg_name) {
		check_purgeable_name(inode);
		free(inode->i_purg_name);
		inode->i_purg_name = NULL;
	}

	if (current) {
		/* Primary link has lowest id, so it comes first in the list */
		if (strcmp(inode->i_name, (char *)current->s_name))
			report("Inode record", "wrong name for primary link.");
		if (inode->i_parent_id != current->s_parent_ino)
			report("Inode record", "bad parent for primary link.");
	} else if (inode->i_first_parent != APFS_PRIV_DIR_INO_NUM) {
		/*
		 * No siblings, so the primary link is the first and only.
		 * Files moved to the private directory preserve their original
		 * name and parent_id, so there's nothing to check.
		 */
		if (strcmp(inode->i_name, inode->i_first_name))
			report("Inode record", "wrong name for only link.");
		if (inode->i_parent_id != inode->i_first_parent)
			report("Inode record", "bad parent for only link.");
	}
	free(inode->i_name);
	inode->i_name = NULL;
	free(inode->i_first_name);
	inode->i_first_name = NULL;

	while (current) {
		struct listed_cnid *cnid;

		/* Put all filesystem object ids in a list to check for reuse */
		cnid = get_listed_cnid(current->s_id);
		cnid_set_state_flag(cnid, CNID_IN_SIBLING_LINK);

		if (!current->s_checked)
			report("Catalog", "orphaned or missing sibling link.");
		if (!current->s_mapped)
			report("Catalog", "no sibling map for link.");

		next = current->s_next;
		free(current->s_name);
		free(current);
		current = next;
		++count;
	}

	if (inode->i_first_parent == APFS_PRIV_DIR_INO_NUM) {
		if (count != 0)
			report("Orphan inode", "has sibling links.");
		return;
	}

	/* Inodes with one link can have a sibling record, but don't need it */
	if (inode->i_link_count == 1 && count == 0)
		return;

	if (count != inode->i_link_count)
		report("Inode record",
		       "link count inconsistent with sibling records.");
}

/**
 * free_inode - Free an inode structure after performing some final checks
 * @entry: the entry to free
 */
static void free_inode(struct htable_entry *entry)
{
	struct inode *inode = (struct inode *)entry;
	struct compress *compress = inode->i_compress;
	struct listed_cnid *cnid;

	/* All of these must still be around for the inodes to access */
	assert(vsb->v_cnid_table);
	assert(vsb->v_dirstat_table);
	assert(vsb->v_dstream_table);

	/* To check for reuse, put all filesystem object ids in a list */
	cnid = get_listed_cnid(inode->i_ino);
	cnid_set_state_flag(cnid, CNID_IN_INODE);

	if (compress) {
		apfs_compress_open(compress);
		apfs_compress_check(compress);
		verify_dstream_hashes(compress->rsrc_dstream, compress);
		apfs_compress_close(compress);
		free(compress->decmpfs);
		free(compress);
		inode->i_compress = NULL;
	}

	check_inode_stats(inode);
	free_inode_names(inode);
	free(entry);
}

static void collect_dirstats(struct htable_entry *entry)
{
	struct inode *inode = (struct inode *)entry;
	u16 filetype = inode->i_mode & S_IFMT;
	struct dirstat *stat = inode->i_dirstat;
	struct dirstat *parent_stat = NULL;

	/* Orphans report a stale parent id, don't take that seriously */
	if (inode->i_first_parent == APFS_PRIV_DIR_INO_NUM)
		return;

	if (inode->i_parent_id >= APFS_MIN_USER_INO_NUM) {
		parent_stat = get_inode(inode->i_parent_id)->i_dirstat;
		if (stat && parent_stat && stat != parent_stat)
			report("Inode record", "dirstat id differs from parent.");
	}

	if (inode->i_flags & APFS_INODE_MAINTAIN_DIR_STATS) {
		assert(stat);
		if (!stat->ds_seen)
			report("Inode record", "missing a dirstats record.");
		if (!(inode->i_flags & APFS_INODE_DIR_STATS_ORIGIN) && !parent_stat)
			report("Directory statistics", "origin missing.");
		stat->ds_child_count += inode->i_nchildren;
	} else if (parent_stat) {
		if (inode->i_flags & APFS_INODE_DIR_STATS_ORIGIN)
			report_unknown("Nested dirstat origins.");
		if (filetype == S_IFDIR)
			report("Inode record", "should gather dir stats for ancestor.");
		else if (inode->i_dstream)
			parent_stat->ds_total_size += inode->i_dstream->d_size;
	}
}

/**
 * free_inode_table - Free the inode hash table and all its entries
 * @table: table to free
 *
 * Also performs some consistency checks that can only be done after the whole
 * catalog has been parsed.
 */
void free_inode_table(struct htable_entry **table)
{
	/*
	 * Collect directory statistics, and check that descendant directories
	 * inherit the APFS_INODE_MAINTAIN_DIR_STATS flag.
	 */
	apply_on_htable(table, collect_dirstats);

	free_htable(table, free_inode);
}

/**
 * get_inode - Find or create an inode structure in the inode hash table
 * @ino: inode number
 *
 * Returns the inode structure, after creating it if necessary.
 */
struct inode *get_inode(u64 ino)
{
	struct htable_entry *entry;

	entry = get_htable_entry(ino, sizeof(struct inode), vsb->v_inode_table);
	return (struct inode *)entry;
}

/**
 * read_sparse_bytes_xfield - Parse and check an xfield that counts sparse bytes
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	struct to receive the results
 *
 * Returns the length of the xfield value.
 */
static int read_sparse_bytes_xfield(char *xval, int len, struct inode *inode)
{
	__le64 *sbytes;

	if (len < 8)
		report("Sparse bytes xfield", "doesn't fit in inode record.");
	sbytes = (__le64 *)xval;

	inode->i_sparse_bytes = le64_to_cpu(*sbytes);

	return sizeof(*sbytes);
}

/**
 * read_document_id_xfield - Parse and check a document id xfield
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	inode structure
 *
 * Returns the length of the xfield value.
 */
static int read_document_id_xfield(char *xval, int len, struct inode *inode)
{
	__le32 *id_raw;
	u32 id;

	if (len < 4)
		report("Document id xfield", "doesn't fit in inode record.");
	id_raw = (__le32 *)xval;
	id = le32_to_cpu(*id_raw);

	if (id < APFS_MIN_DOC_ID)
		report("Document id xfield", "invalid id in use.");
	if (id >= vsb->v_next_doc_id)
		report("Document id xfield", "free id in use.");

	return sizeof(*id_raw);
}

/**
 * read_rdev_xfield - Parse and check a device identifier xfield
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	struct to receive the results
 *
 * Returns the length of the xfield value.
 */
static int read_rdev_xfield(char *xval, int len, struct inode *inode)
{
	u16 filetype = inode->i_mode & S_IFMT;
	__le32 *rdev;

	assert(filetype); /* Mode must be set before parsing xfields */
	if (filetype != S_IFCHR && filetype != S_IFBLK)
		report("Inode record", "not device but has device identifier.");

	if (len < 4)
		report("Device ID xfield", "doesn't fit in inode record.");
	rdev = (__le32 *)xval;

	inode->i_rdev = le32_to_cpu(*rdev);
	return sizeof(*rdev);
}

/**
 * read_name_xfield - Parse a name xfield and check its consistency
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	struct to receive the results
 *
 * Returns the length of the xfield value.
 */
static int read_name_xfield(char *xval, int len, struct inode *inode)
{
	int xlen;

	xlen = strnlen(xval, len - 1) + 1;
	if (xval[xlen - 1] != 0)
		report("Name xfield", "name with no null termination");

	inode->i_name = malloc(xlen);
	if (!inode->i_name)
		system_error();
	strcpy(inode->i_name, xval);

	return xlen;
}

/**
 * read_dstream_xfield - Parse a dstream xfield and check its consistency
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	struct to receive the results
 *
 * Returns the length of the xfield value.
 */
static int read_dstream_xfield(char *xval, int len, struct inode *inode)
{
	struct apfs_dstream *dstream_raw;
	struct dstream *dstream;
	u64 size, alloced_size;
	u64 crypid;

	if ((inode->i_mode & S_IFMT) != S_IFREG)
		report("Inode record", "has dstream but isn't a regular file.");

	if (len < sizeof(*dstream_raw))
		report("Dstream xfield", "doesn't fit in inode record.");
	dstream_raw = (struct apfs_dstream *)xval;

	size = le64_to_cpu(dstream_raw->size);
	alloced_size = le64_to_cpu(dstream_raw->alloced_size);
	crypid = le64_to_cpu(dstream_raw->default_crypto_id);
	if (crypid && crypid != APFS_CRYPTO_SW_ID) {
		/*
		 * I'm not yet sure how this crypto cloning thing is supposed
		 * to work, but it's very common (TODO).
		 */
		if (crypid == APFS_UNASSIGNED_CRYPTO_ID) {
			if (!(inode->i_flags & APFS_INODE_WAS_CLONED))
				report("Dstream xfield", "not a clone but has unassigned crypto.");
		} else {
			struct crypto_state *crypto = get_crypto_state(crypid);
			++crypto->c_references;
			if (inode->i_flags & APFS_INODE_ACTIVE_FILE_TRIMMED)
				crypto->c_overprov = true;
		}
	}

	dstream = get_dstream(inode->i_private_id);
	if (dstream->d_references) {
		/* A dstream structure for this id has already been seen */
		if (dstream->d_obj_type != APFS_TYPE_INODE)
			report("Dstream xfield", "shared by inode and xattr.");
		if (dstream->d_size != size)
			report("Dstream xfield",
			       "inconsistent size for stream.");
		if (dstream->d_alloced_size != alloced_size)
			report("Dstream xfield",
			       "inconsistent allocated size for stream.");
	} else {
		dstream->d_obj_type = APFS_TYPE_INODE;
		dstream->d_size = size;
		dstream->d_alloced_size = alloced_size;
	}
	dstream->d_xattr = false;

	dstream->d_references++;
	dstream->d_owner = dstream->d_id;
	inode->i_dstream = dstream;
	return sizeof(*dstream_raw);
}

/**
 * read_dir_stats_xfield - Parse a dir stats xfield and check its consistency
 * @xval:	pointer to the xfield value
 * @len:	remaining length of the inode value
 * @inode:	struct to receive the results
 *
 * Returns the length of the xfield value.
 */
static int read_dir_stats_xfield(char *xval, int len, struct inode *inode)
{
	u16 filetype = inode->i_mode & S_IFMT;
	__le64 *oid = NULL;
	struct dirstat *stats = NULL;

	/* TODO: I'm yet to see a file with stats, this is probably wrong */
	if (filetype != S_IFDIR)
		report("Dir stats xfield", "inode is not a directory.");

	if (len < sizeof(*oid))
		report("Dir stats xfield", "doesn't fit in inode record.");
	oid = (__le64 *)xval;

	stats = inode->i_dirstat = get_dirstat(le64_to_cpu(*oid));
	if (inode->i_flags & APFS_INODE_DIR_STATS_ORIGIN) {
		/* TODO: this will be a problem for nested origins */
		if (stats->ds_origin_seen)
			report("Dir stats", "has two origins.");
		stats->ds_origin_seen = true;
		stats->ds_origin = inode->i_ino;
	}
	return sizeof(*oid);
}

static int read_purgeable_flags_xfield(char *xval, int len, struct inode *inode)
{
	__le64 *flags;

	if (len < sizeof(*flags))
		report("Purgeable flags xfield", "doesn't fit in inode record.");
	flags = (__le64 *)xval;
	/*
	 * TODO: Figure out these flags. So far I have only seen them set to
	 * 0x10005, in a directory that was not purgeable itself, but had mostly
	 * purgeable entries.
	 */
	inode->i_purg_flags = le64_to_cpu(*flags);
	return sizeof(*flags);
}

/**
 * check_xfield_flags - Run common flag checks for all xfield types
 * @flags: flags to check
 */
void check_xfield_flags(u8 flags)
{
	if (flags & APFS_XF_RESERVED_4 || flags & APFS_XF_RESERVED_40 ||
	    flags & APFS_XF_RESERVED_80)
		report("Inode xfield", "reserved flag in use.");

	if (flags & APFS_XF_USER_FIELD && flags & APFS_XF_SYSTEM_FIELD)
		report("Inode xfield", "created by both user and kernel.");
}

/**
 * check_xfield_inode_flags - Check that xfields are consistent with inode flags
 * @bmap:	bitmap of xfield types seen in the inode
 * @flags:	inode flags
 *
 * This doesn't check the finder info flag, that happens when we free the inode.
 */
static void check_xfield_inode_flags(u16 bmap, u64 flags)
{
	if (xbmap_test(bmap, APFS_INO_EXT_TYPE_DIR_STATS_KEY) !=
	    (bool)(flags & APFS_INODE_MAINTAIN_DIR_STATS))
		report("Inode record", "wrong setting for dir stats flag.");
	if (xbmap_test(bmap, APFS_INO_EXT_TYPE_SPARSE_BYTES) !=
	    (bool)(flags & APFS_INODE_IS_SPARSE))
		report("Inode record", "wrong setting for sparse flag.");

	if (xbmap_test(bmap, APFS_INO_EXT_TYPE_PURGEABLE_FLAGS) != (bool)(flags & APFS_INODE_HAS_PURGEABLE_FLAGS))
		report("Inode record", "wrong setting for purgeable flags option.");
}

/**
 * parse_inode_xfields - Parse and check an inode extended fields
 * @xblob:	pointer to the beginning of the xfields in the inode value
 * @len:	length of the xfields
 * @inode:	struct to receive the results
 *
 * Internal consistency of @key must be checked before calling this function.
 */
static void parse_inode_xfields(struct apfs_xf_blob *xblob, int len,
				struct inode *inode)
{
	u16 filetype = inode->i_mode & S_IFMT;
	struct apfs_x_field *xfield;
	u16 type_bitmap = 0;
	char *xval;
	int xcount;
	int i;

	if (len == 0) { /* No extended fields */
		check_xfield_inode_flags(type_bitmap, inode->i_flags);
		return;
	}

	len -= sizeof(*xblob);
	if (len < 0)
		report("Inode records", "no room for extended fields.");

	xcount = le16_to_cpu(xblob->xf_num_exts);
	if (!xcount)
		report("Inode record", "xfield blob has no xfields.");

	xfield = (struct apfs_x_field *)xblob->xf_data;
	xval = (char *)xfield + xcount * sizeof(xfield[0]);
	len -= xcount * sizeof(xfield[0]);
	if (len < 0)
		report("Inode record", "number of xfields cannot fit.");

	/* The official reference seems to be wrong here */
	if (le16_to_cpu(xblob->xf_used_data) != len)
		report("Inode record", "value size incompatible with xfields.");

	for (i = 0; i < le16_to_cpu(xblob->xf_num_exts); ++i) {
		int xlen, xpad_len;
		u8 xflags = xfield[i].x_flags;

		check_xfield_flags(xflags);

		switch (xfield[i].x_type) {
		case APFS_INO_EXT_TYPE_FS_UUID:
			xlen = 16;
			report_unknown("UUID xfield");
			break;
		case APFS_INO_EXT_TYPE_PREV_FSIZE:
			xlen = 8;
			report_crash("Inode xfield");
			if (xflags != 0)
				report("Previous size xfield", "wrong flags.");
			break;
		case APFS_INO_EXT_TYPE_SNAP_XID:
			xlen = 8;
			report_unknown("Snapshot id xfield");
			break;
		case APFS_INO_EXT_TYPE_DELTA_TREE_OID:
			xlen = 8;
			report_unknown("Snapshot's extent delta list xfield");
			break;
		case APFS_INO_EXT_TYPE_SPARSE_BYTES:
			xlen = read_sparse_bytes_xfield(xval, len, inode);
			if (xflags != (APFS_XF_SYSTEM_FIELD |
				       APFS_XF_CHILDREN_INHERIT))
				report("Sparse bytes xfield", "wrong flags.");
			break;
		case APFS_INO_EXT_TYPE_DOCUMENT_ID:
			xlen = read_document_id_xfield(xval, len, inode);
			report_unknown("Document id xfield");
			break;
		case APFS_INO_EXT_TYPE_FINDER_INFO:
			xlen = 32;
			report_unknown("Finder info xfield");
			break;
		case APFS_INO_EXT_TYPE_RDEV:
			xlen = read_rdev_xfield(xval, len, inode);
			break;
		case APFS_INO_EXT_TYPE_NAME:
			xlen = read_name_xfield(xval, len, inode);
			if (xflags != APFS_XF_DO_NOT_COPY)
				report("Name xfield", "wrong flags.");
			break;
		case APFS_INO_EXT_TYPE_DSTREAM:
			xlen = read_dstream_xfield(xval, len, inode);
			if (xflags != APFS_XF_SYSTEM_FIELD)
				report("Data stream xfield", "wrong flags.");
			break;
		case APFS_INO_EXT_TYPE_DIR_STATS_KEY:
			xlen = read_dir_stats_xfield(xval, len, inode);
			if (xflags != (APFS_XF_SYSTEM_FIELD | APFS_XF_DO_NOT_COPY))
				report("Dir stats xfield", "wrong flags.");
			break;
		case APFS_INO_EXT_TYPE_PURGEABLE_FLAGS:
			xlen = read_purgeable_flags_xfield(xval, len, inode);
			break;
		case APFS_INO_EXT_TYPE_ORIG_SYNC_ROOT_ID:
			xlen = 8;
			report_unknown("Sync root id xfield");
			break;
		case APFS_INO_EXT_TYPE_RESERVED_6:
		case APFS_INO_EXT_TYPE_RESERVED_9:
		case APFS_INO_EXT_TYPE_RESERVED_12:
			report("Inode xfield", "reserved type in use.");
			break;
		default:
			report("Inode xfield", "invalid type.");
		}

		if (xbmap_test(type_bitmap, xfield[i].x_type))
			report("Inode record", "two xfields of the same type.");
		xbmap_set(&type_bitmap, xfield[i].x_type);

		if (xlen != le16_to_cpu(xfield[i].x_size))
			report("Inode xfield", "wrong size");
		len -= xlen;
		xval += xlen;

		/* Attribute length is padded with zeroes to a multiple of 8 */
		xpad_len = ROUND_UP(xlen, 8) - xlen;
		len -= xpad_len;
		if (len < 0)
			report("Inode xfield", "does not fit in record value.");

		for (; xpad_len; ++xval, --xpad_len)
			if (*xval)
				report("Inode xfield", "non-zero padding.");
	}

	if (len)
		report("Inode record", "length of xfields does not add up.");

	check_xfield_inode_flags(type_bitmap, inode->i_flags);
	inode->i_xfield_bmap = type_bitmap;

	if ((filetype == S_IFCHR || filetype == S_IFBLK) && !xbmap_test(type_bitmap, APFS_INO_EXT_TYPE_RDEV))
		report("Inode record", "device file with no device ID.");
}

/**
 * ino_after_cloneinfo_epoch - Is WAS_EVER_CLONED valid for this inode?
 * @ino: inode number to check
 */
static bool ino_after_cloneinfo_epoch(u64 ino)
{
	struct apfs_superblock *raw = vsb->v_raw;
	u64 id_epoch, xid;

	id_epoch = le64_to_cpu(raw->apfs_cloneinfo_id_epoch);
	xid = le64_to_cpu(raw->apfs_cloneinfo_xid);

	/*
	 * The epoch hasn't even started for this filesystem, all flags should
	 * be assumed corrupted.
	 */
	if (!xid)
		return false;

	/*
	 * This filesystem was last mounted by a buggy implementation, so flags
	 * for newer inodes may have become corrupted.
	 */
	if (xid != vsb->v_last_xid)
		return false;

	/*
	 * This filesystem was never even mounted by a buggy implementation:
	 * all flags should be correct.
	 */
	if (!id_epoch)
		return true;

	/*
	 * The reference seems to claim that this should be '>', but I have my
	 * doubts beause I've seen fresh images with epoch == MIN_USER_INO_NUM.
	 * There is no harm in being more strict here and waiting to see if we
	 * trip on something.
	 */
	return ino >= id_epoch;
}

/**
 * check_inode_internal_flags - Check basic consistency of inode flags
 * @flags:	flags to check
 * @ino:	inode number
 */
static void check_inode_internal_flags(u64 flags, u64 ino)
{
	if ((flags & APFS_VALID_INTERNAL_INODE_FLAGS) != flags)
		report("Inode record", "invalid flags in use.");

	if ((flags & APFS_INODE_DIR_STATS_ORIGIN) &&
	    !(flags & APFS_INODE_MAINTAIN_DIR_STATS))
		report("Inode record", "incompatible directory stats flags.");
	if (flags & APFS_INODE_HAS_RSRC_FORK && flags & APFS_INODE_NO_RSRC_FORK)
		report("Inode record", "incompatible resource fork flags.");

	if (flags & APFS_INODE_BEING_TRUNCATED)
		report_crash("Inode internal flags");

	if (flags & APFS_INODE_PINNED_MASK) {
		/*
		 * Preboot volume seems to be mostly pinned to main, even in
		 * non-fusion drives.
		 */
		if ((flags & APFS_INODE_PINNED_TO_TIER2) || apfs_volume_role() != APFS_VOL_ROLE_PREBOOT)
			report_unknown("Fusion drive");
		if ((flags & APFS_INODE_PINNED_TO_TIER2) && (flags & APFS_INODE_PINNED_TO_MAIN))
			report("Inode record", "pinned to both tiers.");
	}
	if (flags & APFS_INODE_ALLOCATION_SPILLEDOVER)
		report_unknown("Fusion drive");

	if (flags & APFS_INODE_IS_APFS_PRIVATE)
		report_unknown("Private implementation inode");

	if (flags & APFS_INODE_WAS_CLONED && !(flags & APFS_INODE_WAS_EVER_CLONED)) {
		if (ino_after_cloneinfo_epoch(ino))
			report("Inode record", "inconsistent clone flags.");
		else
			printf("Warning: bad WAS_EVER_CLONED flag for inode 0x%llx\n", (unsigned long long)ino);
	}
}

/**
 * inos_valid_for_sysvol_in_group - Can a system vol in a group use these inos?
 * @ino:	inode number
 * @parent_ino:	parent inode number
 */
static bool inos_valid_for_sysvol_in_group(u64 ino, u64 parent_ino)
{
	/*
	 * Reserved inode numbers in volume groups always seem to come
	 * from the data range, even if this contradicts the reference.
	 */
	if (ino < APFS_MIN_USER_INO_NUM && parent_ino < APFS_MIN_USER_INO_NUM)
		return true;
	if (ino < APFS_UNIFIED_ID_SPACE_MARK + APFS_MIN_USER_INO_NUM)
		return false;

	if (parent_ino < APFS_MIN_USER_INO_NUM)
		return true;
	if (parent_ino < APFS_UNIFIED_ID_SPACE_MARK + APFS_MIN_USER_INO_NUM)
		return false;

	return true;
}

/**
 * check_inode_ids - Check that an inode id is consistent with its parent id
 * @ino:	inode number
 * @parent_ino:	parent inode number
 */
void check_inode_ids(u64 ino, u64 parent_ino)
{
	if (ino >= vsb->v_next_obj_id || parent_ino >= vsb->v_next_obj_id)
		report("Inode record", "free inode number in use.");

	if (ino < APFS_MIN_USER_INO_NUM) {
		switch (ino) {
		case APFS_INVALID_INO_NUM:
		case APFS_ROOT_DIR_PARENT:
		case APFS_PURGEABLE_DIR_INO_NUM:
			report("Inode record", "invalid inode number.");
		case APFS_ROOT_DIR_INO_NUM:
		case APFS_PRIV_DIR_INO_NUM:
		case APFS_SNAP_DIR_INO_NUM:
			/* All children of this fake parent? TODO: check this */
			if (parent_ino != APFS_ROOT_DIR_PARENT)
				report("Root inode record", "bad parent id");
			break;
		default:
			report("Inode record", "reserved inode number.");
		}
		return;
	}

	if (parent_ino < APFS_MIN_USER_INO_NUM) {
		switch (parent_ino) {
		case APFS_INVALID_INO_NUM:
			report("Inode record", "invalid parent inode number.");
		case APFS_ROOT_DIR_PARENT:
			report("Inode record", "root parent id for nonroot.");
		case APFS_ROOT_DIR_INO_NUM:
		case APFS_PRIV_DIR_INO_NUM:
		case APFS_SNAP_DIR_INO_NUM:
		case APFS_PURGEABLE_DIR_INO_NUM:
			/* These are fine */
			break;
		default:
			report("Inode record", "reserved parent inode number.");
		}
	}

	if (apfs_is_data_volume_in_group()) {
		if (ino >= APFS_UNIFIED_ID_SPACE_MARK || parent_ino >= APFS_UNIFIED_ID_SPACE_MARK)
			report("Inode record", "bad number for data volume inode.");
	} else if (apfs_is_system_volume_in_group()) {
		if (!inos_valid_for_sysvol_in_group(ino, parent_ino))
			report("Inode record", "bad number for system volume inode.");
	}
}

/**
 * parse_inode_record - Parse an inode record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_inode_record(struct apfs_inode_key *key,
			struct apfs_inode_val *val, int len)
{
	struct inode *inode;
	u16 mode, filetype;
	u32 def_prot_class, bsd_flags;

	if (len < sizeof(*val))
		report("Inode record", "value is too small.");

	inode = get_inode(cat_cnid(&key->hdr));
	if (inode->i_seen)
		report("Catalog", "inode numbers are repeated.");
	inode->i_seen = true;
	inode->i_private_id = le64_to_cpu(val->private_id);

	inode->i_parent_id = le64_to_cpu(val->parent_id);
	check_inode_ids(inode->i_ino, inode->i_parent_id);
	if (inode->i_parent_id == APFS_PRIV_DIR_INO_NUM) {
		/* The reported parent id is not updated when orphaned */
		report("Inode record", "parent is private directory.");
	}

	if (inode->i_ino == APFS_ROOT_DIR_INO_NUM)
		vsb->v_has_root = true;
	if (inode->i_ino == APFS_PRIV_DIR_INO_NUM)
		vsb->v_has_priv = true;

	inode->i_flags = le64_to_cpu(val->internal_flags);
	check_inode_internal_flags(inode->i_flags, inode->i_ino);
	if (inode->i_ino != inode->i_private_id && !(inode->i_flags & (APFS_INODE_WAS_CLONED | APFS_INODE_WAS_EVER_CLONED)))
		report("Inode record", "not a clone but changed private id.");

	def_prot_class = le32_to_cpu(val->default_protection_class);
	if (def_prot_class > APFS_PROTECTION_CLASS_F || def_prot_class == 5)
		report("Inode record", "invalid default protection class");

	bsd_flags = le32_to_cpu(val->bsd_flags);
	if (bsd_flags & APFS_INOBSD_COMPRESSED) {
		inode->i_compress = calloc(1, sizeof(*inode->i_compress));
		if (!inode->i_compress)
			system_error();
	}

	mode = le16_to_cpu(val->mode);
	filetype = mode & S_IFMT;

	/* A dentry may have already set the mode, but only the type bits */
	if (inode->i_mode && inode->i_mode != filetype)
		report("Inode record", "file mode doesn't match dentry type.");
	inode->i_mode = mode;

	switch (filetype) {
	case S_IFREG:
		vsb->v_file_count++;
		break;
	case S_IFDIR:
		if (inode->i_ino >= APFS_MIN_USER_INO_NUM)
			vsb->v_dir_count++;
		break;
	case S_IFLNK:
		vsb->v_symlink_count++;
		break;
	case S_IFSOCK:
	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
		vsb->v_special_count++;
		break;
	default:
		report("Inode record", "invalid file mode.");
	}

	inode->i_nlink = le32_to_cpu(val->nlink);
	inode->i_owner = le32_to_cpu(val->owner);

	if (le16_to_cpu(val->pad1))
		report("Inode record", "padding should be zeroes.");
	if (!(inode->i_flags & APFS_INODE_HAS_UNCOMPRESSED_SIZE) && le64_to_cpu(val->uncompressed_size))
		report("Inode record", "should not report uncompressed size.");

	parse_inode_xfields((struct apfs_xf_blob *)val->xfields,
			    len - sizeof(*val), inode);
}

/**
 * get_sibling - Find or create a sibling link structure for an inode
 * @id:		sibling id
 * @inode:	the inode
 *
 * Returns the sibling structure, after creating it if necessary.
 */
struct sibling *get_sibling(u64 id, struct inode *inode)
{
	struct sibling **entry_p = &inode->i_siblings;
	struct sibling *entry = *entry_p;
	struct sibling *new;

	/* Siblings are ordered by id in the inode's linked list */
	while (entry) {
		if (id == entry->s_id)
			return entry;
		if (id < entry->s_id)
			break;

		entry_p = &entry->s_next;
		entry = *entry_p;
	}

	new = calloc(1, sizeof(*new));
	if (!new)
		system_error();

	new->s_checked = false;
	new->s_id = id;
	new->s_next = entry;
	*entry_p = new;
	return new;
}

/**
 * set_or_check_sibling - Set or check the fields of a sibling structure
 * @parent_id:	parent id
 * @namelen:	length of the name
 * @name:	name of the sibling
 * @sibling:	the sibling structure
 *
 * When first called for @sibling, sets the three given fields.  On the second
 * call, checks that they are set to the correct values.
 */
void set_or_check_sibling(u64 parent_id, int namelen, u8 *name,
			  struct sibling *sibling)
{
	/* Whichever was read first, dentry or sibling, sets the fields */
	if (!sibling->s_name) {
		sibling->s_parent_ino = parent_id;
		sibling->s_name_len = namelen;

		sibling->s_name = malloc(namelen);
		if (!sibling->s_name)
			system_error();
		strcpy((char *)sibling->s_name, (char *)name);
		return;
	}

	/* Fields already set, check them */
	if (sibling->s_name_len != namelen)
		report("Sibling record", "name length doesn't match dentry's.");
	if (strcmp((char *)sibling->s_name, (char *)name))
		report("Sibling record", "name doesn't match dentry's.");
	if (sibling->s_parent_ino != parent_id)
		report("Sibling record", "parent id doesn't match dentry's.");
	sibling->s_checked = true;
}

/**
 * parse_sibling_record - Parse and check a sibling link record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_sibling_record(struct apfs_sibling_link_key *key,
			  struct apfs_sibling_val *val, int len)
{
	struct inode *inode;
	struct sibling *sibling;
	int namelen;

	if (len < sizeof(*val))
		report("Sibling link record", "value is too small.");
	namelen = le16_to_cpu(val->name_len);

	if (len != sizeof(*val) + namelen)
		report("Sibling link record", "wrong size of value.");
	if (val->name[namelen - 1] != 0)
		report("Sibling link record", "name lacks NULL-termination.");
	/* Name length doesn't need checking: it's the same for the dentry */

	inode = get_inode(cat_cnid(&key->hdr));
	if (!inode->i_seen) /* The b-tree keys are in order */
		report("Sibling link record", "inode is missing");

	sibling = get_sibling(le64_to_cpu(key->sibling_id), inode);

	/* It seems that sibling ids come from the same pool as inode numbers */
	if (sibling->s_id < APFS_MIN_USER_INO_NUM)
		report("Sibling record", "invalid sibling id.");
	if (sibling->s_id >= vsb->v_next_obj_id)
		report("Sibling record", "free id in use.");

	set_or_check_sibling(le64_to_cpu(val->parent_id), namelen, val->name,
			     sibling);
}

/**
 * parse_sibling_record - Parse and check a sibling map record value
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_sibling_map_record(struct apfs_sibling_map_key *key,
			      struct apfs_sibling_map_val *val, int len)
{
	struct inode *inode;
	struct sibling *sibling;

	if (len != sizeof(*val))
		report("Sibling map record", "wrong size of value.");

	inode = get_inode(le64_to_cpu(val->file_id));
	sibling = get_sibling(cat_cnid(&key->hdr), inode);
	sibling->s_mapped = true;
}
