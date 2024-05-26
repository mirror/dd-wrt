/*
 * Copyright (c) 2010, 2011, 2012, 2013
 * Phillip Lougher <phillip@squashfs.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * xz_wrapper.c
 *
 * Support for XZ (LZMA2) compression using XZ Utils liblzma
 * http://tukaani.org/xz/
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lzma.h>
#include <pthread.h>
#include <libgen.h>

#include "squashfs_fs.h"
#include "xz_wrapper.h"
#include "compressor.h"
#include "lzma_xz_options.h"

#define DBVERSION 3
static struct bcj bcj[] = { { "x86", LZMA_FILTER_X86, 0 },
			    { "powerpc", LZMA_FILTER_POWERPC, 0 },
			    { "ia64", LZMA_FILTER_IA64, 0 },
			    { "arm", LZMA_FILTER_ARM, 0 },
			    { "armthumb", LZMA_FILTER_ARMTHUMB, 0 },
			    { "sparc", LZMA_FILTER_SPARC, 0 },
			    { "arm64", LZMA_FILTER_ARM64, 0 },
			    { "riscv", LZMA_FILTER_RISCV, 0 },
			    //			    { "swizzle16", LZMA_FILTER_SWIZZLE16, 0 },
			    //			    { "swizzle32", LZMA_FILTER_SWIZZLE32, 0 },
			    //			    { "swizzle64", LZMA_FILTER_SWIZZLE64, 0 },
			    { "delta", LZMA_FILTER_DELTA, 0 },
			    { NULL, LZMA_VLI_UNKNOWN, 0 } };

static int filter_count = 1;

static int xz_options(char *argv[], int argc)
{
	if (strcmp(argv[0], "-Xbcj") == 0) {
		int i;
		char *name;

		if (argc < 2) {
			fprintf(stderr, "xz: -Xbcj missing filter\n");
			return -2;
		}

		name = argv[1];
		while (name[0] != '\0') {
			for (i = 0; bcj[i].name; i++) {
				int n = strlen(bcj[i].name);
				if ((strncmp(name, bcj[i].name, n) == 0) && (name[n] == '\0' || name[n] == ',')) {
					if (bcj[i].selected == 0) {
						bcj[i].selected = 1;
						filter_count++;
					}
					name += name[n] == ',' ? n + 1 : n;
					break;
				}
			}
			if (bcj[i].name == NULL) {
				fprintf(stderr, "xz: -Xbcj unrecognised "
						"filter\n");
				return -2;
			}
		}
		return 1;
	} else {
		return lzma_xz_options(argv, argc, LZMA_OPT_XZ);
	}
}

/*
 * This function is called after all options have been parsed.
 * It is used to do post-processing on the compressor options using
 * values that were not expected to be known at option parse time.
 *
 * In this case block_size may not be known until after -Xdict-size has
 * been processed (in the case where -b is specified after -Xdict-size)
 *
 * This function returns 0 on successful post processing, or
 *			-1 on error
 */
static int xz_options_post(int block_size)
{
	return lzma_xz_options_post(block_size, LZMA_OPT_XZ);
}

static void *xz_dump_options(int block_size, int *size)
{
	int i, flags = 0;

	for (i = 0; bcj[i].name; i++)
		flags |= bcj[i].selected << i;

	return lzma_xz_dump_options(block_size, size, flags);
}

/*
 * This function is a helper specifically for the append mode of
 * mksquashfs.  Its purpose is to set the internal compressor state
 * to the stored compressor options in the passed compressor options
 * structure.
 *
 * In effect this function sets up the compressor options
 * to the same state they were when the filesystem was originally
 * generated, this is to ensure on appending, the compressor uses
 * the same compression options that were used to generate the
 * original filesystem.
 *
 * Note, even if there are no compressor options, this function is still
 * called with an empty compressor structure (size == 0), to explicitly
 * set the default options, this is to ensure any user supplied
 * -X options on the appending mksquashfs command line are over-ridden
 *
 * This function returns 0 on sucessful extraction of options, and
 *			-1 on error
 */
static int xz_extract_options(int block_size, void *buffer, int size)
{
	int ret = lzma_xz_extract_options(block_size, buffer, size, LZMA_OPT_XZ);

	if (!ret) {
		int i;
		struct lzma_xz_options *opts = lzma_xz_get_options();
		for (i = 0; bcj[i].name; i++) {
			if ((opts->flags >> i) & 1) {
				bcj[i].selected = 1;
				filter_count++;
			} else
				bcj[i].selected = 0;
		}
	}
	return ret;
}

