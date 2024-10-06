/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#include <stdlib.h>
#include <string.h>
#include <apfs/raw.h>
#include <apfs/types.h>
#include "apfsck.h"
#include "compress.h"
#include "extents.h"
#include "inode.h"
#include "key.h"
#include "super.h"
#include "xattr.h"

/**
 * check_xattr_flags - Check basic consistency of xattr flags
 * @flags: the flags
 */
static void check_xattr_flags(u16 flags)
{
	if ((flags & APFS_XATTR_VALID_FLAGS) != flags)
		report("Xattr record", "invalid flags in use.");
	if (flags & APFS_XATTR_RESERVED_8)
		report("Xattr record", "reserved flag in use.");
	if (flags & APFS_XATTR_UNKNOWN_10)
		report_unknown("0x10 xattr flag");

	if ((bool)(flags & APFS_XATTR_DATA_STREAM) ==
	    (bool)(flags & APFS_XATTR_DATA_EMBEDDED))
		report("Xattr record", "must be either embedded or dstream.");
}

/**
 * parse_xattr_dstream - Parse a xattr dstream struct and check for corruption
 * @xstream: the dstream structure
 *
 * Returns a pointer to the in-memory dstream structure.
 */
static struct dstream *parse_xattr_dstream(struct apfs_xattr_dstream *xstream)
{
	struct dstream *dstream;
	struct apfs_dstream *dstream_raw = &xstream->dstream;
	u64 id = le64_to_cpu(xstream->xattr_obj_id);
	u64 size, alloced_size;
	u64 crypid;

	size = le64_to_cpu(dstream_raw->size);
	alloced_size = le64_to_cpu(dstream_raw->alloced_size);
	crypid = le64_to_cpu(dstream_raw->default_crypto_id);
	if (crypid && crypid != APFS_CRYPTO_SW_ID)
		++get_crypto_state(crypid)->c_references;

	dstream = get_dstream(id);
	if (dstream->d_references) {
		/* A dstream structure for this id has already been seen */
		if (dstream->d_obj_type != APFS_TYPE_XATTR)
			report("Xattr dstream", "shared by inode and xattr.");
		if (dstream->d_size != size)
			report("Xattr dstream",
			       "inconsistent size for stream.");
		if (dstream->d_alloced_size != alloced_size)
			report("Xattr dstream",
			       "inconsistent allocated size for stream.");
	} else {
		dstream->d_obj_type = APFS_TYPE_XATTR;
		dstream->d_size = size;
		dstream->d_alloced_size = alloced_size;
	}
	dstream->d_references++;
	return dstream;
}

/**
 * parse_decmpfs - Parse the contents of a compression xattr
 * @inode:	inode for the file
 * @xdata:	contents of the xattr
 * @len:	length of @xdata
 */
static void parse_decmpfs(struct inode *inode, u8 *xdata, int len)
{
	struct compress *compress = inode->i_compress;
	struct apfs_compress_hdr *hdr = NULL;
	u32 algo;

	if (!compress)
		report("Inode", "is not compressed but has decmpfs xattr.");
	if (len < sizeof(*hdr))
		report("Decmpfs xattr", "too small for the header.");
	hdr = (struct apfs_compress_hdr *)xdata;

	if (hdr->signature)
		report_unknown("Compression signature");
	algo = le32_to_cpu(hdr->algo);
	compress->size = le64_to_cpu(hdr->size);

	compress->decmpfs = malloc(len);
	if (!compress->decmpfs)
		system_error();
	memcpy(compress->decmpfs, xdata, len);
	compress->decmpfs_len = len;

	switch (algo) {
	case APFS_COMPRESS_PLAIN_ATTR:
	case APFS_COMPRESS_ZLIB_ATTR:
		if (compress->rsrc_dstream)
			report("Compressed inode", "should not have a resource fork.");
		if (apfs_volume_is_sealed()) {
			compress->rsrc_dstream = get_dstream(inode->i_ino);
			compress->rsrc_dstream->d_inline = true;
		}
		break;
	case APFS_COMPRESS_PLAIN_RSRC:
	case APFS_COMPRESS_ZLIB_RSRC:
	case APFS_COMPRESS_LZBITMAP_RSRC:
		if (len != sizeof(*hdr))
			report("Decmpfs xattr", "too big for non-inline.");
		if (!compress->rsrc_dstream)
			report("Inode", "should have resource fork for compression.");
		break;
	default:
		report_unknown("Compression algorithm");
	}
}

/**
 * parse_xattr_record - Parse a xattr record value and check for corruption
 * @key:	pointer to the raw key
 * @val:	pointer to the raw value
 * @len:	length of the raw value
 *
 * Internal consistency of @key must be checked before calling this function.
 */
