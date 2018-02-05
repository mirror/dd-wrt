/*
 * Copyright (c) 2007, 2011 SGI
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

#include "libxfs.h"
#include "libxlog.h"
#include "bmap.h"
#include "command.h"
#include "metadump.h"
#include "io.h"
#include "output.h"
#include "type.h"
#include "init.h"
#include "sig.h"
#include "xfs_metadump.h"
#include "fprint.h"
#include "faddr.h"
#include "field.h"
#include "dir2.h"

#define DEFAULT_MAX_EXT_SIZE	MAXEXTLEN

/*
 * It's possible that multiple files in a directory (or attributes
 * in a file) produce the same obfuscated name.  If that happens, we
 * try to create another one.  After several rounds of this though,
 * we just give up and leave the original name as-is.
 */
#define	DUP_MAX		5	/* Max duplicates before we give up */

/* copy all metadata structures to/from a file */

static int	metadump_f(int argc, char **argv);
static void	metadump_help(void);

/*
 * metadump commands issue info/wornings/errors to standard error as
 * metadump supports stdout as a destination.
 *
 * All static functions return zero on failure, while the public functions
 * return zero on success.
 */

static const cmdinfo_t	metadump_cmd =
	{ "metadump", NULL, metadump_f, 0, -1, 0,
		N_("[-a] [-e] [-g] [-m max_extent] [-w] [-o] filename"),
		N_("dump metadata to a file"), metadump_help };

static FILE		*outf;		/* metadump file */

static xfs_metablock_t 	*metablock;	/* header + index + buffers */
static __be64		*block_index;
static char		*block_buffer;

static int		num_indices;
static int		cur_index;

static xfs_ino_t	cur_ino;

static int		show_progress = 0;
static int		stop_on_read_error = 0;
static int		max_extent_size = DEFAULT_MAX_EXT_SIZE;
static int		obfuscate = 1;
static int		zero_stale_data = 1;
static int		show_warnings = 0;
static int		progress_since_warning = 0;
static bool		stdout_metadump;

void
metadump_init(void)
{
	add_command(&metadump_cmd);
}

static void
metadump_help(void)
{
	dbprintf(_(
"\n"
" The 'metadump' command dumps the known metadata to a compact file suitable\n"
" for compressing and sending to an XFS maintainer for corruption analysis \n"
" or xfs_repair failures.\n\n"
" Options:\n"
"   -a -- Copy full metadata blocks without zeroing unused space\n"
"   -e -- Ignore read errors and keep going\n"
"   -g -- Display dump progress\n"
"   -m -- Specify max extent size in blocks to copy (default = %d blocks)\n"
"   -o -- Don't obfuscate names and extended attributes\n"
"   -w -- Show warnings of bad metadata information\n"
"\n"), DEFAULT_MAX_EXT_SIZE);
}

static void
print_warning(const char *fmt, ...)
{
	char		buf[200];
	va_list		ap;

	if (seenint())
		return;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	buf[sizeof(buf)-1] = '\0';

	fprintf(stderr, "%s%s: %s\n", progress_since_warning ? "\n" : "",
			progname, buf);
	progress_since_warning = 0;
}

static void
print_progress(const char *fmt, ...)
{
	char		buf[60];
	va_list		ap;
	FILE		*f;

	if (seenint())
		return;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	buf[sizeof(buf)-1] = '\0';

	f = stdout_metadump ? stderr : stdout;
	fprintf(f, "\r%-59s", buf);
	fflush(f);
	progress_since_warning = 1;
}

/*
 * A complete dump file will have a "zero" entry in the last index block,
 * even if the dump is exactly aligned, the last index will be full of
 * zeros. If the last index entry is non-zero, the dump is incomplete.
 * Correspondingly, the last chunk will have a count < num_indices.
 *
 * Return 0 for success, -1 for failure.
 */

static int
write_index(void)
{
	/*
	 * write index block and following data blocks (streaming)
	 */
	metablock->mb_count = cpu_to_be16(cur_index);
	if (fwrite(metablock, (cur_index + 1) << BBSHIFT, 1, outf) != 1) {
		print_warning("error writing to file: %s", strerror(errno));
		return -errno;
	}

	memset(block_index, 0, num_indices * sizeof(__be64));
	cur_index = 0;
	return 0;
}

/*
 * Return 0 for success, -errno for failure.
 */
static int
write_buf_segment(
	char		*data,
	int64_t		off,
	int		len)
{
	int		i;
	int		ret;

	for (i = 0; i < len; i++, off++, data += BBSIZE) {
		block_index[cur_index] = cpu_to_be64(off);
		memcpy(&block_buffer[cur_index << BBSHIFT], data, BBSIZE);
		if (++cur_index == num_indices) {
			ret = write_index();
			if (ret)
				return -EIO;
		}
	}
	return 0;
}

/*
 * we want to preserve the state of the metadata in the dump - whether it is
 * intact or corrupt, so even if the buffer has a verifier attached to it we
 * don't want to run it prior to writing the buffer to the metadump image.
 *
 * The only reason for running the verifier is to recalculate the CRCs on a
 * buffer that has been obfuscated. i.e. a buffer than metadump modified itself.
 * In this case, we only run the verifier if the buffer was not corrupt to begin
 * with so that we don't accidentally correct buffers with CRC or errors in them
 * when we are obfuscating them.
 */
static int
write_buf(
	iocur_t		*buf)
{
	struct xfs_buf	*bp = buf->bp;
	int		i;
	int		ret;

	/*
	 * Run the write verifier to recalculate the buffer CRCs and check
	 * metadump didn't introduce a new corruption. Warn if the verifier
	 * failed, but still continue to dump it into the output file.
	 */
	if (buf->need_crc && bp && bp->b_ops && !bp->b_error) {
		bp->b_ops->verify_write(bp);
		if (bp->b_error) {
			print_warning(
			    "obfuscation corrupted block at %s bno 0x%llx/0x%x",
				bp->b_ops->name,
				(long long)bp->b_bn, bp->b_bcount);
		}
	}

	/* handle discontiguous buffers */
	if (!buf->bbmap) {
		ret = write_buf_segment(buf->data, buf->bb, buf->blen);
		if (ret)
			return ret;
	} else {
		int	len = 0;
		for (i = 0; i < buf->bbmap->nmaps; i++) {
			ret = write_buf_segment(buf->data + BBTOB(len),
						buf->bbmap->b[i].bm_bn,
						buf->bbmap->b[i].bm_len);
			if (ret)
				return ret;
			len += buf->bbmap->b[i].bm_len;
		}
	}
	return seenint() ? -EINTR : 0;
}

/*
 * We could be processing a corrupt block, so we can't trust any of
 * the offsets or lengths to be within the buffer range. Hence check
 * carefully!
 */
static void
zero_btree_node(
	struct xfs_btree_block	*block,
	typnm_t			btype)
{
	int			nrecs;
	xfs_bmbt_ptr_t		*bpp;
	xfs_bmbt_key_t		*bkp;
	xfs_inobt_ptr_t		*ipp;
	xfs_inobt_key_t		*ikp;
	xfs_alloc_ptr_t		*app;
	xfs_alloc_key_t		*akp;
	char			*zp1, *zp2;
	char			*key_end;

	nrecs = be16_to_cpu(block->bb_numrecs);
	if (nrecs < 0)
		return;

	switch (btype) {
	case TYP_BMAPBTA:
	case TYP_BMAPBTD:
		if (nrecs > mp->m_bmap_dmxr[1])
			return;

		bkp = XFS_BMBT_KEY_ADDR(mp, block, 1);
		bpp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);
		zp1 = (char *)&bkp[nrecs];
		zp2 = (char *)&bpp[nrecs];
		key_end = (char *)bpp;
		break;
	case TYP_INOBT:
	case TYP_FINOBT:
		if (nrecs > mp->m_inobt_mxr[1])
			return;

		ikp = XFS_INOBT_KEY_ADDR(mp, block, 1);
		ipp = XFS_INOBT_PTR_ADDR(mp, block, 1, mp->m_inobt_mxr[1]);
		zp1 = (char *)&ikp[nrecs];
		zp2 = (char *)&ipp[nrecs];
		key_end = (char *)ipp;
		break;
	case TYP_BNOBT:
	case TYP_CNTBT:
		if (nrecs > mp->m_alloc_mxr[1])
			return;

		akp = XFS_ALLOC_KEY_ADDR(mp, block, 1);
		app = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);
		zp1 = (char *)&akp[nrecs];
		zp2 = (char *)&app[nrecs];
		key_end = (char *)app;
		break;
	default:
		return;
	}


	/* Zero from end of keys to beginning of pointers */
	memset(zp1, 0, key_end - zp1);

	/* Zero from end of pointers to end of block */
	memset(zp2, 0, (char *)block + mp->m_sb.sb_blocksize - zp2);
}

/*
 * We could be processing a corrupt block, so we can't trust any of
 * the offsets or lengths to be within the buffer range. Hence check
 * carefully!
 */
static void
zero_btree_leaf(
	struct xfs_btree_block	*block,
	typnm_t			btype)
{
	int			nrecs;
	struct xfs_bmbt_rec	*brp;
	struct xfs_inobt_rec	*irp;
	struct xfs_alloc_rec	*arp;
	char			*zp;

	nrecs = be16_to_cpu(block->bb_numrecs);
	if (nrecs < 0)
		return;

	switch (btype) {
	case TYP_BMAPBTA:
	case TYP_BMAPBTD:
		if (nrecs > mp->m_bmap_dmxr[0])
			return;

		brp = XFS_BMBT_REC_ADDR(mp, block, 1);
		zp = (char *)&brp[nrecs];
		break;
	case TYP_INOBT:
	case TYP_FINOBT:
		if (nrecs > mp->m_inobt_mxr[0])
			return;

		irp = XFS_INOBT_REC_ADDR(mp, block, 1);
		zp = (char *)&irp[nrecs];
		break;
	case TYP_BNOBT:
	case TYP_CNTBT:
		if (nrecs > mp->m_alloc_mxr[0])
			return;

		arp = XFS_ALLOC_REC_ADDR(mp, block, 1);
		zp = (char *)&arp[nrecs];
		break;
	default:
		return;
	}

	/* Zero from end of records to end of block */
	memset(zp, 0, (char *)block + mp->m_sb.sb_blocksize - zp);
}

static void
zero_btree_block(
	struct xfs_btree_block	*block,
	typnm_t			btype)
{
	int			level;

	level = be16_to_cpu(block->bb_level);

	if (level > 0)
		zero_btree_node(block, btype);
	else
		zero_btree_leaf(block, btype);
}

static int
scan_btree(
	xfs_agnumber_t	agno,
	xfs_agblock_t	agbno,
	int		level,
	typnm_t		btype,
	void		*arg,
	int		(*func)(struct xfs_btree_block	*block,
				xfs_agnumber_t		agno,
				xfs_agblock_t		agbno,
				int			level,
				typnm_t			btype,
				void			*arg))
{
	int		rval = 0;

	push_cur();
	set_cur(&typtab[btype], XFS_AGB_TO_DADDR(mp, agno, agbno), blkbb,
			DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		print_warning("cannot read %s block %u/%u", typtab[btype].name,
				agno, agbno);
		rval = !stop_on_read_error;
		goto pop_out;
	}

	if (zero_stale_data) {
		zero_btree_block(iocur_top->data, btype);
		iocur_top->need_crc = 1;
	}

	if (write_buf(iocur_top))
		goto pop_out;

	if (!(*func)(iocur_top->data, agno, agbno, level - 1, btype, arg))
		goto pop_out;
	rval = 1;
pop_out:
	pop_cur();
	return rval;
}

