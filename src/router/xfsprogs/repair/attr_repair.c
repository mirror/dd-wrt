// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "globals.h"
#include "err_protos.h"
#include "attr_repair.h"
#include "dinode.h"
#include "bmap.h"
#include "protos.h"
#include "dir2.h"
#include "da_util.h"

static int xfs_acl_valid(struct xfs_mount *mp, struct xfs_acl *daclp);
static int xfs_mac_valid(xfs_mac_label_t *lp);

/*
 * da node check/verify functions that the attribute tree relies on are first in
 * the file before the actual attribute code. This used to be shared with the
 * dir v1 code, but that format is no longer supported yb the userspace
 * utilities and hence is now specific to the attribute tree implementation.
 */

typedef unsigned char	da_freemap_t;

/*
 * Allocate a freespace map for directory or attr leaf blocks (1 bit per byte)
 * 1 == used, 0 == free.
 */
static da_freemap_t *
alloc_da_freemap(struct xfs_mount *mp)
{
	return calloc(1, mp->m_sb.sb_blocksize / NBBY);
}

/*
 * Set the he range [start, stop) in the directory freemap.
 *
 * Returns 1 if there is a conflict or 0 if everything's good.
 *
 * Within a char, the lowest bit of the char represents the byte with
 * the smallest address
 */
static int
set_da_freemap(xfs_mount_t *mp, da_freemap_t *map, int start, int stop)
{
	const da_freemap_t mask = 0x1;
	int i;

	if (start > stop)  {
		/*
		 * allow == relation since [x, x) claims 1 byte
		 */
		do_warn(_("bad range claimed [%d, %d) in da block\n"),
			start, stop);
		return(1);
	}

	if (stop > mp->m_sb.sb_blocksize)  {
		do_warn(
	_("byte range end [%d %d) in da block larger than blocksize %d\n"),
			start, stop, mp->m_sb.sb_blocksize);
		return(1);
	}

	for (i = start; i < stop; i ++)  {
		if (map[i / NBBY] & (mask << i % NBBY))  {
			do_warn(_("multiply claimed byte %d in da block\n"), i);
			return(1);
		}
		map[i / NBBY] |= (mask << i % NBBY);
	}

	return(0);
}

/*
 * For attribute repair, there are 3 formats to worry about. First, is
 * shortform attributes which reside in the inode. Second is the leaf
 * form, and lastly the btree. Much of this models after the directory
 * structure so code resembles the directory repair cases.
 * For shortform case, if an attribute looks corrupt, it is removed.
 * If that leaves the shortform down to 0 attributes, it's okay and
 * will appear to just have a null attribute fork. Some checks are done
 * for validity of the value field based on what the security needs are.
 * Calls will be made to xfs_mac_valid or xfs_acl_valid routines if the
 * security attributes exist. They will be cleared if invalid.
 * No other values will be checked. The DMF folks do not have current
 * requirements, but may in the future.
 *
 * For leaf block attributes, it requires more processing. One sticky
 * point is that the attributes can be local (within the leaf) or
 * remote (outside the leaf in other blocks). Thinking of local only
 * if you get a bad attribute, and want to delete just one, it's a-okay
 * if it remains large enough to still be a leaf block attribute. Otherwise,
 * it may have to be converted to shortform. How to convert this and when
 * is an issue. This call is happening in Phase3. Phase5 will capture empty
 * blocks, but Phase6 allows you to use the libxfs library which knows
 * how to handle attributes in the kernel for converting formats. What we
 * could do is mark an attribute to be cleared now, but in phase6 somehow
 * have it cleared for real and then the format changed to shortform if
 * applicable. Since this requires more work than I anticipate can be
 * accomplished for the next release, we will instead just say any bad
 * attribute in the leaf block will make the entire attribute fork be
 * cleared. The simplest way to do that is to ignore the leaf format, and
 * call clear_dinode_attr to just make a shortform attribute fork with
 * zero entries.
 *
 * Another issue with handling repair on leaf attributes is the remote
 * blocks. To make sure that they look good and are not used multiple times
 * by the attribute fork, some mechanism to keep track of all them is necessary.
 * Do this in the future, time permitting. For now, note that there is no
 * check for remote blocks and their allocations.
 *
 * For btree formatted attributes, the model can follow directories. That
 * would mean go down the tree to the leftmost leaf. From there moving down
 * the links and processing each. They would call back up the tree, to verify
 * that the tree structure is okay. Any problems will result in the attribute
 * fork being emptied and put in shortform format.
 */

/*
 * This routine just checks what security needs are for attribute values
 * only called when root flag is set, otherwise these names could exist in
 * in user attribute land without a conflict.
 * If value is non-zero, then a remote attribute is being passed in
 */
