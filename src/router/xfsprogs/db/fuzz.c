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

#include "libxfs.h"
#include <ctype.h>
#include <time.h>
#include "bit.h"
#include "block.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "flist.h"
#include "io.h"
#include "init.h"
#include "output.h"
#include "print.h"
#include "write.h"
#include "malloc.h"

static int	fuzz_f(int argc, char **argv);
static void     fuzz_help(void);

static const cmdinfo_t	fuzz_cmd =
	{ "fuzz", NULL, fuzz_f, 0, -1, 0, N_("[-c] [-d] field fuzzcmd..."),
	  N_("fuzz values on disk"), fuzz_help };

void
fuzz_init(void)
{
	if (!expert_mode)
		return;

	add_command(&fuzz_cmd);
	srand48(clock());
}

static void
fuzz_help(void)
{
	dbprintf(_(
"\n"
" The 'fuzz' command fuzzes fields in any on-disk data structure.  For\n"
" block fuzzing, see the 'blocktrash' or 'write' commands."
"\n"
" Examples:\n"
"  Struct mode: 'fuzz core.uid zeroes'    - set an inode uid field to 0.\n"
"               'fuzz crc ones'           - set a crc filed to all ones.\n"
"               'fuzz bno[11] firstbit'   - set the high bit of a block array.\n"
"               'fuzz keys[5].startblock add'    - increase a btree key value.\n"
"               'fuzz uuid random'        - randomize the superblock uuid.\n"
"\n"
" Type 'fuzz' by itself for a list of specific commands.\n\n"
" Specifying the -c option will allow writes of invalid (corrupt) data with\n"
" an invalid CRC. Specifying the -d option will allow writes of invalid data,\n"
" but still recalculate the CRC so we are forced to check and detect the\n"
" invalid data appropriately.\n\n"
));

}

static int
fuzz_f(
	int		argc,
	char		**argv)
{
	pfunc_t	pf;
	extern char *progname;
	int c;
	bool corrupt = false;	/* Allow write of bad data w/ invalid CRC */
	bool invalid_data = false; /* Allow write of bad data w/ valid CRC */
	struct xfs_buf_ops local_ops;
	const struct xfs_buf_ops *stashed_ops = NULL;

	if (x.isreadonly & LIBXFS_ISREADONLY) {
		dbprintf(_("%s started in read only mode, fuzzing disabled\n"),
			progname);
		return 0;
	}

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}

	pf = cur_typ->pfunc;
	if (pf == NULL) {
		dbprintf(_("no handler function for type %s, fuzz unsupported.\n"),
			 cur_typ->name);
		return 0;
	}

	while ((c = getopt(argc, argv, "cd")) != EOF) {
		switch (c) {
		case 'c':
			corrupt = true;
			break;
		case 'd':
			invalid_data = true;
			break;
		default:
			dbprintf(_("bad option for fuzz command\n"));
			return 0;
		}
	}

	if (corrupt && invalid_data) {
		dbprintf(_("Cannot specify both -c and -d options\n"));
		return 0;
	}

	if (invalid_data &&
	    iocur_top->typ->crc_off == TYP_F_NO_CRC_OFF &&
	    xfs_sb_version_hascrc(&mp->m_sb)) {
		dbprintf(_("Cannot recalculate CRCs on this type of object\n"));
		return 0;
	}

	argc -= optind;
	argv += optind;

	/*
	 * If the buffer has no verifier or we are using standard verifier
	 * paths, then just fuzz it and return
	 */
	if (!iocur_top->bp->b_ops ||
	    !(corrupt || invalid_data)) {
		(*pf)(DB_FUZZ, cur_typ->fields, argc, argv);
		return 0;
	}


	/* Temporarily remove write verifier to write bad data */
	stashed_ops = iocur_top->bp->b_ops;
	local_ops.verify_read = stashed_ops->verify_read;
	iocur_top->bp->b_ops = &local_ops;

	if (!xfs_sb_version_hascrc(&mp->m_sb)) {
		local_ops.verify_write = xfs_dummy_verify;
	} else if (corrupt) {
		local_ops.verify_write = xfs_dummy_verify;
		dbprintf(_("Allowing fuzz of corrupted data and bad CRC\n"));
	} else if (iocur_top->typ->crc_off == TYP_F_CRC_FUNC) {
		local_ops.verify_write = iocur_top->typ->set_crc;
		dbprintf(_("Allowing fuzz of corrupted data with good CRC\n"));
	} else { /* invalid data */
		local_ops.verify_write = xfs_verify_recalc_crc;
		dbprintf(_("Allowing fuzz of corrupted data with good CRC\n"));
	}

	(*pf)(DB_FUZZ, cur_typ->fields, argc, argv);

	iocur_top->bp->b_ops = stashed_ops;

	return 0;
}

