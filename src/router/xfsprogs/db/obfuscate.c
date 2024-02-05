// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2007, 2011 SGI
 * All Rights Reserved.
 */
#include "libxfs.h"
#include "init.h"
#include "obfuscate.h"

static inline unsigned char
random_filename_char(void)
{
	static unsigned char filename_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						"abcdefghijklmnopqrstuvwxyz"
						"0123456789-_";

	return filename_alphabet[random() % (sizeof filename_alphabet - 1)];
}

#define rol32(x,y)		(((x) << (y)) | ((x) >> (32 - (y))))

/*
 * Given a name and its hash value, massage the name in such a way
 * that the result is another name of equal length which shares the
 * same hash value.
 */
void
obfuscate_name(
	xfs_dahash_t	hash,
	size_t		name_len,
	unsigned char	*name,
	bool		is_dirent)
{
	unsigned char	*oldname = NULL;
	unsigned char	*newp;
	int		i;
	xfs_dahash_t	new_hash;
	unsigned char	*first;
	unsigned char	high_bit;
	int		tries = 0;
	bool		is_ci_name = is_dirent && xfs_has_asciici(mp);
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

	if (is_ci_name) {
		oldname = malloc(name_len);
		if (!oldname)
			return;
		memcpy(oldname, name, name_len);
	}

again:
	newp = name;
	new_hash = 0;

	/*
	 * If we cannot generate a ci-compatible obfuscated name after 1000
	 * tries, don't bother obfuscating the name.
	 */
	if (tries++ > 1000) {
		memcpy(name, oldname, name_len);
		goto out_free;
	}

	/*
	 * The beginning of the obfuscated name can be pretty much
	 * anything, so fill it in with random characters.
	 * Accumulate its new hash value as we go.
	 */
	for (i = 0; i < name_len - 5; i++) {
		*newp = random_filename_char();
		if (is_ci_name)
			new_hash = xfs_ascii_ci_xfrm(*newp) ^
							rol32(new_hash, 7);
		else
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

		/*
		 * If ascii-ci is enabled, uppercase characters are converted
		 * to lowercase characters while computing the name hash.  If
		 * any of the necessary correction bytes are uppercase, the
		 * hash of the new name will not match.  Try again with a
		 * different prefix.
		 */
		if (is_ci_name && xfs_ascii_ci_need_xfrm(*newp))
			goto again;

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

		if (is_ci_name && xfs_ascii_ci_need_xfrm(*first))
			goto again;

		ASSERT(!is_invalid_char(*first));
	}
out_free:
	free(oldname);
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
int
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