static int
valuecheck(
	struct xfs_mount *mp,
	char		*namevalue,
	char		*value,
	int		namelen,
	int		valuelen)
{
	/* for proper alignment issues, get the structs and memmove the values */
	xfs_mac_label_t macl;
	void *valuep;
	int clearit = 0;

	if ((namelen == SGI_ACL_FILE_SIZE &&
	     strncmp(namevalue, SGI_ACL_FILE, SGI_ACL_FILE_SIZE) == 0) ||
	    (namelen == SGI_ACL_DEFAULT_SIZE &&
	     strncmp(namevalue, SGI_ACL_DEFAULT, SGI_ACL_DEFAULT_SIZE) == 0)) {
		if (value == NULL) {
			valuep = malloc(valuelen);
			if (!valuep)
				do_error(_("No memory for ACL check!\n"));
			memcpy(valuep, namevalue + namelen, valuelen);
		} else
			valuep = value;

		if (xfs_acl_valid(mp, valuep) != 0) {
			clearit = 1;
			do_warn(
	_("entry contains illegal value in attribute named SGI_ACL_FILE "
	  "or SGI_ACL_DEFAULT\n"));
		}

		if (valuep != value)
			free(valuep);

	} else if (strncmp(namevalue, SGI_MAC_FILE, SGI_MAC_FILE_SIZE) == 0) {
		if (value == NULL) {
			memset(&macl, 0, sizeof(xfs_mac_label_t));
			memmove(&macl, namevalue+namelen, valuelen);
			valuep = &macl;
		} else
			valuep = value;

		if (xfs_mac_valid((xfs_mac_label_t *)valuep) != 1) { /* 1 is valid */
			 /*
			 * if sysconf says MAC enabled,
			 *	temp = mac_from_text("msenhigh/mintlow", NULL)
			 *	copy it to value, update valuelen, totsize
			 *	This causes pushing up or down of all following
			 *	attributes, forcing a attribute format change!!
			 * else clearit = 1;
			 */
			clearit = 1;
			do_warn(
	_("entry contains illegal value in attribute named SGI_MAC_LABEL\n"));
		}
	} else if (strncmp(namevalue, SGI_CAP_FILE, SGI_CAP_FILE_SIZE) == 0) {
		if ( valuelen != sizeof(xfs_cap_set_t)) {
			clearit = 1;
			do_warn(
	_("entry contains illegal value in attribute named SGI_CAP_FILE\n"));
		}
	}

	return(clearit);
}


/*
 * this routine validates the attributes in shortform format.
 * a non-zero return repair value means certain attributes are bogus
 * and were cleared if possible. Warnings do not generate error conditions
 * if you cannot modify the structures. repair is set to 1, if anything
 * was fixed.
 */
static int
process_shortform_attr(
	struct xfs_mount		*mp,
	xfs_ino_t			ino,
	struct xfs_dinode		*dip,
	int				*repair)
{
	struct xfs_attr_shortform	*asf;
	struct xfs_attr_sf_entry	*currententry, *nextentry, *tempentry;
	int				i, junkit;
	int				currentsize, remainingspace;

	*repair = 0;

	asf = (struct xfs_attr_shortform *) XFS_DFORK_APTR(dip);

	/* Assumption: hdr.totsize is less than a leaf block and was checked
	 * by lclinode for valid sizes. Check the count though.
	*/
	if (asf->hdr.count == 0)
		/* then the total size should just be the header length */
		if (be16_to_cpu(asf->hdr.totsize) != sizeof(xfs_attr_sf_hdr_t)) {
			/* whoops there's a discrepancy. Clear the hdr */
			if (!no_modify) {
				do_warn(
	_("there are no attributes in the fork for inode %" PRIu64 "\n"),
					ino);
				asf->hdr.totsize =
					cpu_to_be16(sizeof(xfs_attr_sf_hdr_t));
				*repair = 1;
				return(1);
			} else {
				do_warn(
	_("would junk the attribute fork since count is 0 for inode %" PRIu64 "\n"),
					ino);
				return(1);
			}
		}

	currentsize = sizeof(xfs_attr_sf_hdr_t);
	remainingspace = be16_to_cpu(asf->hdr.totsize) - currentsize;
	nextentry = &asf->list[0];
	for (i = 0; i < asf->hdr.count; i++)  {
		currententry = nextentry;
		junkit = 0;

		/* don't go off the end if the hdr.count was off */
		if ((currentsize + (sizeof(struct xfs_attr_sf_entry) - 1)) >
						be16_to_cpu(asf->hdr.totsize))
			break; /* get out and reset count and totSize */

		/* if the namelen is 0, can't get to the rest of the entries */
		if (currententry->namelen == 0) {
			do_warn(_("zero length name entry in attribute fork,"));
			if (!no_modify) {
				do_warn(
	_(" truncating attributes for inode %" PRIu64 " to %d\n"), ino, i);
				*repair = 1;
				break;	/* and then update hdr fields */
			} else {
				do_warn(
	_(" would truncate attributes for inode %" PRIu64 " to %d\n"), ino, i);
				break;
			}
		} else {
			/* It's okay to have a 0 length valuelen, but do a
			 * rough check to make sure we haven't gone outside of
			 * totsize.
			 */
			if (remainingspace < currententry->namelen ||
					((remainingspace - currententry->
					namelen) < currententry->valuelen)) {
				do_warn(
	_("name or value attribute lengths are too large,\n"));
				if (!no_modify) {
					do_warn(
	_(" truncating attributes for inode %" PRIu64 " to %d\n"),
						ino, i);
					*repair = 1;
					break; /* and then update hdr fields */
				} else {
					do_warn(
	_(" would truncate attributes for inode %" PRIu64 " to %d\n"),
						ino, i);
					break;
				}
			}
		}

		/* namecheck checks for null chars in attr names. */
		if (!libxfs_attr_namecheck(currententry->nameval,
					   currententry->namelen)) {
			do_warn(
	_("entry contains illegal character in shortform attribute name\n"));
			junkit = 1;
		}

		if (currententry->flags & XFS_ATTR_INCOMPLETE) {
			do_warn(
	_("entry has INCOMPLETE flag on in shortform attribute\n"));
			junkit = 1;
		}

		/* Only check values for root security attributes */
		if (currententry->flags & XFS_ATTR_ROOT)
		       junkit |= valuecheck(mp,
					(char *)&currententry->nameval[0],
					NULL, currententry->namelen,
					currententry->valuelen);

		remainingspace = remainingspace -
					xfs_attr_sf_entsize(currententry);

		if (junkit) {
			if (!no_modify) {
				/* get rid of only this entry */
				do_warn(
	_("removing attribute entry %d for inode %" PRIu64 "\n"),
					i, ino);
				tempentry = (struct xfs_attr_sf_entry *)
					((intptr_t) currententry +
					 xfs_attr_sf_entsize(currententry));
				memmove(currententry,tempentry,remainingspace);
				asf->hdr.count -= 1;
				i--; /* no worries, it will wrap back to 0 */
				*repair = 1;
				continue; /* go back up now */
			} else {
				do_warn(
	_("would remove attribute entry %d for inode %" PRIu64 "\n"),
					i, ino);
			}
		}

		/* Let's get ready for the next entry... */
		nextentry = (struct xfs_attr_sf_entry *)((intptr_t) nextentry +
			 		xfs_attr_sf_entsize(currententry));
		currentsize = currentsize + xfs_attr_sf_entsize(currententry);

	} /* end the loop */

	if (asf->hdr.count != i)  {
		if (no_modify)  {
			do_warn(
	_("would have corrected attribute entry count in inode %" PRIu64 " from %d to %d\n"),
				ino, asf->hdr.count, i);
		} else  {
			do_warn(
	_("corrected attribute entry count in inode %" PRIu64 ", was %d, now %d\n"),
				ino, asf->hdr.count, i);
			asf->hdr.count = i;
			*repair = 1;
		}
	}

	/* ASSUMPTION: currentsize <= totsize */
	if (be16_to_cpu(asf->hdr.totsize) != currentsize)  {
		if (no_modify)  {
			do_warn(
	_("would have corrected attribute totsize in inode %" PRIu64 " from %d to %d\n"),
				ino, be16_to_cpu(asf->hdr.totsize),
				currentsize);
		} else  {
			do_warn(
	_("corrected attribute entry totsize in inode %" PRIu64 ", was %d, now %d\n"),
				ino, be16_to_cpu(asf->hdr.totsize),
				currentsize);
			asf->hdr.totsize = cpu_to_be16(currentsize);
			*repair = 1;
		}
	}

	return(*repair);
}

