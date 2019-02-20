/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"

#include <ctype.h>

/* FIXME: calculate this. */
#define INT_SHIFT 5

dm_bitset_t dm_bitset_create(struct dm_pool *mem, unsigned num_bits)
{
	unsigned n = (num_bits / DM_BITS_PER_INT) + 2;
	size_t size = sizeof(int) * n;
	dm_bitset_t bs;
	
	if (mem)
		bs = dm_pool_zalloc(mem, size);
	else
		bs = dm_zalloc(size);

	if (!bs)
		return NULL;

	*bs = num_bits;

	return bs;
}

void dm_bitset_destroy(dm_bitset_t bs)
{
	dm_free(bs);
}

int dm_bitset_equal(dm_bitset_t in1, dm_bitset_t in2)
{
	int i;

	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		if (in1[i] != in2[i])
			return 0;

	return 1;
}

void dm_bit_and(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2)
{
	int i;

	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		out[i] = in1[i] & in2[i];
}
void dm_bit_union(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2)
{
	int i;
	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		out[i] = in1[i] | in2[i];
}

static int _test_word(uint32_t test, int bit)
{
	uint32_t tb = test >> bit;

	return (tb ? ffs(tb) + bit - 1 : -1);
}

static int _test_word_rev(uint32_t test, int bit)
{
	uint32_t tb = test << (DM_BITS_PER_INT - 1 - bit);

	return (tb ? bit - clz(tb) : -1);
}

int dm_bit_get_next(dm_bitset_t bs, int last_bit)
{
	int bit, word;
	uint32_t test;

	last_bit++;		/* otherwise we'll return the same bit again */

	/*
	 * bs[0] holds number of bits
	 */
	while (last_bit < (int) bs[0]) {
		word = last_bit >> INT_SHIFT;
		test = bs[word + 1];
		bit = last_bit & (DM_BITS_PER_INT - 1);

		if ((bit = _test_word(test, bit)) >= 0)
			return (word * DM_BITS_PER_INT) + bit;

		last_bit = last_bit - (last_bit & (DM_BITS_PER_INT - 1)) +
		    DM_BITS_PER_INT;
	}

	return -1;
}

int dm_bit_get_prev(dm_bitset_t bs, int last_bit)
{
	int bit, word;
	uint32_t test;

	last_bit--;		/* otherwise we'll return the same bit again */

	/*
	 * bs[0] holds number of bits
	 */
	while (last_bit >= 0) {
		word = last_bit >> INT_SHIFT;
		test = bs[word + 1];
		bit = last_bit & (DM_BITS_PER_INT - 1);

		if ((bit = _test_word_rev(test, bit)) >= 0)
			return (word * DM_BITS_PER_INT) + bit;

		last_bit = (last_bit & ~(DM_BITS_PER_INT - 1)) - 1;
	}

	return -1;
}

int dm_bit_get_first(dm_bitset_t bs)
{
	return dm_bit_get_next(bs, -1);
}

int dm_bit_get_last(dm_bitset_t bs)
{
	return dm_bit_get_prev(bs, bs[0] + 1);
}

/*
 * Based on the Linux kernel __bitmap_parselist from lib/bitmap.c
 */
dm_bitset_t dm_bitset_parse_list(const char *str, struct dm_pool *mem,
				 size_t min_num_bits)
{
	unsigned a, b;
	int c, old_c, totaldigits, ndigits, nmaskbits;
	int at_start, in_range;
	dm_bitset_t mask = NULL;
	const char *start = str;
	size_t len;

scan:
	len = strlen(str);
	totaldigits = c = 0;
	nmaskbits = 0;
	do {
		at_start = 1;
		in_range = 0;
		a = b = 0;
		ndigits = totaldigits;

		/* Get the next value or range of values */
		while (len) {
			old_c = c;
			c = *str++;
			len--;
			if (isspace(c))
				continue;

			/* A '\0' or a ',' signal the end of a value or range */
			if (c == '\0' || c == ',')
				break;
			/*
			* whitespaces between digits are not allowed,
			* but it's ok if whitespaces are on head or tail.
			* when old_c is whilespace,
			* if totaldigits == ndigits, whitespace is on head.
			* if whitespace is on tail, it should not run here.
			* as c was ',' or '\0',
			* the last code line has broken the current loop.
			*/
			if ((totaldigits != ndigits) && isspace(old_c))
				goto_bad;

			if (c == '-') {
				if (at_start || in_range)
					goto_bad;
				b = 0;
				in_range = 1;
				at_start = 1;
				continue;
			}

			if (!isdigit(c))
				goto_bad;

			b = b * 10 + (c - '0');
			if (!in_range)
				a = b;
			at_start = 0;
			totaldigits++;
		}
		if (ndigits == totaldigits)
			continue;
		/* if no digit is after '-', it's wrong */
		if (at_start && in_range)
			goto_bad;
		if (!(a <= b))
			goto_bad;
		if (b >= nmaskbits)
			nmaskbits = b + 1;
		while ((a <= b) && mask) {
			dm_bit_set(mask, a);
			a++;
		}
	} while (len && c == ',');

	if (!mask) {
		if (min_num_bits && (nmaskbits < min_num_bits))
			nmaskbits = min_num_bits;

		if (!(mask = dm_bitset_create(mem, nmaskbits)))
			goto_bad;
		str = start;
		goto scan;
	}

	return mask;
bad:
	if (mask) {
		if (mem)
			dm_pool_free(mem, mask);
		else
			dm_bitset_destroy(mask);
	}
	return NULL;
}

#if defined(__GNUC__)
/*
 * Maintain backward compatibility with older versions that did not
 * accept a 'min_num_bits' argument to dm_bitset_parse_list().
 */
dm_bitset_t dm_bitset_parse_list_v1_02_129(const char *str, struct dm_pool *mem);
dm_bitset_t dm_bitset_parse_list_v1_02_129(const char *str, struct dm_pool *mem)
{
	return dm_bitset_parse_list(str, mem, 0);
}
DM_EXPORT_SYMBOL(dm_bitset_parse_list, 1_02_129);

#else /* if defined(__GNUC__) */

#endif
