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

#include <libxfs.h>
#include "bmap.h"
#include "command.h"
#include "metadump.h"
#include "io.h"
#include "output.h"
#include "type.h"
#include "init.h"
#include "sig.h"
#include "xfs_metadump.h"

#define DEFAULT_MAX_EXT_SIZE	1000

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
		N_("[-e] [-g] [-m max_extent] [-w] [-o] filename"),
		N_("dump metadata to a file"), metadump_help };

static FILE		*outf;		/* metadump file */

static xfs_metablock_t 	*metablock;	/* header + index + buffers */
static __be64		*block_index;
static char		*block_buffer;

static int		num_indicies;
static int		cur_index;

static xfs_ino_t	cur_ino;

static int		show_progress = 0;
static int		stop_on_read_error = 0;
static int		max_extent_size = DEFAULT_MAX_EXT_SIZE;
static int		dont_obfuscate = 0;
static int		show_warnings = 0;
static int		progress_since_warning = 0;

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

	f = (outf == stdout) ? stderr : stdout;
	fprintf(f, "\r%-59s", buf);
	fflush(f);
	progress_since_warning = 1;
}

/*
 * A complete dump file will have a "zero" entry in the last index block,
 * even if the dump is exactly aligned, the last index will be full of
 * zeros. If the last index entry is non-zero, the dump is incomplete.
 * Correspondingly, the last chunk will have a count < num_indicies.
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
		return 0;
	}

	memset(block_index, 0, num_indicies * sizeof(__be64));
	cur_index = 0;
	return 1;
}

static int
write_buf(
	iocur_t		*buf)
{
	char		*data;
	__int64_t	off;
	int		i;

	for (i = 0, off = buf->bb, data = buf->data;
			i < buf->blen;
			i++, off++, data += BBSIZE) {
		block_index[cur_index] = cpu_to_be64(off);
		memcpy(&block_buffer[cur_index << BBSHIFT], data, BBSIZE);
		if (++cur_index == num_indicies) {
			if (!write_index())
				return 0;
		}
	}
	return !seenint();
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
	if (!write_buf(iocur_top))
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
			 (xfs_drfsbno_t)(mp->m_sb.sb_agcount - 1) *
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

/* filename and extended attribute obfuscation routines */

