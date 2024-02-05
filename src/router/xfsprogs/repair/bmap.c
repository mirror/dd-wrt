// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005,2008 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "err_protos.h"
#include "bmap.h"

/*
 * Track the logical to physical block mapping for inodes.
 *
 * Repair only processes one inode at a given time per thread, and the
 * block map does not have to outlive the processing of a single inode.
 *
 * The combination of those factors means we can use pthreads thread-local
 * storage to store the block map, and we can re-use the allocation over
 * and over again.
 */

pthread_key_t	dblkmap_key;
pthread_key_t	ablkmap_key;

blkmap_t *
blkmap_alloc(
	xfs_extnum_t	nex,
	int		whichfork)
{
	pthread_key_t	key;
	blkmap_t	*blkmap;

	ASSERT(whichfork == XFS_DATA_FORK || whichfork == XFS_ATTR_FORK);

	if (nex < 1)
		nex = 1;

#if (__BITS_PER_LONG == 32)	/* on 64-bit platforms this is never true */
	if (nex > BLKMAP_NEXTS_MAX) {
		do_warn(
	_("Number of extents requested in blkmap_alloc (%llu) overflows 32 bits.\n"
	  "If this is not a corruption, then you will need a 64 bit system\n"
	  "to repair this filesystem.\n"),
			(unsigned long long)nex);
		return NULL;
	}
#endif

	key = whichfork ? ablkmap_key : dblkmap_key;
	blkmap = pthread_getspecific(key);
	if (!blkmap || blkmap->naexts < nex) {
		blkmap = realloc(blkmap, BLKMAP_SIZE(nex));
		if (!blkmap) {
			do_warn(_("malloc failed in blkmap_alloc (%llu bytes)\n"),
				(unsigned long long)BLKMAP_SIZE(nex));
			return NULL;
		}
		pthread_setspecific(key, blkmap);
		blkmap->naexts = nex;
	}

	blkmap->nexts = 0;
	return blkmap;
}

/*
 * Free a block map.
 *
 * If the map is a large, uncommon size (say for hundreds of thousands of
 * extents) then free it to release the memory. This prevents us from pinning
 * large tracts of memory due to corrupted fork values or one-off fragmented
 * files. Otherwise we have nothing to do but keep the memory around for the
 * next inode.
 * When the thread is done, it should do an unconditional, final free.
 */
void
blkmap_free(
	blkmap_t	*blkmap)
{
	if (!blkmap)
		return;

	/* consider more than 100k extents rare */
	if (blkmap->naexts < 100 * 1024)
		return;

	if (blkmap == pthread_getspecific(dblkmap_key))
		pthread_setspecific(dblkmap_key, NULL);
	else
		pthread_setspecific(ablkmap_key, NULL);

	free(blkmap);
}

void
blkmap_free_final(void)
{
	blkmap_t	*blkmap;

	blkmap = pthread_getspecific(dblkmap_key);
	pthread_setspecific(dblkmap_key, NULL);
	free(blkmap);

	blkmap = pthread_getspecific(ablkmap_key);
	pthread_setspecific(ablkmap_key, NULL);
	free(blkmap);
}

/*
 * Get one entry from a block map.
 */
xfs_fsblock_t
blkmap_get(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o)
{
	bmap_ext_t	*ext = blkmap->exts;
	int		i;

	for (i = 0; i < blkmap->nexts; i++, ext++) {
		if (o >= ext->startoff && o < ext->startoff + ext->blockcount)
			return ext->startblock + (o - ext->startoff);
	}
	return NULLFSBLOCK;
}

/*
 * Get a chunk of entries from a block map - only used for reading dirv2 blocks
 */
int
blkmap_getn(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o,
	xfs_filblks_t	nb,
	bmap_ext_t	**bmpp,
	bmap_ext_t	*bmpp_single)
{
	bmap_ext_t	*bmp = NULL;
	bmap_ext_t	*ext;
	int		i;
	int		nex;

	if (nb == 1) {
		/*
		 * in the common case, when mp->m_dirblkfsbs == 1,
		 * avoid additional malloc/free overhead
		 */
		bmpp_single->startblock = blkmap_get(blkmap, o);
		goto single_ext;
	}
	ext = blkmap->exts;
	nex = 0;
	for (i = 0; i < blkmap->nexts; i++, ext++) {

		if (ext->startoff >= o + nb)
			break;
		if (ext->startoff + ext->blockcount <= o)
			continue;

		/*
		 * if all the requested blocks are in one extent (also common),
		 * use the bmpp_single option as well
		 */
		if (!bmp && o >= ext->startoff &&
		    o + nb <= ext->startoff + ext->blockcount) {
			bmpp_single->startblock =
				 ext->startblock + (o - ext->startoff);
			goto single_ext;
		}

		/*
		 * rare case - multiple extents for a single dir block
		 */
		if (!bmp)
			bmp = malloc(nb * sizeof(bmap_ext_t));
		if (!bmp)
			do_error(_("blkmap_getn malloc failed (%" PRIu64 " bytes)\n"),
						nb * sizeof(bmap_ext_t));

		bmp[nex].startblock = ext->startblock + (o - ext->startoff);
		bmp[nex].blockcount = min(nb, ext->blockcount -
				(bmp[nex].startblock - ext->startblock));
		o += bmp[nex].blockcount;
		nb -= bmp[nex].blockcount;
		nex++;
	}
	*bmpp = bmp;
	return nex;

single_ext:
	bmpp_single->blockcount = nb;
	bmpp_single->startoff = 0;	/* not even used by caller! */
	*bmpp = bmpp_single;
	return (bmpp_single->startblock != NULLFSBLOCK) ? 1 : 0;
}