/* free space tree copy routines */

static int
valid_bno(
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno)
{
	if (agno < (mp->m_sb.sb_agcount - 1) && agbno > 0 &&
			agbno <= mp->m_sb.sb_agblocks)
		return 1;
	if (agno == (mp->m_sb.sb_agcount - 1) && agbno > 0 &&
			agbno <= (mp->m_sb.sb_dblocks -
			 (xfs_rfsblock_t)(mp->m_sb.sb_agcount - 1) *
			 mp->m_sb.sb_agblocks))
		return 1;

	return 0;
}


static int
scanfunc_freesp(
	struct xfs_btree_block	*block,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	int			level,
	typnm_t			btype,
	void			*arg)
{
	xfs_alloc_ptr_t		*pp;
	int			i;
	int			numrecs;

	if (level == 0)
		return 1;

	numrecs = be16_to_cpu(block->bb_numrecs);
	if (numrecs > mp->m_alloc_mxr[1]) {
		if (show_warnings)
			print_warning("invalid numrecs (%u) in %s block %u/%u",
				numrecs, typtab[btype].name, agno, agbno);
		return 1;
	}

	pp = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);
	for (i = 0; i < numrecs; i++) {
		if (!valid_bno(agno, be32_to_cpu(pp[i]))) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
					"in %s block %u/%u",
					agno, be32_to_cpu(pp[i]),
					typtab[btype].name, agno, agbno);
			continue;
		}
		if (!scan_btree(agno, be32_to_cpu(pp[i]), level, btype, arg,
				scanfunc_freesp))
			return 0;
	}
	return 1;
}

static int
copy_free_bno_btree(
	xfs_agnumber_t	agno,
	xfs_agf_t	*agf)
{
	xfs_agblock_t	root;
	int		levels;

	root = be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]);
	levels = be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNO]);

	/* validate root and levels before processing the tree */
	if (root == 0 || root > mp->m_sb.sb_agblocks) {
		if (show_warnings)
			print_warning("invalid block number (%u) in bnobt "
					"root in agf %u", root, agno);
		return 1;
	}
	if (levels >= XFS_BTREE_MAXLEVELS) {
		if (show_warnings)
			print_warning("invalid level (%u) in bnobt root "
					"in agf %u", levels, agno);
		return 1;
	}

	return scan_btree(agno, root, levels, TYP_BNOBT, agf, scanfunc_freesp);
}

static int
copy_free_cnt_btree(
	xfs_agnumber_t	agno,
	xfs_agf_t	*agf)
{
	xfs_agblock_t	root;
	int		levels;

	root = be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]);
	levels = be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNT]);

	/* validate root and levels before processing the tree */
	if (root == 0 || root > mp->m_sb.sb_agblocks) {
		if (show_warnings)
			print_warning("invalid block number (%u) in cntbt "
					"root in agf %u", root, agno);
		return 1;
	}
	if (levels >= XFS_BTREE_MAXLEVELS) {
		if (show_warnings)
			print_warning("invalid level (%u) in cntbt root "
					"in agf %u", levels, agno);
		return 1;
	}

	return scan_btree(agno, root, levels, TYP_CNTBT, agf, scanfunc_freesp);
}

static int
scanfunc_rmapbt(
	struct xfs_btree_block	*block,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	int			level,
	typnm_t			btype,
	void			*arg)
{
	xfs_rmap_ptr_t		*pp;
	int			i;
	int			numrecs;

	if (level == 0)
		return 1;

	numrecs = be16_to_cpu(block->bb_numrecs);
	if (numrecs > mp->m_rmap_mxr[1]) {
		if (show_warnings)
			print_warning("invalid numrecs (%u) in %s block %u/%u",
				numrecs, typtab[btype].name, agno, agbno);
		return 1;
	}

	pp = XFS_RMAP_PTR_ADDR(block, 1, mp->m_rmap_mxr[1]);
	for (i = 0; i < numrecs; i++) {
		if (!valid_bno(agno, be32_to_cpu(pp[i]))) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
					"in %s block %u/%u",
					agno, be32_to_cpu(pp[i]),
					typtab[btype].name, agno, agbno);
			continue;
		}
		if (!scan_btree(agno, be32_to_cpu(pp[i]), level, btype, arg,
				scanfunc_rmapbt))
			return 0;
	}
	return 1;
}

static int
copy_rmap_btree(
	xfs_agnumber_t	agno,
	struct xfs_agf	*agf)
{
	xfs_agblock_t	root;
	int		levels;

	if (!xfs_sb_version_hasrmapbt(&mp->m_sb))
		return 1;

	root = be32_to_cpu(agf->agf_roots[XFS_BTNUM_RMAP]);
	levels = be32_to_cpu(agf->agf_levels[XFS_BTNUM_RMAP]);

	/* validate root and levels before processing the tree */
	if (root == 0 || root > mp->m_sb.sb_agblocks) {
		if (show_warnings)
			print_warning("invalid block number (%u) in rmapbt "
					"root in agf %u", root, agno);
		return 1;
	}
	if (levels >= XFS_BTREE_MAXLEVELS) {
		if (show_warnings)
			print_warning("invalid level (%u) in rmapbt root "
					"in agf %u", levels, agno);
		return 1;
	}

	return scan_btree(agno, root, levels, TYP_RMAPBT, agf, scanfunc_rmapbt);
}

static int
scanfunc_refcntbt(
	struct xfs_btree_block	*block,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	int			level,
	typnm_t			btype,
	void			*arg)
{
	xfs_refcount_ptr_t	*pp;
	int			i;
	int			numrecs;

	if (level == 0)
		return 1;

	numrecs = be16_to_cpu(block->bb_numrecs);
	if (numrecs > mp->m_refc_mxr[1]) {
		if (show_warnings)
			print_warning("invalid numrecs (%u) in %s block %u/%u",
				numrecs, typtab[btype].name, agno, agbno);
		return 1;
	}

	pp = XFS_REFCOUNT_PTR_ADDR(block, 1, mp->m_refc_mxr[1]);
	for (i = 0; i < numrecs; i++) {
		if (!valid_bno(agno, be32_to_cpu(pp[i]))) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
					"in %s block %u/%u",
					agno, be32_to_cpu(pp[i]),
					typtab[btype].name, agno, agbno);
			continue;
		}
		if (!scan_btree(agno, be32_to_cpu(pp[i]), level, btype, arg,
				scanfunc_refcntbt))
			return 0;
	}
	return 1;
}

static int
copy_refcount_btree(
	xfs_agnumber_t	agno,
	struct xfs_agf	*agf)
{
	xfs_agblock_t	root;
	int		levels;

	if (!xfs_sb_version_hasreflink(&mp->m_sb))
		return 1;

	root = be32_to_cpu(agf->agf_refcount_root);
	levels = be32_to_cpu(agf->agf_refcount_level);

	/* validate root and levels before processing the tree */
	if (root == 0 || root > mp->m_sb.sb_agblocks) {
		if (show_warnings)
			print_warning("invalid block number (%u) in refcntbt "
					"root in agf %u", root, agno);
		return 1;
	}
	if (levels >= XFS_BTREE_MAXLEVELS) {
		if (show_warnings)
			print_warning("invalid level (%u) in refcntbt root "
					"in agf %u", levels, agno);
		return 1;
	}

	return scan_btree(agno, root, levels, TYP_REFCBT, agf, scanfunc_refcntbt);
}

/* filename and extended attribute obfuscation routines */

struct name_ent {
	struct name_ent		*next;
	xfs_dahash_t		hash;
	int			namelen;
	unsigned char		name[1];
};

#define NAME_TABLE_SIZE		4096

static struct name_ent		*nametable[NAME_TABLE_SIZE];

static void
nametable_clear(void)
{
	int		i;
	struct name_ent	*ent;

	for (i = 0; i < NAME_TABLE_SIZE; i++) {
		while ((ent = nametable[i])) {
			nametable[i] = ent->next;
			free(ent);
		}
	}
}

/*
 * See if the given name is already in the name table.  If so,
 * return a pointer to its entry, otherwise return a null pointer.
 */
static struct name_ent *
nametable_find(xfs_dahash_t hash, int namelen, unsigned char *name)
{
	struct name_ent	*ent;

	for (ent = nametable[hash % NAME_TABLE_SIZE]; ent; ent = ent->next) {
		if (ent->hash == hash && ent->namelen == namelen &&
				!memcmp(ent->name, name, namelen))
			return ent;
	}
	return NULL;
}

/*
 * Add the given name to the name table.  Returns a pointer to the
 * name's new entry, or a null pointer if an error occurs.
 */
static struct name_ent *
nametable_add(xfs_dahash_t hash, int namelen, unsigned char *name)
{
	struct name_ent	*ent;

	ent = malloc(sizeof *ent + namelen);
	if (!ent)
		return NULL;

	ent->namelen = namelen;
	memcpy(ent->name, name, namelen);
	ent->hash = hash;
	ent->next = nametable[hash % NAME_TABLE_SIZE];

	nametable[hash % NAME_TABLE_SIZE] = ent;

	return ent;
}

#define is_invalid_char(c)	((c) == '/' || (c) == '\0')
#define rol32(x,y)		(((x) << (y)) | ((x) >> (32 - (y))))

static inline unsigned char
random_filename_char(void)
{
	static unsigned char filename_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						"abcdefghijklmnopqrstuvwxyz"
						"0123456789-_";

	return filename_alphabet[random() % (sizeof filename_alphabet - 1)];
}

#define	ORPHANAGE	"lost+found"
#define	ORPHANAGE_LEN	(sizeof (ORPHANAGE) - 1)

static inline int
is_orphanage_dir(
	struct xfs_mount	*mp,
	xfs_ino_t		dir_ino,
	size_t			name_len,
	unsigned char		*name)
{
	return dir_ino == mp->m_sb.sb_rootino &&
			name_len == ORPHANAGE_LEN &&
			!memcmp(name, ORPHANAGE, ORPHANAGE_LEN);
}

/*
 * Determine whether a name is one we shouldn't obfuscate because
 * it's an orphan (or the "lost+found" directory itself).  Note
 * "cur_ino" is the inode for the directory currently being
 * processed.
 *
 * Returns 1 if the name should NOT be obfuscated or 0 otherwise.
 */
static int
in_lost_found(
	xfs_ino_t		ino,
	int			namelen,
	unsigned char		*name)
{
	static xfs_ino_t	orphanage_ino = 0;
	char			s[24];	/* 21 is enough (64 bits in decimal) */
	int			slen;

	/* Record the "lost+found" inode if we haven't done so already */

	ASSERT(ino != 0);
	if (!orphanage_ino && is_orphanage_dir(mp, cur_ino, namelen, name))
		orphanage_ino = ino;

	/* We don't obfuscate the "lost+found" directory itself */

	if (ino == orphanage_ino)
		return 1;

	/* Most files aren't in "lost+found" at all */

	if (cur_ino != orphanage_ino)
		return 0;

	/*
	 * Within "lost+found", we don't obfuscate any file whose
	 * name is the same as its inode number.  Any others are
	 * stray files and can be obfuscated.
	 */
	slen = snprintf(s, sizeof (s), "%llu", (unsigned long long) ino);

	return slen == namelen && !memcmp(name, s, namelen);
}

/*
 * Given a name and its hash value, massage the name in such a way
 * that the result is another name of equal length which shares the
 * same hash value.
 */