/* This routine brings in blocks from disk one by one and assembles them
 * in the value buffer. If get_bmapi gets smarter later to return an extent
 * or list of extents, that would be great. For now, we don't expect too
 * many blocks per remote value, so one by one is sufficient.
 */
static int
rmtval_get(xfs_mount_t *mp, xfs_ino_t ino, blkmap_t *blkmap,
		xfs_dablk_t blocknum, int valuelen, char* value)
{
	xfs_fsblock_t	bno;
	struct xfs_buf	*bp;
	int		clearit = 0, i = 0, length = 0, amountdone = 0;
	int		hdrsize = 0;
	int		error;

	if (xfs_has_crc(mp))
		hdrsize = sizeof(struct xfs_attr3_rmt_hdr);

	/* ASSUMPTION: valuelen is a valid number, so use it for looping */
	/* Note that valuelen is not a multiple of blocksize */
	while (amountdone < valuelen) {
		bno = blkmap_get(blkmap, blocknum + i);
		if (bno == NULLFSBLOCK) {
			do_warn(
	_("remote block for attributes of inode %" PRIu64 " is missing\n"), ino);
			clearit = 1;
			break;
		}
		error = -libxfs_buf_read(mp->m_dev, XFS_FSB_TO_DADDR(mp, bno),
				XFS_FSB_TO_BB(mp, 1), LIBXFS_READBUF_SALVAGE,
				&bp, &xfs_attr3_rmt_buf_ops);
		if (error) {
			do_warn(
	_("can't read remote block for attributes of inode %" PRIu64 "\n"), ino);
			clearit = 1;
			break;
		}

		if (bp->b_error == -EFSBADCRC || bp->b_error == -EFSCORRUPTED) {
			do_warn(
	_("Corrupt remote block for attributes of inode %" PRIu64 "\n"), ino);
			libxfs_buf_relse(bp);
			clearit = 1;
			break;
		}

		ASSERT(mp->m_sb.sb_blocksize == BBTOB(bp->b_length));

		length = min(BBTOB(bp->b_length) - hdrsize, valuelen - amountdone);
		memmove(value, bp->b_addr + hdrsize, length);
		amountdone += length;
		value += length;
		i++;
		libxfs_buf_relse(bp);
	}
	return (clearit);
}

/* The block is read in. The magic number and forward / backward
 * links are checked by the caller process_leaf_attr.
 * If any problems occur the routine returns with non-zero. In
 * this case the next step is to clear the attribute fork, by
 * changing it to shortform and zeroing it out. Forkoff need not
 * be changed.
 */

static int
process_leaf_attr_local(
	struct xfs_mount	*mp,
	xfs_attr_leafblock_t	*leaf,
	int			i,
	xfs_attr_leaf_entry_t	*entry,
	xfs_dahash_t		last_hashval,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino)
{
	xfs_attr_leaf_name_local_t *local;

	local = xfs_attr3_leaf_name_local(leaf, i);
	if (local->namelen == 0 ||
	    !libxfs_attr_namecheck(local->nameval,
				   local->namelen)) {
		do_warn(
	_("attribute entry %d in attr block %u, inode %" PRIu64 " has bad name (namelen = %d)\n"),
			i, da_bno, ino, local->namelen);
		return -1;
	}

	/* Check on the hash value. Checking order of values
	 * is not necessary, since one wrong clears the whole
	 * fork. If the ordering's wrong, it's caught here or
	 * the kernel code has a bug with transaction logging
	 * or attributes itself. Being paranoid, let's check
	 * ordering anyway in case both the name value and the
	 * hashvalue were wrong but matched. Unlikely, however.
	 */
	if (be32_to_cpu(entry->hashval) != libxfs_da_hashname(
				&local->nameval[0], local->namelen) ||
				be32_to_cpu(entry->hashval) < last_hashval) {
		do_warn(
	_("bad hashvalue for attribute entry %d in attr block %u, inode %" PRIu64 "\n"),
			i, da_bno, ino);
		return -1;
	}

	/* Only check values for root security attributes */
	if (entry->flags & XFS_ATTR_ROOT) {
		if (valuecheck(mp, (char *)&local->nameval[0], NULL,
				local->namelen, be16_to_cpu(local->valuelen))) {
			do_warn(
	_("bad security value for attribute entry %d in attr block %u, inode %" PRIu64 "\n"),
				i, da_bno, ino);
			return -1;
		}
	}
	return xfs_attr_leaf_entsize_local(local->namelen,
						be16_to_cpu(local->valuelen));
}