struct name_ent {
	struct name_ent		*next;
	xfs_dahash_t		hash;
	int			namelen;
	uchar_t			name[1];
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
nametable_find(xfs_dahash_t hash, int namelen, uchar_t *name)
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
nametable_add(xfs_dahash_t hash, int namelen, uchar_t *name)
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

static inline uchar_t
random_filename_char(void)
{
	static uchar_t filename_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
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
	uchar_t			*name)
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
	uchar_t			*name)
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
	uchar_t		*name)
{
	uchar_t		*newp = name;
	int		i;
	xfs_dahash_t	new_hash = 0;
	uchar_t		*first;
	uchar_t		high_bit;
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
	uchar_t		*name,
	uint32_t	bitseq)
{
	int	index;
	size_t	offset;
	uchar_t	*p0, *p1;
	uchar_t	m0, m1;
	struct {
	    int		byte;	/* Offset from start within name */
	    uchar_t	bit;	/* Bit within that byte */
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
	uchar_t		*name,
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
handle_duplicate_name(xfs_dahash_t hash, size_t name_len, uchar_t *name)
{
	uchar_t		new_name[name_len + 1];
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
	uchar_t			*name)
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
obfuscate_sf_dir(
	xfs_dinode_t		*dip)
{
	xfs_dir2_sf_t		*sfp;
	xfs_dir2_sf_entry_t	*sfep;
	__uint64_t		ino_dir_size;
	int			i;

	sfp = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(dip);
	ino_dir_size = be64_to_cpu(dip->di_size);
	if (ino_dir_size > XFS_DFORK_DSIZE(dip, mp)) {
		ino_dir_size = XFS_DFORK_DSIZE(dip, mp);
		if (show_warnings)
			print_warning("invalid size in dir inode %llu",
					(long long)cur_ino);
	}

	sfep = xfs_dir2_sf_firstentry(sfp);
	for (i = 0; (i < sfp->hdr.count) &&
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
			if (i != sfp->hdr.count - 1)
				break;
			namelen = ino_dir_size - ((char *)&sfep->name[0] -
					 (char *)sfp);
		} else if ((char *)sfep - (char *)sfp +
				xfs_dir2_sf_entsize_byentry(sfp, sfep) >
				ino_dir_size) {
			if (show_warnings)
				print_warning("entry length in dir inode %llu "
					"overflows space", (long long)cur_ino);
			if (i != sfp->hdr.count - 1)
				break;
			namelen = ino_dir_size - ((char *)&sfep->name[0] -
					 (char *)sfp);
		}

		generate_obfuscated_name(xfs_dir2_sf_get_inumber(sfp,
				xfs_dir2_sf_inumberp(sfep)), namelen,
				&sfep->name[0]);

		sfep = (xfs_dir2_sf_entry_t *)((char *)sfep +
				xfs_dir2_sf_entsize_byname(sfp, namelen));
	}
}

static void
obfuscate_sf_symlink(
	xfs_dinode_t		*dip)
{
	__uint64_t		len;
	char			*buf;

	len = be64_to_cpu(dip->di_size);
	if (len > XFS_DFORK_DSIZE(dip, mp)) {
		if (show_warnings)
			print_warning("invalid size (%d) in symlink inode %llu",
					len, (long long)cur_ino);
		len = XFS_DFORK_DSIZE(dip, mp);
	}

	buf = (char *)XFS_DFORK_DPTR(dip);
	while (len > 0)
		buf[--len] = random() % 127 + 1;
}

static void
obfuscate_sf_attr(
	xfs_dinode_t		*dip)
{
	/*
	 * with extended attributes, obfuscate the names and zero the actual
	 * values.
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

		generate_obfuscated_name(0, asfep->namelen, &asfep->nameval[0]);
		memset(&asfep->nameval[asfep->namelen], 0, asfep->valuelen);

		asfep = (xfs_attr_sf_entry_t *)((char *)asfep +
				XFS_ATTR_SF_ENTSIZE(asfep));
	}
}

/*
 * dir_data structure is used to track multi-fsblock dir2 blocks between extent
 * processing calls.
 */

static struct dir_data_s {
	int			end_of_data;
	int			block_index;
	int			offset_to_entry;
	int			bad_block;
} dir_data;

static void
obfuscate_dir_data_blocks(
	char			*block,
	xfs_dfiloff_t		offset,
	xfs_dfilblks_t		count,
	int			is_block_format)
{
	/*
	 * we have to rely on the fileoffset and signature of the block to
	 * handle it's contents. If it's invalid, leave it alone.
	 * for multi-fsblock dir blocks, if a name crosses an extent boundary,
	 * ignore it and continue.
	 */
	int			c;
	int			dir_offset;
	char			*ptr;
	char			*endptr;

	if (is_block_format && count != mp->m_dirblkfsbs)
		return; /* too complex to handle this rare case */

	for (c = 0, endptr = block; c < count; c++) {

		if (dir_data.block_index == 0) {
			int		wantmagic;

			if (offset % mp->m_dirblkfsbs != 0)
				return;	/* corrupted, leave it alone */

			dir_data.bad_block = 0;

			if (is_block_format) {
				xfs_dir2_leaf_entry_t	*blp;
				xfs_dir2_block_tail_t	*btp;

				btp = xfs_dir2_block_tail_p(mp,
						(xfs_dir2_block_t *)block);
				blp = xfs_dir2_block_leaf_p(btp);
				if ((char *)blp > (char *)btp)
					blp = (xfs_dir2_leaf_entry_t *)btp;

				dir_data.end_of_data = (char *)blp - block;
				wantmagic = XFS_DIR2_BLOCK_MAGIC;
			} else { /* leaf/node format */
				dir_data.end_of_data = mp->m_dirblkfsbs <<
						mp->m_sb.sb_blocklog;
				wantmagic = XFS_DIR2_DATA_MAGIC;
			}
			dir_data.offset_to_entry = offsetof(xfs_dir2_data_t, u);

			if (be32_to_cpu(((xfs_dir2_data_hdr_t*)block)->magic) !=
					wantmagic) {
				if (show_warnings)
					print_warning("invalid magic in dir "
						"inode %llu block %ld",
						(long long)cur_ino,
						(long)offset);
				dir_data.bad_block = 1;
			}
		}
		dir_data.block_index++;
		if (dir_data.block_index == mp->m_dirblkfsbs)
			dir_data.block_index = 0;

		if (dir_data.bad_block)
			continue;

		dir_offset = (dir_data.block_index << mp->m_sb.sb_blocklog) +
				dir_data.offset_to_entry;

		ptr = endptr + dir_data.offset_to_entry;
		endptr += mp->m_sb.sb_blocksize;

		while (ptr < endptr && dir_offset < dir_data.end_of_data) {
			xfs_dir2_data_entry_t	*dep;
			xfs_dir2_data_unused_t	*dup;
			int			length;

			dup = (xfs_dir2_data_unused_t *)ptr;

			if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
				int	length = be16_to_cpu(dup->length);
				if (dir_offset + length > dir_data.end_of_data ||
						length == 0 || (length &
						 (XFS_DIR2_DATA_ALIGN - 1))) {
					if (show_warnings)
						print_warning("invalid length "
							"for dir free space in "
							"inode %llu",
							(long long)cur_ino);
					dir_data.bad_block = 1;
					break;
				}
				if (be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)) !=
						dir_offset) {
					dir_data.bad_block = 1;
					break;
				}
				dir_offset += length;
				ptr += length;
				if (dir_offset >= dir_data.end_of_data ||
						ptr >= endptr)
					break;
			}

			dep = (xfs_dir2_data_entry_t *)ptr;
			length = xfs_dir2_data_entsize(dep->namelen);

			if (dir_offset + length > dir_data.end_of_data ||
					ptr + length > endptr) {
				if (show_warnings)
					print_warning("invalid length for "
						"dir entry name in inode %llu",
						(long long)cur_ino);
				break;
			}
			if (be16_to_cpu(*xfs_dir2_data_entry_tag_p(dep)) !=
					dir_offset) {
				dir_data.bad_block = 1;
				break;
			}
			generate_obfuscated_name(be64_to_cpu(dep->inumber),
					dep->namelen, &dep->name[0]);
			dir_offset += length;
			ptr += length;
		}
		dir_data.offset_to_entry = dir_offset &
						(mp->m_sb.sb_blocksize - 1);
	}
}