static void
obfuscate_name(
	xfs_dahash_t	hash,
	size_t		name_len,
	unsigned char	*name)
{
	unsigned char	*newp = name;
	int		i;
	xfs_dahash_t	new_hash = 0;
	unsigned char	*first;
	unsigned char	high_bit;
	int		shift;

	/*
	 * Our obfuscation algorithm requires at least 5-character
	 * names, so don't bother if the name is too short.  We
	 * work backward from a hash value to determine the last
	 * five bytes in a name required to produce a new name
	 * with the same hash.
	 */
	if (name_len < 5)
		return;

	/*
	 * The beginning of the obfuscated name can be pretty much
	 * anything, so fill it in with random characters.
	 * Accumulate its new hash value as we go.
	 */
	for (i = 0; i < name_len - 5; i++) {
		*newp = random_filename_char();
		new_hash = *newp ^ rol32(new_hash, 7);
		newp++;
	}

	/*
	 * Compute which five bytes need to be used at the end of
	 * the name so the hash of the obfuscated name is the same
	 * as the hash of the original.  If any result in an invalid
	 * character, flip a bit and arrange for a corresponding bit
	 * in a neighboring byte to be flipped as well.  For the
	 * last byte, the "neighbor" to change is the first byte
	 * we're computing here.
	 */
	new_hash = rol32(new_hash, 3) ^ hash;

	first = newp;
	high_bit = 0;
	for (shift = 28; shift >= 0; shift -= 7) {
		*newp = (new_hash >> shift & 0x7f) ^ high_bit;
		if (is_invalid_char(*newp)) {
			*newp ^= 1;
			high_bit = 0x80;
		} else
			high_bit = 0;
		ASSERT(!is_invalid_char(*newp));
		newp++;
	}

	/*
	 * If we flipped a bit on the last byte, we need to fix up
	 * the matching bit in the first byte.  The result will
	 * be a valid character, because we know that first byte
	 * has 0's in its upper four bits (it was produced by a
	 * 28-bit right-shift of a 32-bit unsigned value).
	 */
	if (high_bit) {
		*first ^= 0x10;
		ASSERT(!is_invalid_char(*first));
	}
	ASSERT(libxfs_da_hashname(name, name_len) == hash);
}

/*
 * Flip a bit in each of two bytes at the end of the given name.
 * This is used in generating a series of alternate names to be used
 * in the event a duplicate is found.
 *
 * The bits flipped are selected such that they both affect the same
 * bit in the name's computed hash value, so flipping them both will
 * preserve the hash.
 *
 * The following diagram aims to show the portion of a computed
 * hash that a given byte of a name affects.
 *
 *	   31    28      24    21	     14		  8 7       3     0
 *	   +-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
 * hash:   | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 *	   +-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
 *	  last-4 ->|	       |<-- last-2 --->|	   |<--- last ---->|
 *		 |<-- last-3 --->|	     |<-- last-1 --->|     |<- last-4
 *			 |<-- last-7 --->|	     |<-- last-5 --->|
 *	   |<-- last-8 --->|	       |<-- last-6 --->|
 *			. . . and so on
 *
 * The last byte of the name directly affects the low-order byte of
 * the hash.  The next-to-last affects bits 7-14, the next one back
 * affects bits 14-21, and so on.  The effect wraps around when it
 * goes beyond the top of the hash (as happens for byte last-4).
 *
 * Bits that are flipped together "overlap" on the hash value.  As
 * an example of overlap, the last two bytes both affect bit 7 in
 * the hash.  That pair of bytes (and their overlapping bits) can be
 * used for this "flip bit" operation (it's the first pair tried,
 * actually).
 *
 * A table defines overlapping pairs--the bytes involved and bits
 * within them--that can be used this way.  The byte offset is
 * relative to a starting point within the name, which will be set
 * to affect the bytes at the end of the name.  The function is
 * called with a "bitseq" value which indicates which bit flip is
 * desired, and this translates directly into selecting which entry
 * in the bit_to_flip[] table to apply.
 *
 * The function returns 1 if the operation was successful.  It
 * returns 0 if the result produced a character that's not valid in
 * a name (either '/' or a '\0').  Finally, it returns -1 if the bit
 * sequence number is beyond what is supported for a name of this
 * length.
 *
 * Discussion
 * ----------
 * (Also see the discussion above find_alternate(), below.)
 *
 * In order to make this function work for any length name, the
 * table is ordered by increasing byte offset, so that the earliest
 * entries can apply to the shortest strings.  This way all names
 * are done consistently.
 *
 * When bit flips occur, they can convert printable characters
 * into non-printable ones.  In an effort to reduce the impact of
 * this, the first bit flips are chosen to affect bytes the end of
 * the name (and furthermore, toward the low bits of a byte).  Those
 * bytes are often non-printable anyway because of the way they are
 * initially selected by obfuscate_name()).  This is accomplished,
 * using later table entries first.
 *
 * Each row in the table doubles the number of alternates that
 * can be generated.  A two-byte name is limited to using only
 * the first row, so it's possible to generate two alternates
 * (the original name, plus the alternate produced by flipping
 * the one pair of bits).  In a 5-byte name, the effect of the
 * first byte overlaps the last by 4 its, and there are 8 bits
 * to flip, allowing for 256 possible alternates.
 *
 * Short names (less than 5 bytes) are never even obfuscated, so for
 * such names the relatively small number of alternates should never
 * really be a problem.
 *
 * Long names (more than 6 bytes, say) are not likely to exhaust
 * the number of available alternates.  In fact, the table could
 * probably have stopped at 8 entries, on the assumption that 256
 * alternates should be enough for most any situation.  The entries
 * beyond those are present mostly for demonstration of how it could
 * be populated with more entries, should it ever be necessary to do
 * so.
 */
static int
flip_bit(
	size_t		name_len,
	unsigned char	*name,
	uint32_t	bitseq)
{
	int	index;
	size_t	offset;
	unsigned char *p0, *p1;
	unsigned char m0, m1;
	struct {
	    int		byte;	/* Offset from start within name */
	    unsigned char bit;	/* Bit within that byte */
	} bit_to_flip[][2] = {	/* Sorted by second entry's byte */
	    { { 0, 0 }, { 1, 7 } },	/* Each row defines a pair */
	    { { 1, 0 }, { 2, 7 } },	/* of bytes and a bit within */
	    { { 2, 0 }, { 3, 7 } },	/* each byte.  Each bit in */
	    { { 0, 4 }, { 4, 0 } },	/* a pair affects the same */
	    { { 0, 5 }, { 4, 1 } },	/* bit in the hash, so flipping */
	    { { 0, 6 }, { 4, 2 } },	/* both will change the name */
	    { { 0, 7 }, { 4, 3 } },	/* while preserving the hash. */
	    { { 3, 0 }, { 4, 7 } },
	    { { 0, 0 }, { 5, 3 } },	/* The first entry's byte offset */
	    { { 0, 1 }, { 5, 4 } },	/* must be less than the second. */
	    { { 0, 2 }, { 5, 5 } },
	    { { 0, 3 }, { 5, 6 } },	/* The table can be extended to */
	    { { 0, 4 }, { 5, 7 } },	/* an arbitrary number of entries */
	    { { 4, 0 }, { 5, 7 } },	/* but there's not much point. */
		/* . . . */
	};

	/* Find the first entry *not* usable for name of this length */

	for (index = 0; index < ARRAY_SIZE(bit_to_flip); index++)
		if (bit_to_flip[index][1].byte >= name_len)
			break;

	/*
	 * Back up to the last usable entry.  If that number is
	 * smaller than the bit sequence number, inform the caller
	 * that nothing this large (or larger) will work.
	 */
	if (bitseq > --index)
		return -1;

	/*
	 * We will be switching bits at the end of name, with a
	 * preference for affecting the last bytes first.  Compute
	 * where in the name we'll start applying the changes.
	 */
	offset = name_len - (bit_to_flip[index][1].byte + 1);
	index -= bitseq;	/* Use later table entries first */

	p0 = name + offset + bit_to_flip[index][0].byte;
	p1 = name + offset + bit_to_flip[index][1].byte;
	m0 = 1 << bit_to_flip[index][0].bit;
	m1 = 1 << bit_to_flip[index][1].bit;

	/* Only change the bytes if it produces valid characters */

	if (is_invalid_char(*p0 ^ m0) || is_invalid_char(*p1 ^ m1))
		return 0;

	*p0 ^= m0;
	*p1 ^= m1;

	return 1;
}

/*
 * This function generates a well-defined sequence of "alternate"
 * names for a given name.  An alternate is a name having the same
 * length and same hash value as the original name.  This is needed
 * because the algorithm produces only one obfuscated name to use
 * for a given original name, and it's possible that result matches
 * a name already seen.  This function checks for this, and if it
 * occurs, finds another suitable obfuscated name to use.
 *
 * Each bit in the binary representation of the sequence number is
 * used to select one possible "bit flip" operation to perform on
 * the name.  So for example:
 *    seq = 0:	selects no bits to flip
 *    seq = 1:	selects the 0th bit to flip
 *    seq = 2:	selects the 1st bit to flip
 *    seq = 3:	selects the 0th and 1st bit to flip
 *    ... and so on.
 *
 * The flip_bit() function takes care of the details of the bit
 * flipping within the name.  Note that the "1st bit" in this
 * context is a bit sequence number; i.e. it doesn't necessarily
 * mean bit 0x02 will be changed.
 *
 * If a valid name (one that contains no '/' or '\0' characters) is
 * produced by this process for the given sequence number, this
 * function returns 1.  If the result is not valid, it returns 0.
 * Returns -1 if the sequence number is beyond the the maximum for
 * names of the given length.
 *
 *
 * Discussion
 * ----------
 * The number of alternates available for a given name is dependent
 * on its length.  A "bit flip" involves inverting two bits in
 * a name--the two bits being selected such that their values
 * affect the name's hash value in the same way.  Alternates are
 * thus generated by inverting the value of pairs of such
 * "overlapping" bits in the original name.  Each byte after the
 * first in a name adds at least one bit of overlap to work with.
 * (See comments above flip_bit() for more discussion on this.)
 *
 * So the number of alternates is dependent on the number of such
 * overlapping bits in a name.  If there are N bit overlaps, there
 * 2^N alternates for that hash value.
 *
 * Here are the number of overlapping bits available for generating
 * alternates for names of specific lengths:
 *	1	0	(must have 2 bytes to have any overlap)
 *	2	1	One bit overlaps--so 2 possible alternates
 *	3	2	Two bits overlap--so 4 possible alternates
 *	4	4	Three bits overlap, so 2^3 alternates
 *	5	8	8 bits overlap (due to wrapping), 256 alternates
 *	6	18	2^18 alternates
 *	7	28	2^28 alternates
 *	   ...
 * It's clear that the number of alternates grows very quickly with
 * the length of the name.  But note that the set of alternates
 * includes invalid names.  And for certain (contrived) names, the
 * number of valid names is a fairly small fraction of the total
 * number of alternates.
 *
 * The main driver for this infrastructure for coming up with
 * alternate names is really related to names 5 (or possibly 6)
 * bytes in length.  5-byte obfuscated names contain no randomly-
 * generated bytes in them, and the chance of an obfuscated name
 * matching an already-seen name is too high to just ignore.  This
 * methodical selection of alternates ensures we don't produce
 * duplicate names unless we have exhausted our options.
 */
