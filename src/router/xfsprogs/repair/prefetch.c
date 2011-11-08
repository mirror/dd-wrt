#include <libxfs.h>
#include <pthread.h>
#include "avl.h"
#include "btree.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "dir.h"
#include "dir2.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "bmap.h"
#include "versions.h"
#include "threads.h"
#include "prefetch.h"
#include "progress.h"

int do_prefetch = 1;

/*
 * Performs prefetching by priming the libxfs cache by using a dedicate thread
 * scanning inodes and reading blocks in ahead of time they are required.
 *
 * Any I/O errors can be safely ignored.
 */

static xfs_mount_t	*mp;
static int 		mp_fd;
static int		pf_max_bytes;
static int		pf_max_bbs;
static int		pf_max_fsbs;
static int		pf_batch_bytes;
static int		pf_batch_fsbs;

static void		pf_read_inode_dirs(prefetch_args_t *, xfs_buf_t *);

/*
 * Buffer priorities for the libxfs cache
 *
 * Directory metadata is ranked higher than other metadata as it's used
 * in phases 3, 4 and 6, while other metadata is only used in 3 & 4.
 */

/* intermediate directory btree nodes - can't be queued */
#define B_DIR_BMAP	CACHE_PREFETCH_PRIORITY + 7
/* directory metadata in secondary queue */
#define B_DIR_META_2	CACHE_PREFETCH_PRIORITY + 6
/* dir metadata that had to fetched from the primary queue to avoid stalling */
#define B_DIR_META_H	CACHE_PREFETCH_PRIORITY + 5
/* single block of directory metadata (can't batch read) */
#define B_DIR_META_S	CACHE_PREFETCH_PRIORITY + 4
/* dir metadata with more than one block fetched in a single I/O */
#define B_DIR_META	CACHE_PREFETCH_PRIORITY + 3
/* inode clusters with directory inodes */
#define B_DIR_INODE	CACHE_PREFETCH_PRIORITY + 2
/* intermediate extent btree nodes */
#define B_BMAP		CACHE_PREFETCH_PRIORITY + 1
/* inode clusters without any directory entries */
#define B_INODE		CACHE_PREFETCH_PRIORITY

/*
 * Test if bit 0 or 2 is set in the "priority tag" of the buffer to see if
 * the buffer is for an inode or other metadata.
 */
#define B_IS_INODE(f)	(((f) & 5) == 0)

#define DEF_BATCH_BYTES	0x10000

#define MAX_BUFS	128

#define IO_THRESHOLD	(MAX_BUFS * 2)

typedef enum pf_which {
	PF_PRIMARY,
	PF_SECONDARY,
	PF_META_ONLY
} pf_which_t;


static inline void
pf_start_processing(
	prefetch_args_t		*args)
{
	if (!args->can_start_processing) {
		pftrace("signalling processing for AG %d", args->agno);

		args->can_start_processing = 1;
		pthread_cond_signal(&args->start_processing);
	}
}

static inline void
pf_start_io_workers(
	prefetch_args_t		*args)
{
	if (!args->can_start_reading) {
		pftrace("signalling reading for AG %d", args->agno);

		args->can_start_reading = 1;
		pthread_cond_broadcast(&args->start_reading);
	}
}