/* Write zeroes to the field */
static bool
fuzz_zeroes(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	char		*out = buf;
	int		bit;

	if (bitoff % NBBY || nbits % NBBY) {
		for (bit = 0; bit < nbits; bit++)
			setbit_l(out, bit + bitoff, 0);
	} else
		memset(out + byteize(bitoff), 0, byteize(nbits));
	return true;
}

/* Write ones to the field */
static bool
fuzz_ones(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	char		*out = buf;
	int		bit;

	if (bitoff % NBBY || nbits % NBBY) {
		for (bit = 0; bit < nbits; bit++)
			setbit_l(out, bit + bitoff, 1);
	} else
		memset(out + byteize(bitoff), 0xFF, byteize(nbits));
	return true;
}

/* Flip the high bit in the (presumably big-endian) field */
static bool
fuzz_firstbit(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	setbit_l((char *)buf, bitoff, !getbit_l((char *)buf, bitoff));
	return true;
}

/* Flip the low bit in the (presumably big-endian) field */
static bool
fuzz_lastbit(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	setbit_l((char *)buf, bitoff + nbits - 1,
			!getbit_l((char *)buf, bitoff + nbits - 1));
	return true;
}

/* Flip the middle bit in the (presumably big-endian) field */
static bool
fuzz_middlebit(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	setbit_l((char *)buf, bitoff + nbits / 2,
			!getbit_l((char *)buf, bitoff + nbits / 2));
	return true;
}

/* Format and shift a number into a buffer for setbitval. */
static char *
format_number(
	uint64_t	val,
	__be64		*out,
	int		bit_length)
{
	int		offset;
	char		*rbuf = (char *)out;

	/*
	 * If the length of the field is not a multiple of a byte, push
	 * the bits up in the field, so the most signicant field bit is
	 * the most significant bit in the byte:
	 *
	 * before:
	 * val  |----|----|----|----|----|--MM|mmmm|llll|
	 * after
	 * val  |----|----|----|----|----|MMmm|mmll|ll00|
	 */
	offset = bit_length % NBBY;
	if (offset)
		val <<= (NBBY - offset);

	/*
	 * convert to big endian and copy into the array
	 * rbuf |----|----|----|----|----|MMmm|mmll|ll00|
	 */
	*out = cpu_to_be64(val);

	/*
	 * Align the array to point to the field in the array.
	 *  rbuf  = |MMmm|mmll|ll00|
	 */
	offset = sizeof(__be64) - 1 - ((bit_length - 1) / sizeof(__be64));
	return rbuf + offset;
}

/* Increase the value by some small prime number. */
static bool
fuzz_add(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	uint64_t	val;
	__be64		out;
	char		*b;

	if (nbits > 64)
		return false;

	val = getbitval(buf, bitoff, nbits, BVUNSIGNED);
	val += (nbits > 8 ? 2017 : 137);
	b = format_number(val, &out, nbits);
	setbitval(buf, bitoff, nbits, b);

	return true;
}

/* Decrease the value by some small prime number. */
static bool
fuzz_sub(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	uint64_t	val;
	__be64		out;
	char		*b;

	if (nbits > 64)
		return false;

	val = getbitval(buf, bitoff, nbits, BVUNSIGNED);
	val -= (nbits > 8 ? 2017 : 137);
	b = format_number(val, &out, nbits);
	setbitval(buf, bitoff, nbits, b);

	return true;
}