static int
find_alternate(
	size_t		name_len,
	unsigned char	*name,
	uint32_t	seq)
{
	uint32_t	bitseq = 0;
	uint32_t	bits = seq;

	if (!seq)
		return 1;	/* alternate 0 is the original name */
	if (name_len < 2)	/* Must have 2 bytes to flip */
		return -1;

	for (bitseq = 0; bits; bitseq++) {
		uint32_t	mask = 1 << bitseq;
		int		fb;

		if (!(bits & mask))
			continue;

		fb = flip_bit(name_len, name, bitseq);
		if (fb < 1)
			return fb ? -1 : 0;
		bits ^= mask;
	}

	return 1;
}

/*
 * Look up the given name in the name table.  If it is already
 * present, iterate through a well-defined sequence of alternate
 * names and attempt to use an alternate name instead.
 *
 * Returns 1 if the (possibly modified) name is not present in the
 * name table.  Returns 0 if the name and all possible alternates
 * are already in the table.
 */
static int
handle_duplicate_name(xfs_dahash_t hash, size_t name_len, unsigned char *name)
{
	unsigned char	new_name[name_len + 1];
	uint32_t	seq = 1;

	if (!nametable_find(hash, name_len, name))
		return 1;	/* No duplicate */

	/* Name is already in use.  Need to find an alternate. */

	do {
		int	found;

		/* Only change incoming name if we find an alternate */
		do {
			memcpy(new_name, name, name_len);
			found = find_alternate(name_len, new_name, seq++);
			if (found < 0)
				return 0;	/* No more to check */
		} while (!found);
	} while (nametable_find(hash, name_len, new_name));

	/*
	 * The alternate wasn't in the table already.  Pass it back
	 * to the caller.
	 */
	memcpy(name, new_name, name_len);

	return 1;
}

static void
generate_obfuscated_name(
	xfs_ino_t		ino,
	int			namelen,
	unsigned char		*name)
{
	xfs_dahash_t		hash;

	/*
	 * We don't obfuscate "lost+found" or any orphan files
	 * therein.  When the name table is used for extended
	 * attributes, the inode number provided is 0, in which
	 * case we don't need to make this check.
	 */
	if (ino && in_lost_found(ino, namelen, name))
		return;

	/*
	 * If the name starts with a slash, just skip over it.  It
	 * isn't included in the hash and we don't record it in the
	 * name table.  Note that the namelen value passed in does
	 * not count the leading slash (if one is present).
	 */
	if (*name == '/')
		name++;

	/* Obfuscate the name (if possible) */

	hash = libxfs_da_hashname(name, namelen);
	obfuscate_name(hash, namelen, name);

	/*
	 * Make sure the name is not something already seen.  If we
	 * fail to find a suitable alternate, we're dealing with a
	 * very pathological situation, and we may end up creating
	 * a duplicate name in the metadump, so issue a warning.
	 */
	if (!handle_duplicate_name(hash, namelen, name)) {
		print_warning("duplicate name for inode %llu "
				"in dir inode %llu\n",
			(unsigned long long) ino,
			(unsigned long long) cur_ino);
		return;
	}

	/* Create an entry for the new name in the name table. */

	if (!nametable_add(hash, namelen, name))
		print_warning("unable to record name for inode %llu "
				"in dir inode %llu\n",
			(unsigned long long) ino,
			(unsigned long long) cur_ino);
}

static void
process_sf_dir(
	xfs_dinode_t		*dip)
{
	struct xfs_dir2_sf_hdr	*sfp;
	xfs_dir2_sf_entry_t	*sfep;
	uint64_t		ino_dir_size;
	int			i;

	sfp = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	ino_dir_size = be64_to_cpu(dip->di_size);
	if (ino_dir_size > XFS_DFORK_DSIZE(dip, mp)) {
		ino_dir_size = XFS_DFORK_DSIZE(dip, mp);
		if (show_warnings)
			print_warning("invalid size in dir inode %llu",
					(long long)cur_ino);
	}

	sfep = xfs_dir2_sf_firstentry(sfp);
	for (i = 0; (i < sfp->count) &&
			((char *)sfep - (char *)sfp < ino_dir_size); i++) {

		/*
		 * first check for bad name lengths. If they are bad, we
		 * have limitations to how much can be obfuscated.
		 */
		int	namelen = sfep->namelen;

		if (namelen == 0) {
			if (show_warnings)
				print_warning("zero length entry in dir inode "
						"%llu", (long long)cur_ino);
			if (i != sfp->count - 1)
				break;
			namelen = ino_dir_size - ((char *)&sfep->name[0] -
					 (char *)sfp);
		} else if ((char *)sfep - (char *)sfp +
				M_DIROPS(mp)->sf_entsize(sfp, sfep->namelen) >
				ino_dir_size) {
			if (show_warnings)
				print_warning("entry length in dir inode %llu "
					"overflows space", (long long)cur_ino);
			if (i != sfp->count - 1)
				break;
			namelen = ino_dir_size - ((char *)&sfep->name[0] -
					 (char *)sfp);
		}

		if (obfuscate)
			generate_obfuscated_name(
					 M_DIROPS(mp)->sf_get_ino(sfp, sfep),
					 namelen, &sfep->name[0]);

		sfep = (xfs_dir2_sf_entry_t *)((char *)sfep +
				M_DIROPS(mp)->sf_entsize(sfp, namelen));
	}

	/* zero stale data in rest of space in data fork, if any */
	if (zero_stale_data && (ino_dir_size < XFS_DFORK_DSIZE(dip, mp)))
		memset(sfep, 0, XFS_DFORK_DSIZE(dip, mp) - ino_dir_size);
}

/*
 * The pathname may not be null terminated. It may be terminated by the end of
 * a buffer or inode literal area, and the start of the next region contains
 * unknown data. Therefore, when we get to the last component of the symlink, we
 * cannot assume that strlen() will give us the right result. Hence we need to
 * track the remaining pathname length and use that instead.
 */
static void
obfuscate_path_components(
	char			*buf,
	uint64_t		len)
{
	unsigned char		*comp = (unsigned char *)buf;
	unsigned char		*end = comp + len;
	xfs_dahash_t		hash;

	while (comp < end) {
		char	*slash;
		int	namelen;

		/* find slash at end of this component */
		slash = strchr((char *)comp, '/');
		if (!slash) {
			/* last (or single) component */
			namelen = strnlen((char *)comp, len);
			hash = libxfs_da_hashname(comp, namelen);
			obfuscate_name(hash, namelen, comp);
			break;
		}
		namelen = slash - (char *)comp;
		/* handle leading or consecutive slashes */
		if (!namelen) {
			comp++;
			len--;
			continue;
		}
		hash = libxfs_da_hashname(comp, namelen);
		obfuscate_name(hash, namelen, comp);
		comp += namelen + 1;
		len -= namelen + 1;
	}
}

static void
process_sf_symlink(
	xfs_dinode_t		*dip)
{
	uint64_t		len;
	char			*buf;

	len = be64_to_cpu(dip->di_size);
	if (len > XFS_DFORK_DSIZE(dip, mp)) {
		if (show_warnings)
			print_warning("invalid size (%d) in symlink inode %llu",
					len, (long long)cur_ino);
		len = XFS_DFORK_DSIZE(dip, mp);
	}

	buf = (char *)XFS_DFORK_DPTR(dip);
	if (obfuscate)
		obfuscate_path_components(buf, len);

	/* zero stale data in rest of space in data fork, if any */
	if (zero_stale_data && len < XFS_DFORK_DSIZE(dip, mp))
		memset(&buf[len], 0, XFS_DFORK_DSIZE(dip, mp) - len);
}

static void
process_sf_attr(
	xfs_dinode_t		*dip)
{
	/*
	 * with extended attributes, obfuscate the names and fill the actual
	 * values with 'v' (to see a valid string length, as opposed to NULLs)
	 */

	xfs_attr_shortform_t	*asfp;
	xfs_attr_sf_entry_t	*asfep;
	int			ino_attr_size;
	int			i;

	asfp = (xfs_attr_shortform_t *)XFS_DFORK_APTR(dip);
	if (asfp->hdr.count == 0)
		return;

	ino_attr_size = be16_to_cpu(asfp->hdr.totsize);
	if (ino_attr_size > XFS_DFORK_ASIZE(dip, mp)) {
		ino_attr_size = XFS_DFORK_ASIZE(dip, mp);
		if (show_warnings)
			print_warning("invalid attr size in inode %llu",
					(long long)cur_ino);
	}

	asfep = &asfp->list[0];
	for (i = 0; (i < asfp->hdr.count) &&
			((char *)asfep - (char *)asfp < ino_attr_size); i++) {

		int	namelen = asfep->namelen;

		if (namelen == 0) {
			if (show_warnings)
				print_warning("zero length attr entry in inode "
						"%llu", (long long)cur_ino);
			break;
		} else if ((char *)asfep - (char *)asfp +
				XFS_ATTR_SF_ENTSIZE(asfep) > ino_attr_size) {
			if (show_warnings)
				print_warning("attr entry length in inode %llu "
					"overflows space", (long long)cur_ino);
			break;
		}

		if (obfuscate) {
			generate_obfuscated_name(0, asfep->namelen,
						 &asfep->nameval[0]);
			memset(&asfep->nameval[asfep->namelen], 'v',
			       asfep->valuelen);
		}

		asfep = (xfs_attr_sf_entry_t *)((char *)asfep +
				XFS_ATTR_SF_ENTSIZE(asfep));
	}

	/* zero stale data in rest of space in attr fork, if any */
	if (zero_stale_data && (ino_attr_size < XFS_DFORK_ASIZE(dip, mp)))
		memset(asfep, 0, XFS_DFORK_ASIZE(dip, mp) - ino_attr_size);
}

static void
process_dir_leaf_block(
	char				*block)
{
	struct xfs_dir2_leaf		*leaf;
	struct xfs_dir3_icleaf_hdr 	leafhdr;

	if (!zero_stale_data)
		return;

	/* Yes, this works for dir2 & dir3.  Difference is padding. */
	leaf = (struct xfs_dir2_leaf *)block;
	M_DIROPS(mp)->leaf_hdr_from_disk(&leafhdr, leaf);

	/* Zero out space from end of ents[] to bests */
	if (leafhdr.magic == XFS_DIR2_LEAF1_MAGIC ||
	    leafhdr.magic == XFS_DIR3_LEAF1_MAGIC) {
		struct xfs_dir2_leaf_tail	*ltp;
		__be16				*lbp;
		struct xfs_dir2_leaf_entry	*ents;
		char				*free; /* end of ents */

		ents = M_DIROPS(mp)->leaf_ents_p(leaf);
		free = (char *)&ents[leafhdr.count];
		ltp = xfs_dir2_leaf_tail_p(mp->m_dir_geo, leaf);
		lbp = xfs_dir2_leaf_bests_p(ltp);
		memset(free, 0, (char *)lbp - free);
		iocur_top->need_crc = 1;
	}
}

