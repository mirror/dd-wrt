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

#include <xfs/libxfs.h>
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

static int	write_f(int argc, char **argv);
static void     write_help(void);

static const cmdinfo_t	write_cmd =
	{ "write", NULL, write_f, 0, -1, 0, N_("[field or value]..."),
	  N_("write value to disk"), write_help };

void
write_init(void)
{
	if (!expert_mode)
		return;

	add_command(&write_cmd);
	srand48(clock());
}

static void
write_help(void)
{
	dbprintf(_(
"\n"
" The 'write' command takes on different personalities depending on the\n"
" type of object being worked with.\n\n"
" Write has 3 modes:\n"
"  'struct mode' - is active anytime you're looking at a filesystem object\n"
"                  which contains individual fields (ex: an inode).\n"
"  'data mode'   - is active anytime you set a disk address directly or set\n"
"                  the type to 'data'.\n"
"  'string mode' - only used for writing symlink blocks.\n"
"\n"
" Examples:\n"
"  Struct mode: 'write core.uid 23'          - set an inode uid field to 23.\n"
"               'write fname \"hello\\000\"'    - write superblock fname.\n"
"               (note: in struct mode strings are not null terminated)\n"
"               'write fname #6669736800'    - write superblock fname with hex.\n"
"               'write uuid 00112233-4455-6677-8899-aabbccddeeff'\n"
"                                            - write superblock uuid.\n"
"  Data mode:   'write fill 0xff' - fill the entire block with 0xff's\n"
"               'write lshift 3' - shift the block 3 bytes to the left\n"
"               'write sequence 1 5' - write a cycle of number [1-5] through\n"
"                                      the entire block.\n"
"  String mode: 'write \"This_is_a_filename\" - write null terminated string.\n"
"\n"
" In data mode type 'write' by itself for a list of specific commands.\n\n"
));

}

static int
write_f(
	int		argc,
	char		**argv)
{
	pfunc_t	pf;
	extern char *progname;

	if (x.isreadonly & LIBXFS_ISREADONLY) {
		dbprintf(_("%s started in read only mode, writing disabled\n"),
			progname);
		return 0;
	}

	if (cur_typ == NULL) {
		dbprintf(_("no current type\n"));
		return 0;
	}

	pf = cur_typ->pfunc;
	if (pf == NULL) {
		dbprintf(_("no handler function for type %s, write unsupported.\n"),
			 cur_typ->name);
		return 0;
	}

	/* move past the "write" command */
	argc--;
	argv++;

	(*pf)(DB_WRITE, cur_typ->fields, argc, argv);

	return 0;
}

/* compare significant portions of commands */

static int
sigcmp(
	char  *s1,
	char  *s2,
	int   sig)
{
	int sigcnt;

	if (!s1 || !s2)
		return 0;

	for (sigcnt = 0; *s1 == *s2; s1++, s2++) {
		sigcnt++;
		if (*s1 == '\0')
			return 1;
	}
	if (*s1 && *s2)
		return 0;

	if (sig && (sigcnt >= sig))
		return 1;

	return 0;
}

/* ARGSUSED */
static void
bwrite_lshift(
	int   start,
	int   len,
	int   shift,
	int   from,
	int   to)
{
	char *base;

	if (shift == -1)
		shift = 1;
	if (start == -1)
		start = 0;
	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	base = (char *)iocur_top->data + start;

	memcpy(base, base+shift, len-shift);
	memset(base+(len-shift), 0, shift);
}

/* ARGSUSED */
static void
bwrite_rshift(
	int   start,
	int   len,
	int   shift,
	int   from,
	int   to)
{
	char *base;

	if (shift == -1)
		shift = 1;
	if (start == -1)
		start = 0;
	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	base = (char *)iocur_top->data + start;

	memcpy(base+shift, base, len-shift);
	memset(base, 0, shift);
}