static void
pf_queue_io(
	prefetch_args_t		*args,
	xfs_fsblock_t		fsbno,
	int			blen,
	int			flag)
{
	xfs_buf_t		*bp;

	bp = libxfs_getbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, fsbno),
			XFS_FSB_TO_BB(mp, blen));
	if (bp->b_flags & LIBXFS_B_UPTODATE) {
		if (B_IS_INODE(flag))
			pf_read_inode_dirs(args, bp);
		XFS_BUF_SET_PRIORITY(bp, XFS_BUF_PRIORITY(bp) +
						CACHE_PREFETCH_PRIORITY);
		libxfs_putbuf(bp);
		return;
	}
	XFS_BUF_SET_PRIORITY(bp, flag);

	pthread_mutex_lock(&args->lock);

	btree_insert(args->io_queue, fsbno, bp);

	if (fsbno > args->last_bno_read) {
		if (B_IS_INODE(flag)) {
			args->inode_bufs_queued++;
			if (args->inode_bufs_queued == IO_THRESHOLD)
				pf_start_io_workers(args);
		}
	} else {
		ASSERT(!B_IS_INODE(flag));
		XFS_BUF_SET_PRIORITY(bp, B_DIR_META_2);
	}

	pftrace("getbuf %c %p (%llu) in AG %d (fsbno = %lu) added to queue"
		"(inode_bufs_queued = %d, last_bno = %lu)", B_IS_INODE(flag) ?
		'I' : 'M', bp, (long long)XFS_BUF_ADDR(bp), args->agno, fsbno,
		args->inode_bufs_queued, args->last_bno_read);

	pf_start_processing(args);

	pthread_mutex_unlock(&args->lock);
}

static int
pf_read_bmbt_reclist(
	prefetch_args_t		*args,
	xfs_bmbt_rec_t		*rp,
	int			numrecs)
{
	int			i;
	xfs_bmbt_irec_t		irec;
	xfs_dfilblks_t		cp = 0;		/* prev count */
	xfs_dfiloff_t		op = 0;		/* prev offset */

	for (i = 0; i < numrecs; i++) {
		libxfs_bmbt_disk_get_all(rp + i, &irec);

		if (((i > 0) && (op + cp > irec.br_startoff)) ||
				(irec.br_blockcount == 0) ||
				(irec.br_startoff >= fs_max_file_offset))
			return 0;

		if (!verify_dfsbno(mp, irec.br_startblock) || !verify_dfsbno(mp,
				irec.br_startblock + irec.br_blockcount - 1))
			return 0;

		if (!args->dirs_only && ((irec.br_startoff +
				irec.br_blockcount) >= mp->m_dirfreeblk))
			break;	/* only Phase 6 reads the free blocks */

		op = irec.br_startoff;
		cp = irec.br_blockcount;

		while (irec.br_blockcount) {
			unsigned int	len;

			pftrace("queuing dir extent in AG %d", args->agno);

			len = (irec.br_blockcount > mp->m_dirblkfsbs) ?
					mp->m_dirblkfsbs : irec.br_blockcount;
			pf_queue_io(args, irec.br_startblock, len, B_DIR_META);
			irec.br_blockcount -= len;
			irec.br_startblock += len;
		}
	}
	return 1;
}

/*
 * simplified version of the main scan_lbtree. Returns 0 to stop.
 */

static int
pf_scan_lbtree(
	xfs_dfsbno_t		dbno,
	int			level,
	int			isadir,
	prefetch_args_t		*args,
	int			(*func)(struct xfs_btree_block	*block,
					int			level,
					int			isadir,
					prefetch_args_t		*args))
{
	xfs_buf_t		*bp;
	int			rc;

	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, dbno),
			XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp)
		return 0;

	XFS_BUF_SET_PRIORITY(bp, isadir ? B_DIR_BMAP : B_BMAP);

	rc = (*func)(XFS_BUF_TO_BLOCK(bp), level - 1, isadir, args);

	libxfs_putbuf(bp);

	return rc;
}

static int
pf_scanfunc_bmap(
	struct xfs_btree_block	*block,
	int			level,
	int			isadir,
	prefetch_args_t		*args)
{
	xfs_bmbt_ptr_t		*pp;
	int 			numrecs;
	int			i;
	xfs_dfsbno_t		dbno;

	/*
	 * do some validation on the block contents
	 */
	if ((be32_to_cpu(block->bb_magic) != XFS_BMAP_MAGIC) ||
			(be16_to_cpu(block->bb_level) != level))
		return 0;

	numrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (numrecs > mp->m_bmap_dmxr[0] || !isadir)
			return 0;
		return pf_read_bmbt_reclist(args,
			XFS_BMBT_REC_ADDR(mp, block, 1), numrecs);
	}

	if (numrecs > mp->m_bmap_dmxr[1])
		return 0;

	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);

	for (i = 0; i < numrecs; i++) {
		dbno = be64_to_cpu(pp[i]);
		if (!verify_dfsbno(mp, dbno))
			return 0;
		if (!pf_scan_lbtree(dbno, level, isadir, args, pf_scanfunc_bmap))
			return 0;
	}
	return 1;
}