static void
process_dir_data_block(
	char		*block,
	xfs_fileoff_t	offset,
	int		is_block_format)
{
	/*
	 * we have to rely on the fileoffset and signature of the block to
	 * handle it's contents. If it's invalid, leave it alone.
	 * for multi-fsblock dir blocks, if a name crosses an extent boundary,
	 * ignore it and continue.
	 */
	int		dir_offset;
	char		*ptr;
	char		*endptr;
	int		end_of_data;
	int		wantmagic;
	struct xfs_dir2_data_hdr *datahdr;

	datahdr = (struct xfs_dir2_data_hdr *)block;

	if (is_block_format) {
		xfs_dir2_leaf_entry_t	*blp;
		xfs_dir2_block_tail_t	*btp;

		btp = xfs_dir2_block_tail_p(mp->m_dir_geo, datahdr);
		blp = xfs_dir2_block_leaf_p(btp);
		if ((char *)blp > (char *)btp)
			blp = (xfs_dir2_leaf_entry_t *)btp;

		end_of_data = (char *)blp - block;
		if (xfs_sb_version_hascrc(&mp->m_sb))
			wantmagic = XFS_DIR3_BLOCK_MAGIC;
		else
			wantmagic = XFS_DIR2_BLOCK_MAGIC;
	} else { /* leaf/node format */
		end_of_data = mp->m_dir_geo->fsbcount << mp->m_sb.sb_blocklog;
		if (xfs_sb_version_hascrc(&mp->m_sb))
			wantmagic = XFS_DIR3_DATA_MAGIC;
		else
			wantmagic = XFS_DIR2_DATA_MAGIC;
	}

	if (be32_to_cpu(datahdr->magic) != wantmagic) {
		if (show_warnings)
			print_warning(
		"invalid magic in dir inode %llu block %ld",
					(long long)cur_ino, (long)offset);
		return;
	}

	dir_offset = M_DIROPS(mp)->data_entry_offset;
	ptr = block + dir_offset;
	endptr = block + mp->m_dir_geo->blksize;

	while (ptr < endptr && dir_offset < end_of_data) {
		xfs_dir2_data_entry_t	*dep;
		xfs_dir2_data_unused_t	*dup;
		int			length;

		dup = (xfs_dir2_data_unused_t *)ptr;

		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			int	length = be16_to_cpu(dup->length);
			if (dir_offset + length > end_of_data ||
			    !length || (length & (XFS_DIR2_DATA_ALIGN - 1))) {
				if (show_warnings)
					print_warning(
			"invalid length for dir free space in inode %llu",
						(long long)cur_ino);
				return;
			}
			if (be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)) !=
					dir_offset)
				return;
			dir_offset += length;
			ptr += length;
			/*
			 * Zero the unused space up to the tag - the tag is
			 * actually at a variable offset, so zeroing &dup->tag
			 * is zeroing the free space in between
			 */
			if (zero_stale_data) {
				int zlen = length -
						sizeof(xfs_dir2_data_unused_t);

				if (zlen > 0) {
					memset(&dup->tag, 0, zlen);
					iocur_top->need_crc = 1;
				}
			}
			if (dir_offset >= end_of_data || ptr >= endptr)
				return;
		}

		dep = (xfs_dir2_data_entry_t *)ptr;
		length = M_DIROPS(mp)->data_entsize(dep->namelen);

		if (dir_offset + length > end_of_data ||
		    ptr + length > endptr) {
			if (show_warnings)
				print_warning(
			"invalid length for dir entry name in inode %llu",
					(long long)cur_ino);
			return;
		}
		if (be16_to_cpu(*M_DIROPS(mp)->data_entry_tag_p(dep)) !=
				dir_offset)
			return;

		if (obfuscate)
			generate_obfuscated_name(be64_to_cpu(dep->inumber),
					 dep->namelen, &dep->name[0]);
		dir_offset += length;
		ptr += length;
		/* Zero the unused space after name, up to the tag */
		if (zero_stale_data) {
			/* 1 byte for ftype; don't bother with conditional */
			int zlen =
				(char *)M_DIROPS(mp)->data_entry_tag_p(dep) -
				(char *)&dep->name[dep->namelen] - 1;
			if (zlen > 0) {
				memset(&dep->name[dep->namelen] + 1, 0, zlen);
				iocur_top->need_crc = 1;
			}
		}
	}
}

static void
process_symlink_block(
	char			*block)
{
	char *link = block;

	if (xfs_sb_version_hascrc(&(mp)->m_sb))
		link += sizeof(struct xfs_dsymlink_hdr);

	if (obfuscate)
		obfuscate_path_components(link, XFS_SYMLINK_BUF_SPACE(mp,
							mp->m_sb.sb_blocksize));
	if (zero_stale_data) {
		size_t	linklen, zlen;

		linklen = strlen(link);
		zlen = mp->m_sb.sb_blocksize - linklen;
		if (xfs_sb_version_hascrc(&mp->m_sb))
			zlen -= sizeof(struct xfs_dsymlink_hdr);
		if (zlen < mp->m_sb.sb_blocksize)
			memset(link + linklen, 0, zlen);
	}
}

#define MAX_REMOTE_VALS		4095

static struct attr_data_s {
	int			remote_val_count;
	xfs_dablk_t		remote_vals[MAX_REMOTE_VALS];
} attr_data;

static inline void
add_remote_vals(
	xfs_dablk_t 		blockidx,
	int			length)
{
	while (length > 0 && attr_data.remote_val_count < MAX_REMOTE_VALS) {
		attr_data.remote_vals[attr_data.remote_val_count] = blockidx;
		attr_data.remote_val_count++;
		blockidx++;
		length -= XFS_ATTR3_RMT_BUF_SPACE(mp, mp->m_sb.sb_blocksize);
	}

	if (attr_data.remote_val_count >= MAX_REMOTE_VALS) {
		print_warning(
"Overflowed attr obfuscation array. No longer obfuscating remote attrs.");
	}
}

/* Handle remote and leaf attributes */
static void
process_attr_block(
	char				*block,
	xfs_fileoff_t			offset)
{
	struct xfs_attr_leafblock	*leaf;
	struct xfs_attr3_icleaf_hdr	hdr;
	int				i;
	int				nentries;
	xfs_attr_leaf_entry_t 		*entry;
	xfs_attr_leaf_name_local_t 	*local;
	xfs_attr_leaf_name_remote_t 	*remote;
	uint32_t			bs = mp->m_sb.sb_blocksize;
	char				*first_name;


	leaf = (xfs_attr_leafblock_t *)block;

	/* Remote attributes - attr3 has XFS_ATTR3_RMT_MAGIC, attr has none */
	if ((be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC) &&
	    (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR3_LEAF_MAGIC)) {
		for (i = 0; i < attr_data.remote_val_count; i++) {
			if (obfuscate && attr_data.remote_vals[i] == offset)
				/* Macros to handle both attr and attr3 */
				memset(block +
					(bs - XFS_ATTR3_RMT_BUF_SPACE(mp, bs)),
				      'v', XFS_ATTR3_RMT_BUF_SPACE(mp, bs));
		}
		return;
	}

	/* Ok, it's a leaf - get header; accounts for crc & non-crc */
	xfs_attr3_leaf_hdr_from_disk(mp->m_attr_geo, &hdr, leaf);

	nentries = hdr.count;
	if (nentries == 0 ||
	    nentries * sizeof(xfs_attr_leaf_entry_t) +
			xfs_attr3_leaf_hdr_size(leaf) >
				XFS_ATTR3_RMT_BUF_SPACE(mp, bs)) {
		if (show_warnings)
			print_warning("invalid attr count in inode %llu",
					(long long)cur_ino);
		return;
	}

	entry = xfs_attr3_leaf_entryp(leaf);
	/* We will move this as we parse */
	first_name = NULL;
	for (i = 0; i < nentries; i++, entry++) {
		int nlen, vlen, zlen;

		/* Grows up; if this name is topmost, move first_name */
		if (!first_name || xfs_attr3_leaf_name(leaf, i) < first_name)
			first_name = xfs_attr3_leaf_name(leaf, i);

		if (be16_to_cpu(entry->nameidx) > mp->m_sb.sb_blocksize) {
			if (show_warnings)
				print_warning(
				"invalid attr nameidx in inode %llu",
						(long long)cur_ino);
			break;
		}
		if (entry->flags & XFS_ATTR_LOCAL) {
			local = xfs_attr3_leaf_name_local(leaf, i);
			if (local->namelen == 0) {
				if (show_warnings)
					print_warning(
				"zero length for attr name in inode %llu",
						(long long)cur_ino);
				break;
			}
			if (obfuscate) {
				generate_obfuscated_name(0, local->namelen,
					&local->nameval[0]);
				memset(&local->nameval[local->namelen], 'v',
					be16_to_cpu(local->valuelen));
			}
			/* zero from end of nameval[] to next name start */
			nlen = local->namelen;
			vlen = be16_to_cpu(local->valuelen);
			zlen = xfs_attr_leaf_entsize_local(nlen, vlen) -
				(sizeof(xfs_attr_leaf_name_local_t) - 1 +
				 nlen + vlen);
			if (zero_stale_data)
				memset(&local->nameval[nlen + vlen], 0, zlen);
		} else {
			remote = xfs_attr3_leaf_name_remote(leaf, i);
			if (remote->namelen == 0 || remote->valueblk == 0) {
				if (show_warnings)
					print_warning(
				"invalid attr entry in inode %llu",
						(long long)cur_ino);
				break;
			}
			if (obfuscate) {
				generate_obfuscated_name(0, remote->namelen,
							 &remote->name[0]);
				add_remote_vals(be32_to_cpu(remote->valueblk),
						be32_to_cpu(remote->valuelen));
			}
			/* zero from end of name[] to next name start */
			nlen = remote->namelen;
			zlen = xfs_attr_leaf_entsize_remote(nlen) -
				(sizeof(xfs_attr_leaf_name_remote_t) - 1 +
				 nlen);
			if (zero_stale_data)
				memset(&remote->name[nlen], 0, zlen);
		}
	}

	/* Zero from end of entries array to the first name/val */
	if (zero_stale_data) {
		struct xfs_attr_leaf_entry *entries;

		entries = xfs_attr3_leaf_entryp(leaf);
		memset(&entries[nentries], 0,
		       first_name - (char *)&entries[nentries]);
	}
}

/* Processes symlinks, attrs, directories ... */
static int
process_single_fsb_objects(
	xfs_fileoff_t	o,
	xfs_fsblock_t	s,
	xfs_filblks_t	c,
	typnm_t		btype,
	xfs_fileoff_t	last)
{
	char		*dp;
	int		ret = 0;
	int		i;

	for (i = 0; i < c; i++) {
		push_cur();
		set_cur(&typtab[btype], XFS_FSB_TO_DADDR(mp, s), blkbb,
				DB_RING_IGN, NULL);

		if (!iocur_top->data) {
			xfs_agnumber_t	agno = XFS_FSB_TO_AGNO(mp, s);
			xfs_agblock_t	agbno = XFS_FSB_TO_AGBNO(mp, s);

			print_warning("cannot read %s block %u/%u (%llu)",
					typtab[btype].name, agno, agbno, s);
			if (stop_on_read_error)
				ret = -EIO;
			goto out_pop;

		}

		if (!obfuscate && !zero_stale_data)
			goto write;

		/* Zero unused part of interior nodes */
		if (zero_stale_data) {
			xfs_da_intnode_t *node = iocur_top->data;
			int magic = be16_to_cpu(node->hdr.info.magic);

			if (magic == XFS_DA_NODE_MAGIC ||
			    magic == XFS_DA3_NODE_MAGIC) {
				struct xfs_da3_icnode_hdr hdr;
				int used;

				M_DIROPS(mp)->node_hdr_from_disk(&hdr, node);
				used = M_DIROPS(mp)->node_hdr_size;

				used += hdr.count
					* sizeof(struct xfs_da_node_entry);

				if (used < mp->m_sb.sb_blocksize) {
					memset((char *)node + used, 0,
						mp->m_sb.sb_blocksize - used);
					iocur_top->need_crc = 1;
				}
			}
		}

		/* Handle leaf nodes */
		dp = iocur_top->data;
		switch (btype) {
		case TYP_DIR2:
			if (o >= mp->m_dir_geo->freeblk) {
				/* TODO, zap any stale data */
				break;
			} else if (o >= mp->m_dir_geo->leafblk) {
				process_dir_leaf_block(dp);
			} else {
				process_dir_data_block(dp, o,
					 last == mp->m_dir_geo->fsbcount);
			}
			iocur_top->need_crc = 1;
			break;
		case TYP_SYMLINK:
			process_symlink_block(dp);
			iocur_top->need_crc = 1;
			break;
		case TYP_ATTR:
			process_attr_block(dp, o);
			iocur_top->need_crc = 1;
			break;
		default:
			break;
		}

write:
		ret = write_buf(iocur_top);
out_pop:
		pop_cur();
		if (ret)
			break;
		o++;
		s++;
	}

	return ret;
}