/* ARGSUSED */
static void
bwrite_lrot(
	int   start,
	int   len,
	int   shift,
	int   from,
	int   to)
{
	char *base;
	char *hold_region;

	if (shift == -1)
		shift = 1;
	if (start == -1)
		start = 0;
	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	base = (char *)iocur_top->data + start;

	hold_region = xmalloc(shift);
	memcpy(hold_region, base, shift);
	memcpy(base, base+shift, len-shift);
	memcpy(base+(len-shift), hold_region, shift);
}

/* ARGSUSED */
static void
bwrite_rrot(
	int   start,
	int   len,
	int   shift,
	int   from,
	int   to)
{
	char *base;
	char *hold_region;

	if (shift == -1)
		shift = 1;
	if (start == -1)
		start = 0;
	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	base = (char *)iocur_top->data + start;

	hold_region = xmalloc(shift);
	memcpy(hold_region, base+(len-shift), shift);
	memmove(base+shift, base, len-shift);
	memcpy(base, hold_region, shift);
}

/* ARGSUSED */
static void
bwrite_seq(
	int   start,
	int   len,
	int   step,
	int   from,
	int   to)
{
	int i;
	int tmp;
	int base;
	int range;
	int top;
	char *buf;

	if (start == -1)
		start = 0;

	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	if (from == -1 || from > 255)
		from = 0;
	if (to == -1 || to > 255)
		to = 255;
	if (step == -1)
		step = 1;

	base = from;
	top = to;
	if (from > to) {
		base = to;
		top = from;
		if (step > 0)
			step = -step;
	}

	range = top - base;
	buf = (char *)iocur_top->data + start;

	tmp = 0;
	for (i = start; i < start+len; i++) {
		*buf++ = tmp + base;
		tmp = (tmp + step)%(range+1);
	}
}

/* ARGSUSED */
static void
bwrite_random(
	int   start,
	int   len,
	int   shift,
	int   from,
	int   to)
{
	int i;
	char *buf;

	if (start == -1)
		start = 0;

	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	buf = (char *)iocur_top->data + start;

	for (i = start; i < start+len; i++)
		*buf++ = (char)lrand48();
}

/* ARGSUSED */
static void
bwrite_fill(
	int   start,
	int   len,
	int   value,
	int   from,
	int   to)
{
	char *base;

	if (value == -1)
		value = 0;
	if (start == -1)
		start = 0;
	if (len == -1)
		len = iocur_top->len - start;

	if (len+start > iocur_top->len) {
		dbprintf(_("length (%d) too large for data block size (%d)"),
			 len, iocur_top->len);
	}

	base = (char *)iocur_top->data + start;

	memset(base, value, len);
}

static struct bw_cmd {
	void	(*cmdfunc)(int,int,int,int,int);
	char	*cmdstr;
	int	sig_chars;
	int	argmin;
	int	argmax;
	int	shiftcount_arg;
	int	from_arg;
	int	to_arg;
	int	start_arg;
	int	len_arg;
	char	*usage;
} bw_cmdtab[] = {
	/* cmd   sig min max sh frm to start len */
	{ bwrite_lshift, "lshift",   2, 0, 3, 1, 0, 0, 2, 3,
		"[shiftcount] [start] [len]", },
	{ bwrite_rshift, "rshift",   2, 0, 3, 1, 0, 0, 2, 3,
		"[shiftcount] [start] [len]", },
	{ bwrite_lrot,   "lrot",     2, 0, 3, 1, 0, 0, 2, 3,
		"[shiftcount] [start] [len]", },
	{ bwrite_rrot,   "rrot",     2, 0, 3, 1, 0, 0, 2, 3,
		"[shiftcount] [start] [len]", },
	{ bwrite_seq,    "sequence", 3, 0, 4, 0, 1, 2, 3, 4,
		"[from] [to] [start] [len]", },
	{ bwrite_random, "random",   3, 0, 2, 0, 0, 0, 1, 2,
		"[start] [len]", },
	{ bwrite_fill,   "fill",     1, 1, 3, 1, 0, 0, 2, 3,
		"num [start] [len]" }
};