static int
process_leaf_attr_remote(
	xfs_attr_leafblock_t	*leaf,
	int			i,
	xfs_attr_leaf_entry_t	*entry,
	xfs_dahash_t		last_hashval,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino,
	xfs_mount_t		*mp,
	blkmap_t		*blkmap)
{
	xfs_attr_leaf_name_remote_t *remotep;
	char*			value;

	remotep = xfs_attr3_leaf_name_remote(leaf, i);

	if (remotep->namelen == 0 ||
	    !libxfs_attr_namecheck(remotep->name,
				   remotep->namelen) ||
	    be32_to_cpu(entry->hashval) !=
			libxfs_da_hashname((unsigned char *)&remotep->name[0],
					   remotep->namelen) ||
	    be32_to_cpu(entry->hashval) < last_hashval ||
	    be32_to_cpu(remotep->valueblk) == 0) {
		do_warn(
	_("inconsistent remote attribute entry %d in attr block %u, ino %" PRIu64 "\n"), i, da_bno, ino);
		return -1;
	}

	value = malloc(be32_to_cpu(remotep->valuelen));
	if (value == NULL) {
		do_warn(
	_("cannot malloc enough for remotevalue attribute for inode %" PRIu64 "\n"),
			ino);
		do_warn(_("SKIPPING this remote attribute\n"));
		goto out;
	}
	if (rmtval_get(mp, ino, blkmap, be32_to_cpu(remotep->valueblk),
				be32_to_cpu(remotep->valuelen), value)) {
		do_warn(
	_("remote attribute get failed for entry %d, inode %" PRIu64 "\n"),
			i, ino);
		goto bad_free_out;
	}
	if ((entry->flags & XFS_ATTR_ROOT) &&
	    valuecheck(mp, (char *)&remotep->name[0], value, remotep->namelen,
				be32_to_cpu(remotep->valuelen))) {
		do_warn(
	_("remote attribute value check failed for entry %d, inode %" PRIu64 "\n"),
			i, ino);
		goto bad_free_out;
	}
	free(value);
out:
	return xfs_attr_leaf_entsize_remote(remotep->namelen);

bad_free_out:
	free(value);
	return -1;
}