/*
 * Static map to aggregate multiple extents into a single directory block.
 */
static struct bbmap mfsb_map;
static int mfsb_length;

static int
process_multi_fsb_objects(
	xfs_fileoff_t	o,
	xfs_fsblock_t	s,
	xfs_filblks_t	c,
	typnm_t		btype,
	xfs_fileoff_t	last)
{
	int		ret = 0;

	switch (btype) {
	case TYP_DIR2:
		break;
	default:
		print_warning("bad type for multi-fsb object %d", btype);
		return -EINVAL;
	}

	while (c > 0) {
		unsigned int	bm_len;

		if (mfsb_length + c >= mp->m_dir_geo->fsbcount) {
			bm_len = mp->m_dir_geo->fsbcount - mfsb_length;
			mfsb_length = 0;
		} else {
			mfsb_length += c;
			bm_len = c;
		}

		mfsb_map.b[mfsb_map.nmaps].bm_bn = XFS_FSB_TO_DADDR(mp, s);
		mfsb_map.b[mfsb_map.nmaps].bm_len = XFS_FSB_TO_BB(mp, bm_len);
		mfsb_map.nmaps++;

		if (mfsb_length == 0) {
			push_cur();
			set_cur(&typtab[btype], 0, 0, DB_RING_IGN, &mfsb_map);
			if (!iocur_top->data) {
				xfs_agnumber_t	agno = XFS_FSB_TO_AGNO(mp, s);
				xfs_agblock_t	agbno = XFS_FSB_TO_AGBNO(mp, s);

				print_warning("cannot read %s block %u/%u (%llu)",
						typtab[btype].name, agno, agbno, s);
				if (stop_on_read_error)
					ret = -1;
				goto out_pop;

			}

			if ((!obfuscate && !zero_stale_data) ||
			     o >= mp->m_dir_geo->leafblk) {
				ret = write_buf(iocur_top);
				goto out_pop;
			}

			process_dir_data_block(iocur_top->data, o,
					       last == mp->m_dir_geo->fsbcount);
			iocur_top->need_crc = 1;
			ret = write_buf(iocur_top);
out_pop:
			pop_cur();
			mfsb_map.nmaps = 0;
			if (ret)
				break;
		}
		c -= bm_len;
		s += bm_len;
	}

	return ret;
}

/* inode copy routines */
static int
process_bmbt_reclist(
	xfs_bmbt_rec_t 		*rp,
	int 			numrecs,
	typnm_t			btype)
{
	int			i;
	xfs_fileoff_t		o, op = NULLFILEOFF;
	xfs_fsblock_t		s;
	xfs_filblks_t		c, cp = NULLFILEOFF;
	int			f;
	xfs_fileoff_t		last;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	int			error;

	if (btype == TYP_DATA)
		return 1;

	convert_extent(&rp[numrecs - 1], &o, &s, &c, &f);
	last = o + c;

	for (i = 0; i < numrecs; i++, rp++) {
		convert_extent(rp, &o, &s, &c, &f);

		/*
		 * ignore extents that are clearly bogus, and if a bogus
		 * one is found, stop processing remaining extents
		 */
		if (i > 0 && op + cp > o) {
			if (show_warnings)
				print_warning("bmap extent %d in %s ino %llu "
					"starts at %llu, previous extent "
					"ended at %llu", i,
					typtab[btype].name, (long long)cur_ino,
					o, op + cp - 1);
			break;
		}

		if (c > max_extent_size) {
			/*
			 * since we are only processing non-data extents,
			 * large numbers of blocks in a metadata extent is
			 * extremely rare and more than likely to be corrupt.
			 */
			if (show_warnings)
				print_warning("suspicious count %u in bmap "
					"extent %d in %s ino %llu", c, i,
					typtab[btype].name, (long long)cur_ino);
			break;
		}

		op = o;
		cp = c;

		agno = XFS_FSB_TO_AGNO(mp, s);
		agbno = XFS_FSB_TO_AGBNO(mp, s);

		if (!valid_bno(agno, agbno)) {
			if (show_warnings)
				print_warning("invalid block number %u/%u "
					"(%llu) in bmap extent %d in %s ino "
					"%llu", agno, agbno, s, i,
					typtab[btype].name, (long long)cur_ino);
			break;
		}

		if (!valid_bno(agno, agbno + c - 1)) {
			if (show_warnings)
				print_warning("bmap extent %i in %s inode %llu "
					"overflows AG (end is %u/%u)", i,
					typtab[btype].name, (long long)cur_ino,
					agno, agbno + c - 1);
			break;
		}

		/* multi-extent blocks require special handling */
		if (btype != TYP_DIR2 || mp->m_dir_geo->fsbcount == 1) {
			error = process_single_fsb_objects(o, s, c, btype, last);
		} else {
			error = process_multi_fsb_objects(o, s, c, btype, last);
		}
		if (error)
			return 0;
	}

	return 1;
}

static int
scanfunc_bmap(
	struct xfs_btree_block	*block,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	int			level,
	typnm_t			btype,
	void			*arg)	/* ptr to itype */
{
	int			i;
	xfs_bmbt_ptr_t		*pp;
	int			nrecs;

	nrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (nrecs > mp->m_bmap_dmxr[0]) {
			if (show_warnings)
				print_warning("invalid numrecs (%u) in %s "
					"block %u/%u", nrecs,
					typtab[btype].name, agno, agbno);
			return 1;
		}
		return process_bmbt_reclist(XFS_BMBT_REC_ADDR(mp, block, 1),
					    nrecs, *(typnm_t*)arg);
	}

	if (nrecs > mp->m_bmap_dmxr[1]) {
		if (show_warnings)
			print_warning("invalid numrecs (%u) in %s block %u/%u",
					nrecs, typtab[btype].name, agno, agbno);
		return 1;
	}
	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);
	for (i = 0; i < nrecs; i++) {
		xfs_agnumber_t	ag;
		xfs_agblock_t	bno;

		ag = XFS_FSB_TO_AGNO(mp, get_unaligned_be64(&pp[i]));
		bno = XFS_FSB_TO_AGBNO(mp, get_unaligned_be64(&pp[i]));

		if (bno == 0 || bno > mp->m_sb.sb_agblocks ||
				ag > mp->m_sb.sb_agcount) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
					"in %s block %u/%u", ag, bno,
					typtab[btype].name, agno, agbno);
			continue;
		}

		if (!scan_btree(ag, bno, level, btype, arg, scanfunc_bmap))
			return 0;
	}
	return 1;
}

static int
process_btinode(
	xfs_dinode_t 		*dip,
	typnm_t			itype)
{
	xfs_bmdr_block_t	*dib;
	int			i;
	xfs_bmbt_ptr_t		*pp;
	int			level;
	int			nrecs;
	int			maxrecs;
	int			whichfork;
	typnm_t			btype;

	whichfork = (itype == TYP_ATTR) ? XFS_ATTR_FORK : XFS_DATA_FORK;
	btype = (itype == TYP_ATTR) ? TYP_BMAPBTA : TYP_BMAPBTD;

	dib = (xfs_bmdr_block_t *)XFS_DFORK_PTR(dip, whichfork);
	level = be16_to_cpu(dib->bb_level);
	nrecs = be16_to_cpu(dib->bb_numrecs);

	if (level > XFS_BM_MAXLEVELS(mp, whichfork)) {
		if (show_warnings)
			print_warning("invalid level (%u) in inode %lld %s "
					"root", level, (long long)cur_ino,
					typtab[btype].name);
		return 1;
	}

	if (level == 0) {
		return process_bmbt_reclist(XFS_BMDR_REC_ADDR(dib, 1),
					    nrecs, itype);
	}

	maxrecs = libxfs_bmdr_maxrecs(XFS_DFORK_SIZE(dip, mp, whichfork), 0);
	if (nrecs > maxrecs) {
		if (show_warnings)
			print_warning("invalid numrecs (%u) in inode %lld %s "
					"root", nrecs, (long long)cur_ino,
					typtab[btype].name);
		return 1;
	}

	pp = XFS_BMDR_PTR_ADDR(dib, 1, maxrecs);
	for (i = 0; i < nrecs; i++) {
		xfs_agnumber_t	ag;
		xfs_agblock_t	bno;

		ag = XFS_FSB_TO_AGNO(mp, get_unaligned_be64(&pp[i]));
		bno = XFS_FSB_TO_AGBNO(mp, get_unaligned_be64(&pp[i]));

		if (bno == 0 || bno > mp->m_sb.sb_agblocks ||
				ag > mp->m_sb.sb_agcount) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
						"in inode %llu %s root", ag,
						bno, (long long)cur_ino,
						typtab[btype].name);
			continue;
		}

		if (!scan_btree(ag, bno, level, btype, &itype, scanfunc_bmap))
			return 0;
	}
	return 1;
}

static int
process_exinode(
	xfs_dinode_t 		*dip,
	typnm_t			itype)
{
	int			whichfork;
	int			used;
	xfs_extnum_t		nex;

	whichfork = (itype == TYP_ATTR) ? XFS_ATTR_FORK : XFS_DATA_FORK;

	nex = XFS_DFORK_NEXTENTS(dip, whichfork);
	used = nex * sizeof(xfs_bmbt_rec_t);
	if (nex < 0 || used > XFS_DFORK_SIZE(dip, mp, whichfork)) {
		if (show_warnings)
			print_warning("bad number of extents %d in inode %lld",
				nex, (long long)cur_ino);
		return 1;
	}

	/* Zero unused data fork past used extents */
	if (zero_stale_data && (used < XFS_DFORK_SIZE(dip, mp, whichfork)))
		memset(XFS_DFORK_PTR(dip, whichfork) + used, 0,
		       XFS_DFORK_SIZE(dip, mp, whichfork) - used);


	return process_bmbt_reclist((xfs_bmbt_rec_t *)XFS_DFORK_PTR(dip,
					whichfork), nex, itype);
}

static int
process_inode_data(
	xfs_dinode_t		*dip,
	typnm_t			itype)
{
	switch (dip->di_format) {
		case XFS_DINODE_FMT_LOCAL:
			if (obfuscate || zero_stale_data)
				switch (itype) {
					case TYP_DIR2:
						process_sf_dir(dip);
						break;

					case TYP_SYMLINK:
						process_sf_symlink(dip);
						break;

					default: ;
				}
			break;

		case XFS_DINODE_FMT_EXTENTS:
			return process_exinode(dip, itype);

		case XFS_DINODE_FMT_BTREE:
			return process_btinode(dip, itype);
	}
	return 1;
}