#define BWRITE_CMD_MAX (sizeof(bw_cmdtab)/sizeof(bw_cmdtab[0]))

static int
convert_oct(
	char *arg,
	int  *ret)
{
	int count;
	int i;
	int val = 0;

	/* only allow 1 case, '\' and 3 octal digits (or less) */

	for (count = 0; count < 3; count++) {
		if (arg[count] == '\0')
			break;

		if ((arg[count] < '0') && (arg[count] > '7'))
			break;
	}

	for (i = 0; i < count; i++) {
		val |= ((arg[(count-1)-i]-'0')&0x07)<<(i*3);
	}

	*ret = val&0xff;

	return(count);
}

#define NYBBLE(x) (isdigit(x)?(x-'0'):(tolower(x)-'a'+0xa))

static char *
convert_arg(
	char *arg,
	int  bit_length)
{
	int i;
	static char *buf = NULL;
	char *rbuf;
	long long *value;
	int alloc_size;
	char *ostr;
	int octval, ret;

	if (bit_length <= 64)
		alloc_size = 8;
	else
		alloc_size = (bit_length+7)/8;

	buf = xrealloc(buf, alloc_size);
	memset(buf, 0, alloc_size);
	value = (long long *)buf;
	rbuf = buf;

	if (*arg == '\"') {
		/* handle strings */

		/* zap closing quote if there is one */
		if ((ostr = strrchr(arg+1, '\"')) != NULL)
			*ostr = '\0';

		ostr = arg+1;
		for (i = 0; i < alloc_size; i++) {
			if (!*ostr)
				break;

			/* do octal */
			if (*ostr == '\\') {
				if (*(ostr+1) >= '0' || *(ostr+1) <= '7') {
					ret = convert_oct(ostr+1, &octval);
					*rbuf++ = octval;
					ostr += ret+1;
					continue;
				}
			}
			*rbuf++ = *ostr++;
		}

		return buf;
	} else if (arg[0] == '#' || ((arg[0] != '-') && strchr(arg,'-'))) {
		/*
		 * handle hex blocks ie
		 *    #00112233445566778899aabbccddeeff
		 * and uuids ie
		 *    1122334455667788-99aa-bbcc-ddee-ff00112233445566778899
		 *
		 * (but if it starts with "-" assume it's just an integer)
		 */
		int bytes=bit_length/8;

		/* skip leading hash */
		if (*arg=='#') arg++;

		while (*arg && bytes--) {
		    /* skip hypens */
		    while (*arg=='-') arg++;

		    /* get first nybble */
		    if (!isxdigit((int)*arg)) return NULL;
		    *rbuf=NYBBLE((int)*arg)<<4;
		    arg++;

		    /* skip more hyphens */
		    while (*arg=='-') arg++;

		    /* get second nybble */
		    if (!isxdigit((int)*arg)) return NULL;
		    *rbuf++|=NYBBLE((int)*arg);
		    arg++;
		}
		if (bytes<0&&*arg) return NULL;
		return buf;
	} else {
		/*
		 * handle integers
		 */
		*value = strtoll(arg, NULL, 0);

#if __BYTE_ORDER == BIG_ENDIAN
		/* hackery for big endian */
		if (bit_length <= 8) {
			rbuf += 7;
		} else if (bit_length <= 16) {
			rbuf += 6;
		} else if (bit_length <= 32) {
			rbuf += 4;
		}
#endif
		return rbuf;
	}
}