#include <malloc.h>
#include <stdio.h>
#if 0
void xz_display_options(void *buffer, int size)
{
	struct comp_opts *comp_opts = buffer;
	int dictionary_size, flags, printed;
	int i, n;

	/* check passed comp opts struct is of the correct length */
	if (size != sizeof(struct comp_opts))
		goto failed;

	SQUASHFS_INSWAP_COMP_OPTS(comp_opts);

	dictionary_size = comp_opts->dictionary_size;
	flags = comp_opts->flags;

	/*
	 * check that the dictionary size seems correct - the dictionary
	 * size should 2^n or 2^n+2^(n+1)
	 */
	n = ffs(dictionary_size) - 1;
	if (dictionary_size != (1 << n) && dictionary_size != ((1 << n) + (1 << (n + 1))))
		goto failed;

	printf("\tDictionary size %d\n", dictionary_size);

	printed = 0;
	for (i = 0; bcj[i].name; i++) {
		if ((flags >> i) & 1) {
			if (printed)
				printf(", ");
			else
				printf("\tFilters selected: ");
			printf("%s", bcj[i].name);
			printed = 1;
		}
	}

	if (!printed)
		printf("\tNo filters specified\n");
	else
		printf("\n");

	return;

failed:
	fprintf(stderr, "xz: error reading stored compressor options from " "filesystem!\n");
}
#endif

struct MATRIXENTRY {
	int pb;
	int lc;
	int lp;
	int used;
};