static void
obfuscate_symlink_blocks(
	char			*block,
	xfs_dfilblks_t		count)
{
	int 			i;

	count <<= mp->m_sb.sb_blocklog;
	for (i = 0; i < count; i++)
		block[i] = random() % 127 + 1;
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
		length -= XFS_LBSIZE(mp);
	}
}

static void
obfuscate_attr_blocks(
	char			*block,
	xfs_dfiloff_t		offset,
	xfs_dfilblks_t		count)
{
	xfs_attr_leafblock_t	*leaf;
	int			c;
	int			i;
	int			nentries;
	xfs_attr_leaf_entry_t 	*entry;
	xfs_attr_leaf_name_local_t *local;
	xfs_attr_leaf_name_remote_t *remote;

	for (c = 0; c < count; c++, offset++, block += XFS_LBSIZE(mp)) {

		leaf = (xfs_attr_leafblock_t *)block;

		if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC) {
			for (i = 0; i < attr_data.remote_val_count; i++) {
				if (attr_data.remote_vals[i] == offset)
					memset(block, 0, XFS_LBSIZE(mp));
			}
			continue;
		}

		nentries = be16_to_cpu(leaf->hdr.count);
		if (nentries * sizeof(xfs_attr_leaf_entry_t) +
				sizeof(xfs_attr_leaf_hdr_t) > XFS_LBSIZE(mp)) {
			if (show_warnings)
				print_warning("invalid attr count in inode %llu",
						(long long)cur_ino);
			continue;
		}

		for (i = 0, entry = &leaf->entries[0]; i < nentries;
				i++, entry++) {
			if (be16_to_cpu(entry->nameidx) > XFS_LBSIZE(mp)) {
				if (show_warnings)
					print_warning("invalid attr nameidx "
							"in inode %llu",
							(long long)cur_ino);
				break;
			}
			if (entry->flags & XFS_ATTR_LOCAL) {
				local = xfs_attr_leaf_name_local(leaf, i);
				if (local->namelen == 0) {
					if (show_warnings)
						print_warning("zero length for "
							"attr name in inode %llu",
							(long long)cur_ino);
					break;
				}
				generate_obfuscated_name(0, local->namelen,
					&local->nameval[0]);
				memset(&local->nameval[local->namelen], 0,
					be16_to_cpu(local->valuelen));
			} else {
				remote = xfs_attr_leaf_name_remote(leaf, i);
				if (remote->namelen == 0 ||
						remote->valueblk == 0) {
					if (show_warnings)
						print_warning("invalid attr "
							"entry in inode %llu",
							(long long)cur_ino);
					break;
				}
				generate_obfuscated_name(0, remote->namelen,
					&remote->name[0]);
				add_remote_vals(be32_to_cpu(remote->valueblk),
					be32_to_cpu(remote->valuelen));
			}
		}
	}
}