/*
 * when we process the inode, we may change the data in the data and/or
 * attribute fork if they are in short form and we are obfuscating names.
 * In this case we need to recalculate the CRC of the inode, but we should
 * only do that if the CRC in the inode is good to begin with. If the crc
 * is not ok, we just leave it alone.
 */
static int
process_inode(
	xfs_agnumber_t		agno,
	xfs_agino_t 		agino,
	xfs_dinode_t 		*dip,
	bool			free_inode)
{
	int			success;
	bool			crc_was_ok = false; /* no recalc by default */
	bool			need_new_crc = false;

	success = 1;
	cur_ino = XFS_AGINO_TO_INO(mp, agno, agino);

	/* we only care about crc recalculation if we will modify the inode. */
	if (obfuscate || zero_stale_data) {
		crc_was_ok = libxfs_verify_cksum((char *)dip,
					mp->m_sb.sb_inodesize,
					offsetof(struct xfs_dinode, di_crc));
	}

	if (free_inode) {
		if (zero_stale_data) {
			/* Zero all of the inode literal area */
			memset(XFS_DFORK_DPTR(dip), 0,
			       XFS_LITINO(mp, dip->di_version));
		}
		goto done;
	}

	/* copy appropriate data fork metadata */
	switch (be16_to_cpu(dip->di_mode) & S_IFMT) {
		case S_IFDIR:
			success = process_inode_data(dip, TYP_DIR2);
			if (dip->di_format == XFS_DINODE_FMT_LOCAL)
				need_new_crc = 1;
			break;
		case S_IFLNK:
			success = process_inode_data(dip, TYP_SYMLINK);
			if (dip->di_format == XFS_DINODE_FMT_LOCAL)
				need_new_crc = 1;
			break;
		case S_IFREG:
			success = process_inode_data(dip, TYP_DATA);
			break;
		default: ;
	}
	nametable_clear();

	/* copy extended attributes if they exist and forkoff is valid */
	if (success &&
	    XFS_DFORK_DSIZE(dip, mp) < XFS_LITINO(mp, dip->di_version)) {
		attr_data.remote_val_count = 0;
		switch (dip->di_aformat) {
			case XFS_DINODE_FMT_LOCAL:
				need_new_crc = 1;
				if (obfuscate || zero_stale_data)
					process_sf_attr(dip);
				break;

			case XFS_DINODE_FMT_EXTENTS:
				success = process_exinode(dip, TYP_ATTR);
				break;

			case XFS_DINODE_FMT_BTREE:
				success = process_btinode(dip, TYP_ATTR);
				break;
		}
		nametable_clear();
	}

done:
	/* Heavy handed but low cost; just do it as a catch-all. */
	if (zero_stale_data)
		need_new_crc = 1;

	if (crc_was_ok && need_new_crc)
		libxfs_dinode_calc_crc(mp, dip);
	return success;
}

static uint32_t	inodes_copied;

static int
copy_inode_chunk(
	xfs_agnumber_t 		agno,
	xfs_inobt_rec_t 	*rp)
{
	xfs_agino_t 		agino;
	int			off;
	xfs_agblock_t		agbno;
	xfs_agblock_t		end_agbno;
	int			i;
	int			rval = 0;
	int			blks_per_buf;
	int			inodes_per_buf;
	int			ioff;

	agino = be32_to_cpu(rp->ir_startino);
	agbno = XFS_AGINO_TO_AGBNO(mp, agino);
	end_agbno = agbno + mp->m_ialloc_blks;
	off = XFS_INO_TO_OFFSET(mp, agino);

	/*
	 * If the fs supports sparse inode records, we must process inodes a
	 * cluster at a time because that is the sparse allocation granularity.
	 * Otherwise, we risk CRC corruption errors on reads of inode chunks.
	 *
	 * Also make sure that that we don't process more than the single record
	 * we've been passed (large block sizes can hold multiple inode chunks).
	 */
	if (xfs_sb_version_hassparseinodes(&mp->m_sb))
		blks_per_buf = xfs_icluster_size_fsb(mp);
	else
		blks_per_buf = mp->m_ialloc_blks;
	inodes_per_buf = min(blks_per_buf << mp->m_sb.sb_inopblog,
			     XFS_INODES_PER_CHUNK);

	/*
	 * Sanity check that we only process a single buffer if ir_startino has
	 * a buffer offset. A non-zero offset implies that the entire chunk lies
	 * within a block.
	 */
	if (off && inodes_per_buf != XFS_INODES_PER_CHUNK) {
		print_warning("bad starting inode offset %d", off);
		return 0;
	}

	if (agino == 0 || agino == NULLAGINO || !valid_bno(agno, agbno) ||
			!valid_bno(agno, XFS_AGINO_TO_AGBNO(mp,
					agino + XFS_INODES_PER_CHUNK - 1))) {
		if (show_warnings)
			print_warning("bad inode number %llu (%u/%u)",
				XFS_AGINO_TO_INO(mp, agno, agino), agno, agino);
		return 1;
	}

	/*
	 * check for basic assumptions about inode chunks, and if any
	 * assumptions fail, don't process the inode chunk.
	 */
	if ((mp->m_sb.sb_inopblock <= XFS_INODES_PER_CHUNK && off != 0) ||
			(mp->m_sb.sb_inopblock > XFS_INODES_PER_CHUNK &&
					off % XFS_INODES_PER_CHUNK != 0) ||
			(xfs_sb_version_hasalign(&mp->m_sb) &&
					mp->m_sb.sb_inoalignmt != 0 &&
					agbno % mp->m_sb.sb_inoalignmt != 0)) {
		if (show_warnings)
			print_warning("badly aligned inode (start = %llu)",
					XFS_AGINO_TO_INO(mp, agno, agino));
		return 1;
	}

	push_cur();
	ioff = 0;
	while (agbno < end_agbno && ioff < XFS_INODES_PER_CHUNK) {
		if (xfs_inobt_is_sparse_disk(rp, ioff))
			goto next_bp;

		set_cur(&typtab[TYP_INODE], XFS_AGB_TO_DADDR(mp, agno, agbno),
			XFS_FSB_TO_BB(mp, blks_per_buf), DB_RING_IGN, NULL);
		if (iocur_top->data == NULL) {
			print_warning("cannot read inode block %u/%u",
				      agno, agbno);
			rval = !stop_on_read_error;
			goto pop_out;
		}

		for (i = 0; i < inodes_per_buf; i++) {
			xfs_dinode_t	*dip;

			dip = (xfs_dinode_t *)((char *)iocur_top->data +
					((off + i) << mp->m_sb.sb_inodelog));

			/* process_inode handles free inodes, too */
			if (!process_inode(agno, agino + ioff + i, dip,
			    XFS_INOBT_IS_FREE_DISK(rp, ioff + i)))
				goto pop_out;

			inodes_copied++;
		}

		if (write_buf(iocur_top))
			goto pop_out;

next_bp:
		agbno += blks_per_buf;
		ioff += inodes_per_buf;
	}

	if (show_progress)
		print_progress("Copied %u of %u inodes (%u of %u AGs)",
				inodes_copied, mp->m_sb.sb_icount, agno,
				mp->m_sb.sb_agcount);
	rval = 1;
pop_out:
	pop_cur();
	return rval;
}

static int
scanfunc_ino(
	struct xfs_btree_block	*block,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	int			level,
	typnm_t			btype,
	void			*arg)
{
	xfs_inobt_rec_t		*rp;
	xfs_inobt_ptr_t		*pp;
	int			i;
	int			numrecs;
	int			finobt = *(int *) arg;

	numrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (numrecs > mp->m_inobt_mxr[0]) {
			if (show_warnings)
				print_warning("invalid numrecs %d in %s "
					"block %u/%u", numrecs,
					typtab[btype].name, agno, agbno);
			numrecs = mp->m_inobt_mxr[0];
		}

		/*
		 * Only copy the btree blocks for the finobt. The inobt scan
		 * copies the inode chunks.
		 */
		if (finobt)
			return 1;

		rp = XFS_INOBT_REC_ADDR(mp, block, 1);
		for (i = 0; i < numrecs; i++, rp++) {
			if (!copy_inode_chunk(agno, rp))
				return 0;
		}
		return 1;
	}

	if (numrecs > mp->m_inobt_mxr[1]) {
		if (show_warnings)
			print_warning("invalid numrecs %d in %s block %u/%u",
				numrecs, typtab[btype].name, agno, agbno);
		numrecs = mp->m_inobt_mxr[1];
	}

	pp = XFS_INOBT_PTR_ADDR(mp, block, 1, mp->m_inobt_mxr[1]);
	for (i = 0; i < numrecs; i++) {
		if (!valid_bno(agno, be32_to_cpu(pp[i]))) {
			if (show_warnings)
				print_warning("invalid block number (%u/%u) "
					"in %s block %u/%u",
					agno, be32_to_cpu(pp[i]),
					typtab[btype].name, agno, agbno);
			continue;
		}
		if (!scan_btree(agno, be32_to_cpu(pp[i]), level,
				btype, arg, scanfunc_ino))
			return 0;
	}
	return 1;
}

static int
copy_inodes(
	xfs_agnumber_t		agno,
	xfs_agi_t		*agi)
{
	xfs_agblock_t		root;
	int			levels;
	int			finobt = 0;

	root = be32_to_cpu(agi->agi_root);
	levels = be32_to_cpu(agi->agi_level);

	/* validate root and levels before processing the tree */
	if (root == 0 || root > mp->m_sb.sb_agblocks) {
		if (show_warnings)
			print_warning("invalid block number (%u) in inobt "
					"root in agi %u", root, agno);
		return 1;
	}
	if (levels >= XFS_BTREE_MAXLEVELS) {
		if (show_warnings)
			print_warning("invalid level (%u) in inobt root "
					"in agi %u", levels, agno);
		return 1;
	}

	if (!scan_btree(agno, root, levels, TYP_INOBT, &finobt, scanfunc_ino))
		return 0;

	if (xfs_sb_version_hasfinobt(&mp->m_sb)) {
		root = be32_to_cpu(agi->agi_free_root);
		levels = be32_to_cpu(agi->agi_free_level);

		finobt = 1;
		if (!scan_btree(agno, root, levels, TYP_INOBT, &finobt,
				scanfunc_ino))
			return 0;
	}

	return 1;
}