/* Randomize the field. */
static bool
fuzz_random(
	void		*buf,
	int		bitoff,
	int		nbits)
{
	int		i, bytes;
	char		*b, *rbuf;

	bytes = byteize_up(nbits);
	rbuf = b = malloc(bytes);
	if (!b) {
		perror("fuzz_random");
		return false;
	}

	for (i = 0; i < bytes; i++)
		*b++ = (char)lrand48();

	setbitval(buf, bitoff, nbits, rbuf);
	free(rbuf);

	return true;
}

struct fuzzcmd {
	const char	*verb;
	bool		(*fn)(void *buf, int bitoff, int nbits);
};

/* Keep these verbs in sync with enum fuzzcmds. */
static struct fuzzcmd fuzzverbs[] = {
	{"zeroes",		fuzz_zeroes},
	{"ones",		fuzz_ones},
	{"firstbit",		fuzz_firstbit},
	{"middlebit",		fuzz_middlebit},
	{"lastbit",		fuzz_lastbit},
	{"add",			fuzz_add},
	{"sub",			fuzz_sub},
	{"random",		fuzz_random},
	{NULL,			NULL},
};

/* ARGSUSED */
void
fuzz_struct(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	const ftattr_t	*fa;
	flist_t		*fl;
	flist_t		*sfl;
	int		bit_length;
	struct fuzzcmd	*fc;
	bool		success;
	int		parentoffset;

	if (argc != 2) {
		dbprintf(_("Usage: fuzz fieldname fuzzcmd\n"));
		dbprintf("Fuzz commands: %s", fuzzverbs->verb);
		for (fc = fuzzverbs + 1; fc->verb != NULL; fc++)
			dbprintf(", %s", fc->verb);
		dbprintf(".\n");
		return;
	}

	fl = flist_scan(argv[0]);
	if (!fl) {
		dbprintf(_("unable to parse '%s'.\n"), argv[0]);
		return;
	}

	/* Find our fuzz verb */
	for (fc = fuzzverbs; fc->verb != NULL; fc++)
		if (!strcmp(fc->verb, argv[1]))
			break;
	if (fc->fn == NULL) {
		dbprintf(_("Unknown fuzz command '%s'.\n"), argv[1]);
		goto out_free;
	}

	/* if we're a root field type, go down 1 layer to get field list */
	if (fields->name[0] == '\0') {
		fa = &ftattrtab[fields->ftyp];
		ASSERT(fa->ftyp == fields->ftyp);
		fields = fa->subfld;
	}

	/* run down the field list and set offsets into the data */
	if (!flist_parse(fields, fl, iocur_top->data, 0)) {
		dbprintf(_("parsing error\n"));
		goto out_free;
	}

	sfl = fl;
	parentoffset = 0;
	while (sfl->child) {
		parentoffset = sfl->offset;
		sfl = sfl->child;
	}

	/*
	 * For structures, fsize * fcount tells us the size of the region we are
	 * modifying, which is usually a single structure member and is pointed
	 * to by the last child in the list.
	 *
	 * However, if the base structure is an array and we have a direct index
	 * into the array (e.g. write bno[5]) then we are returned a single
	 * flist object with the offset pointing directly at the location we
	 * need to modify. The length of the object we are modifying is then
	 * determined by the size of the individual array entry (fsize) and the
	 * indexes defined in the object, not the overall size of the array
	 * (which is what fcount returns).
	 */
	bit_length = fsize(sfl->fld, iocur_top->data, parentoffset, 0);
	if (sfl->fld->flags & FLD_ARRAY)
		bit_length *= sfl->high - sfl->low + 1;
	else
		bit_length *= fcount(sfl->fld, iocur_top->data, parentoffset);

	/* Fuzz the value */
	success = fc->fn(iocur_top->data, sfl->offset, bit_length);
	if (!success) {
		dbprintf(_("unable to fuzz field '%s'\n"), argv[0]);
		goto out_free;
	}

	/* Write the fuzzed value back */
	write_cur();

	flist_print(fl);
	print_flist(fl);
out_free:
	flist_free(fl);
}