/* ARGSUSED */
void
write_struct(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	const ftattr_t	*fa;
	flist_t		*fl;
	flist_t         *sfl;
	int             bit_length;
	char            *buf;
	int		parentoffset;

	if (argc != 2) {
		dbprintf(_("usage: write fieldname value\n"));
		return;
	}

	fl = flist_scan(argv[0]);
	if (!fl) {
		dbprintf(_("unable to parse '%s'.\n"), argv[0]);
		return;
	}

	/* if we're a root field type, go down 1 layer to get field list */
	if (fields->name[0] == '\0') {
		fa = &ftattrtab[fields->ftyp];
		ASSERT(fa->ftyp == fields->ftyp);
		fields = fa->subfld;
	}

	/* run down the field list and set offsets into the data */
	if (!flist_parse(fields, fl, iocur_top->data, 0)) {
		flist_free(fl);
		dbprintf(_("parsing error\n"));
		return;
	}

	sfl = fl;
	parentoffset = 0;
	while (sfl->child) {
		parentoffset = sfl->offset;
		sfl = sfl->child;
	}

	bit_length = fsize(sfl->fld, iocur_top->data, parentoffset, 0);
	bit_length *= fcount(sfl->fld, iocur_top->data, parentoffset);

	/* convert this to a generic conversion routine */
	/* should be able to handle str, num, or even labels */

	buf = convert_arg(argv[1], bit_length);
	if (!buf) {
		dbprintf(_("unable to convert value '%s'.\n"), argv[1]);
		return;
	}

	setbitval(iocur_top->data, sfl->offset, bit_length, buf);
	write_cur();

	flist_print(fl);
	print_flist(fl);
	flist_free(fl);
}

/* ARGSUSED */
void
write_string(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	char *buf;
	int i;

	if (argc != 1) {
		dbprintf(_("usage (in string mode): write \"string...\"\n"));
		return;
	}

	buf = convert_arg(argv[0], (int)((strlen(argv[0])+1)*8));
	for (i = 0; i < iocur_top->len; i++) {
		((char *)iocur_top->data)[i] = *buf;
		if (*buf++ == '\0')
			break;
	}

	/* write back to disk */
	write_cur();
}

/* ARGSUSED */
void
write_block(
	const field_t	*fields,
	int		argc,
	char		**argv)
{
	int i;
	int shiftcount = -1;
	int start = -1;
	int len = -1;
	int from = -1;
	int to = -1;
	struct bw_cmd *cmd = NULL;

	if (argc <= 1 || argc > 5)
		goto block_usage;

	for (i = 0; i < BWRITE_CMD_MAX; i++) {
		if (sigcmp(argv[0], bw_cmdtab[i].cmdstr,
			   bw_cmdtab[i].sig_chars)) {
			cmd = &bw_cmdtab[i];
			break;
		}
	}

	if (!cmd) {
		dbprintf(_("write: invalid subcommand\n"));
		goto block_usage;
	}

	if ((argc < cmd->argmin + 1) || (argc > cmd->argmax + 1)) {
		dbprintf(_("write %s: invalid number of arguments\n"),
			 cmd->cmdstr);
		goto block_usage;
	}

	if (cmd->shiftcount_arg && (cmd->shiftcount_arg < argc))
		shiftcount = (int)strtoul(argv[cmd->shiftcount_arg], NULL, 0);
	if (cmd->start_arg && (cmd->start_arg < argc))
		start =  (int)strtoul(argv[cmd->start_arg], NULL, 0);
	if (cmd->len_arg && (cmd->len_arg < argc))
		len = (int)strtoul(argv[cmd->len_arg], NULL, 0);
	if (cmd->from_arg  && (cmd->len_arg < argc))
		from = (int)strtoul(argv[cmd->from_arg], NULL, 0);
	if (cmd->to_arg && (cmd->len_arg < argc))
		to = (int)strtoul(argv[cmd->to_arg], NULL, 0);

	cmd->cmdfunc(start, len, shiftcount, from, to);

	/* write back to disk */
	write_cur();
	return;

  block_usage:

	dbprintf(_("usage: write (in data mode)\n"));
	for (i = 0; i < BWRITE_CMD_MAX; i++) {
		dbprintf("              %-9.9s %s\n",
			 bw_cmdtab[i].cmdstr, bw_cmdtab[i].usage);
	}
	dbprintf("\n");
	return;
}