/*
 * Return the last offset in a block map.
 */
xfs_fileoff_t
blkmap_last_off(
	blkmap_t	*blkmap)
{
	bmap_ext_t	*ext;

	if (!blkmap->nexts)
		return NULLFILEOFF;
	ext = blkmap->exts + blkmap->nexts - 1;
	return ext->startoff + ext->blockcount;
}

/**
 * blkmap_next_off - Return next logical block offset in a block map.
 * @blkmap:	blockmap to use
 * @o:		current file logical block number
 * @t:		current extent index into blockmap (in/out)
 *
 * Given a logical block offset in a file, return the next mapped logical offset
 * The map index "t" tracks the current extent number in the block map, and
 * is updated automatically if the returned offset resides within the next
 * mapped extent.
 *
 * If the blockmap contains no extents, or no more logical offsets are mapped,
 * or the extent index exceeds the number of extents in the map,
 * return NULLFILEOFF.
 *
 * If offset o is beyond extent index t, the first offset in the next extent
 * after extent t will be returned.
 *
 * Intended to be called starting with offset 0, index 0, and iterated.
 */
xfs_fileoff_t
blkmap_next_off(
	blkmap_t	*blkmap,
	xfs_fileoff_t	o,
	int		*t)
{
	bmap_ext_t	*ext;

	if (!blkmap->nexts)
		return NULLFILEOFF;
	if (o == NULLFILEOFF) {
		*t = 0;
		return blkmap->exts[0].startoff;
	}
	if (*t >= blkmap->nexts)
		return NULLFILEOFF;
	ext = blkmap->exts + *t;
	if (o < ext->startoff + ext->blockcount - 1)
		return o + 1;
	if (*t == blkmap->nexts - 1)
		return NULLFILEOFF;
	(*t)++;
	return ext[1].startoff;
}

/*
 * Make a block map larger.
 */
static blkmap_t *
blkmap_grow(
	blkmap_t	*blkmap)
{
	pthread_key_t	key = dblkmap_key;
	blkmap_t	*new_blkmap;
	int		new_naexts;

	/* reduce the number of reallocations for large files */
	if (blkmap->naexts < 1000)
		new_naexts = blkmap->naexts + 4;
	else if (blkmap->naexts < 10000)
		new_naexts = blkmap->naexts + 100;
	else
		new_naexts = blkmap->naexts + 1000;

	if (pthread_getspecific(key) != blkmap) {
		key = ablkmap_key;
		ASSERT(pthread_getspecific(key) == blkmap);
	}

#if (__BITS_PER_LONG == 32)	/* on 64-bit platforms this is never true */
	if (new_naexts > BLKMAP_NEXTS_MAX) {
		do_error(
	_("Number of extents requested in blkmap_grow (%d) overflows 32 bits.\n"
	  "You need a 64 bit system to repair this filesystem.\n"),
			new_naexts);
		return NULL;
	}
#endif
	if (new_naexts <= 0) {
		do_error(
	_("Number of extents requested in blkmap_grow (%d) overflowed the\n"
	  "maximum number of supported extents (%d).\n"),
			new_naexts, BLKMAP_NEXTS_MAX);
		return NULL;
	}

	new_blkmap = realloc(blkmap, BLKMAP_SIZE(new_naexts));
	if (!new_blkmap) {
		do_error(_("realloc failed in blkmap_grow\n"));
		return NULL;
	}
	new_blkmap->naexts = new_naexts;
	pthread_setspecific(key, new_blkmap);
	return new_blkmap;
}

/*
 * Set an extent into a block map.
 *
 * If this function fails, it leaves the blkmapp untouched so the caller can
 * handle the error and free the blkmap appropriately.
 */
int
blkmap_set_ext(
	blkmap_t	**blkmapp,
	xfs_fileoff_t	o,
	xfs_fsblock_t	b,
	xfs_filblks_t	c)
{
	blkmap_t	*blkmap = *blkmapp;
	xfs_extnum_t	i;

	if (blkmap->nexts == blkmap->naexts) {
		blkmap = blkmap_grow(blkmap);
		if (!blkmap)
			return ENOMEM;
		*blkmapp = blkmap;
	}

	ASSERT(blkmap->nexts < blkmap->naexts);

	if (blkmap->nexts == 0) {
		i = 0;
		goto insert;
	}

	/*
	 * The most common insert pattern comes from an ascending offset order
	 * bmapbt scan. In this case, the extent being added will end up at the
	 * end of the array. Hence do a reverse order search for the insertion
	 * point so we don't needlessly scan the entire array on every
	 * insertion.
	 *
	 * Also, use "plus 1" indexing for the loop counter so when we break out
	 * of the loop we are at the correct index for insertion.
	 */
	for (i = blkmap->nexts; i > 0; i--) {
		if (blkmap->exts[i - 1].startoff < o)
			break;
	}

	/* make space for the new extent */
	memmove(blkmap->exts + i + 1,
		blkmap->exts + i,
		sizeof(bmap_ext_t) * (blkmap->nexts - i));

insert:
	blkmap->exts[i].startoff = o;
	blkmap->exts[i].startblock = b;
	blkmap->exts[i].blockcount = c;
	blkmap->nexts++;
	return 0;
}