#if 0
static struct MATRIXENTRY matrix[] = {
	{ 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 }, { 0, 0, 3 }, { 0, 0, 4 }, { 0, 1, 0 }, { 0, 1, 1 }, { 0, 1, 2 }, { 0, 1, 3 },
	{ 0, 1, 4 }, { 0, 2, 0 }, { 0, 2, 1 }, { 0, 2, 2 }, { 0, 2, 3 }, { 0, 2, 4 }, { 0, 3, 0 }, { 0, 3, 1 }, { 0, 3, 2 },
	{ 0, 3, 3 }, { 0, 3, 4 }, { 0, 4, 0 }, { 0, 4, 1 }, { 0, 4, 2 }, { 0, 4, 3 }, { 0, 4, 4 }, { 1, 0, 0 }, { 1, 0, 1 },
	{ 1, 0, 2 }, { 1, 0, 3 }, { 1, 0, 4 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 1, 2 }, { 1, 1, 3 }, { 1, 1, 4 }, { 1, 2, 0 },
	{ 1, 2, 1 }, { 1, 2, 2 }, { 1, 2, 3 }, { 1, 2, 4 }, { 1, 3, 0 }, { 1, 3, 1 }, { 1, 3, 2 }, { 1, 3, 3 }, { 1, 3, 4 },
	{ 1, 4, 0 }, { 1, 4, 1 }, { 1, 4, 2 }, { 1, 4, 3 }, { 1, 4, 4 }, { 2, 0, 0 }, { 2, 0, 1 }, { 2, 0, 2 }, { 2, 0, 3 },
	{ 2, 0, 4 }, { 2, 1, 0 }, { 2, 1, 1 }, { 2, 1, 2 }, { 2, 1, 3 }, { 2, 1, 4 }, { 2, 2, 0 }, { 2, 2, 1 }, { 2, 2, 2 },
	{ 2, 2, 3 }, { 2, 2, 4 }, { 2, 3, 0 }, { 2, 3, 1 }, { 2, 3, 2 }, { 2, 3, 3 }, { 2, 3, 4 }, { 2, 4, 0 }, { 2, 4, 1 },
	{ 2, 4, 2 }, { 2, 4, 3 }, { 2, 4, 4 }, { 3, 0, 0 }, { 3, 0, 1 }, { 3, 0, 2 }, { 3, 0, 3 }, { 3, 0, 4 }, { 3, 1, 0 },
	{ 3, 1, 1 }, { 3, 1, 2 }, { 3, 1, 3 }, { 3, 1, 4 }, { 3, 2, 0 }, { 3, 2, 1 }, { 3, 2, 2 }, { 3, 2, 3 }, { 3, 2, 4 },
	{ 3, 3, 0 }, { 3, 3, 1 }, { 3, 3, 2 }, { 3, 3, 3 }, { 3, 3, 4 }, { 3, 4, 0 }, { 3, 4, 1 }, { 3, 4, 2 }, { 3, 4, 3 },
	{ 3, 4, 4 }, { 4, 0, 0 }, { 4, 0, 1 }, { 4, 0, 2 }, { 4, 0, 3 }, { 4, 0, 4 }, { 4, 1, 0 }, { 4, 1, 1 }, { 4, 1, 2 },
	{ 4, 1, 3 }, { 4, 1, 4 }, { 4, 2, 0 }, { 4, 2, 1 }, { 4, 2, 2 }, { 4, 2, 3 }, { 4, 2, 4 }, { 4, 3, 0 }, { 4, 3, 1 },
	{ 4, 3, 2 }, { 4, 3, 3 }, { 4, 3, 4 }, { 4, 4, 0 }, { 4, 4, 1 }, { 4, 4, 2 }, { 4, 4, 3 }, { 4, 4, 4 },
};
#endif
static struct MATRIXENTRY matrix[] = {
	{ 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 }, { 0, 0, 3 }, { 0, 0, 4 }, { 0, 1, 0 }, { 0, 1, 1 }, { 0, 1, 2 }, { 0, 1, 3 },
	{ 0, 2, 0 }, { 0, 2, 1 }, { 0, 2, 2 }, { 0, 3, 0 }, { 0, 3, 1 }, { 0, 4, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 1, 0, 2 },
	{ 1, 0, 3 }, { 1, 0, 4 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 1, 2 }, { 1, 1, 3 }, { 1, 2, 0 }, { 1, 2, 1 }, { 1, 2, 2 },
	{ 1, 3, 0 }, { 1, 3, 1 }, { 1, 4, 0 }, { 2, 0, 0 }, { 2, 0, 1 }, { 2, 0, 2 }, { 2, 0, 3 }, { 2, 0, 4 }, { 2, 1, 0 },
	{ 2, 1, 1 }, { 2, 1, 2 }, { 2, 1, 3 }, { 2, 2, 0 }, { 2, 2, 1 }, { 2, 2, 2 }, { 2, 3, 0 }, { 2, 3, 1 }, { 2, 4, 0 },
	{ 3, 0, 0 }, { 3, 0, 1 }, { 3, 0, 2 }, { 3, 0, 3 }, { 3, 0, 4 }, { 3, 1, 0 }, { 3, 1, 1 }, { 3, 1, 2 }, { 3, 1, 3 },
	{ 3, 2, 0 }, { 3, 2, 1 }, { 3, 2, 2 }, { 3, 3, 0 }, { 3, 3, 1 }, { 3, 4, 0 }, { 4, 0, 0 }, { 4, 0, 1 }, { 4, 0, 2 },
	{ 4, 0, 3 }, { 4, 0, 4 }, { 4, 1, 0 }, { 4, 1, 1 }, { 4, 1, 2 }, { 4, 1, 3 }, { 4, 2, 0 }, { 4, 2, 1 }, { 4, 3, 0 },
};
static pthread_spinlock_t p_mutex;

//static struct MATRIXENTRY matrix[4*4*4];

/*
 * This function is called by mksquashfs to initialise the
 * compressor, before compress() is called.
 *
 * This function returns 0 on success, and
 *			-1 on error
 */