/* inode copy routines */

static int
process_bmbt_reclist(
	xfs_bmbt_rec_t 		*rp,
	int 			numrecs,
	typnm_t			btype)
{
	int			i;
	xfs_dfiloff_t		o, op = NULLDFILOFF;
	xfs_dfsbno_t		s;
	xfs_dfilblks_t		c, cp = NULLDFILOFF;
	int			f;
	xfs_dfiloff_t		last;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;

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

		push_cur();
		set_cur(&typtab[btype], XFS_FSB_TO_DADDR(mp, s), c * blkbb,
				DB_RING_IGN, NULL);
		if (iocur_top->data == NULL) {
			print_warning("cannot read %s block %u/%u (%llu)",
					typtab[btype].name, agno, agbno, s);
			if (stop_on_read_error) {
				pop_cur();
				return 0;
			}
		} else {
			if (!dont_obfuscate)
			    switch (btype) {
				case TYP_DIR2:
					if (o < mp->m_dirleafblk)
						obfuscate_dir_data_blocks(
							iocur_top->data, o, c,
							last == mp->m_dirblkfsbs);
					break;

				case TYP_SYMLINK:
					obfuscate_symlink_blocks(
						iocur_top->data, c);
					break;

				case TYP_ATTR:
					obfuscate_attr_blocks(iocur_top->data,
						o, c);
					break;

				default: ;
			    }
			if (!write_buf(iocur_top)) {
				pop_cur();
				return 0;
			}
		}
		pop_cur();
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

		ag = XFS_FSB_TO_AGNO(mp, be64_to_cpu(pp[i]));
		bno = XFS_FSB_TO_AGBNO(mp, be64_to_cpu(pp[i]));

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

	maxrecs = xfs_bmdr_maxrecs(mp, XFS_DFORK_SIZE(dip, mp, whichfork), 0);
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

		ag = XFS_FSB_TO_AGNO(mp, be64_to_cpu(pp[i]));
		bno = XFS_FSB_TO_AGBNO(mp, be64_to_cpu(pp[i]));

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
	xfs_extnum_t		nex;

	whichfork = (itype == TYP_ATTR) ? XFS_ATTR_FORK : XFS_DATA_FORK;

	nex = XFS_DFORK_NEXTENTS(dip, whichfork);
	if (nex < 0 || nex > XFS_DFORK_SIZE(dip, mp, whichfork) /
						sizeof(xfs_bmbt_rec_t)) {
		if (show_warnings)
			print_warning("bad number of extents %d in inode %lld",
				nex, (long long)cur_ino);
		return 1;
	}

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
			if (!dont_obfuscate)
				switch (itype) {
					case TYP_DIR2:
						obfuscate_sf_dir(dip);
						break;

					case TYP_SYMLINK:
						obfuscate_sf_symlink(dip);
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

static int
process_inode(
	xfs_agnumber_t		agno,
	xfs_agino_t 		agino,
	xfs_dinode_t 		*dip)
{
	int			success;

	success = 1;
	cur_ino = XFS_AGINO_TO_INO(mp, agno, agino);

	/* copy appropriate data fork metadata */
	switch (be16_to_cpu(dip->di_mode) & S_IFMT) {
		case S_IFDIR:
			memset(&dir_data, 0, sizeof(dir_data));
			success = process_inode_data(dip, TYP_DIR2);
			break;
		case S_IFLNK:
			success = process_inode_data(dip, TYP_SYMLINK);
			break;
		case S_IFREG:
			success = process_inode_data(dip, TYP_DATA);
			break;
		default: ;
	}
	nametable_clear();

	/* copy extended attributes if they exist and forkoff is valid */
	if (success && XFS_DFORK_DSIZE(dip, mp) < XFS_LITINO(mp)) {
		attr_data.remote_val_count = 0;
		switch (dip->di_aformat) {
			case XFS_DINODE_FMT_LOCAL:
				if (!dont_obfuscate)
					obfuscate_sf_attr(dip);
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
	return success;
}

static __uint32_t	inodes_copied = 0;

static int
copy_inode_chunk(
	xfs_agnumber_t 		agno,
	xfs_inobt_rec_t 	*rp)
{
	xfs_agino_t 		agino;
	int			off;
	xfs_agblock_t		agbno;
	int			i;
	int			rval = 0;

	agino = be32_to_cpu(rp->ir_startino);
	agbno = XFS_AGINO_TO_AGBNO(mp, agino);
	off = XFS_INO_TO_OFFSET(mp, agino);

	if (agino == 0 || agino == NULLAGINO || !valid_bno(agno, agbno) ||
			!valid_bno(agno, XFS_AGINO_TO_AGBNO(mp,
					agino + XFS_INODES_PER_CHUNK - 1))) {
		if (show_warnings)
			print_warning("bad inode number %llu (%u/%u)",
				XFS_AGINO_TO_INO(mp, agno, agino), agno, agino);
		return 1;
	}

	push_cur();
	set_cur(&typtab[TYP_INODE], XFS_AGB_TO_DADDR(mp, agno, agbno),
			XFS_FSB_TO_BB(mp, XFS_IALLOC_BLOCKS(mp)),
			DB_RING_IGN, NULL);
	if (iocur_top->data == NULL) {
		print_warning("cannot read inode block %u/%u", agno, agbno);
		rval = !stop_on_read_error;
		goto pop_out;
	}

	/*
	 * check for basic assumptions about inode chunks, and if any
	 * assumptions fail, don't process the inode chunk.
	 */

	if ((mp->m_sb.sb_inopblock <= XFS_INODES_PER_CHUNK && off != 0) ||
			(mp->m_sb.sb_inopblock > XFS_INODES_PER_CHUNK &&
					off % XFS_INODES_PER_CHUNK != 0) ||
			(xfs_sb_version_hasalign(&mp->m_sb) &&
					agbno % mp->m_sb.sb_inoalignmt != 0)) {
		if (show_warnings)
			print_warning("badly aligned inode (start = %llu)",
					XFS_AGINO_TO_INO(mp, agno, agino));
		goto skip_processing;
	}

	/*
	 * scan through inodes and copy any btree extent lists, directory
	 * contents and extended attributes.
	 */
	for (i = 0; i < XFS_INODES_PER_CHUNK; i++) {
		xfs_dinode_t            *dip;

		if (XFS_INOBT_IS_FREE_DISK(rp, i))
			continue;

		dip = (xfs_dinode_t *)((char *)iocur_top->data +
				((off + i) << mp->m_sb.sb_inodelog));

		if (!process_inode(agno, agino + i, dip))
			goto pop_out;
	}
skip_processing:
	if (!write_buf(iocur_top))
		goto pop_out;

	inodes_copied += XFS_INODES_PER_CHUNK;

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

	numrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (numrecs > mp->m_inobt_mxr[0]) {
			if (show_warnings)
				print_warning("invalid numrecs %d in %s "
					"block %u/%u", numrecs,
					typtab[btype].name, agno, agbno);
			numrecs = mp->m_inobt_mxr[0];
		}
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

	return scan_btree(agno, root, levels, TYP_INOBT, agi, scanfunc_ino);
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
		if (!write_buf(iocur_top))
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
		if (!write_buf(iocur_top))
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
		if (!write_buf(iocur_top))
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
		if (!write_buf(iocur_top))
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

	if (ino == 0)
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

	return copy_ino(mp->m_sb.sb_gquotino, TYP_DQBLK);
}

static int
copy_log(void)
{
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
	return write_buf(iocur_top);
}

static int
metadump_f(
	int 		argc,
	char 		**argv)
{
	xfs_agnumber_t	agno;
	int		c;
	int		start_iocur_sp;
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

	while ((c = getopt(argc, argv, "egm:ow")) != EOF) {
		switch (c) {
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
				dont_obfuscate = 1;
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

	block_index = (__be64 *)((char *)metablock + sizeof(xfs_metablock_t));
	block_buffer = (char *)metablock + BBSIZE;
	num_indicies = (BBSIZE - sizeof(xfs_metablock_t)) / sizeof(__be64);
	cur_index = 0;
	start_iocur_sp = iocur_sp;

	if (strcmp(argv[optind], "-") == 0) {
		if (isatty(fileno(stdout))) {
			print_warning("cannot write to a terminal");
			free(metablock);
			return 0;
		}
		outf = stdout;
	} else {
		outf = fopen(argv[optind], "wb");
		if (outf == NULL) {
			print_warning("cannot create dump file");
			free(metablock);
			return 0;
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
		exitcode = !write_index();

	if (progress_since_warning)
		fputc('\n', (outf == stdout) ? stderr : stdout);

	if (outf != stdout)
		fclose(outf);

	/* cleanup iocur stack */
	while (iocur_sp > start_iocur_sp)
		pop_cur();

	free(metablock);

	return 0;
}