static void
pf_read_btinode(
	prefetch_args_t		*args,
	xfs_dinode_t		*dino,
	int			isadir)
{
	xfs_bmdr_block_t	*dib;
	xfs_bmbt_ptr_t		*pp;
	int			i;
	int			level;
	int			numrecs;
	int			dsize;
	xfs_dfsbno_t		dbno;

	dib = (xfs_bmdr_block_t *)XFS_DFORK_DPTR(dino);

	level = be16_to_cpu(dib->bb_level);
	numrecs = be16_to_cpu(dib->bb_numrecs);

	if ((numrecs == 0) || (level == 0) ||
			(level > XFS_BM_MAXLEVELS(mp, XFS_DATA_FORK)))
		return;
	/*
	 * use bmdr/dfork_dsize since the root block is in the data fork
	 */
	if (XFS_BMDR_SPACE_CALC(numrecs) > XFS_DFORK_DSIZE(dino, mp))
		return;

	dsize = XFS_DFORK_DSIZE(dino, mp);
	pp = XFS_BMDR_PTR_ADDR(dib, 1, xfs_bmdr_maxrecs(mp, dsize, 0));

	for (i = 0; i < numrecs; i++) {
		dbno = be64_to_cpu(pp[i]);
		if (!verify_dfsbno(mp, dbno))
			break;
		if (!pf_scan_lbtree(dbno, level, isadir, args, pf_scanfunc_bmap))
			break;
	}
}

static void
pf_read_exinode(
	prefetch_args_t		*args,
	xfs_dinode_t		*dino)
{
	pf_read_bmbt_reclist(args, (xfs_bmbt_rec_t *)XFS_DFORK_DPTR(dino),
			be32_to_cpu(dino->di_nextents));
}

static void
pf_read_inode_dirs(
	prefetch_args_t		*args,
	xfs_buf_t		*bp)
{
	xfs_dinode_t		*dino;
	int			icnt = 0;
	int			hasdir = 0;
	int			isadir;

	for (icnt = 0; icnt < (XFS_BUF_COUNT(bp) >> mp->m_sb.sb_inodelog); icnt++) {
		dino = xfs_make_iptr(mp, bp, icnt);

		/*
		 * We are only prefetching directory contents in extents
		 * and btree nodes for other inodes
		 */
		isadir = (be16_to_cpu(dino->di_mode) & S_IFMT) == S_IFDIR;
		hasdir |= isadir;

		if (dino->di_format <= XFS_DINODE_FMT_LOCAL)
			continue;

		if (!isadir && (dino->di_format == XFS_DINODE_FMT_EXTENTS ||
				args->dirs_only))
			continue;

		/*
		 * do some checks on the inode to see if we can prefetch
		 * its directory data. It's a cut down version of
		 * process_dinode_int() in dinode.c.
		 */
		if (dino->di_format > XFS_DINODE_FMT_BTREE)
			continue;

		if (be16_to_cpu(dino->di_magic) != XFS_DINODE_MAGIC)
			continue;

		if (!XFS_DINODE_GOOD_VERSION(dino->di_version) ||
				(!fs_inode_nlink && dino->di_version > 1))
			continue;

		if (be64_to_cpu(dino->di_size) <= XFS_DFORK_DSIZE(dino, mp))
			continue;

		if ((dino->di_forkoff != 0) &&
				(dino->di_forkoff >= (XFS_LITINO(mp) >> 3)))
			continue;

		switch (dino->di_format) {
			case XFS_DINODE_FMT_EXTENTS:
				pf_read_exinode(args, dino);
				break;
			case XFS_DINODE_FMT_BTREE:
				pf_read_btinode(args, dino, isadir);
				break;
		}
	}
	if (hasdir)
		XFS_BUF_SET_PRIORITY(bp, B_DIR_INODE);
}