static int xz_init(void **strm, int block_size, int datablock)
{
	int i, j, filters = datablock ? filter_count : 1;
	struct filter *filter = malloc(filters * sizeof(struct filter));
	struct xz_stream *stream;
	struct lzma_xz_options *opts = lzma_xz_get_options();

	/*	int a,b,c,cnt=0;
	for (a=0;a<4;a++)
	for (b=0;b<4;b++)
	for (c=0;c<4;c++)  {
	    matrix[cnt].pb = a;
	    matrix[cnt].lc = b;
	    matrix[cnt++].lp = c;
	}*/

	if (filter == NULL)
		goto failed;
	static int inited = 0;
	if (!inited)
		pthread_spin_init(&p_mutex, 0);
	inited = 1;
	stream = *strm = malloc(sizeof(struct xz_stream));
	if (stream == NULL)
		goto failed2;

	stream->filter = filter;
	stream->filters = filters;

	memset(filter, 0, filters * sizeof(struct filter));

	stream->dictionary_size = datablock ? opts->dict_size : SQUASHFS_METADATA_SIZE;

	filter[0].filter[0].id = LZMA_FILTER_LZMA2;
	filter[0].filter[0].options = &stream->opt;
	filter[0].filter[1].id = LZMA_VLI_UNKNOWN;

	for (i = 0, j = 1; datablock && bcj[i].name; i++) {
		if (bcj[i].selected) {
			filter[j].buffer = malloc(block_size * 4);
			if (filter[j].buffer == NULL)
				goto failed3;
			filter[j].filter[0].id = bcj[i].id;
			if (bcj[i].id == LZMA_FILTER_DELTA) {
				static lzma_options_delta opt;
				opt.dist = 1;
				filter[j].filter[0].options = &opt;
			}
			filter[j].filter[1].id = LZMA_FILTER_LZMA2;
			filter[j].filter[1].options = &stream->opt;
			filter[j].filter[2].id = LZMA_VLI_UNKNOWN;
			j++;
		}
	}

	return 0;

failed3:
	for (i = 1; i < filters; i++)
		free(filter[i].buffer);
	free(stream);

failed2:
	free(filter);

failed:
	return -1;
}

static int xz_compress2(void *strm, unsigned char *dest, void *src, int size, int block_size, int *error, int lc, int lp, int pb,
			int *filterid)
{
	int i;
	lzma_ret res = 0;
	struct xz_stream *stream = strm;
	struct filter *selected = NULL;
	struct lzma_xz_options *opts = lzma_xz_get_options();

	stream->filter[0].buffer = dest;

	for (i = 0; i < stream->filters; i++) {
		uint32_t preset = opts->preset;
		struct filter *filter = &stream->filter[i];
		filter->length = 0;
		preset |= LZMA_PRESET_EXTREME;

		if (lzma_lzma_preset(&stream->opt, preset))
			goto failed;

		stream->opt.lc = lc;
		stream->opt.lp = lp;
		stream->opt.pb = pb;
		stream->opt.nice_len = 273;

		stream->opt.dict_size = stream->dictionary_size;

		filter->length = 0;
		res = lzma_stream_buffer_encode(filter->filter, LZMA_CHECK_CRC32, NULL, src, size, filter->buffer, &filter->length,
						block_size);
		if (res == LZMA_OK) {
			if (!selected || selected->length > filter->length) {
				*filterid = i;
				selected = filter;
			}
		} else if (res != LZMA_BUF_ERROR) {
			//			printf("fail %d\n",res);
			goto failed;
		}
	}

	if (!selected) {
		/*
		 * Output buffer overflow.  Return out of buffer space
		 */
		return 0;
	}

	if (selected->buffer != dest) {
		memcpy(dest, selected->buffer, selected->length);
		/*      dest[selected->length++] = lc;
		   dest[selected->length++] = lp;
		   dest[selected->length++] = pb;
		   dest[selected->length++] = opts->fb; */
	} else {
	}
	*error = 0;
	return (int)selected->length;

failed:
	/*
	 * All other errors return failure, with the compressor
	 * specific error code in *error
	 */
	*error = res;
	return -1;
}

static int xz_compress2_byfilter(void *strm, unsigned char *dest, void *src, int size, int block_size, int *error, int lc, int lp,
				 int pb, int i)
{
	lzma_ret res = 0;
	struct xz_stream *stream = strm;
	struct filter *selected = NULL;
	struct lzma_xz_options *opts = lzma_xz_get_options();

	stream->filter[0].buffer = dest;

	uint32_t preset = opts->preset;
	struct filter *filter = &stream->filter[i];
	filter->length = 0;
	preset |= LZMA_PRESET_EXTREME;

	if (lzma_lzma_preset(&stream->opt, preset))
		goto failed;

	stream->opt.lc = lc;
	stream->opt.lp = lp;
	stream->opt.pb = pb;
	stream->opt.nice_len = 273;

	stream->opt.dict_size = stream->dictionary_size;

	filter->length = 0;
	res = lzma_stream_buffer_encode(filter->filter, LZMA_CHECK_CRC32, NULL, src, size, filter->buffer, &filter->length,
					block_size);
	if (res == LZMA_OK) {
		if (!selected || selected->length > filter->length) {
			selected = filter;
		}
	} else if (res != LZMA_BUF_ERROR)
		goto failed;

	if (!selected) {
		/*
		 * Output buffer overflow.  Return out of buffer space
		 */
		return 0;
	}

	if (selected->buffer != dest) {
		memcpy(dest, selected->buffer, selected->length);
		/*      dest[selected->length++] = lc;
		   dest[selected->length++] = lp;
		   dest[selected->length++] = pb;
		   dest[selected->length++] = opts->fb; */
	} else {
	}
	*error = 0;
	return (int)selected->length;

failed:
	/*
	 * All other errors return failure, with the compressor
	 * specific error code in *error
	 */
	*error = res;
	return -1;
}

