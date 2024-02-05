// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "incore.h"
#include "err_protos.h"
#include "dinode.h"
#include "dir2.h"
#include "bmap.h"
#include "da_util.h"
#include "prefetch.h"
#include "progress.h"

/*
 * Known bad inode list.  These are seen when the leaf and node
 * block linkages are incorrect.
 */
struct dir2_bad {
	xfs_ino_t	ino;
	struct dir2_bad	*next;
};

static struct dir2_bad	*dir2_bad_list;
pthread_mutex_t		dir2_bad_list_lock = PTHREAD_MUTEX_INITIALIZER;

static void
dir2_add_badlist(
	xfs_ino_t	ino)
{
	struct dir2_bad	*l;

	l = malloc(sizeof(*l));
	if (!l) {
		do_error(
_("malloc failed (%zu bytes) dir2_add_badlist:ino %" PRIu64 "\n"),
			sizeof(*l), ino);
		exit(1);
	}
	pthread_mutex_lock(&dir2_bad_list_lock);
	l->next = dir2_bad_list;
	dir2_bad_list = l;
	l->ino = ino;
	pthread_mutex_unlock(&dir2_bad_list_lock);
}

bool
dir2_is_badino(
	xfs_ino_t	ino)
{
	struct dir2_bad	*l;
	bool		ret = false;

	pthread_mutex_lock(&dir2_bad_list_lock);
	for (l = dir2_bad_list; l; l = l->next) {
		if (l->ino == ino) {
			ret = true;
			break;
		}
	}
	pthread_mutex_unlock(&dir2_bad_list_lock);
	return ret;
}

/*
 * Fix up a shortform directory which was in long form (i8count set)
 * and is now in short form (i8count clear).
 * Return pointer to the end of the data when done.
 */
void
process_sf_dir2_fixi8(
	struct xfs_mount	*mp,
	struct xfs_dir2_sf_hdr	*sfp,
	xfs_dir2_sf_entry_t	**next_sfep)
{
	xfs_ino_t		ino;
	struct xfs_dir2_sf_hdr	*newsfp;
	xfs_dir2_sf_entry_t	*newsfep;
	struct xfs_dir2_sf_hdr	*oldsfp;
	xfs_dir2_sf_entry_t	*oldsfep;
	int			oldsize;

	newsfp = sfp;
	oldsize = (intptr_t)*next_sfep - (intptr_t)sfp;
	oldsfp = malloc(oldsize);
	if (oldsfp == NULL) {
		do_error(_("couldn't malloc dir2 shortform copy\n"));
		exit(1);
	}
	memmove(oldsfp, newsfp, oldsize);
	newsfp->count = oldsfp->count;
	newsfp->i8count = 0;
	ino = libxfs_dir2_sf_get_parent_ino(oldsfp);
	libxfs_dir2_sf_put_parent_ino(newsfp, ino);
	oldsfep = xfs_dir2_sf_firstentry(oldsfp);
	newsfep = xfs_dir2_sf_firstentry(newsfp);
	while ((int)((char *)oldsfep - (char *)oldsfp) < oldsize) {
		newsfep->namelen = oldsfep->namelen;
		xfs_dir2_sf_put_offset(newsfep,
			xfs_dir2_sf_get_offset(oldsfep));
		memmove(newsfep->name, oldsfep->name, newsfep->namelen);
		ino = libxfs_dir2_sf_get_ino(mp, oldsfp, oldsfep);
		libxfs_dir2_sf_put_ino(mp, newsfp, newsfep, ino);
		oldsfep = libxfs_dir2_sf_nextentry(mp, oldsfp, oldsfep);
		newsfep = libxfs_dir2_sf_nextentry(mp, newsfp, newsfep);
	}
	*next_sfep = newsfep;
	free(oldsfp);
}

/*
 * Regenerate legal (minimal) offsets for the shortform directory.
 */
static void
process_sf_dir2_fixoff(
	xfs_mount_t		*mp,
	struct xfs_dinode	*dip)
{
	int			i;
	int			offset;
	xfs_dir2_sf_entry_t	*sfep;
	struct xfs_dir2_sf_hdr	*sfp;

	sfp = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	sfep = xfs_dir2_sf_firstentry(sfp);
	offset = mp->m_dir_geo->data_first_offset;

	for (i = 0; i < sfp->count; i++) {
		xfs_dir2_sf_put_offset(sfep, offset);
		offset += libxfs_dir2_data_entsize(mp, sfep->namelen);
		sfep = libxfs_dir2_sf_nextentry(mp, sfp, sfep);
	}
}

/*
 * this routine performs inode discovery and tries to fix things
 * in place.  available redundancy -- inode data size should match
 * used directory space in inode.
 * a non-zero return value means the directory is bogus and should be blasted.
 */