/*
 * pf_batch_read must be called with the lock locked.
 */

static void
pf_batch_read(
	prefetch_args_t		*args,
	pf_which_t		which,
	void			*buf)
{
	xfs_buf_t		*bplist[MAX_BUFS];
	unsigned int		num;
	off64_t			first_off, last_off, next_off;
	int			len, size;
	int			i;
	int			inode_bufs;
	unsigned long		fsbno = 0;
	unsigned long		max_fsbno;
	char			*pbuf;

	for (;;) {
		num = 0;
		if (which == PF_SECONDARY) {
			bplist[0] = btree_find(args->io_queue, 0, &fsbno);
			max_fsbno = MIN(fsbno + pf_max_fsbs,
							args->last_bno_read);
		} else {
			bplist[0] = btree_find(args->io_queue,
						args->last_bno_read, &fsbno);
			max_fsbno = fsbno + pf_max_fsbs;
		}
		while (bplist[num] && num < MAX_BUFS && fsbno < max_fsbno) {
			if (which != PF_META_ONLY ||
			    !B_IS_INODE(XFS_BUF_PRIORITY(bplist[num])))
				num++;
			bplist[num] = btree_lookup_next(args->io_queue, &fsbno);
		}
		if (!num)
			return;

		/*
		 * do a big read if 25% of the potential buffer is useful,
		 * otherwise, find as many close together blocks and
		 * read them in one read
		 */
		first_off = LIBXFS_BBTOOFF64(XFS_BUF_ADDR(bplist[0]));
		last_off = LIBXFS_BBTOOFF64(XFS_BUF_ADDR(bplist[num-1])) +
			XFS_BUF_SIZE(bplist[num-1]);
		while (last_off - first_off > pf_max_bytes) {
			num--;
			last_off = LIBXFS_BBTOOFF64(XFS_BUF_ADDR(bplist[num-1])) +
				XFS_BUF_SIZE(bplist[num-1]);
		}
		if (num < ((last_off - first_off) >> (mp->m_sb.sb_blocklog + 3))) {
			/*
			 * not enough blocks for one big read, so determine
			 * the number of blocks that are close enough.
			 */
			last_off = first_off + XFS_BUF_SIZE(bplist[0]);
			for (i = 1; i < num; i++) {
				next_off = LIBXFS_BBTOOFF64(XFS_BUF_ADDR(bplist[i])) +
						XFS_BUF_SIZE(bplist[i]);
				if (next_off - last_off > pf_batch_bytes)
					break;
				last_off = next_off;
			}
			num = i;
		}

		for (i = 0; i < num; i++) {
			if (btree_delete(args->io_queue, XFS_DADDR_TO_FSB(mp,
					XFS_BUF_ADDR(bplist[i]))) == NULL)
				do_error(_("prefetch corruption\n"));
		}

		if (which == PF_PRIMARY) {
			for (inode_bufs = 0, i = 0; i < num; i++) {
				if (B_IS_INODE(XFS_BUF_PRIORITY(bplist[i])))
					inode_bufs++;
			}
			args->inode_bufs_queued -= inode_bufs;
			if (inode_bufs && (first_off >> mp->m_sb.sb_blocklog) >
					pf_batch_fsbs)
				args->last_bno_read = (first_off >> mp->m_sb.sb_blocklog);
		}
#ifdef XR_PF_TRACE
		pftrace("reading bbs %llu to %llu (%d bufs) from %s queue in AG %d (last_bno = %lu, inode_bufs = %d)",
			(long long)XFS_BUF_ADDR(bplist[0]),
			(long long)XFS_BUF_ADDR(bplist[num-1]), num,
			(which != PF_SECONDARY) ? "pri" : "sec", args->agno,
			args->last_bno_read, args->inode_bufs_queued);
#endif
		pthread_mutex_unlock(&args->lock);

		/*
		 * now read the data and put into the xfs_but_t's
		 */
		len = pread64(mp_fd, buf, (int)(last_off - first_off), first_off);
		if (len > 0) {
			/*
			 * go through the xfs_buf_t list copying from the
			 * read buffer into the xfs_buf_t's and release them.
			 */
			last_off = first_off;
			for (i = 0; i < num; i++) {

				pbuf = ((char *)buf) + (LIBXFS_BBTOOFF64(XFS_BUF_ADDR(bplist[i])) - first_off);
				size = XFS_BUF_SIZE(bplist[i]);
				if (len < size)
					break;
				memcpy(XFS_BUF_PTR(bplist[i]), pbuf, size);
				bplist[i]->b_flags |= LIBXFS_B_UPTODATE;
				len -= size;
				if (B_IS_INODE(XFS_BUF_PRIORITY(bplist[i])))
					pf_read_inode_dirs(args, bplist[i]);
				else if (which == PF_META_ONLY)
					XFS_BUF_SET_PRIORITY(bplist[i],
								B_DIR_META_H);
				else if (which == PF_PRIMARY && num == 1)
					XFS_BUF_SET_PRIORITY(bplist[i],
								B_DIR_META_S);
			}
		}
		for (i = 0; i < num; i++) {
			pftrace("putbuf %c %p (%llu) in AG %d",
				B_IS_INODE(XFS_BUF_PRIORITY(bplist[i])) ? 'I' : 'M',
				bplist[i], (long long)XFS_BUF_ADDR(bplist[i]),
				args->agno);
			libxfs_putbuf(bplist[i]);
		}
		pthread_mutex_lock(&args->lock);
		if (which != PF_SECONDARY) {
			pftrace("inode_bufs_queued for AG %d = %d", args->agno,
				args->inode_bufs_queued);
			/*
			 * if primary inode queue running low, process metadata
			 * in boths queues to avoid I/O starvation as the
			 * processing thread would be waiting for a metadata
			 * buffer
			 */
			if (which == PF_PRIMARY && !args->queuing_done &&
					args->inode_bufs_queued < IO_THRESHOLD) {
				pftrace("reading metadata bufs from primary queue for AG %d",
					args->agno);

				pf_batch_read(args, PF_META_ONLY, buf);

				pftrace("reading bufs from secondary queue for AG %d",
					args->agno);

				pf_batch_read(args, PF_SECONDARY, buf);
			}
		}
	}
}