typedef unsigned long uLongf;

#include "md5.h"
#include <unistd.h>
typedef struct DBENTRY {
	char md5sum[16];
	unsigned short fail : 1, pb : 5, lc : 5, lp : 5;
	unsigned char filterid;
} __attribute__((packed)) DBENTRY;

static char *getdbname(char *name)
{
	char temp[1024];
	readlink("/proc/self/exe", temp, sizeof(temp));
	dirname(temp);
	sprintf(name, "%s/matrix.db", temp);
	return name;
}

static FILE *opendatabase(char *mode)
{
	char name[1024];
	getdbname(name);
	FILE *fp = fopen(name, mode);
	return fp;
}

static void unlinkdatabase(void)
{
	char name[1024];
	getdbname(name);
	unlink(name);
	return;
}
static DBENTRY *db = NULL;
static size_t dblen;
static pthread_spinlock_t p_mutex;
static int matchcount = 0;
static int unmatchcount = 0;
static int counts[256];
static int checkparameters(char *src, int len, int *pb, int *lc, int *lp, int *fail, int *filterid, char *sum)
{
	md5_ctx_t MD;
	dd_md5_begin(&MD);
	dd_md5_hash(src, len, &MD);
	dd_md5_end(sum, &MD);
	pthread_spin_lock(&p_mutex);
	*fail = 0;

	FILE *in;
	if (!db) {
		in = opendatabase("rb");
		if (!in) {
			pthread_spin_unlock(&p_mutex);
			return -1;
		}
		fseek(in, 0, SEEK_END);
		dblen = ftell(in) - 1;
		if (dblen > 512 * 1024) {
			fclose(in);
			unlinkdatabase();
			pthread_spin_unlock(&p_mutex);
			return -1;
		}

		if (!dblen) {
			fclose(in);
			pthread_spin_unlock(&p_mutex);
			return -1;
		}

		db = malloc(dblen);
		rewind(in);
		if (!db) {
			fclose(in);
			pthread_spin_unlock(&p_mutex);
			return -1;
		}
		int version = getc(in);
		if (version != DBVERSION) {
			fclose(in);
			unlinkdatabase();
			dblen = 0;
			pthread_spin_unlock(&p_mutex);
			return -1;
		}
		fread(db, dblen, 1, in);
		fclose(in);
	}
	size_t i;
	for (i = 0; i < dblen / sizeof(*db); i++) {
		if (!memcmp(db[i].md5sum, sum, 16)) {
			*pb = db[i].pb;
			*lc = db[i].lc;
			*lp = db[i].lp;
			*fail = db[i].fail;
			*filterid = db[i].filterid;
			pthread_spin_unlock(&p_mutex);
			return 0;
		}
	}
	pthread_spin_unlock(&p_mutex);
	return -1;
}

static void writeparameters(int pb, int lc, int lp, int fail, int filterid, char *sum)
{
	pthread_spin_lock(&p_mutex);
	counts[filterid]++;
	db = realloc(db, dblen + sizeof(*db));
	size_t nextoffset = dblen / sizeof(*db);
	dblen += sizeof(*db);
	memcpy(db[nextoffset].md5sum, sum, 16);
	db[nextoffset].fail = fail;
	db[nextoffset].pb = pb;
	db[nextoffset].lp = lp;
	db[nextoffset].lc = lc;
	db[nextoffset].filterid = filterid;
	pthread_spin_unlock(&p_mutex);
}

static void writedb(void)
{
	pthread_spin_lock(&p_mutex);
	FILE *out = opendatabase("wb");
	if (!out) {
		pthread_spin_unlock(&p_mutex);
		return;
	}
	putc(DBVERSION, out);
	fwrite(db, dblen, 1, out);
	fclose(out);
	pthread_spin_unlock(&p_mutex);
}