/* ARGSUSED */
static int
process_sf_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	int			*dino_dirty, /* out: 1 if dinode buffer dirty */
	char			*dirname,    /* directory pathname */
	xfs_ino_t		*parent,     /* out: NULLFSINO if no entry */
	int			*repair)     /* out: 1 if dir was fixed up */
{
	int			bad_offset;
	int			bad_sfnamelen;
	int			i;
	int			i8;
	int64_t			ino_dir_size;
	int			ino_off;
	ino_tree_node_t		*irec_p;
	int			junkit;
	char			*junkreason = NULL;
	xfs_ino_t		lino;
	int			max_size;
	char			name[MAXNAMELEN + 1];
	int			namelen;
	xfs_dir2_sf_entry_t	*next_sfep;
	int			num_entries;
	int			offset;
	struct xfs_dir2_sf_hdr	*sfp;
	xfs_dir2_sf_entry_t	*sfep;
	int			tmp_elen;
	int			tmp_len;
	xfs_dir2_sf_entry_t	*tmp_sfep;

	sfp = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	max_size = XFS_DFORK_DSIZE(dip, mp);
	num_entries = sfp->count;
	ino_dir_size = be64_to_cpu(dip->di_size);
	offset = mp->m_dir_geo->data_first_offset;
	bad_offset = *repair = 0;

	ASSERT(ino_dir_size <= max_size);

	/*
	 * Initialize i8 based on size of parent inode number.
	 */
	i8 = (libxfs_dir2_sf_get_parent_ino(sfp) > XFS_DIR2_MAX_SHORT_INUM);

	/*
	 * check for bad entry count
	 */
	if (num_entries * libxfs_dir2_sf_entsize(mp, sfp, 1) +
		    xfs_dir2_sf_hdr_size(0) > max_size || num_entries == 0)
		num_entries = 0xFF;

	/*
	 * run through entries, stop at first bad entry, don't need
	 * to check for .. since that's encoded in its own field
	 */
	next_sfep = xfs_dir2_sf_firstentry(sfp);
	for (i = 0;
	     i < num_entries && ino_dir_size > (char *)next_sfep - (char *)sfp;
	     i++) {
		tmp_sfep = NULL;
		sfep = next_sfep;
		junkit = 0;
		bad_sfnamelen = 0;
		lino = libxfs_dir2_sf_get_ino(mp, sfp, sfep);
		/*
		 * if entry points to self, junk it since only '.' or '..'
		 * should do that and shortform dirs don't contain either
		 * entry.  if inode number is invalid, trash entry.
		 * if entry points to special inodes, trash it.
		 * if inode is unknown but number is valid,
		 * add it to the list of uncertain inodes.  don't
		 * have to worry about an entry pointing to a
		 * deleted lost+found inode because the entry was
		 * deleted at the same time that the inode was cleared.
		 */
		if (lino == ino) {
			junkit = 1;
			junkreason = _("current");
		} else if (!libxfs_verify_dir_ino(mp, lino)) {
			junkit = 1;
			junkreason = _("invalid");
		} else if (lino == mp->m_sb.sb_rbmino)  {
			junkit = 1;
			junkreason = _("realtime bitmap");
		} else if (lino == mp->m_sb.sb_rsumino)  {
			junkit = 1;
			junkreason = _("realtime summary");
		} else if (lino == mp->m_sb.sb_uquotino)  {
			junkit = 1;
			junkreason = _("user quota");
		} else if (lino == mp->m_sb.sb_gquotino)  {
			junkit = 1;
			junkreason = _("group quota");
		} else if (lino == mp->m_sb.sb_pquotino)  {
			junkit = 1;
			junkreason = _("project quota");
		} else if ((irec_p = find_inode_rec(mp,
					XFS_INO_TO_AGNO(mp, lino),
					XFS_INO_TO_AGINO(mp, lino))) != NULL) {
			/*
			 * if inode is marked free and we're in inode
			 * discovery mode, leave the entry alone for now.
			 * if the inode turns out to be used, we'll figure
			 * that out when we scan it.  If the inode really
			 * is free, we'll hit this code again in phase 4
			 * after we've finished inode discovery and blow
			 * out the entry then.
			 */
			ino_off = XFS_INO_TO_AGINO(mp, lino) -
				irec_p->ino_startnum;
			ASSERT(is_inode_confirmed(irec_p, ino_off));
			if (is_inode_free(irec_p, ino_off) && !ino_discovery) {
				junkit = 1;
				junkreason = _("free");
			}
		} else if (ino_discovery) {
			/*
			 * put the inode on the uncertain list.  we'll
			 * pull the inode off the list and check it later.
			 * if the inode turns out be bogus, we'll delete
			 * this entry in phase 6.
			 */
			add_inode_uncertain(mp, lino, 0);
		} else  {
			/*
			 * blow the entry out.  we know about all
			 * undiscovered entries now (past inode discovery
			 * phase) so this is clearly a bogus entry.
			 */
			junkit = 1;
			junkreason = _("non-existent");
		}
		namelen = sfep->namelen;
		if (junkit)
			do_warn(
_("entry \"%*.*s\" in shortform directory %" PRIu64 " references %s inode %" PRIu64 "\n"),
				namelen, namelen, sfep->name, ino, junkreason,
				lino);

		/* is dir namelen 0 or does this entry extend past dir size? */
		if (namelen == 0) {
			junkreason = _("is zero length");
			bad_sfnamelen = 1;
		} else if ((intptr_t) sfep - (intptr_t) sfp +
				libxfs_dir2_sf_entsize(mp, sfp, sfep->namelen)
							> ino_dir_size)  {
			junkreason = _("extends past end of dir");
			bad_sfnamelen = 1;
		}

		if (bad_sfnamelen) {
			do_warn(
_("entry #%d %s in shortform dir %" PRIu64),
				i, junkreason, ino);
			if (!no_modify)
				do_warn(_(", junking %d entries\n"),
					num_entries - i);
			else
				do_warn(_(", would junk %d entries\n"),
					num_entries - i);
			/*
			 * don't process the rest of the directory,
			 * break out of processing loop
			 */
			break;
		}

		/*
		 * check for illegal chars in name.
		 * no need to check for bad length because
		 * the length value is stored in a byte
		 * so it can't be too big, it can only wrap
		 */
		if (!libxfs_dir2_namecheck(sfep->name, namelen)) {
			/*
			 * junk entry
			 */
			do_warn(
_("entry contains illegal character in shortform dir %" PRIu64 "\n"),
				ino);
			junkit = 1;
		}

		if (xfs_dir2_sf_get_offset(sfep) < offset) {
			do_warn(
_("entry contains offset out of order in shortform dir %" PRIu64 "\n"),
				ino);
			bad_offset = 1;
		}
		offset = xfs_dir2_sf_get_offset(sfep) +
					libxfs_dir2_data_entsize(mp, namelen);

		/*
		 * junk the entry by copying up the rest of the
		 * fork over the current entry and decrementing
		 * the entry count.  if we're in no_modify mode,
		 * just issue the warning instead.  then continue
		 * the loop with the next_sfep pointer set to the
		 * correct place in the fork and other counters
		 * properly set to reflect the deletion if it
		 * happened.
		 */
		if (junkit)  {
			memmove(name, sfep->name, namelen);
			name[namelen] = '\0';

			if (!no_modify)  {
				tmp_elen = libxfs_dir2_sf_entsize(mp, sfp,
								sfep->namelen);
				be64_add_cpu(&dip->di_size, -tmp_elen);
				ino_dir_size -= tmp_elen;

				tmp_sfep = (xfs_dir2_sf_entry_t *)
					((intptr_t) sfep + tmp_elen);
				tmp_len = max_size - ((intptr_t) tmp_sfep
							- (intptr_t) sfp);

				memmove(sfep, tmp_sfep, tmp_len);

				sfp->count -= 1;
				num_entries--;
				memset((void *) ((intptr_t) sfep + tmp_len), 0,
					tmp_elen);

				/*
				 * reset the tmp value to the current
				 * pointer so we'll process the entry
				 * we just moved up
				 */
				tmp_sfep = sfep;

				/*
				 * WARNING:  drop the index i by one
				 * so it matches the decremented count
				 * for accurate comparisons later
				 */
				i--;

				*dino_dirty = 1;
				*repair = 1;

				do_warn(
_("junking entry \"%s\" in directory inode %" PRIu64 "\n"),
					name, ino);
			} else  {
				do_warn(
_("would have junked entry \"%s\" in directory inode %" PRIu64 "\n"),
					name, ino);
			}
		} else if (lino > XFS_DIR2_MAX_SHORT_INUM)
			i8++;
		/*
		 * go onto next entry unless we've just junked an
		 * entry in which the current entry pointer points
		 * to an unprocessed entry.  have to take into zero-len
		 * entries into account in no modify mode since we
		 * calculate size based on next_sfep.
		 */
		next_sfep = (tmp_sfep == NULL)
			? (xfs_dir2_sf_entry_t *) ((intptr_t) sfep
							+ ((!bad_sfnamelen)
				? libxfs_dir2_sf_entsize(mp, sfp, sfep->namelen)
				: libxfs_dir2_sf_entsize(mp, sfp, namelen)))
			: tmp_sfep;
	}

	/* sync up sizes and entry counts */

	if (sfp->count != i) {
		if (no_modify) {
			do_warn(
_("would have corrected entry count in directory %" PRIu64 " from %d to %d\n"),
				ino, sfp->count, i);
		} else {
			do_warn(
_("corrected entry count in directory %" PRIu64 ", was %d, now %d\n"),
				ino, sfp->count, i);
			sfp->count = i;
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	if (sfp->i8count != i8)  {
		if (no_modify)  {
			do_warn(
_("would have corrected i8 count in directory %" PRIu64 " from %d to %d\n"),
				ino, sfp->i8count, i8);
		} else {
			do_warn(
_("corrected i8 count in directory %" PRIu64 ", was %d, now %d\n"),
				ino, sfp->i8count, i8);
			if (i8 == 0)
				process_sf_dir2_fixi8(mp, sfp, &next_sfep);
			else
				sfp->i8count = i8;
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	if ((intptr_t)next_sfep - (intptr_t)sfp != ino_dir_size)  {
		if (no_modify)  {
			do_warn(
_("would have corrected directory %" PRIu64 " size from %" PRId64 " to %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfep - (intptr_t)sfp);
		} else  {
			do_warn(
_("corrected directory %" PRIu64 " size, was %" PRId64 ", now %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfep - (intptr_t)sfp);

			dip->di_size = cpu_to_be64(
					(intptr_t)next_sfep - (intptr_t)sfp);
			*dino_dirty = 1;
			*repair = 1;
		}
	}
	if (offset + (sfp->count + 2) * sizeof(xfs_dir2_leaf_entry_t) +
			sizeof(xfs_dir2_block_tail_t) > mp->m_dir_geo->blksize) {
		do_warn(_("directory %" PRIu64 " offsets too high\n"), ino);
		bad_offset = 1;
	}
	if (bad_offset) {
		if (no_modify) {
			do_warn(
_("would have corrected entry offsets in directory %" PRIu64 "\n"),
				ino);
		} else {
			do_warn(
_("corrected entry offsets in directory %" PRIu64 "\n"),
				ino);
			process_sf_dir2_fixoff(mp, dip);
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	/*
	 * check parent (..) entry
	 */
	*parent = libxfs_dir2_sf_get_parent_ino(sfp);
	/*
	 * If this function is called during inode discovery (phase 3), it will
	 * set a bad sf dir parent pointer to the root directory. This fixes
	 * the directory enough to pass the inode fork verifier in phase 6 when
	 * we try to reset the parent pointer to the correct value. There is no
	 * need to re-check the parent pointer during phase 4.
	 */
	if (!ino_discovery)
		return 0;

	/*
	 * if parent entry is bogus, null it out.  we'll fix it later .
	 * If the validation fails for the root inode we fix it in
	 * the next else case.
	 */
	if (!libxfs_verify_dir_ino(mp, *parent) && ino != mp->m_sb.sb_rootino) {
		do_warn(
_("bogus .. inode number (%" PRIu64 ") in directory inode %" PRIu64 ", "),
				*parent, ino);
		*parent = NULLFSINO;
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			libxfs_dir2_sf_put_parent_ino(sfp, mp->m_sb.sb_rootino);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(_("would clear inode number\n"));
		}
	} else if (ino == mp->m_sb.sb_rootino && ino != *parent) {
		/*
		 * root directories must have .. == .
		 */
		if (!no_modify)  {
			do_warn(
_("corrected root directory %" PRIu64 " .. entry, was %" PRIu64 ", now %" PRIu64 "\n"),
				ino, *parent, ino);
			*parent = ino;
			libxfs_dir2_sf_put_parent_ino(sfp, ino);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(
_("would have corrected root directory %" PRIu64 " .. entry from %" PRIu64" to %" PRIu64 "\n"),
				ino, *parent, ino);
		}
	} else if (ino == *parent && ino != mp->m_sb.sb_rootino)  {
		/*
		 * likewise, non-root directories can't have .. pointing
		 * to .
		 */
		*parent = NULLFSINO;
		do_warn(
_("bad .. entry in directory inode %" PRIu64 ", points to self, "),
			ino);
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			libxfs_dir2_sf_put_parent_ino(sfp, mp->m_sb.sb_rootino);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(_("would clear inode number\n"));
		}
	}

	return(0);
}

/*
 * Process one directory data block.
 */
/* ARGSUSED */
static int
process_dir2_data(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	char			*dirname, /* directory pathname */
	xfs_ino_t		*parent,  /* out: NULLFSINO if entry not exist*/
	struct xfs_buf		*bp,
	int			*dot,	  /* out: 1 if there's a dot else 0 */
	int			*dotdot,  /* out: 1 if there's a dotdot else 0*/
	xfs_dablk_t		da_bno,
	char			*endptr,
	int			*dirty)
{
	int			badbest;
	xfs_dir2_data_free_t	*bf;
	int			clearino;
	char			*clearreason = NULL;
	struct xfs_dir2_data_hdr *d;
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_free_t	*dfp;
	xfs_dir2_data_unused_t	*dup;
	int			freeseen;
	int			i;
	int			ino_off;
	ino_tree_node_t		*irec_p;
	int			junkit;
	int			lastfree;
	int			nm_illegal;
	char			*ptr;
	xfs_ino_t		ent_ino;

	d = bp->b_addr;
	bf = libxfs_dir2_data_bestfree_p(mp, d);
	ptr = (char *)d + mp->m_dir_geo->data_entry_offset;
	badbest = lastfree = freeseen = 0;
	if (be16_to_cpu(bf[0].length) == 0) {
		badbest |= be16_to_cpu(bf[0].offset) != 0;
		freeseen |= 1 << 0;
	}
	if (be16_to_cpu(bf[1].length) == 0) {
		badbest |= be16_to_cpu(bf[1].offset) != 0;
		freeseen |= 1 << 1;
	}
	if (be16_to_cpu(bf[2].length) == 0) {
		badbest |= be16_to_cpu(bf[2].offset) != 0;
		freeseen |= 1 << 2;
	}
	badbest |= be16_to_cpu(bf[0].length) < be16_to_cpu(bf[1].length);
	badbest |= be16_to_cpu(bf[1].length) < be16_to_cpu(bf[2].length);
	while (ptr < endptr) {
		dup = (xfs_dir2_data_unused_t *)ptr;
		/*
		 * If it's unused, look for the space in the bestfree table.
		 * If we find it, account for that, else make sure it doesn't
		 * need to be there.
		 */
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			if (ptr + be16_to_cpu(dup->length) > endptr ||
			    be16_to_cpu(dup->length) == 0 ||
			    (be16_to_cpu(dup->length) & (XFS_DIR2_DATA_ALIGN - 1)))
				break;
			if (be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)) !=
							(char *)dup - (char *)d)
				break;
			badbest |= lastfree != 0;
			dfp = xfs_dir2_data_freefind(d, bf, dup);
			if (dfp) {
				i = dfp - bf;
				badbest |= (freeseen & (1 << i)) != 0;
				freeseen |= 1 << i;
			} else
				badbest |= be16_to_cpu(dup->length) >
					be16_to_cpu(bf[2].length);
			ptr += be16_to_cpu(dup->length);
			lastfree = 1;
			continue;
		}
		dep = (xfs_dir2_data_entry_t *)ptr;
		if (ptr + libxfs_dir2_data_entsize(mp, dep->namelen) > endptr)
			break;
		if (be16_to_cpu(*libxfs_dir2_data_entry_tag_p(mp, dep)) !=
		    				(char *)dep - (char *)d)
			break;
		ptr += libxfs_dir2_data_entsize(mp, dep->namelen);
		lastfree = 0;
	}
	/*
	 * Dropped out before we processed everything, give up.
	 * Phase 6 will kill this block if we don't kill the inode.
	 */
	if (ptr != endptr) {
		do_warn(_("corrupt block %u in directory inode %" PRIu64 "\n"),
			da_bno, ino);
		if (!no_modify)
			do_warn(_("\twill junk block\n"));
		else
			do_warn(_("\twould junk block\n"));
		return 1;
	}
	ptr = (char *)d + mp->m_dir_geo->data_entry_offset;
	/*
	 * Process the entries now.
	 */
	while (ptr < endptr) {
		dup = (xfs_dir2_data_unused_t *)ptr;
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			ptr += be16_to_cpu(dup->length);
			continue;
		}
		dep = (xfs_dir2_data_entry_t *)ptr;
		ent_ino = be64_to_cpu(dep->inumber);
		clearino = 1;
		clearreason = NULL;
		/*
		 * We may have to blow out an entry because of bad inode
		 * numbers.  Do NOT touch the name until after we've computed
		 * the hashvalue and done a namecheck() on the name.
		 *
		 * Conditions must either set clearino to zero or set
		 * clearreason why it's being cleared.
		 */
		if (!ino_discovery && dep->name[0] == '/') {
			/*
			 * Don't do a damned thing.  We already found this
			 * (or did it ourselves) during phase 3.
			 */
			clearino = 0;
		} else if (!libxfs_verify_dir_ino(mp, ent_ino)) {
			/*
			 * Bad inode number.  Clear the inode number and the
			 * entry will get removed later.  We don't trash the
			 * directory since it's still structurally intact.
			 */
			clearreason = _("invalid");
		} else if (ent_ino == mp->m_sb.sb_rbmino) {
			clearreason = _("realtime bitmap");
		} else if (ent_ino == mp->m_sb.sb_rsumino) {
			clearreason = _("realtime summary");
		} else if (ent_ino == mp->m_sb.sb_uquotino) {
			clearreason = _("user quota");
		} else if (ent_ino == mp->m_sb.sb_gquotino) {
			clearreason = _("group quota");
		} else if (ent_ino == mp->m_sb.sb_pquotino) {
			clearreason = _("project quota");
		} else {
			irec_p = find_inode_rec(mp,
						XFS_INO_TO_AGNO(mp, ent_ino),
						XFS_INO_TO_AGINO(mp, ent_ino));
			if (irec_p == NULL) {
				if (ino_discovery) {
					add_inode_uncertain(mp, ent_ino, 0);
					clearino = 0;
				} else
					clearreason = _("non-existent");
			} else {
				/*
				 * Inode recs should have only confirmed
				 * inodes in them.
				 */
				ino_off = XFS_INO_TO_AGINO(mp, ent_ino)
							- irec_p->ino_startnum;
				ASSERT(is_inode_confirmed(irec_p, ino_off));
				/*
				 * If inode is marked free and we're in inode
				 * discovery mode, leave the entry alone for
				 * now.  If the inode turns out to be used,
				 * we'll figure that out when we scan it.
				 * If the inode really is free, we'll hit this
				 * code again in phase 4 after we've finished
				 * inode discovery and blow out the entry then.
				 */
				if (!ino_discovery && is_inode_free(irec_p,
								ino_off))
					clearreason = _("free");
				else
					clearino = 0;
			}
		}
		ASSERT((clearino == 0 && clearreason == NULL) ||
			(clearino != 0 && clearreason != NULL));
		if (clearino)
			do_warn(
_("entry \"%*.*s\" at block %d offset %" PRIdPTR " in directory inode %" PRIu64
  " references %s inode %" PRIu64 "\n"),
				dep->namelen, dep->namelen, dep->name,
				da_bno, (intptr_t)ptr - (intptr_t)d, ino,
				clearreason, ent_ino);

		/*
		 * We have a special dot & dotdot fixer-upper below which can
		 * sort out the proper inode number, so don't clear it.
		 */
		if ((dep->namelen == 1 && dep->name[0] == '.') ||
		    (dep->namelen == 2 &&
		     dep->name[0] == '.' && dep->name[1] == '.')) {
			clearino = 0;
			clearreason = NULL;
		}

		/*
		 * If the name length is 0 (illegal) make it 1 and blast
		 * the entry.
		 */
		if (dep->namelen == 0) {
			do_warn(
_("entry at block %u offset %" PRIdPTR " in directory inode %" PRIu64
  "has 0 namelength\n"),
				da_bno, (intptr_t)ptr - (intptr_t)d, ino);
			if (!no_modify)
				dep->namelen = 1;
			clearino = 1;
		}
		/*
		 * If needed to clear the inode number, do it now.
		 */
		if (clearino) {
			if (!no_modify) {
				do_warn(
_("\tclearing inode number in entry at offset %" PRIdPTR "...\n"),
					(intptr_t)ptr - (intptr_t)d);
				dep->name[0] = '/';
				*dirty = 1;
			} else {
				do_warn(
_("\twould clear inode number in entry at offset %" PRIdPTR "...\n"),
					(intptr_t)ptr - (intptr_t)d);
			}
		}
		/*
		 * Only complain about illegal names in phase 3 (when inode
		 * discovery is turned on).  Otherwise, we'd complain a lot
		 * during phase 4.
		 */
		junkit = dep->name[0] == '/';
		nm_illegal = !libxfs_dir2_namecheck(dep->name, dep->namelen);
		if (ino_discovery && nm_illegal) {
			do_warn(
_("entry at block %u offset %" PRIdPTR " in directory inode %" PRIu64 " has illegal name \"%*.*s\": "),
				da_bno, (intptr_t)ptr - (intptr_t)d, ino,
				dep->namelen, dep->namelen, dep->name);
			junkit = 1;
		}

		/*
		 * Ensure we write back bad entries for later processing
		 */
		if (!no_modify && dep->name[0] == '/') {
			*dirty = 1;
			junkit = 0;
		}

		/*
		 * Special .. entry processing.
		 */
		if (dep->namelen == 2 &&
		    dep->name[0] == '.' && dep->name[1] == '.') {
			if (!*dotdot) {
				(*dotdot)++;
				*parent = ent_ino;
				/*
				 * What if .. == .?  Legal only in the root
				 * inode.  Blow out entry and set parent to
				 * NULLFSINO otherwise.
				 */
				if (ino == ent_ino &&
						ino != mp->m_sb.sb_rootino) {
					*parent = NULLFSINO;
					do_warn(
_("bad .. entry in directory inode %" PRIu64 ", points to self: "),
						ino);
					junkit = 1;
				}
				/*
				 * We have to make sure that . == .. in the
				 * root inode.
				 */
				else if (ino != ent_ino &&
						ino == mp->m_sb.sb_rootino) {
					do_warn(
_("bad .. entry in root directory inode %" PRIu64 ", was %" PRIu64 ": "),
						ino, ent_ino);
					if (!no_modify) {
						do_warn(_("correcting\n"));
						dep->inumber = cpu_to_be64(ino);
						*dirty = 1;
					} else {
						do_warn(_("would correct\n"));
					}
					*parent = ino;
				}
				/*
				 * Make sure our parent directory doesn't point
				 * off into space.
				 */
				if (!junkit &&
				    *parent != NULLFSINO &&
				    !libxfs_verify_ino(mp, *parent)) {
					do_warn(
_("bad .. entry in directory inode %" PRIu64 ", was %" PRIu64 ": "),
						ino, *parent);
					if (!no_modify) {
						do_warn(_("correcting\n"));
					} else {
						do_warn(_("would correct\n"));
					}
					*parent = NULLFSINO;
				}
			}
			/*
			 * Can't fix the directory unless we know which ..
			 * entry is the right one.  Both have valid inode
			 * numbers or we wouldn't be here.  So since both
			 * seem equally valid, trash this one.
			 */
			else {
				do_warn(
_("multiple .. entries in directory inode %" PRIu64 ": "),
					ino);
				junkit = 1;
			}
		}
		/*
		 * Special . entry processing.
		 */
		else if (dep->namelen == 1 && dep->name[0] == '.') {
			if (!*dot) {
				(*dot)++;
				if (ent_ino != ino) {
					do_warn(
_("bad . entry in directory inode %" PRIu64 ", was %" PRIu64 ": "),
						ino, ent_ino);
					if (!no_modify) {
						do_warn(_("correcting\n"));
						dep->inumber = cpu_to_be64(ino);
						*dirty = 1;
					} else {
						do_warn(_("would correct\n"));
					}
				}
			} else {
				do_warn(
_("multiple . entries in directory inode %" PRIu64 ": "),
					ino);
				junkit = 1;
			}
		}
		/*
		 * All other entries -- make sure only . references self.
		 */
		else if (ent_ino == ino) {
			do_warn(
_("entry \"%*.*s\" in directory inode %" PRIu64 " points to self: "),
				dep->namelen, dep->namelen, dep->name, ino);
			junkit = 1;
		}
		/*
		 * Clear junked entries.
		 */
		if (junkit) {
			if (!no_modify) {
				dep->name[0] = '/';
				*dirty = 1;
				do_warn(_("clearing entry\n"));
			} else {
				do_warn(_("would clear entry\n"));
			}
		}
		/*
		 * Advance to the next entry.
		 */
		ptr += libxfs_dir2_data_entsize(mp, dep->namelen);
	}
	/*
	 * Check the bestfree table.
	 */
	if (freeseen != 7 || badbest) {
		do_warn(
_("bad bestfree table in block %u in directory inode %" PRIu64 ": "),
			da_bno, ino);
		if (!no_modify) {
			do_warn(_("repairing table\n"));
			libxfs_dir2_data_freescan(mp, d, &i);
			*dirty = 1;
		} else {
			do_warn(_("would repair table\n"));
		}
	}
	return 0;
}

/*
 * Process a block-format directory.
 */
/* ARGSUSED */
static int
process_block_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	int			*dino_dirty, /* out: 1 if dinode buffer dirty */
	char			*dirname,    /* directory pathname */
	xfs_ino_t		*parent,     /* out: NULLFSINO if no entry */
	blkmap_t		*blkmap,
	int			*dot,	     /* out: 1 if there's a dot else 0 */
	int			*dotdot,     /* out: 1 if there's a dotdot else 0 */
	int			*repair)     /* out: 1 if something was fixed */
{
	struct xfs_dir2_data_hdr *block;
	xfs_dir2_leaf_entry_t	*blp;
	bmap_ext_t		*bmp;
	struct xfs_buf		*bp;
	xfs_dir2_block_tail_t	*btp;
	int			nex;
	int			rval;
	bmap_ext_t		lbmp;
	int			dirty = 0;

	*repair = *dot = *dotdot = 0;
	*parent = NULLFSINO;
	nex = blkmap_getn(blkmap, mp->m_dir_geo->datablk,
				mp->m_dir_geo->fsbcount, &bmp, &lbmp);
	if (nex == 0) {
		do_warn(
_("block %u for directory inode %" PRIu64 " is missing\n"),
			mp->m_dir_geo->datablk, ino);
		return 1;
	}
	bp = da_read_buf(mp, nex, bmp, &xfs_dir3_block_buf_ops);
	if (bmp != &lbmp)
		free(bmp);
	if (bp == NULL) {
		do_warn(
_("can't read block %u for directory inode %" PRIu64 "\n"),
			mp->m_dir_geo->datablk, ino);
		return 1;
	}
	if (bp->b_error == -EFSCORRUPTED) {
		do_warn(
_("corrupt directory block %u for inode %" PRIu64 "\n"),
			mp->m_dir_geo->datablk, ino);
		libxfs_buf_relse(bp);
		return 1;
	}
	/*
	 * Verify the block
	 */
	block = bp->b_addr;
	if (!(be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC ||
	      be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC))
		do_warn(
_("bad directory block magic # %#x in block %u for directory inode %" PRIu64 "\n"),
			be32_to_cpu(block->magic), mp->m_dir_geo->datablk, ino);
	/*
	 * process the data area
	 * this also checks & fixes the bestfree
	 */
	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	blp = xfs_dir2_block_leaf_p(btp);
	/*
	 * Don't let this go past the end of the block.
	 */
	if ((char *)blp > (char *)btp)
		blp = (xfs_dir2_leaf_entry_t *)btp;
	rval = process_dir2_data(mp, ino, dip, ino_discovery, dirname, parent,
		bp, dot, dotdot, mp->m_dir_geo->datablk, (char *)blp, &dirty);
	/* If block looks ok but CRC didn't match, make sure to recompute it. */
	if (!rval && bp->b_error == -EFSBADCRC)
		dirty = 1;
	if (dirty && !no_modify) {
		*repair = 1;
		libxfs_buf_mark_dirty(bp);
		libxfs_buf_relse(bp);
	} else
		libxfs_buf_relse(bp);
	return rval;
}

/*
 * Validates leaf contents, node format directories only.
 * magic number and sibling pointers checked by caller.
 * Returns 0 if block is ok, 1 if the block is bad.
 * Looking for: out of order hash values, bad stale counts.
 */
static int
process_leaf_block_dir2(
	xfs_mount_t		*mp,
	xfs_dir2_leaf_t		*leaf,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino,
	xfs_dahash_t		last_hashval,
	xfs_dahash_t		*next_hashval)
{
	int			i;
	int			stale;
	struct xfs_dir2_leaf_entry *ents;
	struct xfs_dir3_icleaf_hdr leafhdr;

	libxfs_dir2_leaf_hdr_from_disk(mp, &leafhdr, leaf);
	ents = leafhdr.ents;

	for (i = stale = 0; i < leafhdr.count; i++) {
		if ((char *)&ents[i] >= (char *)leaf + mp->m_dir_geo->blksize) {
			do_warn(
_("bad entry count in block %u of directory inode %" PRIu64 "\n"),
				da_bno, ino);
			return 1;
		}
		if (be32_to_cpu(ents[i].address) == XFS_DIR2_NULL_DATAPTR)
			stale++;
		else if (be32_to_cpu(ents[i].hashval) < last_hashval) {
			do_warn(
_("bad hash ordering in block %u of directory inode %" PRIu64 "\n"),
				da_bno, ino);
			return 1;
		}
		*next_hashval = last_hashval = be32_to_cpu(ents[i].hashval);
	}
	if (stale != leafhdr.stale) {
		do_warn(
_("bad stale count in block %u of directory inode %" PRIu64 "\n"),
			da_bno, ino);
		return 1;
	}
	return 0;
}

/*
 * Returns 0 if the directory is ok, 1 if it has to be rebuilt.
 */
static int
process_leaf_level_dir2(
	xfs_mount_t		*mp,
	da_bt_cursor_t		*da_cursor,
	int			*repair)
{
	bmap_ext_t		*bmp;
	struct xfs_buf		*bp;
	int			buf_dirty;
	xfs_dahash_t		current_hashval;
	xfs_dablk_t		da_bno;
	xfs_dahash_t		greatest_hashval;
	xfs_ino_t		ino;
	xfs_dir2_leaf_t		*leaf;
	int			nex;
	xfs_dablk_t		prev_bno;
	bmap_ext_t		lbmp;
	struct xfs_dir3_icleaf_hdr leafhdr;

	da_bno = da_cursor->level[0].bno;
	ino = da_cursor->ino;
	prev_bno = 0;
	bmp = NULL;
	current_hashval = 0;
	greatest_hashval = 0;
	buf_dirty = 0;

	do {
		nex = blkmap_getn(da_cursor->blkmap, da_bno,
				mp->m_dir_geo->fsbcount, &bmp, &lbmp);
		/*
		 * Directory code uses 0 as the NULL block pointer since 0
		 * is the root block and no directory block pointer can point
		 * to the root block of the btree.
		 */
		ASSERT(da_bno != 0);

		if (nex == 0) {
			do_warn(
_("can't map block %u for directory inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}
		bp = da_read_buf(mp, nex, bmp, &xfs_dir3_leafn_buf_ops);
		if (bmp != &lbmp)
			free(bmp);
		bmp = NULL;
		if (bp == NULL) {
			do_warn(
_("can't read file block %u for directory inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}
		if (bp->b_error == -EFSCORRUPTED) {
			do_warn(
_("corrupt directory leafn block %u for inode %" PRIu64 "\n"),
				da_bno, ino);
			libxfs_buf_relse(bp);
			goto error_out;
		}
		leaf = bp->b_addr;
		libxfs_dir2_leaf_hdr_from_disk(mp, &leafhdr, leaf);
		/*
		 * Check magic number for leaf directory btree block.
		 */
		if (!(leafhdr.magic == XFS_DIR2_LEAFN_MAGIC ||
		      leafhdr.magic == XFS_DIR3_LEAFN_MAGIC)) {
			do_warn(
_("bad directory leaf magic # %#x for directory inode %" PRIu64 " block %u\n"),
				leafhdr.magic, ino, da_bno);
			libxfs_buf_relse(bp);
			goto error_out;
		}
		buf_dirty = 0;
		/*
		 * For each block, process the block, verify its path,
		 * then get next block.  Update cursor values along the way.
		 */
		if (process_leaf_block_dir2(mp, leaf, da_bno, ino,
				current_hashval, &greatest_hashval)) {
			libxfs_buf_relse(bp);
			goto error_out;
		}
		/*
		 * Index can be set to hdr.count so match the indices of the
		 * interior blocks -- which at the end of the block will point
		 * to 1 after the final real entry in the block.
		 */
		da_cursor->level[0].hashval = greatest_hashval;
		da_cursor->level[0].bp = bp;
		da_cursor->level[0].bno = da_bno;
		da_cursor->level[0].index = leafhdr.count;
		da_cursor->level[0].dirty = buf_dirty;

		if (leafhdr.back != prev_bno) {
			do_warn(
_("bad sibling back pointer for block %u in directory inode %" PRIu64 "\n"),
				da_bno, ino);
			libxfs_buf_relse(bp);
			goto error_out;
		}
		prev_bno = da_bno;
		da_bno = leafhdr.forw;
		if (da_bno != 0) {
			if (verify_da_path(mp, da_cursor, 0, XFS_DATA_FORK)) {
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
			buf_dirty = 1;
		ASSERT(buf_dirty == 0 || (buf_dirty && !no_modify));
		if (buf_dirty && !no_modify) {
			*repair = 1;
			libxfs_buf_mark_dirty(bp);
			libxfs_buf_relse(bp);
		} else
			libxfs_buf_relse(bp);
	} while (da_bno != 0);
	if (verify_final_da_path(mp, da_cursor, 0, XFS_DATA_FORK)) {
		/*
		 * Verify the final path up (right-hand-side) if still ok.
		 */
		do_warn(_("bad hash path in directory %" PRIu64 "\n"), ino);
		goto error_out;
	}
	/*
	 * Redundant but just for testing.
	 */
	release_da_cursor(mp, da_cursor, 0);
	return 0;

error_out:
	/*
	 * Release all buffers holding interior btree blocks.
	 */
	err_release_da_cursor(mp, da_cursor, 0);
	if (bmp && (bmp != &lbmp))
		free(bmp);
	return 1;
}

/*
 * Return 1 if the directory's leaf/node space is corrupted and
 * needs to be rebuilt, 0 if it's ok.
 */
static int
process_node_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	blkmap_t		*blkmap,
	int			*repair)
{
	xfs_dablk_t		bno;
	da_bt_cursor_t		da_cursor;

	/*
	 * Try again -- traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along the way.
	 * Then walk the leaf blocks left-to-right, calling a parent
	 * verification routine each time we traverse a block.
	 */
	memset(&da_cursor, 0, sizeof(da_cursor));
	da_cursor.ino = ino;
	da_cursor.dip = dip;
	da_cursor.blkmap = blkmap;

	/*
	 * Now process interior node.
	 */
	if (traverse_int_dablock(mp, &da_cursor, &bno, XFS_DATA_FORK) == 0)
		return 1;

	/*
	 * Skip directories with a root marked XFS_DIR2_LEAFN_MAGIC
	 *
	 * Be careful here: If any level of the da cursor was filled out then
	 * the directory has a da btree containing an invalid before pointer to
	 * dblock 0, and we should move on to rebuilding the directory.  If no
	 * levels in the da cursor got filled out, then we just have a single
	 * leafn block and we're done.
	 */
	if (bno == 0) {
		if (da_cursor.active > 0) {
			err_release_da_cursor(mp, &da_cursor, 0);
			return 1;
		} else {
			release_da_cursor(mp, &da_cursor, 0);
			return 0;
		}
	} else {
		/*
		 * Now pass cursor and bno into leaf-block processing routine.
		 * The leaf dir level routine checks the interior paths up to
		 * the root including the final right-most path.
		 */
		return process_leaf_level_dir2(mp, &da_cursor, repair);
	}
}

/*
 * Process leaf and node directories.
 * Process the data blocks then, if it's a node directory, check
 * the consistency of those blocks.
 */
static int
process_leaf_node_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	char			*dirname, /* directory pathname */
	xfs_ino_t		*parent,  /* out: NULLFSINO if no entry */
	blkmap_t		*blkmap,
	int			*dot,	  /* out: 1 if there's a dot else 0 */
	int			*dotdot,  /* out: 1 if there's a dotdot else 0*/
	int			*repair,  /* out: 1 if something was fixed */
	int			isnode)	  /* node directory not leaf */
{
	bmap_ext_t		*bmp;
	struct xfs_buf		*bp;
	struct xfs_dir2_data_hdr *data;
	xfs_fileoff_t		dbno;
	int			good;
	int			i;
	xfs_fileoff_t		ndbno;
	int			nex;
	int			t;
	bmap_ext_t		lbmp;
	int			dirty = 0;

	*repair = *dot = *dotdot = good = 0;
	*parent = NULLFSINO;
	ndbno = NULLFILEOFF;
	while ((dbno = blkmap_next_off(blkmap, ndbno, &t)) < mp->m_dir_geo->leafblk) {
		nex = blkmap_getn(blkmap, dbno, mp->m_dir_geo->fsbcount, &bmp, &lbmp);
		/* Advance through map to last dfs block in this dir block */
		ndbno = dbno;
		while (ndbno < dbno + mp->m_dir_geo->fsbcount - 1) {
			ndbno = blkmap_next_off(blkmap, ndbno, &t);
		}
		if (nex == 0) {
			do_warn(
_("block %" PRIu64 " for directory inode %" PRIu64 " is missing\n"),
				dbno, ino);
			continue;
		}
		bp = da_read_buf(mp, nex, bmp, &xfs_dir3_data_buf_ops);
		if (bmp != &lbmp)
			free(bmp);
		if (bp == NULL) {
			do_warn(
_("can't read block %" PRIu64 " for directory inode %" PRIu64 "\n"),
				dbno, ino);
			continue;
		}
		if (bp->b_error == -EFSCORRUPTED) {
			do_warn(
_("corrupt directory data block %" PRIu64 " for inode %" PRIu64 "\n"),
				dbno, ino);
			libxfs_buf_relse(bp);
			continue;
		}
		data = bp->b_addr;
		if (!(be32_to_cpu(data->magic) == XFS_DIR2_DATA_MAGIC ||
		      be32_to_cpu(data->magic) == XFS_DIR3_DATA_MAGIC))
			do_warn(
_("bad directory block magic # %#x in block %" PRIu64 " for directory inode %" PRIu64 "\n"),
				be32_to_cpu(data->magic), dbno, ino);
		i = process_dir2_data(mp, ino, dip, ino_discovery, dirname,
			parent, bp, dot, dotdot, (xfs_dablk_t)dbno,
			(char *)data + mp->m_dir_geo->blksize, &dirty);
		if (i == 0) {
			good++;
			/* Maybe just CRC is wrong. Make sure we correct it. */
			if (bp->b_error == -EFSBADCRC)
				dirty = 1;
		}
		if (dirty && !no_modify) {
			*repair = 1;
			libxfs_buf_mark_dirty(bp);
			libxfs_buf_relse(bp);
		} else
			libxfs_buf_relse(bp);
	}
	if (good == 0)
		return 1;
	if (!isnode)
		return 0;
	if (dir2_is_badino(ino))
		return 0;

	if (process_node_dir2(mp, ino, dip, blkmap, repair))
		dir2_add_badlist(ino);
	return 0;

}

/*
 * Returns 1 if things are bad (directory needs to be junked)
 * and 0 if things are ok.  If ino_discovery is 1, add unknown
 * inodes to uncertain inode list.
 */
int
process_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	int			*dino_dirty,
	char			*dirname,
	xfs_ino_t		*parent,
	blkmap_t		*blkmap)
{
	int			dot;
	int			dotdot;
	xfs_fileoff_t		last;
	int			repair;
	int			res;

	*parent = NULLFSINO;
	dot = dotdot = 0;
	last = 0;

	/*
	 * branch off depending on the type of inode.  This routine
	 * is only called ONCE so all the subordinate routines will
	 * fix '.' and junk '..' if they're bogus.
	 */
	if (blkmap)
		last = blkmap_last_off(blkmap);
	if (be64_to_cpu(dip->di_size) <= XFS_DFORK_DSIZE(dip, mp) &&
			dip->di_format == XFS_DINODE_FMT_LOCAL) {
		dot = dotdot = 1;
		res = process_sf_dir2(mp, ino, dip, ino_discovery, dino_dirty,
			dirname, parent, &repair);
	} else if (last == mp->m_dir_geo->fsbcount &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE)) {
		res = process_block_dir2(mp, ino, dip, ino_discovery,
			dino_dirty, dirname, parent, blkmap, &dot, &dotdot,
			&repair);
	} else if (last >= mp->m_dir_geo->leafblk + mp->m_dir_geo->fsbcount &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE)) {
		res = process_leaf_node_dir2(mp, ino, dip, ino_discovery,
			dirname, parent, blkmap, &dot, &dotdot, &repair,
			last > mp->m_dir_geo->leafblk + mp->m_dir_geo->fsbcount);
	} else {
		do_warn(_("bad size/format for directory %" PRIu64 "\n"), ino);
		return 1;
	}
	/*
	 * bad . entries in all directories will be fixed up in phase 6
	 */
	if (dot == 0) {
		do_warn(_("no . entry for directory %" PRIu64 "\n"), ino);
	}

	/*
	 * shortform dirs always have a .. entry.  .. for all longform
	 * directories will get fixed in phase 6. .. for other shortform
	 * dirs also get fixed there.  .. for a shortform root was
	 * fixed in place since we know what it should be
	 */
	if (dotdot == 0 && ino != mp->m_sb.sb_rootino) {
		do_warn(_("no .. entry for directory %" PRIu64 "\n"), ino);
	} else if (dotdot == 0 && ino == mp->m_sb.sb_rootino) {
		do_warn(_("no .. entry for root directory %" PRIu64 "\n"), ino);
		need_root_dotdot = 1;
	}

	ASSERT((ino != mp->m_sb.sb_rootino && ino != *parent) ||
		(ino == mp->m_sb.sb_rootino &&
			(ino == *parent || need_root_dotdot == 1)));

	return res;
}
