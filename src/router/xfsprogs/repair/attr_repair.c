/*
 * Copyright (c) 2000-2002,2004-2005 Silicon Graphics, Inc.
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

#include <libxfs.h>
#include "globals.h"
#include "err_protos.h"
#include "attr_repair.h"
#include "dir.h"
#include "dinode.h"
#include "bmap.h"
#include "protos.h"

static int xfs_acl_valid(xfs_acl_disk_t *daclp);
static int xfs_mac_valid(xfs_mac_label_t *lp);


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

int
valuecheck(char *namevalue, char *value, int namelen, int valuelen)
{
	/* for proper alignment issues, get the structs and memmove the values */
	xfs_mac_label_t macl;
	xfs_acl_t thisacl;
	void *valuep;
	int clearit = 0;

	if ((strncmp(namevalue, SGI_ACL_FILE, SGI_ACL_FILE_SIZE) == 0) ||
			(strncmp(namevalue, SGI_ACL_DEFAULT,
				SGI_ACL_DEFAULT_SIZE) == 0)) {
		if (value == NULL) {
			memset(&thisacl, 0, sizeof(xfs_acl_t));
			memmove(&thisacl, namevalue+namelen, valuelen);
			valuep = &thisacl;
		} else
			valuep = value;

		if (xfs_acl_valid((xfs_acl_disk_t *)valuep) != 0) {
			clearit = 1;
			do_warn(
	_("entry contains illegal value in attribute named SGI_ACL_FILE "
	  "or SGI_ACL_DEFAULT\n"));
		}
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
int
process_shortform_attr(
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		*repair)
{
	xfs_attr_shortform_t	*asf;
	xfs_attr_sf_entry_t	*currententry, *nextentry, *tempentry;
	int			i, junkit;
	int			currentsize, remainingspace;

	*repair = 0;

	asf = (xfs_attr_shortform_t *) XFS_DFORK_APTR(dip);

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
		if ((currentsize + (sizeof(xfs_attr_sf_entry_t) - 1)) >
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

		/* namecheck checks for / and null terminated for file names.
		 * attributes names currently follow the same rules.
		*/
		if (namecheck((char *)&currententry->nameval[0],
						currententry->namelen))  {
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
		       junkit = valuecheck((char *)&currententry->nameval[0],
					NULL, currententry->namelen, 
					currententry->valuelen);

		remainingspace = remainingspace -
					XFS_ATTR_SF_ENTSIZE(currententry);

		if (junkit) {
			if (!no_modify) {
				/* get rid of only this entry */
				do_warn(
	_("removing attribute entry %d for inode %" PRIu64 "\n"),
					i, ino);
				tempentry = (xfs_attr_sf_entry_t *)
					((__psint_t) currententry +
					 XFS_ATTR_SF_ENTSIZE(currententry));
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
		nextentry = (xfs_attr_sf_entry_t *)((__psint_t) nextentry +
			 		XFS_ATTR_SF_ENTSIZE(currententry));
		currentsize = currentsize + XFS_ATTR_SF_ENTSIZE(currententry);

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
	xfs_dfsbno_t	bno;
	xfs_buf_t	*bp;
	int		clearit = 0, i = 0, length = 0, amountdone = 0;

	/* ASSUMPTION: valuelen is a valid number, so use it for looping */
	/* Note that valuelen is not a multiple of blocksize */
	while (amountdone < valuelen) {
		bno = blkmap_get(blkmap, blocknum + i);
		if (bno == NULLDFSBNO) {
			do_warn(
	_("remote block for attributes of inode %" PRIu64 " is missing\n"), ino);
			clearit = 1;
			break;
		}
		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, bno),
				XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_warn(
	_("can't read remote block for attributes of inode %" PRIu64 "\n"), ino);
			clearit = 1;
			break;
		}
		ASSERT(mp->m_sb.sb_blocksize == XFS_BUF_COUNT(bp));
		length = MIN(XFS_BUF_COUNT(bp), valuelen - amountdone);
		memmove(value, XFS_BUF_PTR(bp), length);
		amountdone += length;
		value += length;
		i++;
		libxfs_putbuf(bp);
	}
	return (clearit);
}

/*
 * freespace map for directory and attribute leaf blocks (1 bit per byte)
 * 1 == used, 0 == free
 */
size_t ts_attr_freemap_size = sizeof(da_freemap_t) * DA_BMAP_SIZE;

/* The block is read in. The magic number and forward / backward
 * links are checked by the caller process_leaf_attr.
 * If any problems occur the routine returns with non-zero. In
 * this case the next step is to clear the attribute fork, by
 * changing it to shortform and zeroing it out. Forkoff need not
 * be changed.
 */

static int
process_leaf_attr_local(
	xfs_attr_leafblock_t	*leaf,
	int			i,
	xfs_attr_leaf_entry_t	*entry,
	xfs_dahash_t		last_hashval,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino)
{
	xfs_attr_leaf_name_local_t *local;

	local = xfs_attr_leaf_name_local(leaf, i);
	if (local->namelen == 0 || namecheck((char *)&local->nameval[0], 
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
		if (valuecheck((char *)&local->nameval[0], NULL, 
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

	remotep = xfs_attr_leaf_name_remote(leaf, i);

	if (remotep->namelen == 0 || namecheck((char *)&remotep->name[0], 
						remotep->namelen) || 
			be32_to_cpu(entry->hashval) != 
				libxfs_da_hashname((uchar_t *)&remotep->name[0], 
						remotep->namelen) ||
			be32_to_cpu(entry->hashval) < last_hashval ||
			be32_to_cpu(remotep->valueblk) == 0) {
		do_warn(
	_("inconsistent remote attribute entry %d in attr block %u, ino %" PRIu64 "\n"), i, da_bno, ino);
		return -1;
	}

	if (!(entry->flags & XFS_ATTR_ROOT))
		goto out;
 
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
	if (valuecheck((char *)&remotep->name[0], value, remotep->namelen,
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

int
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
	da_freemap_t *attr_freemap = ts_attr_freemap();

	clearit = usedbs = 0;
	*repair = 0;
	firstb = mp->m_sb.sb_blocksize;
	stop = sizeof(xfs_attr_leaf_hdr_t);

	/* does the count look sorta valid? */
	if (be16_to_cpu(leaf->hdr.count) * sizeof(xfs_attr_leaf_entry_t)
			+ sizeof(xfs_attr_leaf_hdr_t) > XFS_LBSIZE(mp)) {
		do_warn(
	_("bad attribute count %d in attr block %u, inode %" PRIu64 "\n"),
			be16_to_cpu(leaf->hdr.count), da_bno, ino);
		return (1);
	}

	init_da_freemap(attr_freemap);
	(void) set_da_freemap(mp, attr_freemap, 0, stop);

	/* go thru each entry checking for problems */
	for (i = 0, entry = &leaf->entries[0]; 
			i < be16_to_cpu(leaf->hdr.count); i++, entry++) {

		/* check if index is within some boundary. */
		if (be16_to_cpu(entry->nameidx) > XFS_LBSIZE(mp)) {
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
		start = (__psint_t)&leaf->entries[i] - (__psint_t)leaf;
		stop = start + sizeof(xfs_attr_leaf_entry_t);
		if (set_da_freemap(mp, attr_freemap, start, stop))  {
			do_warn(
	_("attribute entry %d in attr block %u, inode %" PRIu64 " claims already used space\n"),
				i, da_bno, ino);
			clearit = 1;
			break;	/* got an overlap */
		}

		if (entry->flags & XFS_ATTR_LOCAL) 
			thissize = process_leaf_attr_local(leaf, i, entry,
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

		if ((leaf->hdr.holes == 0 && 
				firstb != be16_to_cpu(leaf->hdr.firstused)) ||
		    		be16_to_cpu(leaf->hdr.firstused) > firstb)  {
			if (!no_modify)  {
				do_warn(
	_("- resetting first used heap value from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.firstused), 
					firstb, da_bno, ino);
				leaf->hdr.firstused = cpu_to_be16(firstb);
				*repair = 1;
			} else  {
				do_warn(
	_("- would reset first used value from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.firstused), 
					firstb, da_bno, ino);
			}
		}

		if (usedbs != be16_to_cpu(leaf->hdr.usedbytes))  {
			if (!no_modify)  {
				do_warn(
	_("- resetting usedbytes cnt from %d to %d in "
	  "block %u of attribute fork of inode %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.usedbytes), 
					usedbs, da_bno, ino);
				leaf->hdr.usedbytes = cpu_to_be16(usedbs);
				*repair = 1;
			} else  {
				do_warn(
	_("- would reset usedbytes cnt from %d to %d in "
	  "block %u of attribute fork of %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.usedbytes), 
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
	return (clearit);  /* and repair */
}


/*
 * returns 0 if the attribute fork is ok, 1 if it has to be junked.
 */
int
process_leaf_attr_level(xfs_mount_t	*mp,
			da_bt_cursor_t	*da_cursor)
{
	int			repair;
	xfs_attr_leafblock_t	*leaf;
	xfs_buf_t		*bp;
	xfs_ino_t		ino;
	xfs_dfsbno_t		dev_bno;
	xfs_dablk_t		da_bno;
	xfs_dablk_t		prev_bno;
	xfs_dahash_t		current_hashval = 0;
	xfs_dahash_t		greatest_hashval;

	da_bno = da_cursor->level[0].bno;
	ino = da_cursor->ino;
	prev_bno = 0;

	do {
		repair = 0;
		dev_bno = blkmap_get(da_cursor->blkmap, da_bno);
		/*
		 * 0 is the root block and no block
		 * pointer can point to the root block of the btree
		 */
		ASSERT(da_bno != 0);

		if (dev_bno == NULLDFSBNO) {
			do_warn(
	_("can't map block %u for attribute fork for inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}

		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, dev_bno),
					XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_warn(
	_("can't read file block %u (fsbno %" PRIu64 ") for attribute fork of inode %" PRIu64 "\n"),
				da_bno, dev_bno, ino);
			goto error_out;
		}

		leaf = (xfs_attr_leafblock_t *)XFS_BUF_PTR(bp);

		/* check magic number for leaf directory btree block */
		if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC) {
			do_warn(
	_("bad attribute leaf magic %#x for inode %" PRIu64 "\n"),
				 leaf->hdr.info.magic, ino);
			libxfs_putbuf(bp);
			goto error_out;
		}

		/*
		 * for each block, process the block, verify its path,
		 * then get next block.  update cursor values along the way
		 */
		if (process_leaf_attr_block(mp, leaf, da_bno, ino,
				da_cursor->blkmap, current_hashval,
				&greatest_hashval, &repair))  {
			libxfs_putbuf(bp);
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
		da_cursor->level[0].index = be16_to_cpu(leaf->hdr.count);
		da_cursor->level[0].dirty = repair;

		if (be32_to_cpu(leaf->hdr.info.back) != prev_bno)  {
			do_warn(
	_("bad sibling back pointer for block %u in attribute fork for inode %" PRIu64 "\n"),
				da_bno, ino);
			libxfs_putbuf(bp);
			goto error_out;
		}

		prev_bno = da_bno;
		da_bno = be32_to_cpu(leaf->hdr.info.forw);

		if (da_bno != 0 && verify_da_path(mp, da_cursor, 0))  {
			libxfs_putbuf(bp);
			goto error_out;
		}

		current_hashval = greatest_hashval;

		if (repair && !no_modify) 
			libxfs_writebuf(bp, 0);
		else 
			libxfs_putbuf(bp);
	} while (da_bno != 0);

	if (verify_final_da_path(mp, da_cursor, 0))  {
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
int
process_node_attr(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	blkmap_t	*blkmap)
{
	xfs_dablk_t			bno;
	int				error = 0;
	da_bt_cursor_t			da_cursor;

	/*
	 * try again -- traverse down left-side of tree until we hit
	 * the left-most leaf block setting up the btree cursor along
	 * the way.  Then walk the leaf blocks left-to-right, calling
	 * a parent-verification routine each time we traverse a block.
	 */
	memset(&da_cursor, 0, sizeof(da_bt_cursor_t));
	da_cursor.active = 0;
	da_cursor.type = 0;
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

/*
 * Start processing for a leaf or fuller btree.
 * A leaf directory is one where the attribute fork is too big for
 * the inode  but is small enough to fit into one btree block
 * outside the inode. This code is modelled after process_leaf_dir_block.
 *
 * returns 0 if things are ok, 1 if bad (attributes needs to be junked)
 * repair is set, if anything was changed, but attributes can live thru it
 */

int
process_longform_attr(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	blkmap_t	*blkmap,
	int		*repair)	/* out - 1 if something was fixed */
{
	xfs_attr_leafblock_t	*leaf;
	xfs_dfsbno_t	bno;
	xfs_buf_t	*bp;
	xfs_dahash_t	next_hashval;
	int		repairlinks = 0;

	*repair = 0;

	bno = blkmap_get(blkmap, 0);

	if ( bno == NULLDFSBNO ) {
		if (dip->di_aformat == XFS_DINODE_FMT_EXTENTS && 
				be16_to_cpu(dip->di_anextents) == 0)
			return(0); /* the kernel can handle this state */
		do_warn(
	_("block 0 of inode %" PRIu64 " attribute fork is missing\n"),
			ino);
		return(1);
	}
	/* FIX FOR bug 653709 -- EKN */
	if (mp->m_sb.sb_agcount < XFS_FSB_TO_AGNO(mp, bno)) {
		do_warn(
	_("agno of attribute fork of inode %" PRIu64 " out of regular partition\n"), ino);
		return(1);
	}

	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, bno),
				XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp) {
		do_warn(
	_("can't read block 0 of inode %" PRIu64 " attribute fork\n"),
			ino);
		return(1);
	}

	/* verify leaf block */
	leaf = (xfs_attr_leafblock_t *)XFS_BUF_PTR(bp);

	/* check sibling pointers in leaf block or root block 0 before
	* we have to release the btree block
	*/
	if (be32_to_cpu(leaf->hdr.info.forw) != 0 || 
				be32_to_cpu(leaf->hdr.info.back) != 0)  {
		if (!no_modify)  {
			do_warn(
	_("clearing forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"),
				ino);
			repairlinks = 1;
			leaf->hdr.info.forw = cpu_to_be32(0);
			leaf->hdr.info.back = cpu_to_be32(0);
		} else  {
			do_warn(
	_("would clear forw/back pointers in block 0 for attributes in inode %" PRIu64 "\n"), ino);
		}
	}

	/*
	 * use magic number to tell us what type of attribute this is.
	 * it's possible to have a node or leaf attribute in either an
	 * extent format or btree format attribute fork.
	 */
	switch (be16_to_cpu(leaf->hdr.info.magic)) {
	case XFS_ATTR_LEAF_MAGIC:	/* leaf-form attribute */
		if (process_leaf_attr_block(mp, leaf, 0, ino, blkmap,
				0, &next_hashval, repair)) {
			/* the block is bad.  lose the attribute fork. */
			libxfs_putbuf(bp);
			return(1);
		}
		*repair = *repair || repairlinks;
		break;

	case XFS_DA_NODE_MAGIC:		/* btree-form attribute */
		/* must do this now, to release block 0 before the traversal */
		if (repairlinks) {
			*repair = 1;
			libxfs_writebuf(bp, 0);
		} else
			libxfs_putbuf(bp);
		return (process_node_attr(mp, ino, dip, blkmap)); /* + repair */
	default:
		do_warn(
	_("bad attribute leaf magic # %#x for dir ino %" PRIu64 "\n"),
			be16_to_cpu(leaf->hdr.info.magic), ino);
		libxfs_putbuf(bp);
		return(1);
	}

	if (*repair && !no_modify)
		libxfs_writebuf(bp, 0);
	else
		libxfs_putbuf(bp);

	return(0);  /* repair may be set */
}


static xfs_acl_t *
xfs_acl_from_disk(xfs_acl_disk_t *dacl)
{
	int			count;
	xfs_acl_t		*acl;
	xfs_acl_entry_t 	*ace;
	xfs_acl_entry_disk_t	*dace, *end;

	count = be32_to_cpu(dacl->acl_cnt);
	end = &dacl->acl_entry[0] + count;
	acl = malloc((int)((char *)end - (char *)dacl));
	if (!acl)
		return NULL;
	acl->acl_cnt = count;
	ace = &acl->acl_entry[0];
	for (dace = &dacl->acl_entry[0]; dace < end; ace++, dace++) {
		ace->ae_tag = be32_to_cpu(dace->ae_tag);
		ace->ae_id = be32_to_cpu(dace->ae_id);
		ace->ae_perm = be16_to_cpu(dace->ae_perm);
	}
	return acl;
}

/*
 * returns 1 if attributes got cleared
 * and 0 if things are ok.
 */
int
process_attributes(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	blkmap_t	*blkmap,
	int		*repair)  /* returned if we did repair */
{
	int		err;
	__u8		aformat = dip->di_aformat;
	xfs_attr_shortform_t *asf;

	asf = (xfs_attr_shortform_t *) XFS_DFORK_APTR(dip);

	if (aformat == XFS_DINODE_FMT_LOCAL) {
		ASSERT(be16_to_cpu(asf->hdr.totsize) <=
			XFS_DFORK_ASIZE(dip, mp));
		err = process_shortform_attr(ino, dip, repair);
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
xfs_acl_valid(xfs_acl_disk_t *daclp)
{
	xfs_acl_t	*aclp;
	xfs_acl_entry_t *entry, *e;
	int user = 0, group = 0, other = 0, mask = 0, mask_required = 0;
	int i, j;

	if (daclp == NULL)
		goto acl_invalid;

	aclp = xfs_acl_from_disk(daclp);
	if (aclp == NULL) {
		do_warn(_("cannot malloc enough for ACL attribute\n"));
		do_warn(_("SKIPPING this ACL\n"));
		return 0;
	}

	if (aclp->acl_cnt > XFS_ACL_MAX_ENTRIES)
		goto acl_invalid;

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