void parse_xattr_record(struct apfs_xattr_key *key,
			struct apfs_xattr_val *val, int len)
{
	struct inode *inode;
	u16 flags;
	u64 content_len;
	u64 dstream_id = 0;

	if (len < sizeof(*val))
		report("Xattr record", "value is too small.");
	len -= sizeof(*val);

	flags = le16_to_cpu(val->flags);
	check_xattr_flags(flags);

	inode = get_inode(cat_cnid(&key->hdr));
	if (!inode->i_seen)
		report("Catalog", "xattr record with no inode.");

	if (flags & APFS_XATTR_DATA_STREAM) {
		struct dstream *dstream;
		struct apfs_xattr_dstream *dstream_raw;

		if (len != sizeof(*dstream_raw))
			report("Xattr record",
			       "bad length for dstream structure.");
		if (len != le16_to_cpu(val->xdata_len))
			/* Never seems to happen, but the docs don't ban it */
			report_weird("Xattr data length for dstream structure");

		dstream_raw = (struct apfs_xattr_dstream *)val->xdata;
		dstream = parse_xattr_dstream(dstream_raw);
		dstream->d_owner = inode->i_ino;
		dstream->d_xattr = true;

		if (strcmp((char *)key->name, APFS_XATTR_NAME_RSRC_FORK) == 0 && inode->i_compress)
			inode->i_compress->rsrc_dstream = dstream;

		content_len = dstream->d_size;
		dstream_id = dstream->d_id;
	} else {
		if (len != le16_to_cpu(val->xdata_len))
			report("Xattr record", "bad length for embedded data.");
		if (len > APFS_XATTR_MAX_EMBEDDED_SIZE)
			report("Xattr record", "embedded data is too long.");

		content_len = len;
	}

	if (!strcmp((char *)key->name, APFS_XATTR_NAME_SYMLINK)) {
		if (!(flags & APFS_XATTR_FILE_SYSTEM_OWNED))
			report("Symlink target xattr", "not owned by system.");
		if (inode->i_xattr_bmap & XATTR_BMAP_SYMLINK)
			report("Catalog", "two targets for same symlink.");
		inode->i_xattr_bmap |= XATTR_BMAP_SYMLINK;
	} else if (!strcmp((char *)key->name, APFS_XATTR_NAME_RSRC_FORK)) {
		if (flags & APFS_XATTR_FILE_SYSTEM_OWNED)
			report("Resource fork xattr", "owned by system.");
		if (inode->i_xattr_bmap & XATTR_BMAP_RSRC_FORK)
			report("Catalog", "two resource forks for same inode.");
		inode->i_xattr_bmap |= XATTR_BMAP_RSRC_FORK;
	} else if (!strcmp((char *)key->name, APFS_XATTR_NAME_SECURITY)) {
		if (flags & APFS_XATTR_FILE_SYSTEM_OWNED)
			report("Security xattr", "owned by system.");
		if (inode->i_xattr_bmap & XATTR_BMAP_SECURITY)
			report("Catalog", "two security xattrs for one inode.");
		inode->i_xattr_bmap |= XATTR_BMAP_SECURITY;
	} else if (!strcmp((char *)key->name, APFS_XATTR_NAME_FINDER_INFO)) {
		if (flags & APFS_XATTR_FILE_SYSTEM_OWNED)
			report("Finder info xattr", "owned by system.");
		if (inode->i_xattr_bmap & XATTR_BMAP_FINDER_INFO)
			report("Catalog", "two finder info xattrs for one inode.");
		inode->i_xattr_bmap |= XATTR_BMAP_FINDER_INFO;
		if (content_len != 32)
			report("Finder info xattr", "wrong size");
	} else if (!strcmp((char *)key->name, APFS_XATTR_NAME_COMPRESSED)) {
		if (flags & APFS_XATTR_FILE_SYSTEM_OWNED)
			report("Decmpfs xattr", "owned by system.");
		if (inode->i_xattr_bmap & XATTR_BMAP_COMPRESSED)
			report("Catalog", "inode has two compressed headers.");
		inode->i_xattr_bmap |= XATTR_BMAP_COMPRESSED;
		if (flags & APFS_XATTR_DATA_STREAM) {
			u8 *decmpfs = NULL;

			decmpfs = malloc(content_len);
			if (!decmpfs)
				system_error();
			read_whole_dstream(dstream_id, decmpfs, content_len);
			parse_decmpfs(inode, decmpfs, content_len);
			free(decmpfs);
		} else {
			parse_decmpfs(inode, val->xdata, len);
		}
	}
}