static int
scan_ag(
	xfs_agnumber_t	agno)
{
	xfs_agf_t	*agf;
	xfs_agi_t	*agi;
	int		stack_count = 0;
	int		rval = 0;

	/* copy the superblock of the AG */
	push_cur();
	stack_count++;
	set_cur(&typtab[TYP_SB], XFS_AG_DADDR(mp, agno, XFS_SB_DADDR),
			XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if (!iocur_top->data) {
		print_warning("cannot read superblock for ag %u", agno);
		if (stop_on_read_error)
			goto pop_out;
	} else {
		/* Replace any filesystem label with "L's" */
		if (obfuscate) {
			struct xfs_sb *sb = iocur_top->data;
			memset(sb->sb_fname, 'L',
			       min(strlen(sb->sb_fname), sizeof(sb->sb_fname)));
			iocur_top->need_crc = 1;
		}
		if (write_buf(iocur_top))
			goto pop_out;
	}

	/* copy the AG free space btree root */
	push_cur();
	stack_count++;
	set_cur(&typtab[TYP_AGF], XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	agf = iocur_top->data;
	if (iocur_top->data == NULL) {
		print_warning("cannot read agf block for ag %u", agno);
		if (stop_on_read_error)
			goto pop_out;
	} else {
		if (write_buf(iocur_top))
			goto pop_out;
	}

	/* copy the AG inode btree root */
	push_cur();
	stack_count++;
	set_cur(&typtab[TYP_AGI], XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	agi = iocur_top->data;
	if (iocur_top->data == NULL) {
		print_warning("cannot read agi block for ag %u", agno);
		if (stop_on_read_error)
			goto pop_out;
	} else {
		if (write_buf(iocur_top))
			goto pop_out;
	}

	/* copy the AG free list header */
	push_cur();
	stack_count++;
	set_cur(&typtab[TYP_AGFL], XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		print_warning("cannot read agfl block for ag %u", agno);
		if (stop_on_read_error)
			goto pop_out;
	} else {
		if (agf && zero_stale_data) {
			/* Zero out unused bits of agfl */
			int i;
			 __be32  *agfl_bno;

			agfl_bno = XFS_BUF_TO_AGFL_BNO(mp, iocur_top->bp);
			i = be32_to_cpu(agf->agf_fllast);

			for (;;) {
				if (++i == XFS_AGFL_SIZE(mp))
					i = 0;
				if (i == be32_to_cpu(agf->agf_flfirst))
					break;
				agfl_bno[i] = cpu_to_be32(NULLAGBLOCK);
			}
			iocur_top->need_crc = 1;
		}
		if (write_buf(iocur_top))
			goto pop_out;
	}

	/* copy AG free space btrees */
	if (agf) {
		if (show_progress)
			print_progress("Copying free space trees of AG %u",
					agno);
		if (!copy_free_bno_btree(agno, agf))
			goto pop_out;
		if (!copy_free_cnt_btree(agno, agf))
			goto pop_out;
		if (!copy_rmap_btree(agno, agf))
			goto pop_out;
		if (!copy_refcount_btree(agno, agf))
			goto pop_out;
	}

	/* copy inode btrees and the inodes and their associated metadata */
	if (agi) {
		if (!copy_inodes(agno, agi))
			goto pop_out;
	}
	rval = 1;
pop_out:
	while (stack_count--)
		pop_cur();
	return rval;
}

static int
copy_ino(
	xfs_ino_t		ino,
	typnm_t			itype)
{
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	xfs_agino_t		agino;
	int			offset;
	int			rval = 0;

	if (ino == 0 || ino == NULLFSINO)
		return 1;

	agno = XFS_INO_TO_AGNO(mp, ino);
	agino = XFS_INO_TO_AGINO(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, agino);
	offset = XFS_AGINO_TO_OFFSET(mp, agino);

	if (agno >= mp->m_sb.sb_agcount || agbno >= mp->m_sb.sb_agblocks ||
			offset >= mp->m_sb.sb_inopblock) {
		if (show_warnings)
			print_warning("invalid %s inode number (%lld)",
					typtab[itype].name, (long long)ino);
		return 1;
	}

	push_cur();
	set_cur(&typtab[TYP_INODE], XFS_AGB_TO_DADDR(mp, agno, agbno),
			blkbb, DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		print_warning("cannot read %s inode %lld",
				typtab[itype].name, (long long)ino);
		rval = !stop_on_read_error;
		goto pop_out;
	}
	off_cur(offset << mp->m_sb.sb_inodelog, mp->m_sb.sb_inodesize);

	cur_ino = ino;
	rval = process_inode_data(iocur_top->data, itype);
pop_out:
	pop_cur();
	return rval;
}


static int
copy_sb_inodes(void)
{
	if (!copy_ino(mp->m_sb.sb_rbmino, TYP_RTBITMAP))
		return 0;

	if (!copy_ino(mp->m_sb.sb_rsumino, TYP_RTSUMMARY))
		return 0;

	if (!copy_ino(mp->m_sb.sb_uquotino, TYP_DQBLK))
		return 0;

	if (!copy_ino(mp->m_sb.sb_gquotino, TYP_DQBLK))
		return 0;

	return copy_ino(mp->m_sb.sb_pquotino, TYP_DQBLK);
}

static int
copy_log(void)
{
	struct xlog	log;
	int		dirty;
	xfs_daddr_t	logstart;
	int		logblocks;
	int		logversion;
	int		cycle = XLOG_INIT_CYCLE;

	if (show_progress)
		print_progress("Copying log");

	push_cur();
	set_cur(&typtab[TYP_LOG], XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart),
			mp->m_sb.sb_logblocks * blkbb, DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		pop_cur();
		print_warning("cannot read log");
		return !stop_on_read_error;
	}

	/* If not obfuscating or zeroing, just copy the log as it is */
	if (!obfuscate && !zero_stale_data)
		goto done;

	dirty = xlog_is_dirty(mp, &log, &x, 0);

	switch (dirty) {
	case 0:
		/* clear out a clean log */
		if (show_progress)
			print_progress("Zeroing clean log");

		logstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart);
		logblocks = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
		logversion = xfs_sb_version_haslogv2(&mp->m_sb) ? 2 : 1;
		if (xfs_sb_version_hascrc(&mp->m_sb))
			cycle = log.l_curr_cycle + 1;

		libxfs_log_clear(NULL, iocur_top->data, logstart, logblocks,
				 &mp->m_sb.sb_uuid, logversion,
				 mp->m_sb.sb_logsunit, XLOG_FMT, cycle, true);
		break;
	case 1:
		/* keep the dirty log */
		if (obfuscate)
			print_warning(
_("Warning: log recovery of an obfuscated metadata image can leak "
"unobfuscated metadata and/or cause image corruption.  If possible, "
"please mount the filesystem to clean the log, or disable obfuscation."));
		break;
	case -1:
		/* log detection error */
		if (obfuscate)
			print_warning(
_("Could not discern log; image will contain unobfuscated metadata in log."));
		break;
	}

done:
	return !write_buf(iocur_top);
}

static int
metadump_f(
	int 		argc,
	char 		**argv)
{
	xfs_agnumber_t	agno;
	int		c;
	int		start_iocur_sp;
	int		outfd = -1;
	int		ret;
	char		*p;

	exitcode = 1;
	show_progress = 0;
	show_warnings = 0;
	stop_on_read_error = 0;

	if (mp->m_sb.sb_magicnum != XFS_SB_MAGIC) {
		print_warning("bad superblock magic number %x, giving up",
				mp->m_sb.sb_magicnum);
		return 0;
	}

	/*
	 * on load, we sanity-checked agcount and possibly set to 1
	 * if it was corrupted and large.
	 */
	if (mp->m_sb.sb_agcount == 1 &&
	    XFS_MAX_DBLOCKS(&mp->m_sb) < mp->m_sb.sb_dblocks) {
		print_warning("truncated agcount, giving up");
		return 0;
	}

	while ((c = getopt(argc, argv, "aegm:ow")) != EOF) {
		switch (c) {
			case 'a':
				zero_stale_data = 0;
				break;
			case 'e':
				stop_on_read_error = 1;
				break;
			case 'g':
				show_progress = 1;
				break;
			case 'm':
				max_extent_size = (int)strtol(optarg, &p, 0);
				if (*p != '\0' || max_extent_size <= 0) {
					print_warning("bad max extent size %s",
							optarg);
					return 0;
				}
				break;
			case 'o':
				obfuscate = 0;
				break;
			case 'w':
				show_warnings = 1;
				break;
			default:
				print_warning("bad option for metadump command");
				return 0;
		}
	}

	if (optind != argc - 1) {
		print_warning("too few options for metadump (no filename given)");
		return 0;
	}

	metablock = (xfs_metablock_t *)calloc(BBSIZE + 1, BBSIZE);
	if (metablock == NULL) {
		print_warning("memory allocation failure");
		return 0;
	}
	metablock->mb_blocklog = BBSHIFT;
	metablock->mb_magic = cpu_to_be32(XFS_MD_MAGIC);

	/* Set flags about state of metadump */
	metablock->mb_info = XFS_METADUMP_INFO_FLAGS;
	if (obfuscate)
		metablock->mb_info |= XFS_METADUMP_OBFUSCATED;
	if (!zero_stale_data)
		metablock->mb_info |= XFS_METADUMP_FULLBLOCKS;

	/* If we'll copy the log, see if the log is dirty */
	if (mp->m_sb.sb_logstart) {
		push_cur();
		set_cur(&typtab[TYP_LOG],
			XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart),
			mp->m_sb.sb_logblocks * blkbb, DB_RING_IGN, NULL);
		if (iocur_top->data) {	/* best effort */
			struct xlog	log;

			if (xlog_is_dirty(mp, &log, &x, 0))
				metablock->mb_info |= XFS_METADUMP_DIRTYLOG;
		}
		pop_cur();
	}

	block_index = (__be64 *)((char *)metablock + sizeof(xfs_metablock_t));
	block_buffer = (char *)metablock + BBSIZE;
	num_indices = (BBSIZE - sizeof(xfs_metablock_t)) / sizeof(__be64);

	/*
	 * A metadump block can hold at most num_indices of BBSIZE sectors;
	 * do not try to dump a filesystem with a sector size which does not
	 * fit within num_indices (i.e. within a single metablock).
	 */
	if (mp->m_sb.sb_sectsize > num_indices * BBSIZE) {
		print_warning("Cannot dump filesystem with sector size %u",
			      mp->m_sb.sb_sectsize);
		free(metablock);
		return 0;
	}

	cur_index = 0;
	start_iocur_sp = iocur_sp;

	if (strcmp(argv[optind], "-") == 0) {
		if (isatty(fileno(stdout))) {
			print_warning("cannot write to a terminal");
			free(metablock);
			return 0;
		}
		/*
		 * Redirect stdout to stderr for the duration of the
		 * metadump operation so that dbprintf and other messages
		 * are sent to the console instead of polluting the
		 * metadump stream.
		 *
		 * We get to do this the hard way because musl doesn't
		 * allow reassignment of stdout.
		 */
		fflush(stdout);
		outfd = dup(STDOUT_FILENO);
		if (outfd < 0) {
			perror("opening dump stream");
			goto out;
		}
		ret = dup2(STDERR_FILENO, STDOUT_FILENO);
		if (ret < 0) {
			perror("redirecting stdout");
			close(outfd);
			goto out;
		}
		outf = fdopen(outfd, "a");
		if (outf == NULL) {
			fprintf(stderr, "cannot create dump stream\n");
			dup2(outfd, STDOUT_FILENO);
			close(outfd);
			goto out;
		}
		stdout_metadump = true;
	} else {
		outf = fopen(argv[optind], "wb");
		if (outf == NULL) {
			print_warning("cannot create dump file");
			goto out;
		}
	}

	exitcode = 0;

	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++) {
		if (!scan_ag(agno)) {
			exitcode = 1;
			break;
		}
	}

	/* copy realtime and quota inode contents */
	if (!exitcode)
		exitcode = !copy_sb_inodes();

	/* copy log if it's internal */
	if ((mp->m_sb.sb_logstart != 0) && !exitcode)
		exitcode = !copy_log();

	/* write the remaining index */
	if (!exitcode)
		exitcode = write_index() < 0;

	if (progress_since_warning)
		fputc('\n', stdout_metadump ? stderr : stdout);

	if (stdout_metadump) {
		fflush(outf);
		fflush(stdout);
		ret = dup2(outfd, STDOUT_FILENO);
		if (ret < 0)
			perror("un-redirecting stdout");
		stdout_metadump = false;
	}
	fclose(outf);

	/* cleanup iocur stack */
	while (iocur_sp > start_iocur_sp)
		pop_cur();
out:
	free(metablock);

	return 0;
}