static int xz_compress(void *s_strm, void *dst, void *src, int sourceLen, int block_size, int *error, int special)
{
	int test1len, test3len = 0;
	int s_fail = 0;
	int i, a;
	test1len = block_size * 2;
	int oldstate;
	int testcount;
	int lp;
	int lc;
	int pb;
	int filterid = 0;
	char md5[16];
	if (!special) {
		int ret = checkparameters(src, sourceLen, &pb, &lc, &lp, &s_fail, &filterid, md5);
		if (!ret) {
			matchcount++;
			if (s_fail) {
				return 0;
			}
			//			printf("db entry %d, lc %d lp %d pb %d filterid %d\n", sizeof(*db), lc, lp, pb, filterid);
			int len = xz_compress2_byfilter(s_strm, dst, src, sourceLen, block_size, error, lc, lp, pb, filterid);
			return len;
		}
		unmatchcount++;
	}
	unsigned char *test2 = (unsigned char *)malloc(block_size * 4);
	s_fail = 1;
	for (testcount = 0; testcount < sizeof(matrix) / sizeof(struct MATRIXENTRY); testcount++) {
		int takelcvalue = matrix[testcount].lc;
		int takepbvalue = matrix[testcount].pb;
		int takelpvalue = matrix[testcount].lp;
		int error2 = 0;
		int f;
		test3len =
			xz_compress2(s_strm, test2, src, sourceLen, block_size, &error2, takelcvalue, takelpvalue, takepbvalue, &f);
		if (!error2 && test3len > 0 && test3len < test1len) {
			test1len = test3len;
			memcpy(dst, test2, test3len);
			s_fail = 0;
			matrix[testcount].used = 1;
			pb = takepbvalue;
			lc = takelcvalue;
			lp = takelpvalue;
			filterid = f;
			*error = error2;
		}
	}
	free(test2);
	if (s_fail)
		test1len = 0;
	if (!special) {
		writeparameters(pb, lc, lp, s_fail, filterid, md5);
	}
	return test1len;
}

static int xz_uncompress(void *dest, void *src, int size, int block_size, int *error)
{
	size_t src_pos = 0;
	size_t dest_pos = 0;
	uint64_t memlimit = MEMLIMIT;
	lzma_ret res = lzma_stream_buffer_decode(&memlimit, 0, NULL, src, &src_pos, size, dest, &dest_pos, block_size);
	if (res == LZMA_OK && size == (int)src_pos)
		return (int)dest_pos;
	else {
		*error = res;
		return -1;
	}
	return res == LZMA_OK && size == (int)src_pos ? (int)dest_pos : -1;
}

void xz_usage()
{
	lzma_xz_usage(LZMA_OPT_XZ);
	fprintf(stderr, "\t  -Xbcj filter1,filter2,...,filterN\n");
	fprintf(stderr, "\t\tCompress using filter1,filter2,...,filterN in");
	fprintf(stderr, " turn\n\t\t(in addition to no filter), and choose");
	fprintf(stderr, " the best compression.\n");
	fprintf(stderr, "\t\tAvailable filters: x86, arm, armthumb,");
	fprintf(stderr, " powerpc, sparc, ia64\n");
}

int xz_deinit(void)
{
	int testcount;
	writedb();
	for (testcount = 0; testcount < sizeof(matrix) / sizeof(struct MATRIXENTRY); testcount++) {
		if (matrix[testcount].used)
			printf("{%d,%d,%d},\n", matrix[testcount].pb, matrix[testcount].lc, matrix[testcount].lp);
	}

	printf("learning db matches %d unmatches %d\n", matchcount, unmatchcount);
	int i;
	for (i=0;i<256;i++)
	    {
	    if (counts[i])
		printf("%d items are encoded with filter %s\n", counts[i], bcj[i].name);
	    }
	return 0;
}

struct compressor xz_comp_ops = { .init = xz_init,
				  .deinit = xz_deinit,
				  .compress = xz_compress,
				  .uncompress = xz_uncompress,
				  .options = xz_options,
				  .options_post = xz_options_post,
				  .dump_options = xz_dump_options,
				  .extract_options = xz_extract_options,
				  //	.display_options = xz_display_options,
				  .usage = xz_usage,
				  .id = XZ_COMPRESSION,
				  .name = "xz",
				  .supported = 1 };
