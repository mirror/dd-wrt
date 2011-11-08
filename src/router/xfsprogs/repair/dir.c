/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "dir.h"
#include "bmap.h"

#if XFS_DIR_LEAF_MAPSIZE >= XFS_ATTR_LEAF_MAPSIZE
#define XR_DA_LEAF_MAPSIZE	XFS_DIR_LEAF_MAPSIZE
#else
#define XR_DA_LEAF_MAPSIZE	XFS_ATTR_LEAF_MAPSIZE
#endif



typedef struct da_hole_map  {
	int	lost_holes;
	int	num_holes;
	struct {
		int	base;
		int	size;
	} hentries[XR_DA_LEAF_MAPSIZE];
} da_hole_map_t;

/*
 * takes a name and length (name need not be null-terminated)
 * and returns 1 if the name contains a '/' or a \0, returns 0
 * otherwise
 */
int
namecheck(char *name, int length)
{
	char *c;
	int i;

	ASSERT(length < MAXNAMELEN);

	for (c = name, i = 0; i < length; i++, c++)  {
		if (*c == '/' || *c == '\0')
			return(1);
	}

	return(0);
}

/*
 * this routine performs inode discovery and tries to fix things
 * in place.  available redundancy -- inode data size should match
 * used directory space in inode.  returns number of valid directory
 * entries.  a non-zero return value means the directory is bogus
 * and should be blasted.
 */