static void *
pf_io_worker(
	void			*param)
{
	prefetch_args_t		*args = param;
	void			*buf = memalign(libxfs_device_alignment(),
						pf_max_bytes);

	if (buf == NULL)
		return NULL;

	pthread_mutex_lock(&args->lock);
	while (!args->queuing_done || !btree_is_empty(args->io_queue)) {
		pftrace("waiting to start prefetch I/O for AG %d", args->agno);

		while (!args->can_start_reading && !args->queuing_done)
			pthread_cond_wait(&args->start_reading, &args->lock);

		pftrace("starting prefetch I/O for AG %d", args->agno);

		pf_batch_read(args, PF_PRIMARY, buf);
		pf_batch_read(args, PF_SECONDARY, buf);

		pftrace("ran out of bufs to prefetch for AG %d", args->agno);

		if (!args->queuing_done)
			args->can_start_reading = 0;
	}
	pthread_mutex_unlock(&args->lock);

	free(buf);

	pftrace("finished prefetch I/O for AG %d", args->agno);

	return NULL;
}

static int
pf_create_prefetch_thread(
	prefetch_args_t		*args);

static void *
pf_queuing_worker(
	void			*param)
{
	prefetch_args_t		*args = param;
	int			num_inos;
	ino_tree_node_t		*irec;
	ino_tree_node_t		*cur_irec;
	int			blks_per_cluster;
	xfs_agblock_t		bno;
	int			i;
	int			err;

	blks_per_cluster =  XFS_INODE_CLUSTER_SIZE(mp) >> mp->m_sb.sb_blocklog;
	if (blks_per_cluster == 0)
		blks_per_cluster = 1;

	for (i = 0; i < PF_THREAD_COUNT; i++) {
		err = pthread_create(&args->io_threads[i], NULL,
				pf_io_worker, args);
		if (err != 0) {
			do_warn(_("failed to create prefetch thread: %s\n"),
				strerror(err));
			if (i == 0) {
				pf_start_processing(args);
				return NULL;
			}
			/*
			 * since we have at least one I/O thread, use them for
			 * prefetch
			 */
			break;
		}
	}
	pftrace("starting prefetch for AG %d", args->agno);

	for (irec = findfirst_inode_rec(args->agno); irec != NULL;
			irec = next_ino_rec(irec)) {

		cur_irec = irec;

		num_inos = XFS_INODES_PER_CHUNK;
		while (num_inos < XFS_IALLOC_INODES(mp) && irec != NULL) {
			irec = next_ino_rec(irec);
			num_inos += XFS_INODES_PER_CHUNK;
		}

		if (args->dirs_only && cur_irec->ino_isa_dir == 0)
			continue;
#ifdef XR_PF_TRACE
		sem_getvalue(&args->ra_count, &i);
		pftrace("queuing irec %p in AG %d, sem count = %d",
			irec, args->agno, i);
#endif
		sem_wait(&args->ra_count);

		num_inos = 0;
		bno = XFS_AGINO_TO_AGBNO(mp, cur_irec->ino_startnum);

		do {
			pf_queue_io(args, XFS_AGB_TO_FSB(mp, args->agno, bno),
					blks_per_cluster,
					(cur_irec->ino_isa_dir != 0) ?
						B_DIR_INODE : B_INODE);
			bno += blks_per_cluster;
			num_inos += inodes_per_cluster;
		} while (num_inos < XFS_IALLOC_INODES(mp));
	}

	pthread_mutex_lock(&args->lock);

	pftrace("finished queuing inodes for AG %d (inode_bufs_queued = %d)",
		args->agno, args->inode_bufs_queued);

	args->queuing_done = 1;
	pf_start_io_workers(args);
	pf_start_processing(args);
	pthread_mutex_unlock(&args->lock);

	/* now wait for the readers to finish */
	for (i = 0; i < PF_THREAD_COUNT; i++)
		if (args->io_threads[i])
			pthread_join(args->io_threads[i], NULL);

	pftrace("prefetch for AG %d finished", args->agno);

	pthread_mutex_lock(&args->lock);

	ASSERT(btree_is_empty(args->io_queue));

	args->prefetch_done = 1;
	if (args->next_args)
		pf_create_prefetch_thread(args->next_args);

	pthread_mutex_unlock(&args->lock);

	return NULL;
}