static int
process_leaf_attr_block(
	xfs_mount_t	*mp,
	xfs_attr_leafblock_t *leaf,
	xfs_dablk_t	da_bno,
	xfs_ino_t	ino,
	blkmap_t	*blkmap,
	xfs_dahash_t	last_hashval,
	xfs_dahash_t	*current_hashval,
	int		*repair)
{
	xfs_attr_leaf_entry_t *entry;
	int  i, start, stop, clearit, usedbs, firstb, thissize;
	da_freemap_t *attr_freemap;
	struct xfs_attr3_icleaf_hdr leafhdr;

	xfs_attr3_leaf_hdr_from_disk(mp->m_attr_geo, &leafhdr, leaf);
	clearit = usedbs = 0;
	firstb = mp->m_sb.sb_blocksize;
	stop = xfs_attr3_leaf_hdr_size(leaf);

	/*
	 * Empty leaf blocks at offset zero can occur as a race between
	 * setxattr and the system going down, so we only take action if we're
	 * running in modify mode.  See xfs_attr3_leaf_verify for details of
	 * how we've screwed this up many times.
	 */
	if (!leafhdr.count && da_bno == 0) {
		if (no_modify) {
			do_log(
	_("would clear empty leaf attr block 0, inode %" PRIu64 "\n"),
					ino);
			return 0;
		}

		do_warn(
	_("will clear empty leaf attr block 0, inode %" PRIu64 "\n"),
				ino);
		return 1;
	}

	/* does the count look sorta valid? */
	if (!leafhdr.count ||
	    leafhdr.count * sizeof(xfs_attr_leaf_entry_t) + stop >
						mp->m_sb.sb_blocksize) {
		do_warn(
	_("bad attribute count %d in attr block %u, inode %" PRIu64 "\n"),
			leafhdr.count, da_bno, ino);
		return 1;
	}

	attr_freemap = alloc_da_freemap(mp);
	(void) set_da_freemap(mp, attr_freemap, 0, stop);

	/* go thru each entry checking for problems */
	for (i = 0, entry = xfs_attr3_leaf_entryp(leaf);
			i < leafhdr.count; i++, entry++) {

		/* check if index is within some boundary. */
		if (be16_to_cpu(entry->nameidx) > mp->m_sb.sb_blocksize) {
			do_warn(
	_("bad attribute nameidx %d in attr block %u, inode %" PRIu64 "\n"),
				be16_to_cpu(entry->nameidx), da_bno, ino);
			clearit = 1;
			break;
		}

		if (entry->flags & XFS_ATTR_INCOMPLETE) {
			/* we are inconsistent state. get rid of us */
			do_warn(
	_("attribute entry #%d in attr block %u, inode %" PRIu64 " is INCOMPLETE\n"),
				i, da_bno, ino);
			clearit = 1;
			break;
		}

		/* mark the entry used */
		start = (intptr_t)entry - (intptr_t)leaf;
		stop = start + sizeof(xfs_attr_leaf_entry_t);
		if (set_da_freemap(mp, attr_freemap, start, stop))  {
			do_warn(
	_("attribute entry %d in attr block %u, inode %" PRIu64 " claims already used space\n"),
				i, da_bno, ino);
			clearit = 1;
			break;	/* got an overlap */
		}

		if (entry->flags & XFS_ATTR_LOCAL)
			thissize = process_leaf_attr_local(mp, leaf, i, entry,
						last_hashval, da_bno, ino);
		else
			thissize = process_leaf_attr_remote(leaf, i, entry,
						last_hashval, da_bno, ino,
						mp, blkmap);
		if (thissize < 0) {
			clearit = 1;
			break;
		}

		*current_hashval = last_hashval = be32_to_cpu(entry->hashval);

		if (set_da_freemap(mp, attr_freemap, be16_to_cpu(entry->nameidx),
				be16_to_cpu(entry->nameidx) + thissize)) {
			do_warn(
	_("attribute entry %d in attr block %u, inode %" PRIu64 " claims used space\n"),
				i, da_bno, ino);
			clearit = 1;
			break;	/* got an overlap */
		}
		usedbs += thissize;
		if (be16_to_cpu(entry->nameidx) < firstb)
			firstb = be16_to_cpu(entry->nameidx);

	} /* end the loop */

	if (!clearit) {
		/* verify the header information is correct */

		/* if the holes flag is set, don't reset first_used unless it's
		 * pointing to used bytes.  we're being conservative here
		 * since the block will get compacted anyhow by the kernel.
		 */

		if ((leafhdr.holes == 0 &&
				firstb != leafhdr.firstused) ||
		    		leafhdr.firstused > firstb)  {
			if (!no_modify)  {
				do_warn(
	_("- resetting first used heap value from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					leafhdr.firstused,
					firstb, da_bno, ino);
				leafhdr.firstused = firstb;
				*repair = 1;
			} else  {
				do_warn(
	_("- would reset first used value from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					leafhdr.firstused,
					firstb, da_bno, ino);
			}
		}

		if (usedbs != leafhdr.usedbytes)  {
			if (!no_modify)  {
				do_warn(
	_("- resetting usedbytes cnt from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					leafhdr.usedbytes,
					usedbs, da_bno, ino);
				leafhdr.usedbytes = usedbs;
				*repair = 1;
			} else  {
				do_warn(
	_("- would reset usedbytes cnt from %d to %d in "
	  "block %u of attribute fork of %" PRIu64 "\n"),
					leafhdr.usedbytes,
					usedbs, da_bno, ino);
			}
		}

		/* there's a lot of work in process_leaf_dir_block to go thru
		* checking for holes and compacting if appropiate. I don't think
		* attributes need all that, so let's just leave the holes. If
		* we discover later that this is a good place to do compaction
		* we can add it then.
		*/
	}
	/*
	 * If we're just going to zap the block, don't pretend like we
	 * repaired it, because repairing the block stops the clear
	 * operation.
	 */
	if (clearit)
		*repair = 0;
	if (*repair)
		xfs_attr3_leaf_hdr_to_disk(mp->m_attr_geo, leaf, &leafhdr);

	free(attr_freemap);
	return (clearit);  /* and repair */
}


/*
 * returns 0 if the attribute fork is ok, 1 if it has to be junked.
 */
static int
process_leaf_attr_level(xfs_mount_t	*mp,
			da_bt_cursor_t	*da_cursor)
{
	int			repair;
	xfs_attr_leafblock_t	*leaf;
	struct xfs_buf		*bp;
	xfs_ino_t		ino;
	xfs_fsblock_t		dev_bno;
	xfs_dablk_t		da_bno;
	xfs_dablk_t		prev_bno;
	xfs_dahash_t		current_hashval = 0;
	xfs_dahash_t		greatest_hashval;
	struct xfs_attr3_icleaf_hdr leafhdr;
	int			error;

	da_bno = da_cursor->level[0].bno;
	ino = da_cursor->ino;
	/*
	 * 0 is the root block and no block
	 * pointer can point to the root block of the btree
	 */
	if (da_bno == 0) {
		do_warn(
	_("btree cycle detected in attribute fork for inode %" PRIu64 "\n"),
			ino);
		goto error_out;
	}

	prev_bno = 0;

	do {
		repair = 0;
		dev_bno = blkmap_get(da_cursor->blkmap, da_bno);
		if (dev_bno == NULLFSBLOCK) {
			do_warn(
	_("can't map block %u for attribute fork for inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}

		error = -libxfs_buf_read(mp->m_dev,
				XFS_FSB_TO_DADDR(mp, dev_bno),
				XFS_FSB_TO_BB(mp, 1), LIBXFS_READBUF_SALVAGE,
				&bp, &xfs_attr3_leaf_buf_ops);
		if (error) {
			do_warn(
	_("can't read file block %u (fsbno %" PRIu64 ") for attribute fork of inode %" PRIu64 "\n"),
				da_bno, dev_bno, ino);
			goto error_out;
		}

		leaf = bp->b_addr;
		xfs_attr3_leaf_hdr_from_disk(mp->m_attr_geo, &leafhdr, leaf);

		/* check magic number for leaf directory btree block */
		if (!(leafhdr.magic == XFS_ATTR_LEAF_MAGIC ||
		      leafhdr.magic == XFS_ATTR3_LEAF_MAGIC)) {
			do_warn(
	_("bad attribute leaf magic %#x for inode %" PRIu64 "\n"),
				 leafhdr.magic, ino);
			libxfs_buf_relse(bp);
			goto error_out;
		}

		/*
		 * for each block, process the block, verify its path,
		 * then get next block.  update cursor values along the way
		 */
		if (process_leaf_attr_block(mp, leaf, da_bno, ino,
				da_cursor->blkmap, current_hashval,
				&greatest_hashval, &repair))  {
			libxfs_buf_relse(bp);
			goto error_out;
		}

		/*
		 * index can be set to hdr.count so match the
		 * indexes of the interior blocks -- which at the
		 * end of the block will point to 1 after the final
		 * real entry in the block
		 */
		da_cursor->level[0].hashval = greatest_hashval;
		da_cursor->level[0].bp = bp;
		da_cursor->level[0].bno = da_bno;
		da_cursor->level[0].index = leafhdr.count;
		da_cursor->level[0].dirty = repair;

		if (leafhdr.back != prev_bno)  {
			do_warn(
	_("bad sibling back pointer for block %u in attribute fork for inode %" PRIu64 "\n"),
				da_bno, ino);
			libxfs_buf_relse(bp);
			goto error_out;
		}

		prev_bno = da_bno;
		da_bno = leafhdr.forw;

		if (da_bno != 0) {
			if (verify_da_path(mp, da_cursor, 0, XFS_ATTR_FORK)) {
				libxfs_buf_relse(bp);
				goto error_out;
			}
		}

		current_hashval = greatest_hashval;
                /*
		 * If block looks ok but CRC didn't match, make sure to
		 * recompute it.
		 */
		if (!no_modify && bp->b_error == -EFSBADCRC)
			repair++;

		if (repair && !no_modify) {
			libxfs_buf_mark_dirty(bp);
			libxfs_buf_relse(bp);
		}
		else
			libxfs_buf_relse(bp);
	} while (da_bno != 0);

	if (verify_final_da_path(mp, da_cursor, 0, XFS_ATTR_FORK))  {
		/*
		 * verify the final path up (right-hand-side) if still ok
		 */
		do_warn(
	_("bad hash path in attribute fork for inode %" PRIu64 "\n"),
			da_cursor->ino);
		goto error_out;
	}

	/* releases all buffers holding interior btree blocks */
	release_da_cursor(mp, da_cursor, 0);
	return(0);

error_out:
	/* release all buffers holding interior btree blocks */
	err_release_da_cursor(mp, da_cursor, 0);
	return(1);
}


/*
 * a node directory is a true btree  -- where the attribute fork
 * has gotten big enough that it is represented as a non-trivial (e.g.
 * has more than just a block) btree.
 *
 * Note that if we run into any problems, we will trash the attribute fork.
 *
 * returns 0 if things are ok, 1 if bad
 * Note this code has been based off process_node_dir.
 */
static int
process_node_attr(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	blkmap_t		*blkmap)
{
	xfs_dablk_t		bno;
	int			error = 0;
	da_bt_cursor_t		da_cursor;

	/*
	 * try again -- traverse down left-side of tree until we hit
	 * the left-most leaf block setting up the btree cursor along
	 * the way.  Then walk the leaf blocks left-to-right, calling
	 * a parent-verification routine each time we traverse a block.
	 */
	memset(&da_cursor, 0, sizeof(da_bt_cursor_t));
	da_cursor.active = 0;
	da_cursor.ino = ino;
	da_cursor.dip = dip;
	da_cursor.greatest_bno = 0;
	da_cursor.blkmap = blkmap;

	/*
	 * now process interior node. don't have any buffers held in this path.
	 */
	error = traverse_int_dablock(mp, &da_cursor, &bno, XFS_ATTR_FORK);
	if (error == 0)
		return(1);  /* 0 means unsuccessful */

	/*
	 * now pass cursor and bno into leaf-block processing routine
	 * the leaf dir level routine checks the interior paths
	 * up to the root including the final right-most path.
	 */

	return (process_leaf_attr_level(mp, &da_cursor));
}

/* check v5 metadata */
static int
__check_attr_header(
	struct xfs_mount	*mp,
	struct xfs_buf		*bp,
	xfs_ino_t		ino)
{
	struct xfs_da3_blkinfo	*info = bp->b_addr;

	if (info->hdr.magic != cpu_to_be16(XFS_ATTR3_LEAF_MAGIC) &&
	    info->hdr.magic != cpu_to_be16(XFS_DA3_NODE_MAGIC))
		return 0;

	/* verify owner */
	if (be64_to_cpu(info->owner) != ino) {
		do_warn(
_("expected owner inode %" PRIu64 ", got %llu, attr block %" PRIu64 "\n"),
			ino, (unsigned long long)be64_to_cpu(info->owner),
			xfs_buf_daddr(bp));
		return 1;
	}
	/* verify block number */
	if (be64_to_cpu(info->blkno) != xfs_buf_daddr(bp)) {
		do_warn(
_("expected block %" PRIu64 ", got %llu, inode %" PRIu64 "attr block\n"),
			xfs_buf_daddr(bp), (unsigned long long)be64_to_cpu(info->blkno),
			ino);
		return 1;
	}
	/* verify uuid */
	if (platform_uuid_compare(&info->uuid, &mp->m_sb.sb_meta_uuid) != 0) {
		do_warn(
_("wrong FS UUID, inode %" PRIu64 " attr block %" PRIu64 "\n"),
			ino, xfs_buf_daddr(bp));
		return 1;
	}

	return 0;
}

static int
process_longform_leaf_root(
	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	struct blkmap		*blkmap,
	int			*repair,
	struct xfs_buf		*bp)
{
	struct xfs_attr3_icleaf_hdr leafhdr;
	xfs_dahash_t		next_hashval;
	int			badness;
	int			repairlinks = 0;

	/*
	 * check sibling pointers in leaf block or root block 0 before
	 * we have to release the btree block
	 */
	xfs_attr3_leaf_hdr_from_disk(mp->m_attr_geo, &leafhdr, bp->b_addr);
	if (leafhdr.forw != 0 || leafhdr.back != 0)  {
		if (!no_modify)  {
			do_warn(
_("clearing forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"),
				ino);
			repairlinks = 1;
			leafhdr.forw = 0;
			leafhdr.back = 0;
			xfs_attr3_leaf_hdr_to_disk(mp->m_attr_geo, bp->b_addr,
					&leafhdr);
		} else  {
			do_warn(
_("would clear forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"), ino);
		}
	}

	badness = process_leaf_attr_block(mp, bp->b_addr, 0, ino, blkmap, 0,
			&next_hashval, repair);
	if (badness) {
		*repair = 0;
		/* the block is bad.  lose the attribute fork. */
		libxfs_buf_relse(bp);
		return 1;
	}

	*repair = *repair || repairlinks;

	if (*repair && !no_modify)
		libxfs_buf_mark_dirty(bp);
	libxfs_buf_relse(bp);

	return 0;
}

static int
process_longform_da_root(
	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	struct blkmap		*blkmap,
	int			*repair,
	struct xfs_buf		*bp)
{
	struct xfs_da3_icnode_hdr	da3_hdr;
	int			repairlinks = 0;
	int			error;

	libxfs_da3_node_hdr_from_disk(mp, &da3_hdr, bp->b_addr);
	/*
	 * check sibling pointers in leaf block or root block 0 before
	 * we have to release the btree block
	 */
	if (da3_hdr.forw != 0 || da3_hdr.back != 0)  {
		if (!no_modify)  {
			do_warn(
_("clearing forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"),
				ino);

			repairlinks = 1;
			da3_hdr.forw = 0;
			da3_hdr.back = 0;
			xfs_da3_node_hdr_to_disk(mp, bp->b_addr, &da3_hdr);
		} else  {
			do_warn(
_("would clear forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"), ino);
		}
	}

	/* must do this now, to release block 0 before the traversal */
	if ((*repair || repairlinks) && !no_modify) {
		*repair = 1;
		libxfs_buf_mark_dirty(bp);
	}
	libxfs_buf_relse(bp);
	error = process_node_attr(mp, ino, dip, blkmap); /* + repair */
	if (error)
		*repair = 0;
	return error;
}

/*
 * Start processing for a leaf or fuller btree.
 * A leaf directory is one where the attribute fork is too big for
 * the inode  but is small enough to fit into one btree block
 * outside the inode. This code is modelled after process_leaf_dir_block.
 *
 * returns 0 if things are ok, 1 if bad (attributes needs to be junked)
 * repair is set, if anything was changed, but attributes can live thru it
 */
static int
process_longform_attr(
	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	struct blkmap		*blkmap,
	int			*repair) /* out - 1 if something was fixed */
{
	xfs_fsblock_t		bno;
	struct xfs_buf		*bp;
	struct xfs_da_blkinfo	*info;
	int			error;

	*repair = 0;

	bno = blkmap_get(blkmap, 0);
	if (bno == NULLFSBLOCK) {
		if (dip->di_aformat == XFS_DINODE_FMT_EXTENTS &&
				xfs_dfork_attr_extents(dip) == 0)
			return(0); /* the kernel can handle this state */
		do_warn(
	_("block 0 of inode %" PRIu64 " attribute fork is missing\n"),
			ino);
		return 1;
	}

	/* FIX FOR bug 653709 -- EKN */
	if (!libxfs_verify_fsbno(mp, bno)) {
		do_warn(
	_("block in attribute fork of inode %" PRIu64 " is not valid\n"), ino);
		return 1;
	}

	error = -libxfs_buf_read(mp->m_dev, XFS_FSB_TO_DADDR(mp, bno),
			XFS_FSB_TO_BB(mp, 1), LIBXFS_READBUF_SALVAGE, &bp,
			&xfs_da3_node_buf_ops);
	if (error) {
		do_warn(
	_("can't read block 0 of inode %" PRIu64 " attribute fork\n"),
			ino);
		return 1;
	}

	if (bp->b_error == -EFSCORRUPTED) {
		do_warn(
	_("corrupt block 0 of inode %" PRIu64 " attribute fork\n"),
			ino);
		libxfs_buf_relse(bp);
		return 1;
	}

	if (bp->b_error == -EFSBADCRC)
		(*repair)++;

	/* is this block sane? */
	if (__check_attr_header(mp, bp, ino)) {
		*repair = 0;
		libxfs_buf_relse(bp);
		return 1;
	}

	/*
	 * use magic number to tell us what type of attribute this is.
	 * it's possible to have a node or leaf attribute in either an
	 * extent format or btree format attribute fork.
	 */
	info = bp->b_addr;
	switch (be16_to_cpu(info->magic)) {
	case XFS_ATTR_LEAF_MAGIC:	/* leaf-form attribute */
	case XFS_ATTR3_LEAF_MAGIC:
		return process_longform_leaf_root(mp, ino, dip, blkmap, repair,
				bp);
	case XFS_DA_NODE_MAGIC:		/* btree-form attribute */
	case XFS_DA3_NODE_MAGIC:
		return process_longform_da_root(mp, ino, dip, blkmap, repair,
				bp);
	default:
		do_warn(
	_("bad attribute leaf magic # %#x for dir ino %" PRIu64 "\n"),
			be16_to_cpu(info->magic), ino);
		libxfs_buf_relse(bp);
		*repair = 0;
		return 1;
	}

	return 0; /* should never get here */
}


static int
xfs_acl_from_disk(
	struct xfs_mount	*mp,
	struct xfs_icacl	**aclp,
	struct xfs_acl		*dacl)
{
	struct xfs_icacl	*acl;
	struct xfs_icacl_entry	*ace;
	struct xfs_acl_entry	*dace;
	int			count;
	int			i;

	count = be32_to_cpu(dacl->acl_cnt);
	if (count > XFS_ACL_MAX_ENTRIES(mp)) {
		do_warn(_("Too many ACL entries, count %d\n"), count);
		*aclp = NULL;
		return EINVAL;
	}


	acl = malloc(sizeof(struct xfs_icacl) +
		     count * sizeof(struct xfs_icacl_entry));
	if (!acl) {
		do_warn(_("cannot malloc enough for ACL attribute\n"));
		do_warn(_("SKIPPING this ACL\n"));
		*aclp = NULL;
		return ENOMEM;
	}

	acl->acl_cnt = count;
	for (i = 0; i < count; i++) {
		ace = &acl->acl_entry[i];
		dace = &dacl->acl_entry[i];

		ace->ae_tag = be32_to_cpu(dace->ae_tag);
		ace->ae_id = be32_to_cpu(dace->ae_id);
		ace->ae_perm = be16_to_cpu(dace->ae_perm);
	}

	*aclp = acl;
	return 0;
}

/*
 * returns 1 if attributes got cleared
 * and 0 if things are ok.
 */
int
process_attributes(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	blkmap_t		*blkmap,
	int			*repair)  /* returned if we did repair */
{
	int			err;
	__u8			aformat = dip->di_aformat;
#ifdef DEBUG
	struct xfs_attr_shortform *asf;

	asf = (struct xfs_attr_shortform *) XFS_DFORK_APTR(dip);
#endif

	if (aformat == XFS_DINODE_FMT_LOCAL) {
		ASSERT(be16_to_cpu(asf->hdr.totsize) <=
			XFS_DFORK_ASIZE(dip, mp));
		err = process_shortform_attr(mp, ino, dip, repair);
	} else if (aformat == XFS_DINODE_FMT_EXTENTS ||
					aformat == XFS_DINODE_FMT_BTREE)  {
			err = process_longform_attr(mp, ino, dip, blkmap,
				repair);
			/* if err, convert this to shortform and clear it */
			/* if repair and no error, it's taken care of */
	} else  {
		do_warn(_("illegal attribute format %d, ino %" PRIu64 "\n"),
			aformat, ino);
		err = 1;
	}
	return (err);  /* and repair */
}

/*
 * Validate an ACL
 */
static int
xfs_acl_valid(
	struct xfs_mount *mp,
	struct xfs_acl	*daclp)
{
	struct xfs_icacl	*aclp = NULL;
	struct xfs_icacl_entry	*entry, *e;
	int user = 0, group = 0, other = 0, mask = 0, mask_required = 0;
	int i, j;

	if (daclp == NULL)
		goto acl_invalid;

	switch (xfs_acl_from_disk(mp, &aclp, daclp)) {
	case ENOMEM:
		return 0;
	case EINVAL:
		goto acl_invalid;
	default:
		break;
	}

	for (i = 0; i < aclp->acl_cnt; i++) {
		entry = &aclp->acl_entry[i];
		if (entry->ae_perm & ~(ACL_READ|ACL_WRITE|ACL_EXECUTE))
			goto acl_invalid;
		switch (entry->ae_tag) {
			case ACL_USER_OBJ:
				if (user++)
					goto acl_invalid;
				break;
			case ACL_GROUP_OBJ:
				if (group++)
					goto acl_invalid;
				break;
			case ACL_OTHER:
				if (other++)
					goto acl_invalid;
				break;
			case ACL_USER:
			case ACL_GROUP:
				for (j = i + 1; j < aclp->acl_cnt; j++) {
					e = &aclp->acl_entry[j];
					if (e->ae_id == entry->ae_id &&
					    e->ae_tag == entry->ae_tag)
						goto acl_invalid;
				}
				mask_required++;
				break;
			case ACL_MASK:
				if (mask++)
					goto acl_invalid;
				break;
			default:
				goto acl_invalid;
		}
	}
	if (!user || !group || !other || (mask_required && !mask))
		goto acl_invalid;
	free(aclp);
	return 0;
acl_invalid:
	free(aclp);
	errno = EINVAL;
	return (-1);
}

/*
 * Check a category or division set to ensure that all values are in
 * ascending order and each division or category appears only once.
 */
static int
__check_setvalue(const unsigned short *list, unsigned short count)
{
	unsigned short i;

	for (i = 1; i < count ; i++)
		if (list[i] <= list[i-1])
			return -1;
	return 0;
}

/*
 * xfs_mac_valid(lp)
 * Check the validity of a MAC label.
 */
static int
xfs_mac_valid(xfs_mac_label_t *lp)
{
	if (lp == NULL)
		return (0);

	/*
	 * if the total category set and division set is greater than 250
	 * report error
	 */
	if ((lp->ml_catcount + lp->ml_divcount) > XFS_MAC_MAX_SETS)
		return(0);

	/*
	 * check whether the msentype value is valid, and do they have
	 * appropriate level, category association.
	 */
	switch (lp->ml_msen_type) {
		case XFS_MSEN_ADMIN_LABEL:
		case XFS_MSEN_EQUAL_LABEL:
		case XFS_MSEN_HIGH_LABEL:
		case XFS_MSEN_MLD_HIGH_LABEL:
		case XFS_MSEN_LOW_LABEL:
		case XFS_MSEN_MLD_LOW_LABEL:
			if (lp->ml_level != 0 || lp->ml_catcount > 0 )
				return (0);
			break;
		case XFS_MSEN_TCSEC_LABEL:
		case XFS_MSEN_MLD_LABEL:
			if (lp->ml_catcount > 0 &&
			    __check_setvalue(lp->ml_list,
					     lp->ml_catcount) == -1)
				return (0);
			break;
		case XFS_MSEN_UNKNOWN_LABEL:
		default:
			return (0);
	}

	/*
	 * check whether the minttype value is valid, and do they have
	 * appropriate grade, division association.
	 */
	switch (lp->ml_mint_type) {
		case XFS_MINT_BIBA_LABEL:
			if (lp->ml_divcount > 0 &&
			    __check_setvalue(lp->ml_list + lp->ml_catcount,
					     lp->ml_divcount) == -1)
				return(0);
			break;
		case XFS_MINT_EQUAL_LABEL:
		case XFS_MINT_HIGH_LABEL:
		case XFS_MINT_LOW_LABEL:
			if (lp->ml_grade != 0 || lp->ml_divcount > 0 )
				return(0);
			break;
		default:
			return(0);
	}

	return (1);
}