/* ARGSUSED */
int
process_shortform_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,	/* out - 1 if dinode buffer dirty? */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry doesn't exist */
	char		*dirname,	/* directory pathname */
	int		*repair)	/* out - 1 if dir was fixed up */
{
	xfs_dir_shortform_t	*sf;
	xfs_dir_sf_entry_t	*sf_entry, *next_sfe, *tmp_sfe;
	xfs_ino_t		lino;
	int			max_size;
	__int64_t		ino_dir_size;
	int			num_entries;
	int			ino_off;
	int			namelen;
	int			i;
	int			junkit;
	int			tmp_len;
	int			tmp_elen;
	int			bad_sfnamelen;
	ino_tree_node_t		*irec_p;
	char			name[MAXNAMELEN + 1];

#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_shortform_dir - inode %llu\n", ino);
#endif

	sf = (xfs_dir_shortform_t *)XFS_DFORK_DPTR(dip);
	max_size = XFS_DFORK_DSIZE(dip, mp);
	num_entries = sf->hdr.count;
	ino_dir_size = be64_to_cpu(dip->di_size);
	*repair = 0;

	ASSERT(ino_dir_size <= max_size);

	/*
	 * check for bad entry count
	 */
	if (num_entries * sizeof(xfs_dir_sf_entry_t) + sizeof(xfs_dir_sf_hdr_t)
			> max_size || num_entries == 0)
		num_entries = 0xFF;

	/*
	 * run through entries, stop at first bad entry, don't need
	 * to check for .. since that's encoded in its own field
	 */
	sf_entry = next_sfe = &sf->list[0];
	for (i = 0; i < num_entries && ino_dir_size >
				(__psint_t)next_sfe - (__psint_t)sf; i++)  {
		tmp_sfe = NULL;
		sf_entry = next_sfe;
		junkit = 0;
		bad_sfnamelen = 0;
		xfs_dir_sf_get_dirino(&sf_entry->inumber, &lino);

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
		if (lino == ino)  {
			junkit = 1;
		} else if (verify_inum(mp, lino))  {
			/*
			 * junk the entry, mark lino as NULL since it's bad
			 */
			do_warn(
	_("invalid inode number %" PRIu64 " in directory %" PRIu64 "\n"), lino, ino);
			lino = NULLFSINO;
			junkit = 1;
		} else if (lino == mp->m_sb.sb_rbmino)  {
			do_warn(
	_("entry in shortform dir %" PRIu64 " references rt bitmap inode %" PRIu64 "\n"),
				ino, lino);
			junkit = 1;
		} else if (lino == mp->m_sb.sb_rsumino)  {
			do_warn(
	_("entry in shortform dir %" PRIu64 " references rt summary inode %" PRIu64 "\n"),
				ino, lino);
			junkit = 1;
		} else if (lino == mp->m_sb.sb_uquotino)  {
			do_warn(
	_("entry in shortform dir %" PRIu64 " references user quota inode %" PRIu64 "\n"),
				ino, lino);
			junkit = 1;
		} else if (lino == mp->m_sb.sb_gquotino)  {
			do_warn(
	_("entry in shortform dir %" PRIu64 " references group quota inode %" PRIu64 "\n"),
				ino, lino);
			junkit = 1;
		} else if ((irec_p = find_inode_rec(mp,
					XFS_INO_TO_AGNO(mp, lino),
					XFS_INO_TO_AGINO(mp, lino))) != NULL)  {
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

			if (!ino_discovery && is_inode_free(irec_p, ino_off))  {
				do_warn(
	_("entry references free inode %" PRIu64 " in shortform directory %" PRIu64 "\n"),
					lino, ino);
				junkit = 1;
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
			do_warn(
	_("entry references non-existent inode %" PRIu64 " in shortform dir %" PRIu64 "\n"),
					lino, ino);
			junkit = 1;
		}

		namelen = sf_entry->namelen;

		if (namelen == 0)  {
			/*
			 * if we're really lucky, this is
			 * the last entry in which case we
			 * can use the dir size to set the
			 * namelen value.  otherwise, forget
			 * it because we're not going to be
			 * able to find the next entry.
			 */
			bad_sfnamelen = 1;

			if (i == num_entries - 1)  {
				namelen = ino_dir_size -
					((__psint_t) &sf_entry->name[0] -
					 (__psint_t) sf);
				if (!no_modify)  {
					do_warn(
		_("zero length entry in shortform dir %" PRIu64 ", resetting to %d\n"),
						ino, namelen);
					sf_entry->namelen = namelen;
				} else  {
					do_warn(
		_("zero length entry in shortform dir %" PRIu64 ", would set to %d\n"),
						ino, namelen);
				}
			} else  {
				do_warn(
		_("zero length entry in shortform dir %" PRIu64 ", "),
					ino);
				if (!no_modify)
					do_warn(_("junking %d entries\n"),
						num_entries - i);
				else
					do_warn(_("would junk %d entries\n"),
						num_entries - i);
				/*
				 * don't process the rest of the directory,
				 * break out of processing looop
				 */
				break;
			}
		} else if ((__psint_t) sf_entry - (__psint_t) sf +
				+ xfs_dir_sf_entsize_byentry(sf_entry)
				> ino_dir_size)  {
			bad_sfnamelen = 1;

			if (i == num_entries - 1)  {
				namelen = ino_dir_size -
					((__psint_t) &sf_entry->name[0] -
					 (__psint_t) sf);
				do_warn(
	_("size of last entry overflows space left in in shortform dir %" PRIu64 ", "),
					ino);
				if (!no_modify)  {
					do_warn(_("resetting to %d\n"),
						namelen);
					sf_entry->namelen = namelen;
					*dino_dirty = 1;
				} else  {
					do_warn(_("would reset to %d\n"),
						namelen);
				}
			} else  {
				do_warn(
	_("size of entry #%d overflows space left in in shortform dir %" PRIu64 "\n"),
					i, ino);
				if (!no_modify)  {
					if (i == num_entries - 1)
						do_warn(
						_("junking entry #%d\n"),
							i);
					else
						do_warn(
						_("junking %d entries\n"),
							num_entries - i);
				} else  {
					if (i == num_entries - 1)
						do_warn(
						_("would junk entry #%d\n"),
							i);
					else
						do_warn(
						_("would junk %d entries\n"),
							num_entries - i);
				}

				break;
			}
		}

		/*
		 * check for illegal chars in name.
		 * no need to check for bad length because
		 * the length value is stored in a byte
		 * so it can't be too big, it can only wrap
		 */
		if (namecheck((char *)&sf_entry->name[0], namelen))  {
			/*
			 * junk entry
			 */
			do_warn(
		_("entry contains illegal character in shortform dir %" PRIu64 "\n"),
				ino);
			junkit = 1;
		}

		/*
		 * junk the entry by copying up the rest of the
		 * fork over the current entry and decrementing
		 * the entry count.  if we're in no_modify mode,
		 * just issue the warning instead.  then continue
		 * the loop with the next_sfe pointer set to the
		 * correct place in the fork and other counters
		 * properly set to reflect the deletion if it
		 * happened.
		 */
		if (junkit)  {
			memmove(name, sf_entry->name, namelen);
			name[namelen] = '\0';

			if (!no_modify)  {
				tmp_elen = xfs_dir_sf_entsize_byentry(sf_entry);
				be64_add_cpu(&dip->di_size, -tmp_elen);
				ino_dir_size -= tmp_elen;

				tmp_sfe = (xfs_dir_sf_entry_t *)
					((__psint_t) sf_entry + tmp_elen);
				tmp_len = max_size - ((__psint_t) tmp_sfe
							- (__psint_t) sf);

				memmove(sf_entry, tmp_sfe, tmp_len);

				sf->hdr.count -= 1;
				num_entries--;
				memset((void *)((__psint_t)sf_entry + tmp_len), 
								0, tmp_elen);

				/*
				 * reset the tmp value to the current
				 * pointer so we'll process the entry
				 * we just moved up
				 */
				tmp_sfe = sf_entry;

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
		}

		/*
		 * go onto next entry unless we've just junked an
		 * entry in which the current entry pointer points
		 * to an unprocessed entry.  have to take into zero-len
		 * entries into account in no modify mode since we
		 * calculate size based on next_sfe.
		 */
		next_sfe = (tmp_sfe == NULL)
			? (xfs_dir_sf_entry_t *) ((__psint_t) sf_entry
				+ ((!bad_sfnamelen)
					? xfs_dir_sf_entsize_byentry(sf_entry)
					: sizeof(xfs_dir_sf_entry_t) - 1
						+ namelen))
			: tmp_sfe;
	}

	/* sync up sizes and entry counts */

	if (sf->hdr.count != i)  {
		if (no_modify)  {
			do_warn(
	_("would have corrected entry count in directory %" PRIu64 " from %d to %d\n"),
			ino, sf->hdr.count, i);
		} else  {
			do_warn(
	_("corrected entry count in directory %" PRIu64 ", was %d, now %d\n"),
			ino, sf->hdr.count, i);
			sf->hdr.count = i;
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	if ((__psint_t) next_sfe - (__psint_t) sf != ino_dir_size)  {
		if (no_modify)  {
			do_warn(
	_("would have corrected directory %" PRIu64 " size from %" PRId64 "to %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfe - (intptr_t )sf);
		} else  {
			do_warn(
	_("corrected directory %" PRIu64 " size, was %" PRId64 ", now %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfe - (intptr_t)sf);

			dip->di_size = cpu_to_be64((__psint_t)next_sfe 
							- (__psint_t)sf);
			*dino_dirty = 1;
			*repair = 1;
		}
	}
	/*
	 * check parent (..) entry
	 */
	xfs_dir_sf_get_dirino(&sf->hdr.parent, parent);

	/*
	 * if parent entry is bogus, null it out.  we'll fix it later .
	 */
	if (verify_inum(mp, *parent))  {
		*parent = NULLFSINO;

		do_warn(
	_("bogus .. inode number (%" PRIu64 ") in directory inode %" PRIu64 ", "),
				*parent, ino);
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			xfs_dir_sf_put_dirino(parent, &sf->hdr.parent);
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
			xfs_dir_sf_put_dirino(parent, &sf->hdr.parent);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(
	_("would have corrected root directory %" PRIu64 " .. entry from %" PRIu64 " to %" PRIu64 "\n"),
				ino, *parent, ino);
		}
	} else if (ino == *parent && ino != mp->m_sb.sb_rootino)  {
		/*
		 * likewise, non-root directories can't have .. pointing
		 * to .
		 */
		*parent = NULLFSINO;
		do_warn(_("bad .. entry in dir ino %" PRIu64 ", points to self, "),
			ino);
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			xfs_dir_sf_put_dirino(parent, &sf->hdr.parent);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(_("would clear inode number\n"));
		}
	}

	return(0);
}

/*
 * freespace map for directory leaf blocks (1 bit per byte)
 * 1 == used, 0 == free
 */
size_t ts_dir_freemap_size = sizeof(da_freemap_t) * DA_BMAP_SIZE;

void
init_da_freemap(da_freemap_t *dir_freemap)
{
	memset(dir_freemap, 0, sizeof(da_freemap_t) * DA_BMAP_SIZE);
}

/*
 * sets directory freemap, returns 1 if there is a conflict
 * returns 0 if everything's good.  the range [start, stop) is set.
 * right now, we just use the static array since only one directory
 * block will be processed at once even though the interface allows
 * you to pass in arbitrary da_freemap_t array's.
 *
 * Within a char, the lowest bit of the char represents the byte with
 * the smallest address
 */
int
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
 * returns 0 if holemap is consistent with reality (as expressed by
 * the da_freemap_t).  returns 1 if there's a conflict.
 */
int
verify_da_freemap(xfs_mount_t *mp, da_freemap_t *map, da_hole_map_t *holes,
			xfs_ino_t ino, xfs_dablk_t da_bno)
{
	int i, j, start, len;
	const da_freemap_t mask = 0x1;

	for (i = 0; i < XFS_DIR_LEAF_MAPSIZE; i++)  {
		if (holes->hentries[i].size == 0)
			continue;

		start = holes->hentries[i].base;
		len = holes->hentries[i].size;

		if (start >= mp->m_sb.sb_blocksize ||
				start + len > mp->m_sb.sb_blocksize)  {
			do_warn(
	_("hole (start %d, len %d) out of range, block %d, dir ino %" PRIu64 "\n"),
				start, len, da_bno, ino);
			return(1);
		}

		for (j = start; j < start + len; j++)  {
			if ((map[j / NBBY] & (mask << (j % NBBY))) != 0)  {
				/*
				 * bad news -- hole claims a used byte is free
				 */
				do_warn(
			_("hole claims used byte %d, block %d, dir ino %" PRIu64 "\n"),
					j, da_bno, ino);
				return(1);
			}
		}
	}

	return(0);
}

void
process_da_freemap(xfs_mount_t *mp, da_freemap_t *map, da_hole_map_t *holes)
{
	int i, j, in_hole, start, length, smallest, num_holes;
	const da_freemap_t mask = 0x1;

	num_holes = in_hole = start = length = 0;

	for (i = 0; i < mp->m_sb.sb_blocksize; i++)  {
		if ((map[i / NBBY] & (mask << (i % NBBY))) == 0)  {
			/*
			 * byte is free (unused)
			 */
			if (in_hole == 1)
				continue;
			/*
			 * start of a new hole
			 */
			in_hole = 1;
			start = i;
		} else  {
			/*
			 * byte is used
			 */
			if (in_hole == 0)
				continue;
			/*
			 * end of a hole
			 */
			in_hole = 0;
			/*
			 * if the hole disappears, throw it away
			 */
			length = i - start;

			if (length <= 0)
				continue;

			num_holes++;

			for (smallest = j = 0; j < XR_DA_LEAF_MAPSIZE; j++)  {
				if (holes->hentries[j].size <
						holes->hentries[smallest].size)
					smallest = j;

			}
			if (length > holes->hentries[smallest].size)  {
				holes->hentries[smallest].base = start;
				holes->hentries[smallest].size = length;
			}
		}
	}

	/*
	 * see if we have a big hole at the end
	 */
	if (in_hole == 1)  {
		/*
		 * duplicate of hole placement code above
		 */
		length = i - start;

		if (length > 0)  {
			num_holes++;

			for (smallest = j = 0; j < XR_DA_LEAF_MAPSIZE; j++)  {
				if (holes->hentries[j].size <
						holes->hentries[smallest].size)
					smallest = j;

			}
			if (length > holes->hentries[smallest].size)  {
				holes->hentries[smallest].base = start;
				holes->hentries[smallest].size = length;
			}
		}
	}

	holes->lost_holes = MAX(num_holes - XR_DA_LEAF_MAPSIZE, 0);
	holes->num_holes = num_holes;

	return;
}

/*
 * returns 1 if the hole info doesn't match, 0 if it does
 */
/* ARGSUSED */
int
compare_da_freemaps(xfs_mount_t *mp, da_hole_map_t *holemap,
			da_hole_map_t *block_hmap, int entries,
			xfs_ino_t ino, xfs_dablk_t da_bno)
{
	int i, k, res, found;

	res = 0;

	/*
	 * we chop holemap->lost_holes down to being two-valued
	 * value (1 or 0) for the test  because the filesystem
	 * value is two-valued
	 */
	if ((holemap->lost_holes > 0 ? 1 : 0) != block_hmap->lost_holes)  {
		if (verbose)  {
			do_warn(
		_("- derived hole value %d, saw %d, block %d, dir ino %" PRIu64 "\n"),
				holemap->lost_holes, block_hmap->lost_holes,
				da_bno, ino);
			res = 1;
		} else
			return(1);
	}

	for (i = 0; i < entries; i++)  {
		for (found = k = 0; k < entries; k++)  {
			if (holemap->hentries[i].base ==
					block_hmap->hentries[k].base
					&& holemap->hentries[i].size ==
					block_hmap->hentries[k].size)
				found = 1;
		}
		if (!found)  {
			if (verbose)  {
				do_warn(
_("- derived hole (base %d, size %d) in block %d, dir inode %" PRIu64 " not found\n"),
					holemap->hentries[i].base,
					holemap->hentries[i].size,
					da_bno, ino);
				res = 1;
			} else
				return(1);
		}
	}

	return(res);
}

#if 0
void
test(xfs_mount_t *mp)
{
	int i = 0;
	da_hole_map_t	holemap;

	init_da_freemap(dir_freemap);
	memset(&holemap, 0, sizeof(da_hole_map_t));

	set_da_freemap(mp, dir_freemap, 0, 50);
	set_da_freemap(mp, dir_freemap, 100, 126);
	set_da_freemap(mp, dir_freemap, 126, 129);
	set_da_freemap(mp, dir_freemap, 130, 131);
	set_da_freemap(mp, dir_freemap, 150, 160);
	process_da_freemap(mp, dir_freemap, &holemap);

	return;
}
#endif


/*
 * walk tree from root to the left-most leaf block reading in
 * blocks and setting up cursor.  passes back file block number of the
 * left-most leaf block if successful (bno).  returns 1 if successful,
 * 0 if unsuccessful.
 */
int
traverse_int_dablock(xfs_mount_t	*mp,
		da_bt_cursor_t		*da_cursor,
		xfs_dablk_t		*rbno,
		int			whichfork)
{
	xfs_dablk_t		bno;
	int			i;
	xfs_da_intnode_t	*node;
	xfs_dfsbno_t		fsbno;
	xfs_buf_t		*bp;

	/*
	 * traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along
	 * the way.
	 */
	bno = 0;
	i = -1;
	node = NULL;
	da_cursor->active = 0;

	do {
		/*
		 * read in each block along the way and set up cursor
		 */
		fsbno = blkmap_get(da_cursor->blkmap, bno);

		if (fsbno == NULLDFSBNO)
			goto error_out;

		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
				XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			if (whichfork == XFS_DATA_FORK)
				do_warn(
	_("can't read block %u (fsbno %" PRIu64 ") for directory inode %" PRIu64 "\n"),
					bno, fsbno, da_cursor->ino);
			else
				do_warn(
	_("can't read block %u (fsbno %" PRIu64 ") for attrbute fork of inode %" PRIu64 "\n"),
					bno, fsbno, da_cursor->ino);
			goto error_out;
		}

		node = (xfs_da_intnode_t *)XFS_BUF_PTR(bp);

		if (be16_to_cpu(node->hdr.info.magic) != XFS_DA_NODE_MAGIC)  {
			do_warn(_("bad dir/attr magic number in inode %" PRIu64 ", "
				  "file bno = %u, fsbno = %" PRIu64 "\n"),
				da_cursor->ino, bno, fsbno);
			libxfs_putbuf(bp);
			goto error_out;
		}
		if (be16_to_cpu(node->hdr.count) >
						mp->m_dir_node_ents)  {
			do_warn(_("bad record count in inode %" PRIu64 ", "
				  "count = %d, max = %d\n"),
				da_cursor->ino,
				be16_to_cpu(node->hdr.count),
				mp->m_dir_node_ents);
			libxfs_putbuf(bp);
			goto error_out;
		}

		/*
		 * maintain level counter
		 */
		if (i == -1)
			i = da_cursor->active = be16_to_cpu(node->hdr.level);
		else  {
			if (be16_to_cpu(node->hdr.level) == i - 1)  {
				i--;
			} else  {
				if (whichfork == XFS_DATA_FORK)
					do_warn(_("bad directory btree for "
						  "directory inode %" PRIu64 "\n"),
						da_cursor->ino);
				else
					do_warn(_("bad attribute fork btree "
						  "for inode %" PRIu64 "\n"),
						da_cursor->ino);
				libxfs_putbuf(bp);
				goto error_out;
			}
		}

		da_cursor->level[i].hashval = be32_to_cpu(
							node->btree[0].hashval);
		da_cursor->level[i].bp = bp;
		da_cursor->level[i].bno = bno;
		da_cursor->level[i].index = 0;
#ifdef XR_DIR_TRACE
		da_cursor->level[i].n = XFS_BUF_TO_DA_INTNODE(bp);
#endif

		/*
		 * set up new bno for next level down
		 */
		bno = be32_to_cpu(node->btree[0].before);
	} while (node != NULL && i > 1);

	/*
	 * now return block number and get out
	 */
	*rbno = da_cursor->level[0].bno = bno;
	return(1);

error_out:
	while (i > 1 && i <= da_cursor->active)  {
		libxfs_putbuf(da_cursor->level[i].bp);
		i++;
	}

	return(0);
}

/*
 * blow out buffer for this level and all the rest above as well
 * if error == 0, we are not expecting to encounter any unreleased
 * buffers (e.g. if we do, it's a mistake).  if error == 1, we're
 * in an error-handling case so unreleased buffers may exist.
 */
void
release_da_cursor_int(xfs_mount_t	*mp,
			da_bt_cursor_t	*cursor,
			int		prev_level,
			int		error)
{
	int	level = prev_level + 1;

	if (cursor->level[level].bp != NULL)  {
		if (!error)  {
			do_warn(_("release_da_cursor_int got unexpected "
				  "non-null bp, dabno = %u\n"),
				cursor->level[level].bno);
		}
		ASSERT(error != 0);

		libxfs_putbuf(cursor->level[level].bp);
		cursor->level[level].bp = NULL;
	}

	if (level < cursor->active)
		release_da_cursor_int(mp, cursor, level, error);

	return;
}

void
release_da_cursor(xfs_mount_t	*mp,
		da_bt_cursor_t	*cursor,
		int		prev_level)
{
	release_da_cursor_int(mp, cursor, prev_level, 0);
}

void
err_release_da_cursor(xfs_mount_t	*mp,
			da_bt_cursor_t	*cursor,
			int		prev_level)
{
	release_da_cursor_int(mp, cursor, prev_level, 1);
}

/*
 * like traverse_int_dablock only it does far less checking
 * and doesn't maintain the cursor.  Just gets you to the
 * leftmost block in the directory.  returns the fsbno
 * of that block if successful, NULLDFSBNO if not.
 */
xfs_dfsbno_t
get_first_dblock_fsbno(xfs_mount_t	*mp,
			xfs_ino_t	ino,
			xfs_dinode_t	*dino)
{
	xfs_dablk_t		bno;
	int			i;
	xfs_da_intnode_t	*node;
	xfs_dfsbno_t		fsbno;
	xfs_buf_t		*bp;

	/*
	 * traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along
	 * the way.
	 */
	bno = 0;
	i = -1;
	node = NULL;

	fsbno = get_bmapi(mp, dino, ino, bno, XFS_DATA_FORK);

	if (fsbno == NULLDFSBNO)  {
		do_warn(_("bmap of block #%u of inode %" PRIu64 " failed\n"),
			bno, ino);
		return(fsbno);
	}

	if (be64_to_cpu(dino->di_size) <= XFS_LBSIZE(mp))
		return(fsbno);

	do {
		/*
		 * walk down left side of btree, release buffers as you
		 * go.  if the root block is a leaf (single-level btree),
		 * just return it.
		 *
		 */

		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
				XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_warn(
	_("can't read block %u (fsbno %" PRIu64 ") for directory inode %" PRIu64 "\n"),
				bno, fsbno, ino);
			return(NULLDFSBNO);
		}

		node = (xfs_da_intnode_t *)XFS_BUF_PTR(bp);

		if (XFS_DA_NODE_MAGIC !=
		    be16_to_cpu(node->hdr.info.magic))  {
			do_warn(
	_("bad dir/attr magic number in inode %" PRIu64 ", file bno = %u, fsbno = %" PRIu64 "\n"),
				ino, bno, fsbno);
			libxfs_putbuf(bp);
			return(NULLDFSBNO);
		}

		if (i == -1)
			i = be16_to_cpu(node->hdr.level);
		bno = be32_to_cpu(node->btree[0].before);

		libxfs_putbuf(bp);

		fsbno = get_bmapi(mp, dino, ino, bno, XFS_DATA_FORK);

		if (fsbno == NULLDFSBNO)  {
			do_warn(_("bmap of block #%u of inode %" PRIu64 " failed\n"),
				bno, ino);
			return(NULLDFSBNO);
		}

		i--;
	} while(i > 0);

	return(fsbno);
}

/*
 * make sure that all entries in all blocks along the right side of
 * of the tree are used and hashval's are consistent.  level is the
 * level of the descendent block.  returns 0 if good (even if it had
 * to be fixed up), and 1 if bad.  The right edge of the tree is
 * technically a block boundary.  this routine should be used then
 * instead of verify_da_path().
 */
int
verify_final_da_path(xfs_mount_t	*mp,
		da_bt_cursor_t		*cursor,
		const int		p_level)
{
	xfs_da_intnode_t	*node;
	xfs_dahash_t		hashval;
	int			bad = 0;
	int			entry;
	int			this_level = p_level + 1;

#ifdef XR_DIR_TRACE
	fprintf(stderr, "in verify_final_da_path, this_level = %d\n",
		this_level);
#endif
	/*
	 * the index should point to the next "unprocessed" entry
	 * in the block which should be the final (rightmost) entry
	 */
	entry = cursor->level[this_level].index;
	node = (xfs_da_intnode_t *)XFS_BUF_PTR(cursor->level[this_level].bp);
	/*
	 * check internal block consistency on this level -- ensure
	 * that all entries are used, encountered and expected hashvals
	 * match, etc.
	 */
	if (entry != be16_to_cpu(node->hdr.count) - 1)  {
		do_warn(_("directory/attribute block used/count "
			  "inconsistency - %d/%hu\n"),
			entry, be16_to_cpu(node->hdr.count));
		bad++;
	}
	/*
	 * hash values monotonically increasing ???
	 */
	if (cursor->level[this_level].hashval >= 
				be32_to_cpu(node->btree[entry].hashval)) {
		do_warn(_("directory/attribute block hashvalue inconsistency, "
			  "expected > %u / saw %u\n"),
			cursor->level[this_level].hashval,
			be32_to_cpu(node->btree[entry].hashval));
		bad++;
	}
	if (be32_to_cpu(node->hdr.info.forw) != 0)  {
		do_warn(_("bad directory/attribute forward block pointer, "
			  "expected 0, saw %u\n"),
			be32_to_cpu(node->hdr.info.forw));
		bad++;
	}
	if (bad) {
		do_warn(_("bad directory block in dir ino %" PRIu64 "\n"),
			cursor->ino);
		return(1);
	}
	/*
	 * keep track of greatest block # -- that gets
	 * us the length of the directory
	 */
	if (cursor->level[this_level].bno > cursor->greatest_bno)
		cursor->greatest_bno = cursor->level[this_level].bno;

	/*
	 * ok, now check descendant block number against this level
	 */
	if (cursor->level[p_level].bno != be32_to_cpu(
						node->btree[entry].before)) {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "bad directory btree pointer, child bno should "
				"be %d, block bno is %d, hashval is %u\n",
			be16_to_cpu(node->btree[entry].before),
			cursor->level[p_level].bno,
			cursor->level[p_level].hashval);
		fprintf(stderr, "verify_final_da_path returns 1 (bad) #1a\n");
#endif
		return(1);
	}

	if (cursor->level[p_level].hashval != be32_to_cpu(
						node->btree[entry].hashval)) {
		if (!no_modify)  {
			do_warn(_("correcting bad hashval in non-leaf "
				  "dir/attr block\n\tin (level %d) in "
				  "inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
			node->btree[entry].hashval = cpu_to_be32(
						cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else  {
			do_warn(_("would correct bad hashval in non-leaf "
				  "dir/attr block\n\tin (level %d) in "
				  "inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
		}
	}

	/*
	 * Note: squirrel hashval away _before_ releasing the
	 * buffer, preventing a use-after-free problem.
	 */
	hashval = be32_to_cpu(node->btree[entry].hashval);

	/*
	 * release/write buffer
	 */
	ASSERT(cursor->level[this_level].dirty == 0 ||
		(cursor->level[this_level].dirty && !no_modify));

	if (cursor->level[this_level].dirty && !no_modify)
		libxfs_writebuf(cursor->level[this_level].bp, 0);
	else
		libxfs_putbuf(cursor->level[this_level].bp);

	cursor->level[this_level].bp = NULL;

	/*
	 * bail out if this is the root block (top of tree)
	 */
	if (this_level >= cursor->active)  {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "verify_final_da_path returns 0 (ok)\n");
#endif
		return(0);
	}
	/*
	 * set hashvalue to correctly reflect the now-validated
	 * last entry in this block and continue upwards validation
	 */
	cursor->level[this_level].hashval = hashval;
	return(verify_final_da_path(mp, cursor, this_level));
}

/*
 * Verifies the path from a descendant block up to the root.
 * Should be called when the descendant level traversal hits
 * a block boundary before crossing the boundary (reading in a new
 * block).
 *
 * the directory/attr btrees work differently to the other fs btrees.
 * each interior block contains records that are <hashval, bno>
 * pairs.  The bno is a file bno, not a filesystem bno.  The last
 * hashvalue in the block <bno> will be <hashval>.  BUT unlike
 * the freespace btrees, the *last* value in each block gets
 * propagated up the tree instead of the first value in each block.
 * that is, the interior records point to child blocks and the *greatest*
 * hash value contained by the child block is the one the block above
 * uses as the key for the child block.
 *
 * level is the level of the descendent block.  returns 0 if good,
 * and 1 if bad.  The descendant block may be a leaf block.
 *
 * the invariant here is that the values in the cursor for the
 * levels beneath this level (this_level) and the cursor index
 * for this level *must* be valid.
 *
 * that is, the hashval/bno info is accurate for all
 * DESCENDANTS and match what the node[index] information
 * for the current index in the cursor for this level.
 *
 * the index values in the cursor for the descendant level
 * are allowed to be off by one as they will reflect the
 * next entry at those levels to be processed.
 *
 * the hashvalue for the current level can't be set until
 * we hit the last entry in the block so, it's garbage
 * until set by this routine.
 *
 * bno and bp for the current block/level are always valid
 * since they have to be set so we can get a buffer for the
 * block.
 */
int
verify_da_path(xfs_mount_t	*mp,
	da_bt_cursor_t		*cursor,
	const int		p_level)
{
	xfs_da_intnode_t	*node;
	xfs_da_intnode_t	*newnode;
	xfs_dfsbno_t		fsbno;
	xfs_dablk_t		dabno;
	xfs_buf_t		*bp;
	int			bad;
	int			entry;
	int			this_level = p_level + 1;

	/*
	 * index is currently set to point to the entry that
	 * should be processed now in this level.
	 */
	entry = cursor->level[this_level].index;
	node = (xfs_da_intnode_t *)XFS_BUF_PTR(cursor->level[this_level].bp);

	/*
	 * if this block is out of entries, validate this
	 * block and move on to the next block.
	 * and update cursor value for said level
	 */
	if (entry >= be16_to_cpu(node->hdr.count))  {
		/*
		 * update the hash value for this level before
		 * validating it.  bno value should be ok since
		 * it was set when the block was first read in.
		 */
		cursor->level[this_level].hashval =
				be32_to_cpu(node->btree[entry - 1].hashval);

		/*
		 * keep track of greatest block # -- that gets
		 * us the length of the directory
		 */
		if (cursor->level[this_level].bno > cursor->greatest_bno)
			cursor->greatest_bno = cursor->level[this_level].bno;

		/*
		 * validate the path for the current used-up block
		 * before we trash it
		 */
		if (verify_da_path(mp, cursor, this_level))
			return(1);
		/*
		 * ok, now get the next buffer and check sibling pointers
		 */
		dabno = be32_to_cpu(node->hdr.info.forw);
		ASSERT(dabno != 0);
		fsbno = blkmap_get(cursor->blkmap, dabno);

		if (fsbno == NULLDFSBNO) {
			do_warn(_("can't get map info for block %u "
				  "of directory inode %" PRIu64 "\n"),
				dabno, cursor->ino);
			return(1);
		}

		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
				XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_warn(
	_("can't read block %u (%" PRIu64 ") for directory inode %" PRIu64 "\n"),
				dabno, fsbno, cursor->ino);
			return(1);
		}

		newnode = (xfs_da_intnode_t *)XFS_BUF_PTR(bp);
		/*
		 * verify magic number and back pointer, sanity-check
		 * entry count, verify level
		 */
		bad = 0;
		if (XFS_DA_NODE_MAGIC != be16_to_cpu(newnode->hdr.info.magic)) {
			do_warn(
	_("bad magic number %x in block %u (%" PRIu64 ") for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.info.magic),
				dabno, fsbno, cursor->ino);
			bad++;
		}
		if (be32_to_cpu(newnode->hdr.info.back) != 
						cursor->level[this_level].bno) {
			do_warn(
	_("bad back pointer in block %u (%"PRIu64 ") for directory inode %" PRIu64 "\n"),
				dabno, fsbno, cursor->ino);
			bad++;
		}
		if (be16_to_cpu(newnode->hdr.count) > mp->m_dir_node_ents) {
			do_warn(
	_("entry count %d too large in block %u (%" PRIu64 ") for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.count),
				dabno, fsbno, cursor->ino);
			bad++;
		}
		if (be16_to_cpu(newnode->hdr.level) != this_level) {
			do_warn(
	_("bad level %d in block %u (%" PRIu64 ") for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.level),
				dabno, fsbno, cursor->ino);
			bad++;
		}
		if (bad)  {
#ifdef XR_DIR_TRACE
			fprintf(stderr, "verify_da_path returns 1 (bad) #4\n");
#endif
			libxfs_putbuf(bp);
			return(1);
		}
		/*
		 * update cursor, write out the *current* level if
		 * required.  don't write out the descendant level
		 */
		ASSERT(cursor->level[this_level].dirty == 0 ||
			(cursor->level[this_level].dirty && !no_modify));

		if (cursor->level[this_level].dirty && !no_modify)
			libxfs_writebuf(cursor->level[this_level].bp, 0);
		else
			libxfs_putbuf(cursor->level[this_level].bp);
		cursor->level[this_level].bp = bp;
		cursor->level[this_level].dirty = 0;
		cursor->level[this_level].bno = dabno;
		cursor->level[this_level].hashval =
					be32_to_cpu(newnode->btree[0].hashval);
#ifdef XR_DIR_TRACE
		cursor->level[this_level].n = newnode;
#endif
		node = newnode;

		entry = cursor->level[this_level].index = 0;
	}
	/*
	 * ditto for block numbers
	 */
	if (cursor->level[p_level].bno !=
				be32_to_cpu(node->btree[entry].before))  {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "bad directory btree pointer, child bno "
			"should be %d, block bno is %d, hashval is %u\n",
			be32_to_cpu(node->btree[entry].before),
			cursor->level[p_level].bno,
			cursor->level[p_level].hashval);
		fprintf(stderr, "verify_da_path returns 1 (bad) #1a\n");
#endif
		return(1);
	}
	/*
	 * ok, now validate last hashvalue in the descendant
	 * block against the hashval in the current entry
	 */
	if (cursor->level[p_level].hashval !=
				be32_to_cpu(node->btree[entry].hashval))  {
		if (!no_modify)  {
			do_warn(_("correcting bad hashval in interior "
				  "dir/attr block\n\tin (level %d) in "
				  "inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
			node->btree[entry].hashval = cpu_to_be32(
						cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else  {
			do_warn(_("would correct bad hashval in interior "
				  "dir/attr block\n\tin (level %d) in "
				  "inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
		}
	}
	/*
	 * increment index for this level to point to next entry
	 * (which should point to the next descendant block)
	 */
	cursor->level[this_level].index++;
#ifdef XR_DIR_TRACE
	fprintf(stderr, "verify_da_path returns 0 (ok)\n");
#endif
	return(0);
}

size_t ts_dirbuf_size = 64*1024;

/*
 * called by both node dir and leaf dir processing routines
 * validates all contents *but* the sibling pointers (forw/back)
 * and the magic number.
 *
 * returns 0 if the directory is ok or has been brought to the
 * stage that it can be fixed up later (in phase 6),
 * 1 if it has to be junked.
 *
 * Right now we fix a lot of things (TBD == to be deleted).
 *
 *	incorrect . entries - inode # is corrected
 *	entries with mismatched hashvalue/name strings - hashvalue reset
 *	entries whose hashvalues are out-of-order - entry marked TBD
 *	.. entries with invalid inode numbers - entry marked TBD
 *	entries with invalid inode numbers - entry marked TBD
 *	multiple . entries - all but the first entry are marked TBD
 *	zero-length entries - entry is deleted
 *	entries with an out-of-bounds name index ptr - entry is deleted
 *
 * entries marked TBD have the first character of the name (which
 *	lives in the heap) have the first character in the name set
 *	to '/' -- an illegal value.
 *
 * entries deleted right here are deleted by blowing away the entry
 *	(but leaving the heap untouched).  any space that was used
 *	by the deleted entry will be reclaimed by the block freespace
 *	(da_freemap) processing code.
 *
 * if two entries claim the same space in the heap (say, due to
 * bad entry name index pointers), we lose the directory.  We could
 * try harder to fix this but it'll do for now.
 */
/* ARGSUSED */
int
process_leaf_dir_block(
	xfs_mount_t		*mp,
	xfs_dir_leafblock_t	*leaf,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino,
	xfs_dahash_t		last_hashval,	/* last hashval encountered */
	int			ino_discovery,
	blkmap_t		*blkmap,
	int			*dot,
	int			*dotdot,
	xfs_ino_t		*parent,
	int			*buf_dirty,	/* is buffer dirty? */
	xfs_dahash_t		*next_hashval)	/* greatest hashval in block */
{
	xfs_ino_t			lino;
	xfs_dir_leaf_entry_t		*entry;
	xfs_dir_leaf_entry_t		*s_entry;
	xfs_dir_leaf_entry_t		*d_entry;
	xfs_dir_leafblock_t		*new_leaf;
	char				*first_byte;
	xfs_dir_leaf_name_t		*namest;
	ino_tree_node_t			*irec_p;
	int				num_entries;
	xfs_dahash_t			hashval;
	int				i;
	int				nm_illegal;
	int				bytes;
	int				start;
	int				stop;
	int				res = 0;
	int				ino_off;
	int				first_used;
	int				bytes_used;
	int				reset_holes;
	int				zero_len_entries;
	char				fname[MAXNAMELEN + 1];
	da_hole_map_t			holemap;
	da_hole_map_t			bholemap;
	unsigned char			*dir_freemap = ts_dir_freemap();

#ifdef XR_DIR_TRACE
	fprintf(stderr, "\tprocess_leaf_dir_block - ino %" PRIu64 "\n", ino);
#endif

	/*
	 * clear static dir block freespace bitmap
	 */
	init_da_freemap(dir_freemap);

	*buf_dirty = 0;
	first_used = mp->m_sb.sb_blocksize;
	zero_len_entries = 0;
	bytes_used = 0;

	i = stop = sizeof(xfs_dir_leaf_hdr_t);
	if (set_da_freemap(mp, dir_freemap, 0, stop))  {
		do_warn(
_("directory block header conflicts with used space in directory inode %" PRIu64 "\n"),
			ino);
		return(1);
	}

	/*
	 * verify structure:  monotonically increasing hash value for
	 * all leaf entries, indexes for all entries must be within
	 * this fs block (trivially true for 64K blocks).  also track
	 * used space so we can check the freespace map.  check for
	 * zero-length entries.  for now, if anything's wrong, we
	 * junk the directory and we'll pick up no-longer referenced
	 * inodes on a later pass.
	 */
	for (i = 0, entry = &leaf->entries[0];
			i < be16_to_cpu(leaf->hdr.count);
			i++, entry++)  {
		/*
		 * check that the name index isn't out of bounds
		 * if it is, delete the entry since we can't
		 * grab the inode #.
		 */
		if (be16_to_cpu(entry->nameidx) >=
		    mp->m_sb.sb_blocksize)  {
			if (!no_modify)  {
				*buf_dirty = 1;

				if (be16_to_cpu(leaf->hdr.count) > 1)  {
					do_warn(
_("nameidx %d for entry #%d, bno %d, ino %" PRIu64 " > fs blocksize, deleting entry\n"),
						be16_to_cpu(entry->nameidx),
						i, da_bno, ino);
					ASSERT(be16_to_cpu(leaf->hdr.count) > i);

					bytes = (be16_to_cpu(leaf->hdr.count) - i) *
						sizeof(xfs_dir_leaf_entry_t);

					/*
					 * compress table unless we're
					 * only dealing with 1 entry
					 * (the last one) in which case
					 * just zero it.
					 */
					if (bytes >
					    sizeof(xfs_dir_leaf_entry_t))  {
						memmove(entry, entry + 1,
							bytes);
						memset((void *)
						((__psint_t) entry + bytes), 0,
						sizeof(xfs_dir_leaf_entry_t));
					} else  {
						memset(entry, 0,
						sizeof(xfs_dir_leaf_entry_t));
					}

					/*
					 * sync vars to match smaller table.
					 * don't have to worry about freespace
					 * map since we haven't set it for
					 * this entry yet.
					 */
					be16_add_cpu(&leaf->hdr.count, -1);
					i--;
					entry--;
				} else  {
					do_warn(
_("nameidx %d, entry #%d, bno %d, ino %" PRIu64 " > fs blocksize, marking entry bad\n"),
						be16_to_cpu(entry->nameidx),
						i, da_bno, ino);
					entry->nameidx = cpu_to_be16(
						mp->m_sb.sb_blocksize -
						sizeof(xfs_dir_leaf_name_t));
					namest = xfs_dir_leaf_namestruct(leaf,
						be16_to_cpu(entry->nameidx));
					lino = NULLFSINO;
					xfs_dir_sf_put_dirino(&lino,
							&namest->inumber);
					namest->name[0] = '/';
				}
			} else  {
				do_warn(
_("nameidx %d, entry #%d, bno %d, ino %" PRIu64 " > fs blocksize, would delete entry\n"),
					be16_to_cpu(entry->nameidx),
					i, da_bno, ino);
			}
			continue;
		}
		/*
		 * inode processing -- make sure the inode
		 * is in our tree or we add it to the uncertain
		 * list if the inode # is valid.  if namelen is 0,
		 * we can still try for the inode as long as nameidx
		 * is ok.
		 */
		namest = xfs_dir_leaf_namestruct(leaf, 
						be16_to_cpu(entry->nameidx));
		xfs_dir_sf_get_dirino(&namest->inumber, &lino);

		/*
		 * we may have to blow out an entry because of bad
		 * inode numbers.  do NOT touch the name until after
		 * we've computed the hashvalue and done a namecheck()
		 * on the name.
		 */
		if (!ino_discovery && lino == NULLFSINO)  {
			/*
			 * don't do a damned thing.  We already
			 * found this (or did it ourselves) during
			 * phase 3.
			 */
		} else if (verify_inum(mp, lino))  {
			/*
			 * bad inode number.  clear the inode
			 * number and the entry will get removed
			 * later.  We don't trash the directory
			 * since it's still structurally intact.
			 */
			do_warn(
_("invalid ino number %" PRIu64 " in dir ino %" PRIu64 ", entry #%d, bno %d\n"),
				lino, ino, i, da_bno);
			if (!no_modify)  {
				do_warn(
				_("\tclearing ino number in entry %d...\n"),
					i);
				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(
				_("\twould clear ino number in entry %d...\n"),
					i);
			}
		} else if (lino == mp->m_sb.sb_rbmino)  {
			do_warn(
_("entry #%d, bno %d in directory %" PRIu64 " references realtime bitmap inode %" PRIu64 "\n"),
				i, da_bno, ino, lino);
			if (!no_modify)  {
				do_warn(
				_("\tclearing ino number in entry %d...\n"),
					i);

				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(
				_("\twould clear ino number in entry %d...\n"),
					i);
			}
		} else if (lino == mp->m_sb.sb_rsumino)  {
			do_warn(
_("entry #%d, bno %d in directory %" PRIu64 " references realtime summary inode %" PRIu64 "\n"),
				i, da_bno, ino, lino);
			if (!no_modify)  {
				do_warn(
				_("\tclearing ino number in entry %d...\n"), i);

				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(
				_("\twould clear ino number in entry %d...\n"),
					i);
			}
		} else if (lino == mp->m_sb.sb_uquotino)  {
			do_warn(
_("entry #%d, bno %d in directory %" PRIu64 " references user quota inode %" PRIu64 "\n"),
				i, da_bno, ino, lino);
			if (!no_modify)  {
				do_warn(
				_("\tclearing ino number in entry %d...\n"),
					i);

				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(
				_("\twould clear ino number in entry %d...\n"),
					i);
			}
		} else if (lino == mp->m_sb.sb_gquotino)  {
			do_warn(
_("entry #%d, bno %d in directory %" PRIu64 " references group quota inode %" PRIu64 "\n"),
				i, da_bno, ino, lino);
			if (!no_modify)  {
				do_warn(
				_("\tclearing ino number in entry %d...\n"),
					i);

				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(
				_("\twould clear ino number in entry %d...\n"),
					i);
			}
		} else if ((irec_p = find_inode_rec(mp,
				XFS_INO_TO_AGNO(mp, lino),
				XFS_INO_TO_AGINO(mp, lino))) != NULL)  {
			/*
			 * inode recs should have only confirmed
			 * inodes in them
			 */
			ino_off = XFS_INO_TO_AGINO(mp, lino) -
					irec_p->ino_startnum;
			ASSERT(is_inode_confirmed(irec_p, ino_off));
			/*
			 * if inode is marked free and we're in inode
			 * discovery mode, leave the entry alone for now.
			 * if the inode turns out to be used, we'll figure
			 * that out when we scan it.  If the inode really
			 * is free, we'll hit this code again in phase 4
			 * after we've finished inode discovery and blow
			 * out the entry then.
			 */
			if (!ino_discovery && is_inode_free(irec_p, ino_off))  {
				if (!no_modify)  {
					do_warn(
_("entry references free inode %" PRIu64 " in directory %" PRIu64 ", will clear entry\n"),
						lino, ino);
					lino = NULLFSINO;
					xfs_dir_sf_put_dirino(&lino,
							&namest->inumber);
					*buf_dirty = 1;
				} else  {
					do_warn(
_("entry references free inode %" PRIu64 " in directory %" PRIu64 ", would clear entry\n"),
						lino, ino);
				}
			}
		} else if (ino_discovery)  {
			add_inode_uncertain(mp, lino, 0);
		} else  {
			do_warn(
	_("bad ino number %" PRIu64 " in dir ino %" PRIu64 ", entry #%d, bno %d\n"),
				lino, ino, i, da_bno);
			if (!no_modify)  {
				do_warn(_("clearing inode number...\n"));
				lino = NULLFSINO;
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				*buf_dirty = 1;
			} else  {
				do_warn(_("would clear inode number...\n"));
			}
		}
		/*
		 * if we have a zero-length entry, trash it.
		 * we may lose the inode (chunk) if we don't
		 * finish the repair successfully and the inode
		 * isn't mentioned anywhere else (like in the inode
		 * tree) but the alternative is to risk losing the
		 * entire directory by trying to use the next byte
		 * to turn the entry into a 1-char entry.  That's
		 * probably a safe bet but if it didn't work, we'd
		 * lose the entire directory the way we currently do
		 * things.  (Maybe we should change that later :-).
		 */
		if (entry->namelen == 0)  {
			*buf_dirty = 1;

			if (be16_to_cpu(leaf->hdr.count) > 1)  {
				do_warn(
	_("entry #%d, dir inode %" PRIu64 ", has zero-len name, deleting entry\n"),
					i, ino);
				ASSERT(be16_to_cpu(leaf->hdr.count) > i);

				bytes = (be16_to_cpu(leaf->hdr.count) - i) *
						sizeof(xfs_dir_leaf_entry_t);

				/*
				 * compress table unless we're
				 * only dealing with 1 entry
				 * (the last one) in which case
				 * just zero it.
				 */
				if (bytes > sizeof(xfs_dir_leaf_entry_t))  {
					memmove(entry, entry + 1, bytes);
					memset((void *)((__psint_t) entry + 
							bytes), 0,
						sizeof(xfs_dir_leaf_entry_t));
				} else  {
					memset(entry, 0,
						sizeof(xfs_dir_leaf_entry_t));
				}

				/*
				 * sync vars to match smaller table.
				 * don't have to worry about freespace
				 * map since we haven't set it for
				 * this entry yet.
				 */
				be16_add_cpu(&leaf->hdr.count, -1);
				i--;
				entry--;
			} else  {
				/*
				 * if it's the only entry, preserve the
				 * inode number for now
				 */
				do_warn(
	_("entry #%d, dir inode %" PRIu64 ", has zero-len name, marking entry bad\n"),
					i, ino);
				entry->nameidx = cpu_to_be16(
						mp->m_sb.sb_blocksize -
						sizeof(xfs_dir_leaf_name_t));
				namest = xfs_dir_leaf_namestruct(leaf,
						be16_to_cpu(entry->nameidx));
				xfs_dir_sf_put_dirino(&lino, &namest->inumber);
				namest->name[0] = '/';
			}
		} else if (be16_to_cpu(entry->nameidx) + entry->namelen > 
							XFS_LBSIZE(mp))  {
			do_warn(
_("bad size, entry #%d in dir inode %" PRIu64 ", block %u -- entry overflows block\n"),
				i, ino, da_bno);

			return(1);
		}

		start = (__psint_t)&leaf->entries[i] - (__psint_t)leaf;;
		stop = start + sizeof(xfs_dir_leaf_entry_t);

		if (set_da_freemap(mp, dir_freemap, start, stop))  {
			do_warn(
_("dir entry slot %d in block %u conflicts with used space in dir inode %" PRIu64 "\n"),
				i, da_bno, ino);
			return(1);
		}

		/*
		 * check if the name is legal.  if so, then
		 * check that the name and hashvalues match.
		 *
		 * if the name is illegal, we don't check the
		 * hashvalue computed from it.  we just make
		 * sure that the hashvalue in the entry is
		 * monotonically increasing wrt to the previous
		 * entry.
		 *
		 * Note that we do NOT have to check the length
		 * because the length is stored in a one-byte
		 * unsigned int which max's out at MAXNAMELEN
		 * making it impossible for the stored length
		 * value to be out of range.
		 */
		memmove(fname, namest->name, entry->namelen);
		fname[entry->namelen] = '\0';
		hashval = libxfs_da_hashname((uchar_t *) fname, entry->namelen);

		/*
		 * only complain about illegal names in phase 3 (when
		 * inode discovery is turned on).  Otherwise, we'd complain
		 * a lot during phase 4.  If the name is illegal, leave
		 * the hash value in that entry alone.
		 */
		nm_illegal = namecheck(fname, entry->namelen);

		if (ino_discovery && nm_illegal)  {
			/*
			 * junk the entry, illegal name
			 */
			if (!no_modify)  {
				do_warn(
_("illegal name \"%s\" in directory inode %" PRIu64 ", entry will be cleared\n"),
					fname, ino);
				namest->name[0] = '/';
				*buf_dirty = 1;
			} else  {
				do_warn(
_("illegal name \"%s\" in directory inode %" PRIu64 ", entry would be cleared\n"),
					fname, ino);
			}
		} else if (!nm_illegal &&
				be32_to_cpu(entry->hashval) != hashval) {
			/*
			 * try resetting the hashvalue to the correct
			 * value for the string, if the string has been
			 * corrupted, too, that will get picked up next
			 */
			do_warn(_("\tmismatched hash value for entry \"%s\"\n"),
				fname);
			if (!no_modify)  {
				do_warn(
		_("\t\tin directory inode %" PRIu64 ".  resetting hash value.\n"),
					ino);
				entry->hashval = cpu_to_be32(hashval);
				*buf_dirty = 1;
			} else  {
				do_warn(
		_("\t\tin directory inode %" PRIu64 ".  would reset hash value.\n"),
					ino);
			}
		}

		/*
		 * now we can mark entries with NULLFSINO's bad
		 */
		if (!no_modify && lino == NULLFSINO)  {
			namest->name[0] = '/';
			*buf_dirty = 1;
		}

		/*
		 * regardless of whether the entry has or hasn't been
		 * marked for deletion, the hash value ordering must
		 * be maintained.
		 */
		if (be32_to_cpu(entry->hashval) < last_hashval)  {
			/*
			 * blow out the entry -- set hashval to sane value
			 * and set the first character in the string to
			 * the illegal value '/'.  Reset the hash value
			 * to the last hashvalue so that verify_da_path
			 * will fix up the interior pointers correctly.
			 * the entry will be deleted later (by routines
			 * that need only the entry #).  We keep the
			 * inode number in the entry so we can attach
			 * the inode to the orphanage later.
			 */
			do_warn(_("\tbad hash ordering for entry \"%s\"\n"),
				fname);
			if (!no_modify)  {
				do_warn(
			_("\t\tin directory inode %" PRIu64 ".  will clear entry\n"),
					ino);
				entry->hashval = cpu_to_be32(last_hashval);
				namest->name[0] = '/';
				*buf_dirty = 1;
			} else  {
				do_warn(
			_("\t\tin directory inode %" PRIu64 ".  would clear entry\n"),
					ino);
			}
		}

		*next_hashval = last_hashval = be32_to_cpu(entry->hashval);

		/*
		 * if heap data conflicts with something,
		 * blow it out and skip the rest of the loop
		 */
		if (set_da_freemap(mp, dir_freemap, be16_to_cpu(entry->nameidx),
				be16_to_cpu(entry->nameidx)
					+ sizeof(xfs_dir_leaf_name_t)
					+ entry->namelen - 1))  {
			do_warn(
_("name \"%s\" (block %u, slot %d) conflicts with used space in dir inode %" PRIu64 "\n"),
				fname, da_bno, i, ino);
			if (!no_modify)  {
				entry->namelen = 0;
				*buf_dirty = 1;

				do_warn(
		_("will clear entry \"%s\" (#%d) in directory inode %" PRIu64 "\n"),
					fname, i, ino);
			} else  {
				do_warn(
		_("would clear entry \"%s\" (#%d)in directory inode %" PRIu64 "\n"),
					fname, i, ino);
			}
			continue;
		}

		/*
		 * keep track of heap stats (first byte used, total bytes used)
		 */
		if (be16_to_cpu(entry->nameidx) < first_used)
			first_used = be16_to_cpu(entry->nameidx);
		bytes_used += entry->namelen;

		/*
		 * special . or .. entry processing
		 */
		if (entry->namelen == 2 && namest->name[0] == '.' &&
						namest->name[1] == '.') {
			/*
			 * the '..' case
			 */
			if (!*dotdot) {
				(*dotdot)++;
				*parent = lino;
#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_leaf_dir_block found .. entry (parent) = %" PRIu64 "\n", lino);
#endif
				/*
				 * what if .. == .?  legal only in
				 * the root inode.  blow out entry
				 * and set parent to NULLFSINO otherwise.
				 */
				if (ino == lino &&
						ino != mp->m_sb.sb_rootino)  {
					*parent = NULLFSINO;
					do_warn(
			_("bad .. entry in dir ino %" PRIu64 ", points to self"),
						ino);
					if (!no_modify)  {
						do_warn(
						_("will clear entry\n"));
						namest->name[0] = '/';
						*buf_dirty = 1;
					} else  {
						do_warn(
						_("would clear entry\n"));
					}
				} else if (ino != lino &&
						ino == mp->m_sb.sb_rootino)  {
					/*
					 * we have to make sure that . == ..
					 * in the root inode
					 */
					if (!no_modify)  {
						do_warn(
		_("correcting .. entry in root inode %" PRIu64 ", was %" PRIu64 "\n"),
							ino, *parent);
						xfs_dir_sf_put_dirino(
							&ino, &namest->inumber);
						*buf_dirty = 1;
					} else  {
						do_warn(
		_("bad .. entry (%" PRIu64 ") in root inode %" PRIu64 " should be %" PRIu64 "\n"),
							*parent,
							ino, ino);
					}
				}
			} else  {
				/*
				 * can't fix the directory unless we know
				 * which .. entry is the right one.  Both
				 * have valid inode numbers, match the hash
				 * value and the hash values are ordered
				 * properly or we wouldn't be here.  So
				 * since both seem equally valid, trash
				 * this one.
				 */
				if (!no_modify)  {
					do_warn(
_("multiple .. entries in directory inode %" PRIu64 ", will clear second entry\n"),
						ino);
					namest->name[0] = '/';
					*buf_dirty = 1;
				} else  {
					do_warn(
_("multiple .. entries in directory inode %" PRIu64 ", would clear second entry\n"),
						ino);
				}
			}
		} else if (entry->namelen == 1 && namest->name[0] == '.')  {
			/*
			 * the '.' case
			 */
			if (!*dot)  {
				(*dot)++;
				if (lino != ino)  {
					if (!no_modify)  {
						do_warn(
_(". in directory inode %" PRIu64 " has wrong value (%" PRIu64 "), fixing entry...\n"),
							ino, lino);
						xfs_dir_sf_put_dirino(&ino,
							&namest->inumber);
						*buf_dirty = 1;
					} else  {
						do_warn(
_(". in directory inode %" PRIu64 " has wrong value (%" PRIu64 ")\n"),
							ino, lino);
					}
				}
			} else  {
				do_warn(
_("multiple . entries in directory inode %" PRIu64 "\n"),
					ino);
				/*
				 * mark entry as to be junked.
				 */
				if (!no_modify)  {
					do_warn(
_("will clear one . entry in directory inode %" PRIu64 "\n"),
						ino);
					namest->name[0] = '/';
					*buf_dirty = 1;
				} else  {
					do_warn(
_("would clear one . entry in directory inode %" PRIu64 "\n"),
						ino);
				}
			}
		} else  {
			/*
			 * all the rest -- make sure only . references self
			 */
			if (lino == ino)  {
				do_warn(
		_("entry \"%s\" in directory inode %" PRIu64 " points to self, "),
					fname, ino);
				if (!no_modify)  {
					do_warn(_("will clear entry\n"));
					namest->name[0] = '/';
					*buf_dirty = 1;
				} else  {
					do_warn(_("would clear entry\n"));
				}
			}
		}
	}

	/*
	 * compare top of heap values and reset as required.  if the
	 * holes flag is set, don't reset first_used unless it's
	 * pointing to used bytes.  we're being conservative here
	 * since the block will get compacted anyhow by the kernel.
	 */
	if ((leaf->hdr.holes == 0 && 
			first_used != be16_to_cpu(leaf->hdr.firstused)) ||
			be16_to_cpu(leaf->hdr.firstused) > first_used)  {
		if (!no_modify)  {
			if (verbose)
				do_warn(
_("- resetting first used heap value from %d to %d in block %u of dir ino %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.firstused),
					first_used, da_bno, ino);
			leaf->hdr.firstused = cpu_to_be16(first_used);
			*buf_dirty = 1;
		} else  {
			if (verbose)
				do_warn(
_("- would reset first used value from %d to %d in block %u of dir ino %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.firstused),
					first_used, da_bno, ino);
		}
	}

	if (bytes_used != be16_to_cpu(leaf->hdr.namebytes))  {
		if (!no_modify)  {
			if (verbose)
				do_warn(
_("- resetting namebytes cnt from %d to %d in block %u of dir inode %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.namebytes),
					bytes_used, da_bno, ino);
			leaf->hdr.namebytes = cpu_to_be16(bytes_used);
			*buf_dirty = 1;
		} else  {
			if (verbose)
				do_warn(
_("- would reset namebytes cnt from %d to %d in block %u of dir inode %" PRIu64 "\n"),
					be16_to_cpu(leaf->hdr.namebytes),
					bytes_used, da_bno, ino);
		}
	}

	/*
	 * If the hole flag is not set, then we know that there can
	 * be no lost holes.  If the hole flag is set, then it's ok
	 * if the on-disk holemap doesn't describe everything as long
	 * as what it does describe doesn't conflict with reality.
	 */

	reset_holes = 0;

	bholemap.lost_holes = leaf->hdr.holes;
	for (i = 0; i < XFS_DIR_LEAF_MAPSIZE; i++)  {
		bholemap.hentries[i].base = be16_to_cpu(leaf->hdr.freemap[i].base);
		bholemap.hentries[i].size = be16_to_cpu(leaf->hdr.freemap[i].size);
	}

	/*
	 * Ok, now set up our own freespace list
	 * (XFS_DIR_LEAF_MAPSIZE (3) * biggest regions)
	 * and see if they match what's in the block
	 */
	memset(&holemap, 0, sizeof(da_hole_map_t));
	process_da_freemap(mp, dir_freemap, &holemap);

	if (zero_len_entries)  {
		reset_holes = 1;
	} else if (leaf->hdr.holes == 0)  {
		if (holemap.lost_holes > 0)  {
			if (verbose)
				do_warn(
	_("- found unexpected lost holes in block %u, dir inode %" PRIu64 "\n"),
					da_bno, ino);

			reset_holes = 1;
		} else if (compare_da_freemaps(mp, &holemap, &bholemap,
				XFS_DIR_LEAF_MAPSIZE, ino, da_bno))  {
			if (verbose)
				do_warn(
		_("- hole info non-optimal in block %u, dir inode %" PRIu64 "\n"),
					da_bno, ino);
			reset_holes = 1;
		}
	} else if (verify_da_freemap(mp, dir_freemap, &holemap, ino, da_bno))  {
		if (verbose)
			do_warn(
		_("- hole info incorrect in block %u, dir inode %" PRIu64 "\n"),
				da_bno, ino);
		reset_holes = 1;
	}

	if (reset_holes)  {
		/*
		 * have to reset block hole info
		 */
		if (verbose)  {
			do_warn(
_("- existing hole info for block %d, dir inode %" PRIu64 " (base, size) - \n"),
				da_bno, ino);
			do_warn("- \t");
			for (i = 0; i < XFS_DIR_LEAF_MAPSIZE; i++)  {
				do_warn(
				"- (%d, %d) ", bholemap.hentries[i].base,
					bholemap.hentries[i].size);
			}
			do_warn(_("- holes flag = %d\n"), bholemap.lost_holes);
		}

		if (!no_modify)  {
			if (verbose)
				do_warn(
			_("- compacting block %u in dir inode %" PRIu64 "\n"),
					da_bno, ino);

			new_leaf = (xfs_dir_leafblock_t *) ts_dirbuf();

			/*
			 * copy leaf block header
			 */
			memmove(&new_leaf->hdr, &leaf->hdr,
				sizeof(xfs_dir_leaf_hdr_t));

			/*
			 * reset count in case we have some zero length entries
			 * that are being junked
			 */
			num_entries = 0;
			first_used = XFS_LBSIZE(mp);
			first_byte = (char *) new_leaf
					+ (__psint_t) XFS_LBSIZE(mp);

			/*
			 * copy entry table and pack names starting from the end
			 * of the block
			 */
			for (i = 0, s_entry = &leaf->entries[0],
					d_entry = &new_leaf->entries[0];
					i < be16_to_cpu(leaf->hdr.count);
					i++, s_entry++)  {
				/*
				 * skip zero-length entries
				 */
				if (s_entry->namelen == 0)
					continue;

				bytes = sizeof(xfs_dir_leaf_name_t)
					+ s_entry->namelen - 1;

				if ((__psint_t) first_byte - bytes <
						sizeof(xfs_dir_leaf_entry_t)
						+ (__psint_t) d_entry)  {
					do_warn(
	_("not enough space in block %u of dir inode %" PRIu64 " for all entries\n"),
						da_bno, ino);
					break;
				}

				first_used -= bytes;
				first_byte -= bytes;

				d_entry->nameidx = cpu_to_be16(first_used);
				d_entry->hashval = s_entry->hashval;
				d_entry->namelen = s_entry->namelen;
				d_entry->pad2 = 0;

				memmove(first_byte, (char *)leaf + 
					be16_to_cpu(s_entry->nameidx), bytes);

				num_entries++;
				d_entry++;
			}

			ASSERT((char *) first_byte >= (char *) d_entry);
			ASSERT(first_used <= XFS_LBSIZE(mp));

			/*
			 * zero space between end of table and top of heap
			 */
			memset(d_entry, 0, (__psint_t) first_byte
					- (__psint_t) d_entry);

			/*
			 * reset header info
			 */
			if (num_entries != be16_to_cpu(new_leaf->hdr.count))
				new_leaf->hdr.count = cpu_to_be16(num_entries);

			new_leaf->hdr.firstused = cpu_to_be16(first_used);
			new_leaf->hdr.holes = 0;
			new_leaf->hdr.pad1 = 0;

			new_leaf->hdr.freemap[0].base = cpu_to_be16(
				(__psint_t) d_entry - (__psint_t) new_leaf);
			new_leaf->hdr.freemap[0].size = cpu_to_be16(
				(__psint_t) first_byte - (__psint_t) d_entry);

			ASSERT(be16_to_cpu(new_leaf->hdr.freemap[0].base) < first_used);
			ASSERT(be16_to_cpu(new_leaf->hdr.freemap[0].base) ==
					(__psint_t) (&new_leaf->entries[0])
					- (__psint_t) new_leaf
					+ i * sizeof(xfs_dir_leaf_entry_t));
			ASSERT(be16_to_cpu(new_leaf->hdr.freemap[0].base) < XFS_LBSIZE(mp));
			ASSERT(be16_to_cpu(new_leaf->hdr.freemap[0].size) < XFS_LBSIZE(mp));
			ASSERT(be16_to_cpu(new_leaf->hdr.freemap[0].base) +
				be16_to_cpu(new_leaf->hdr.freemap[0].size) == first_used);

			new_leaf->hdr.freemap[1].base = 0;
			new_leaf->hdr.freemap[1].size = 0;
			new_leaf->hdr.freemap[2].base = 0;
			new_leaf->hdr.freemap[2].size = 0;

			/*
			 * final step, copy block back
			 */
			memmove(leaf, new_leaf, mp->m_sb.sb_blocksize);

			*buf_dirty = 1;
		} else  {
			if (verbose)
				do_warn(
			_("- would compact block %u in dir inode %" PRIu64 "\n"),
					da_bno, ino);
		}
	}
#if 0
	if (!no_modify)  {
		/*
		 * now take care of deleting or marking the entries with
		 * zero-length namelen's
		 */
		junk_zerolen_dir_leaf_entries(mp, leaf, ino, buf_dirty);
	}
#endif
#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_leaf_dir_block returns %d\n", res);
#endif
	return((res > 0) ? 1 : 0);
}

/*
 * returns 0 if the directory is ok, 1 if it has to be junked.
 */
int
process_leaf_dir_level(xfs_mount_t	*mp,
			da_bt_cursor_t	*da_cursor,
			int		ino_discovery,
			int		*repair,
			int		*dot,
			int		*dotdot,
			xfs_ino_t	*parent)
{
	xfs_dir_leafblock_t	*leaf;
	xfs_buf_t		*bp;
	xfs_ino_t		ino;
	xfs_dfsbno_t		dev_bno;
	xfs_dablk_t		da_bno;
	xfs_dablk_t		prev_bno;
	int			res = 0;
	int			buf_dirty = 0;
	xfs_daddr_t		bd_addr;
	xfs_dahash_t		current_hashval = 0;
	xfs_dahash_t		greatest_hashval;

#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_leaf_dir_level - ino %" PRIu64 "\n", da_cursor->ino);
#endif
	*repair = 0;
	da_bno = da_cursor->level[0].bno;
	ino = da_cursor->ino;
	prev_bno = 0;

	do {
		dev_bno = blkmap_get(da_cursor->blkmap, da_bno);
		/*
		 * directory code uses 0 as the NULL block pointer
		 * since 0 is the root block and no directory block
		 * pointer can point to the root block of the btree
		 */
		ASSERT(da_bno != 0);

		if (dev_bno == NULLDFSBNO) {
			do_warn(
			_("can't map block %u for directory inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}

		bd_addr = (xfs_daddr_t)XFS_FSB_TO_DADDR(mp, dev_bno);

		bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, dev_bno),
					XFS_FSB_TO_BB(mp, 1), 0);
		if (!bp) {
			do_warn(
	_("can't read file block %u (fsbno %" PRIu64 ", daddr %" PRId64 ") "
	  "for directory inode %" PRIu64 "\n"),
				da_bno, dev_bno, bd_addr, ino);
			goto error_out;
		}

		leaf = (xfs_dir_leafblock_t *)XFS_BUF_PTR(bp);

		/*
		 * check magic number for leaf directory btree block
		 */
		if (XFS_DIR_LEAF_MAGIC !=
		    be16_to_cpu(leaf->hdr.info.magic)) {
			do_warn(
	_("bad directory leaf magic # %#x for dir ino %" PRIu64"\n"),
				be16_to_cpu(leaf->hdr.info.magic),
				ino);
			libxfs_putbuf(bp);
			goto error_out;
		}
		/*
		 * keep track of greatest block # -- that gets
		 * us the length of the directory
		 */
		if (da_bno > da_cursor->greatest_bno)
			da_cursor->greatest_bno = da_bno;

		buf_dirty = 0;
		/*
		 * for each block, process the block, verify its path,
		 * then get next block.  update cursor values along the way
		 */
		if (process_leaf_dir_block(mp, leaf, da_bno, ino,
				current_hashval, ino_discovery,
				da_cursor->blkmap, dot, dotdot, parent,
				&buf_dirty, &greatest_hashval))  {
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
		da_cursor->level[0].dirty = buf_dirty;

		if (be32_to_cpu(leaf->hdr.info.back) != prev_bno)  {
			do_warn(
	_("bad sibling back pointer for directory block %u in directory inode %" PRIu64 "\n"),
				da_bno, ino);
			libxfs_putbuf(bp);
			goto error_out;
		}

		prev_bno = da_bno;
		da_bno = be32_to_cpu(leaf->hdr.info.forw);

		if (da_bno != 0)
			if (verify_da_path(mp, da_cursor, 0))  {
				libxfs_putbuf(bp);
				goto error_out;
			}

		current_hashval = greatest_hashval;

		ASSERT(buf_dirty == 0 || (buf_dirty && !no_modify));

		if (buf_dirty && !no_modify)  {
			*repair = 1;
			libxfs_writebuf(bp, 0);
		}
		else
			libxfs_putbuf(bp);
	} while (da_bno != 0 && res == 0);

	if (verify_final_da_path(mp, da_cursor, 0))  {
		/*
		 * verify the final path up (right-hand-side) if still ok
		 */
		do_warn(_("bad hash path in directory %" PRIu64 "\n"), da_cursor->ino);
		goto error_out;
	}

#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_leaf_dir_level returns %d (%s)\n",
		res, ((res) ? "bad" : "ok"));
#endif
	/*
	 * redundant but just for testing
	 */
	release_da_cursor(mp, da_cursor, 0);

	return(res);

error_out:
	/*
	 * release all buffers holding interior btree blocks
	 */
	err_release_da_cursor(mp, da_cursor, 0);

	return(1);
}

/*
 * a node directory is a true btree directory -- where the directory
 * has gotten big enough that it is represented as a non-trivial (e.g.
 * has more than just a root block) btree.
 *
 * Note that if we run into any problems, we trash the
 * directory.  Even if it's the root directory,
 * we'll be able to traverse all the disconnected
 * subtrees later (phase 6).
 *
 * one day, if we actually fix things, we'll set repair to 1 to
 * indicate that we have or that we should.
 *
 * dirname can be set to NULL if the name is unknown (or to
 * the string representation of the inode)
 *
 * returns 0 if things are ok, 1 if bad (directory needs to be junked)
 */
/* ARGSUSED */
int
process_node_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	blkmap_t	*blkmap,
	int		*dot,
	int		*dotdot,
	xfs_ino_t	*parent,	/* out - parent ino #  or NULLFSINO */
	char		*dirname,
	int		*repair)
{
	xfs_dablk_t			bno;
	int				error = 0;
	da_bt_cursor_t			da_cursor;

#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_node_dir - ino %" PRIu64 "\n", ino);
#endif
	*repair = *dot = *dotdot = 0;
	*parent = NULLFSINO;

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
	 * now process interior node
	 */

	error = traverse_int_dablock(mp, &da_cursor, &bno, XFS_DATA_FORK);

	if (error == 0)
		return(1);

	/*
	 * now pass cursor and bno into leaf-block processing routine
	 * the leaf dir level routine checks the interior paths
	 * up to the root including the final right-most path.
	 */

	error = process_leaf_dir_level(mp, &da_cursor, ino_discovery,
					repair, dot, dotdot, parent);

	if (error)
		return(1);

	/*
	 * sanity check inode size
	 */
	if (be64_to_cpu(dip->di_size) <
			(da_cursor.greatest_bno + 1) * mp->m_sb.sb_blocksize)  {
		if ((xfs_fsize_t) da_cursor.greatest_bno
				* mp->m_sb.sb_blocksize > UINT_MAX)  {
			do_warn(
_("out of range internal directory block numbers (inode %" PRIu64 ")\n"),
				ino);
			return(1);
		}

		do_warn(
_("setting directory inode (%" PRIu64 ") size to %" PRIu64 " bytes, was %" PRId64 " bytes\n"),
			ino, (xfs_dfiloff_t) (da_cursor.greatest_bno + 1)
				* mp->m_sb.sb_blocksize,
			(__int64_t)be64_to_cpu(dip->di_size));

		dip->di_size = cpu_to_be64((da_cursor.greatest_bno + 1) 
						* mp->m_sb.sb_blocksize);
	}
	return(0);
}

/*
 * a leaf directory is one where the directory is too big for
 * the inode data fork but is small enough to fit into one
 * directory btree block (filesystem block) outside the inode
 *
 * returns NULLFSINO if the directory is cannot be salvaged
 * and the .. ino if things are ok (even if the directory had
 * to be altered to make it ok).
 *
 * dirname can be set to NULL if the name is unknown (or to
 * the string representation of the inode)
 *
 * returns 0 if things are ok, 1 if bad (directory needs to be junked)
 */
/* ARGSUSED */
int
process_leaf_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,
	blkmap_t	*blkmap,
	int		*dot,		/* out - 1 if there is a dot, else 0 */
	int		*dotdot,	/* out - 1 if there's a dotdot, else 0 */
	xfs_ino_t	*parent,	/* out - parent ino #  or NULLFSINO */
	char		*dirname,	/* in - directory pathname */
	int		*repair)	/* out - 1 if something was fixed */
{
	xfs_dir_leafblock_t	*leaf;
	xfs_dahash_t	next_hashval;
	xfs_dfsbno_t	bno;
	xfs_buf_t	*bp;
	int		buf_dirty = 0;

#ifdef XR_DIR_TRACE
	fprintf(stderr, "process_leaf_dir - ino %" PRIu64 "\n", ino);
#endif
	*repair = *dot = *dotdot = 0;
	*parent = NULLFSINO;

	bno = blkmap_get(blkmap, 0);
	if (bno == NULLDFSBNO) {
		do_warn(_("block 0 for directory inode %" PRIu64 " is missing\n"),
			ino);
		return(1);
	}
	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, bno),
			XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp) {
		do_warn(_("can't read block 0 for directory inode %" PRIu64 "\n"),
			ino);
		return(1);
	}
	/*
	 * verify leaf block
	 */
	leaf = (xfs_dir_leafblock_t *)XFS_BUF_PTR(bp);

	/*
	 * check magic number for leaf directory btree block
	 */
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_DIR_LEAF_MAGIC) {
		do_warn(_("bad directory leaf magic # %#x for dir ino %" PRIu64 "\n"),
			be16_to_cpu(leaf->hdr.info.magic), ino);
		libxfs_putbuf(bp);
		return(1);
	}

	if (process_leaf_dir_block(mp, leaf, 0, ino, 0, ino_discovery, blkmap,
			dot, dotdot, parent, &buf_dirty, &next_hashval)) {
		/*
		 * the block is bad.  lose the directory.
		 * XXX - later, we should try and just lose
		 * the block without losing the entire directory
		 */
		ASSERT(*dotdot == 0 || (*dotdot == 1 && *parent != NULLFSINO));
		libxfs_putbuf(bp);
		return(1);
	}

	/*
	 * check sibling pointers in leaf block (above doesn't do it)
	 */
	if (leaf->hdr.info.forw || leaf->hdr.info.back)  {
		if (!no_modify)  {
			do_warn(_("clearing forw/back pointers for "
				  "directory inode %" PRIu64 "\n"), ino);
			buf_dirty = 1;
			leaf->hdr.info.forw = 0;
			leaf->hdr.info.back = 0;
		} else  {
			do_warn(_("would clear forw/back pointers for "
				  "directory inode %" PRIu64 "\n"), ino);
		}
	}

	ASSERT(buf_dirty == 0 || (buf_dirty && !no_modify));

	if (buf_dirty && !no_modify)
		libxfs_writebuf(bp, 0);
	else
		libxfs_putbuf(bp);

	return(0);
}

/*
 * returns 1 if things are bad (directory needs to be junked)
 * and 0 if things are ok.  If ino_discovery is 1, add unknown
 * inodes to uncertain inode list.
 */
int
process_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,
	char		*dirname,
	xfs_ino_t	*parent,
	blkmap_t	*blkmap)
{
	int		dot;
	int		dotdot;
	int		repair = 0;
	int		res = 0;

	*parent = NULLFSINO;
	dot = dotdot = 0;

	/*
	 * branch off depending on the type of inode.  This routine
	 * is only called ONCE so all the subordinate routines will
	 * fix '.' and junk '..' if they're bogus.
	 */
	if (be64_to_cpu(dip->di_size) <= XFS_DFORK_DSIZE(dip, mp))  {
		dot = 1;
		dotdot = 1;
		if (process_shortform_dir(mp, ino, dip, ino_discovery,
				dino_dirty, parent, dirname, &repair))
			res = 1;
	} else if (be64_to_cpu(dip->di_size) <= XFS_LBSIZE(mp))  {
		if (process_leaf_dir(mp, ino, dip, ino_discovery,
				dino_dirty, blkmap, &dot, &dotdot,
				parent, dirname, &repair))
			res = 1;
	} else  {
		if (process_node_dir(mp, ino, dip, ino_discovery,
				blkmap, &dot, &dotdot,
				parent, dirname, &repair))
			res = 1;
	}
	/*
	 * bad . entries in all directories will be fixed up in phase 6
	 */
	if (dot == 0) 
		do_warn(_("no . entry for directory %" PRIu64 "\n"), ino);

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

#ifdef XR_DIR_TRACE
	fprintf(stderr, "(process_dir), parent of %" PRIu64 " is %" PRIu64 "\n", ino, parent);
#endif
	return(res);
}