static int
pf_create_prefetch_thread(
	prefetch_args_t		*args)
{
	int			err;

	pftrace("creating queue thread for AG %d", args->agno);

	err = pthread_create(&args->queuing_thread, NULL,
			pf_queuing_worker, args);
	if (err != 0) {
		do_warn(_("failed to create prefetch thread: %s\n"),
			strerror(err));
		cleanup_inode_prefetch(args);
	}

	return err == 0;
}

void
init_prefetch(
	xfs_mount_t		*pmp)
{
	mp = pmp;
	mp_fd = libxfs_device_to_fd(mp->m_dev);
	pf_max_bytes = sysconf(_SC_PAGE_SIZE) << 7;
	pf_max_bbs = pf_max_bytes >> BBSHIFT;
	pf_max_fsbs = pf_max_bytes >> mp->m_sb.sb_blocklog;
	pf_batch_bytes = DEF_BATCH_BYTES;
	pf_batch_fsbs = DEF_BATCH_BYTES >> (mp->m_sb.sb_blocklog + 1);
}

prefetch_args_t *
start_inode_prefetch(
	xfs_agnumber_t		agno,
	int			dirs_only,
	prefetch_args_t		*prev_args)
{
	prefetch_args_t		*args;
	long			max_queue;

	if (!do_prefetch || agno >= mp->m_sb.sb_agcount)
		return NULL;

	args = calloc(1, sizeof(prefetch_args_t));

	btree_init(&args->io_queue);
	if (pthread_mutex_init(&args->lock, NULL) != 0)
		do_error(_("failed to initialize prefetch mutex\n"));
	if (pthread_cond_init(&args->start_reading, NULL) != 0)
		do_error(_("failed to initialize prefetch cond var\n"));
	if (pthread_cond_init(&args->start_processing, NULL) != 0)
		do_error(_("failed to initialize prefetch cond var\n"));
	args->agno = agno;
	args->dirs_only = dirs_only;

	/*
	 * use only 1/8 of the libxfs cache as we are only counting inodes
	 * and not any other associated metadata like directories
	 */

	max_queue = libxfs_bcache->c_maxcount / thread_count / 8;
	if (XFS_INODE_CLUSTER_SIZE(mp) > mp->m_sb.sb_blocksize)
		max_queue = max_queue * (XFS_INODE_CLUSTER_SIZE(mp) >>
				mp->m_sb.sb_blocklog) / XFS_IALLOC_BLOCKS(mp);

	sem_init(&args->ra_count, 0, max_queue);

	if (!prev_args) {
		if (!pf_create_prefetch_thread(args))
			return NULL;
	} else {
		pthread_mutex_lock(&prev_args->lock);
		if (prev_args->prefetch_done) {
			if (!pf_create_prefetch_thread(args))
				args = NULL;
		} else
			prev_args->next_args = args;
		pthread_mutex_unlock(&prev_args->lock);
	}

	return args;
}

void
wait_for_inode_prefetch(
	prefetch_args_t		*args)
{
	if (args == NULL)
		return;

	pthread_mutex_lock(&args->lock);

	while (!args->can_start_processing) {
		pftrace("waiting to start processing AG %d", args->agno);

		pthread_cond_wait(&args->start_processing, &args->lock);
	}
	pftrace("can start processing AG %d", args->agno);

	pthread_mutex_unlock(&args->lock);
}

void
cleanup_inode_prefetch(
	prefetch_args_t		*args)
{
	if (args == NULL)
		return;

	pftrace("waiting AG %d prefetch to finish", args->agno);

	if (args->queuing_thread)
		pthread_join(args->queuing_thread, NULL);

	pftrace("AG %d prefetch done", args->agno);

	pthread_mutex_destroy(&args->lock);
	pthread_cond_destroy(&args->start_reading);
	pthread_cond_destroy(&args->start_processing);
	sem_destroy(&args->ra_count);
	btree_destroy(args->io_queue);

	free(args);
}

#ifdef XR_PF_TRACE

static FILE	*pf_trace_file;

void
pftrace_init(void)
{
	pf_trace_file = fopen("/tmp/xfs_repair_prefetch.trace", "w");
	setvbuf(pf_trace_file, NULL, _IOLBF, 1024);
}

void
pftrace_done(void)
{
	fclose(pf_trace_file);
}

void
_pftrace(const char *func, const char *msg, ...)
{
	char		buf[200];
	struct timeval	tv;
	va_list 	args;

	gettimeofday(&tv, NULL);

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf)-1] = '\0';
	va_end(args);

	fprintf(pf_trace_file, "%lu.%06lu  %s: %s\n", tv.tv_sec, tv.tv_usec,
		func, buf);
}

#endif
