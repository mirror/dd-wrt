/* ltest.c -- very comprehensive test driver for the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


#include <lzoconf.h>

#if defined(LZO_HAVE_CONFIG_H)
#  include <config.h>
#  include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#if defined(__DJGPP__) || defined(__BORLANDC__)
#  include <dir.h>
#endif
#if defined(HAVE_UNISTD_H) || defined(__DJGPP__) || defined(__EMX__)
#  include <unistd.h>
#endif
#include <lzoutil.h>

#if defined(__LZO_DOS) || defined(__LZO_WIN)
#  define HAVE_STRICMP 1
#  define HAVE_STRNICMP 1
#endif
#if defined(HAVE_STRNICMP) && !defined(HAVE_STRNCASECMP)
#  define strncasecmp	strnicmp
#endif

#if 0
#  define is_digit(x)	(isdigit((unsigned char)(x)))
#  define is_space(x)	(isspace((unsigned char)(x)))
#else
#  define is_digit(x)	((unsigned)(x) - '0' <= 9)
#  define is_space(x)	((x)==' ' || (x)=='\t' || (x)=='\r' || (x)=='\n')
#endif


/*************************************************************************
// getopt & high resolution timer
**************************************************************************/

#include "mygetopt.h"
#include "mygetopt.ch"

#include "timer.h"


/*************************************************************************
// compression include section
**************************************************************************/

#define HAVE_LZO1_H
#define HAVE_LZO1A_H
#define HAVE_LZO1B_H
#define HAVE_LZO1C_H
#define HAVE_LZO1F_H
#define HAVE_LZO1X_H
#define HAVE_LZO1Y_H
#define HAVE_LZO1Z_H
#define HAVE_LZO2A_H

#if 1 && defined(MFX) && !defined(LZO_HAVE_CONFIG_H) && !defined(HAVE_ZLIB_H)
#define HAVE_ZLIB_H
#endif
#if defined(NO_ZLIB_H)
#undef HAVE_ZLIB_H
#endif

#if defined(__LZO_DOS16)
/* don't make this test program too big */
#undef HAVE_LZO1_H
#undef HAVE_LZO1A_H
#undef HAVE_LZO1C_H
#undef HAVE_LZO1Z_H
#undef HAVE_LZO2A_H
#undef HAVE_LZO2B_H
#undef HAVE_ZLIB_H
#endif


/* LZO algorithms */
#if defined(HAVE_LZO1_H)
#  include <lzo1.h>
#endif
#if defined(HAVE_LZO1A_H)
#  include <lzo1a.h>
#endif
#if defined(HAVE_LZO1B_H)
#  include <lzo1b.h>
#endif
#if defined(HAVE_LZO1C_H)
#  include <lzo1c.h>
#endif
#if defined(HAVE_LZO1F_H)
#  include <lzo1f.h>
#endif
#if defined(HAVE_LZO1X_H)
#  include <lzo1x.h>
#endif
#if defined(HAVE_LZO1Y_H)
#  include <lzo1y.h>
#endif
#if defined(HAVE_LZO1Z_H)
#  include <lzo1z.h>
#endif
#if defined(HAVE_LZO2A_H)
#  include <lzo2a.h>
#endif
#if defined(HAVE_LZO2B_H)
#  include <lzo2b.h>
#endif

/* other compressors */
#if defined(HAVE_ZLIB_H)
#  include <zlib.h>
#  define ALG_ZLIB
#endif
#if defined(MFX)
#  include "maint/t_config.ch"
#endif


/*************************************************************************
// enumerate all methods
**************************************************************************/

enum {
/* compression algorithms */
	M_LZO1B_1     =     1,
	M_LZO1B_2, M_LZO1B_3, M_LZO1B_4, M_LZO1B_5,
	M_LZO1B_6, M_LZO1B_7, M_LZO1B_8, M_LZO1B_9,

	M_LZO1C_1     =    11,
	M_LZO1C_2, M_LZO1C_3, M_LZO1C_4, M_LZO1C_5,
	M_LZO1C_6, M_LZO1C_7, M_LZO1C_8, M_LZO1C_9,

	M_LZO1        =    21,
	M_LZO1A       =    31,

	M_LZO1B_99    =   901,
	M_LZO1B_999   =   902,
	M_LZO1C_99    =   911,
	M_LZO1C_999   =   912,
	M_LZO1_99     =   921,
	M_LZO1A_99    =   931,

	M_LZO1F_1     =    61,
	M_LZO1F_999   =   962,
	M_LZO1X_1     =    71,
	M_LZO1X_1_11  =   111,
	M_LZO1X_1_12  =   112,
	M_LZO1X_1_15  =   115,
	M_LZO1XT_1    =   171,
	M_LZO1X_999   =   972,
	M_LZO1Y_1     =    81,
	M_LZO1Y_999   =   982,
	M_LZO1Z_999   =   992,

	M_LZO2A_999   =   942,
	M_LZO2B_999   =   952,

	M_LAST_LZO_COMPRESSOR = 998,

/* other compressors */
#if defined(ALG_ZLIB)
	M_ZLIB_8_1 =  1101,
	M_ZLIB_8_2, M_ZLIB_8_3, M_ZLIB_8_4, M_ZLIB_8_5,
	M_ZLIB_8_6, M_ZLIB_8_7, M_ZLIB_8_8, M_ZLIB_8_9,
#endif

/* dummy compressor - for speed comparision */
	M_MEMCPY      =   999,

	M_LAST_COMPRESSOR = 4999,

/* dummy algorithms - for speed comparision */
	M_MEMSET      =  5001,

/* checksum algorithms - for speed comparision */
	M_ADLER32     =  6001,
	M_CRC32       =  6002,
#if defined(ALG_ZLIB)
	M_Z_ADLER32   =  6011,
	M_Z_CRC32     =  6012,
#endif

	M_UNUSED
};


#if defined(LZO_99_UNSUPPORTED)
#define M_LZO1_99		(-1)
#define M_LZO1A_99		(-1)
#define M_LZO1B_99		(-1)
#define M_LZO1C_99		(-1)
#endif
#if defined(LZO_999_UNSUPPORTED)
#define M_LZO1B_999		(-1)
#define M_LZO1C_999		(-1)
#define M_LZO1F_999		(-1)
#define M_LZO1X_999		(-1)
#define M_LZO1Y_999		(-1)
#define M_LZO1Z_999		(-1)
#define M_LZO2A_999		(-1)
#define M_LZO2B_999		(-1)
#endif


/*************************************************************************
// command line options
**************************************************************************/

#if defined(MAINT)
int opt_verbose = 1;
#else
int opt_verbose = 2;
#endif

int opt_c_loops = 0;
int opt_d_loops = 0;
const char *opt_calgary_corpus_path = NULL;
const char *opt_dump_compressed_data = NULL;

lzo_bool opt_use_safe_decompressor = 0;
lzo_bool opt_use_asm_decompressor = 0;
lzo_bool opt_use_asm_fast_decompressor = 0;
lzo_bool opt_optimize_compressed_data = 0;

int opt_dict = 0;
lzo_int opt_max_dict_len = LZO_INT_MAX;
const char *opt_dictionary_file = NULL;

lzo_bool opt_read_from_stdin = 0;

/* set these to 1 to measure the speed impact of a checksum */
lzo_bool opt_compute_adler32 = 0;
lzo_bool opt_compute_crc32 = 0;
static lzo_uint32 adler_in, adler_out;
static lzo_uint32 crc_in, crc_out;

lzo_bool opt_execution_time = 0;

lzo_bool opt_totals = 0;
static unsigned long total_n = 0;
static unsigned long total_c_len = 0;
static unsigned long total_d_len = 0;
static unsigned long total_blocks = 0;
static double total_c_kbs = 0.0;
static double total_d_kbs = 0.0;

lzo_bool opt_query = 0;		/* query interface is not implemented yet */

static const lzo_bool opt_try_to_compress_0_bytes = 1;

static const char *argv0 = "";


#if defined(HAVE_LZO1X_H)
int default_method = M_LZO1X_1;
#elif defined(HAVE_LZO1B_H)
int default_method = M_LZO1B_1;
#elif defined(HAVE_LZO1C_H)
int default_method = M_LZO1C_1;
#elif defined(HAVE_LZO1F_H)
int default_method = M_LZO1F_1;
#elif defined(HAVE_LZO1Y_H)
int default_method = M_LZO1Y_1;
#else
int default_method = M_MEMCPY;
#endif


static const int benchmark_methods[] = {
	M_LZO1B_1, M_LZO1B_9,
	M_LZO1C_1, M_LZO1C_9,
	M_LZO1F_1,
	M_LZO1X_1,
	0
};

static const int x1_methods[] = {
	M_LZO1, M_LZO1A, M_LZO1B_1, M_LZO1C_1, M_LZO1F_1, M_LZO1X_1, M_LZO1Y_1,
	0
};

static const int x99_methods[] = {
	M_LZO1_99, M_LZO1A_99, M_LZO1B_99, M_LZO1C_99,
	0
};

static const int x999_methods[] = {
	M_LZO1B_999, M_LZO1C_999, M_LZO1F_999, M_LZO1X_999, M_LZO1Y_999,
	M_LZO1Z_999,
	M_LZO2A_999,
	0
};


/* exit codes of this test program */
#define EXIT_OK			0
#define EXIT_USAGE		1
#define EXIT_FILE		2
#define EXIT_MEM		3
#define EXIT_ADLER		4
#define EXIT_LZO_ERROR	5
#define EXIT_LZO_INIT	6
#define EXIT_INTERNAL	7


/*************************************************************************
// memory setup
**************************************************************************/

#if 0 && (UINT_MAX >= LZO_0xffffffffL)
#define USE_MALLOC
#endif


#if defined(__LZO_STRICT_16BIT)

#define BLOCK_SIZE		32000u
#define MAX_BLOCK_SIZE	(55*1024u)
#define DATA_LEN		(63*1024u)
#define WRK_LEN			(32*1024u)

#elif defined(__LZO_TOS) || defined(__LZO_WIN16)

/* adjust memory so that it works on a 4 meg machine */
#define BLOCK_SIZE		(256*1024l)
#define MAX_BLOCK_SIZE	(1*BLOCK_SIZE)
#define DATA_LEN		(1024*1024l)
#define WRK_LEN			(768*1024l)
#define USE_CORPUS

#elif (UINT_MAX >= LZO_0xffffffffL)		/* 32 bit or more */

#define BLOCK_SIZE		(256*1024l)
#define MAX_BLOCK_SIZE	(5*256*1024l)
#define DATA_LEN		(5*256*1024l)
#ifndef WRK_LEN
#define WRK_LEN			(768*1024l * ((sizeof(long)+3)/4))
#endif
#define DICT_LEN		0xbfff
#define USE_DUMP
#define USE_CORPUS

#else

/* DOS 16 bit - have to squeeze everything into ~600 kB  */
#define BLOCK_SIZE		32000u
#define MAX_BLOCK_SIZE	40000u
#define DATA_LEN		(64*1024l)
#define WRK_LEN			(64*1024l)

#endif


static lzo_uint opt_block_size = BLOCK_SIZE;
static lzo_uint opt_max_data_len = 0;

static lzo_byte _block1[MAX_BLOCK_SIZE * 9L / 8 + 256 + 1024];
static lzo_byte _block2[MAX_BLOCK_SIZE * 9L / 8 + 256 + 1024];
static lzo_byte _wrkmem[16 + WRK_LEN + 256 + 16];

#ifdef USE_MALLOC
#undef DATA_LEN
static lzo_byte *_data = NULL;
#else
static lzo_byte _data[DATA_LEN + 256];
#endif

#ifdef DICT_LEN
#define USE_DICT
static lzo_byte _dict[DICT_LEN];
static lzo_byte *dict = _dict;
static lzo_uint32 dict_adler32;
#else
static lzo_byte *dict = NULL;
#endif
static lzo_uint dict_len = 0;

/* align memory blocks (cache issues) */
static lzo_byte *block1 = NULL;
static lzo_byte *block2 = NULL;
static lzo_byte *wrkmem = NULL;
static lzo_byte *data   = NULL;

static void align_mem(void)
{
	block1 = LZO_PTR_ALIGN_UP(_block1,256);
	block2 = LZO_PTR_ALIGN_UP(_block2,256);
	wrkmem = LZO_PTR_ALIGN_UP(_wrkmem+16,256);
}


/* lock all memory to avoid any swapping to disk */
#if 0 && defined(__DJGPP__)
#  include <crt0.h>
   int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY;
#endif


/*************************************************************************
// dictionary support
**************************************************************************/

#ifdef USE_DICT
static void init_default_dict(void)
{
	lzo_uint i, j, d, dd;

	dict_len = DICT_LEN;
	lzo_memset(dict,0,dict_len);

	/* this default dictionary does not provide good contexts... */
	dd = dict_len;
	if (dd >= 16 * 256)
	{
		dd -= 16 * 256;
		for (i = 0, d = dd; i < 256; i++)
			for (j = 0; j < 16; j++)
				dict[d++] = (unsigned char) i;
	}

	dict_adler32 = lzo_adler32(0,NULL,0);
	dict_adler32 = lzo_adler32(dict_adler32,dict,dict_len);
}


static void read_dict(const char *file_name)
{
	FILE *f;

	dict_len = 0;
	f = fopen(file_name,"rb");
	if (f)
	{
		dict_len = lzo_fread(f,dict,DICT_LEN);
		fclose(f);
		dict_adler32 = lzo_adler32(0,NULL,0);
		dict_adler32 = lzo_adler32(dict_adler32,dict,dict_len);
	}
}
#endif


/*************************************************************************
// compression database
**************************************************************************/

typedef struct
{
	const char *			name;
	int 					id;
	lzo_uint32				mem_compress;
	lzo_uint32				mem_decompress;
	lzo_compress_t			compress;
	lzo_optimize_t			optimize;
	lzo_decompress_t		decompress;
	lzo_decompress_asm_t	decompress_safe;
	lzo_decompress_asm_t	decompress_asm;
	lzo_decompress_asm_t	decompress_asm_safe;
	lzo_decompress_asm_t	decompress_asm_fast;
	lzo_decompress_asm_t	decompress_asm_fast_safe;
	lzo_compress_dict_t		compress_dict;
	lzo_decompress_dict_t	decompress_dict_safe;
}
compress_t;

#include "asm.h"

#include "wrap.h"
#define M_PRIVATE       LZO_PRIVATE
#define m_uint          lzo_uint
#define m_uint32        lzo_uint32
#define m_voidp         lzo_voidp
#define m_bytep         lzo_bytep
#define m_uintp         lzo_uintp
#include "wrapmisc.h"

static const compress_t compress_database[] = {
#include "db.h"
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};


/*************************************************************************
// method info
**************************************************************************/

static
lzo_decompress_t get_decomp_info ( const compress_t *c, const char **nn )
{
	lzo_decompress_t d = 0;
	const char *n = NULL;

	/* safe has priority over asm/fast */
	if (!d && opt_use_safe_decompressor && opt_use_asm_fast_decompressor)
	{
		d = c->decompress_asm_fast_safe;
		n = " [fs]";
	}
	if (!d && opt_use_safe_decompressor && opt_use_asm_decompressor)
	{
		d = c->decompress_asm_safe;
		n = " [as]";
	}
	if (!d && opt_use_safe_decompressor)
	{
		d = c->decompress_safe;
		n = " [s]";
	}
	if (!d && opt_use_asm_fast_decompressor)
	{
		d = c->decompress_asm_fast;
		n = " [f]";
	}
	if (!d && opt_use_asm_decompressor)
	{
		d = c->decompress_asm;
		n = " [a]";
	}
	if (!d)
	{
		d = c->decompress;
		n = "";
	}
	if (!d)
		n = "(null)";

	if (opt_dict && c->decompress_dict_safe)
		n = "";

	if (nn)
		*nn = n;
	return d;
}


static
const compress_t *info ( int method, FILE *f )
{
	const compress_t *c = NULL;
	const compress_t *db = compress_database;
	size_t size = sizeof(compress_database) / sizeof(*(compress_database));
	size_t i;

	if (method > 0)
	{
		for (i = 0; i < size && db->name != NULL; i++, db++)
		{
			if (method == db->id &&
			    WRK_LEN >= db->mem_compress &&
			    WRK_LEN >= db->mem_decompress)
			{
				c = db;
				break;
			}
		}
	}

	if (f != NULL)
	{
		if (c == NULL)
		{
			fprintf(f,"%s: invalid method %d\n",argv0,method);
			exit(EXIT_USAGE);
		}
		else
		{
			const char *n;
			get_decomp_info(c,&n);
			fprintf(f,"%s%s",c->name,n);
		}
	}
	return c;
}


static
const compress_t *find_method ( const char *name )
{
	const compress_t *db = compress_database;
	size_t size = sizeof(compress_database) / sizeof(*(compress_database));
	size_t i;

	for (i = 0; i < size && db->name != NULL; i++, db++)
	{
		size_t n = strlen(db->name);

#if defined(HAVE_STRNCASECMP)
		if (strncasecmp(name,db->name,n) == 0 && (!name[n] || name[n] == ','))
			return db;
#else
		if (strncmp(name,db->name,n) == 0 && (!name[n] || name[n] == ','))
			return db;
#endif
	}
	return NULL;
}


static
lzo_bool is_compressor ( const compress_t *c )
{
	return (c->id <= M_LAST_COMPRESSOR || c->id >= 9721);
}


/*************************************************************************
// check that memory gets accessed within bounds
**************************************************************************/

#if 0 || defined(LZO_DEBUG)
static const lzo_bool opt_check_mem = 1;
#else
static const lzo_bool opt_check_mem = 0;
#endif

void init_mem_checker ( lzo_byte *mem, lzo_byte *mem_base,
                        lzo_uint32 l, unsigned char random_byte )
{
	lzo_uint i;
	lzo_uint len = (lzo_uint) l;

	if (opt_check_mem && mem && mem_base)
	{
		for (i = 0; i < 16; i++)
			mem[len+i] = random_byte++;
		for (i = 0; i < 16 && mem - i > mem_base; i++)
			mem[-1-i] = random_byte++;
#if 0 || defined(LZO_DEBUG)
		/* fill in garbage */
		random_byte |= 1;
		for (i = 0; i < len; i++, random_byte += 2)
			mem[i] = random_byte;
#endif
	}
}


int check_mem ( lzo_byte *mem, lzo_byte *mem_base,
                lzo_uint32 l, unsigned char random_byte )
{
	lzo_uint i;
	lzo_uint len = (lzo_uint) l;

	if (opt_check_mem && mem && mem_base)
	{
		for (i = 0; i < 16; i++)
			if (mem[len+i] != random_byte++)
				return -1;
		for (i = 0; i < 16 && mem - i > mem_base; i++)
			if (mem[-1-i] != random_byte++)
				return -1;
	}
	return 0;
}


/*************************************************************************
// compress a block
**************************************************************************/

static
int do_compress ( const compress_t *c,
                  const lzo_byte *src, lzo_uint  src_len,
                        lzo_byte *dst, lzo_uintp dst_len )
{
	int r = -100;

	if (c && c->compress && WRK_LEN >= c->mem_compress)
	{
#if defined(__LZO_CHECKER)
		/* malloc a block of the exact size to detect any overrun */
		lzo_byte *w = wrkmem;
		if (c->mem_compress > 0)
		{
			wrkmem = malloc(c->mem_compress);
			/* must initialize memory - fill in garbage */
			{
				lzo_uint32 i;
				unsigned char random_byte = (unsigned char) src_len;
				random_byte |= 1;
				for (i = 0; i < c->mem_compress; i++, random_byte += 2)
					wrkmem[i] = random_byte;
			}
		}
		else
			wrkmem = NULL;
#else
		unsigned char random_byte = (unsigned char) src_len;
		init_mem_checker(wrkmem,_wrkmem,c->mem_compress,random_byte);
#endif

		if (opt_dict && c->compress_dict)
			r = c->compress_dict(src,src_len,dst,dst_len,wrkmem,dict,dict_len);
		else
			r = c->compress(src,src_len,dst,dst_len,wrkmem);

#if defined(__LZO_CHECKER)
		if (wrkmem)
			free(wrkmem);
		wrkmem = w;
#else
		if (check_mem(wrkmem,_wrkmem,c->mem_compress,random_byte) != 0)
			printf("WARNING: wrkmem overwrite error (compress) !!!\n");
#endif
	}

	if (r == 0 && opt_compute_adler32)
	{
		lzo_uint32 adler;
		adler = lzo_adler32(0, NULL, 0);
		adler = lzo_adler32(adler, src, src_len);
		adler_in = adler;
	}
	if (r == 0 && opt_compute_crc32)
	{
		lzo_uint32 crc;
		crc = lzo_crc32(0, NULL, 0);
		crc = lzo_crc32(crc, src, src_len);
		crc_in = crc;
	}

	return r;
}


/*************************************************************************
// decompress a block
**************************************************************************/

static
int do_decompress ( const compress_t *c, lzo_decompress_t d,
                    const lzo_byte *src, lzo_uint  src_len,
                          lzo_byte *dst, lzo_uintp dst_len )
{
	int r = -100;

   	if (c && d && WRK_LEN >= c->mem_decompress)
	{
		unsigned char random_byte = (unsigned char) src_len;
		init_mem_checker(wrkmem,_wrkmem,c->mem_decompress,random_byte);

		if (opt_dict && c->decompress_dict_safe)
			r = c->decompress_dict_safe(src,src_len,dst,dst_len,wrkmem,dict,dict_len);
		else
			r = d(src,src_len,dst,dst_len,wrkmem);

		if (check_mem(wrkmem,_wrkmem,c->mem_decompress,random_byte) != 0)
			printf("WARNING: wrkmem overwrite error (decompress) !!!\n");
	}

	if (r == 0 && opt_compute_adler32)
	{
		lzo_uint32 adler;
		adler = lzo_adler32(0, NULL, 0);
		adler = lzo_adler32(adler, dst, *dst_len);
		adler_out = adler;
	}
	if (r == 0 && opt_compute_crc32)
	{
		lzo_uint32 crc;
		crc = lzo_crc32(0, NULL, 0);
		crc = lzo_crc32(crc, dst, *dst_len);
		crc_out = crc;
	}

	return r;
}


/*************************************************************************
// optimize a block
**************************************************************************/

static
int do_optimize  ( const compress_t *c,
                         lzo_byte *src, lzo_uint  src_len,
                         lzo_byte *dst, lzo_uintp dst_len )
{
	if (c && c->optimize && WRK_LEN >= c->mem_decompress)
		return c->optimize(src,src_len,dst,dst_len,wrkmem);
	else
		return 0;
}


/***********************************************************************
// read a file
************************************************************************/

static const char *last_file = NULL;
static lzo_uint last_len = 0;

static void unload_file(void)
{
	last_file = NULL;
	last_len = 0;
#ifdef USE_MALLOC
	if (_data)
	{
		free(_data);
		data = _data = NULL;
	}
#endif
}


int load_file ( const char *file_name, lzo_uintp len )
{
	FILE *f;
	long ll = -1;
	unsigned long ul;
	lzo_uint l;
	int r;

#if 0
	if (last_file && strcmp(file_name,last_file) == 0)
	{
		*len = last_len;
		return EXIT_OK;
	}
#endif

	*len = 0;
	unload_file();

	f = fopen(file_name,"rb");
	if (f == NULL)
	{
		fprintf(stderr,"%s: ",file_name);
		perror("fopen");
		fflush(stderr);
		return EXIT_FILE;
	}
    r = fseek(f,0,SEEK_END);
	if (r == 0)
	{
		ll = ftell(f);
    	r = fseek(f,0,SEEK_SET);
	}
	if (r != 0 || ll < 0)
	{
		fprintf(stderr,"%s: ",file_name);
		perror("fseek");
		fflush(stderr);
		return EXIT_FILE;
	}
	ul = ll;

#ifdef USE_MALLOC
	l = (ul >= LZO_UINT_MAX - 512 ? LZO_UINT_MAX - 512 : (lzo_uint) ul);
	if (opt_max_data_len > 0 && l > opt_max_data_len)
		l = opt_max_data_len;
	_data = malloc (l + 256);
	if (_data == NULL)
	{
		perror("malloc");
		fflush(stderr);
		return EXIT_MEM;
	}
#else
	l = (ul >= DATA_LEN ? (lzo_uint) DATA_LEN : (lzo_uint) ul);
	if (opt_max_data_len > 0 && l > opt_max_data_len)
		l = opt_max_data_len;
#endif
	data = LZO_PTR_ALIGN_UP(_data,256);
	l = lzo_fread(f,data,l);
	if (fclose(f) != 0)
	{
		unload_file();
		fprintf(stderr,"%s: ",file_name);
		perror("fclose");
		fflush(stderr);
		return EXIT_FILE;
	}

	last_file = file_name;
	last_len = l;
	*len = l;
	return EXIT_OK;
}


/***********************************************************************
// print some compression statistics
************************************************************************/

static
void print_stats ( const char *method_name, const char *file_name,
                   int t_loops, int c_loops, int d_loops,
                   my_clock_t t_time, my_clock_t c_time, my_clock_t d_time,
                   unsigned long c_len, unsigned long d_len,
                   unsigned blocks )
{
	unsigned long x_len = d_len;
	unsigned long t_bytes, c_bytes, d_bytes;
	double t_secs, c_secs, d_secs;
	double t_kbs, c_kbs, d_kbs;
	double perc, bits;

	perc = (d_len > 0) ? c_len * 100.0 / d_len : 0;
	bits = perc * 0.08;

	c_bytes = x_len * c_loops * t_loops;
	d_bytes = x_len * d_loops * t_loops;
	t_bytes = c_bytes + d_bytes;

	c_secs = c_time / (double)(MY_CLOCKS_PER_SEC);
	c_kbs = (c_secs > 0.001) ? (c_bytes / c_secs) / 1000.0 : 0;
	d_secs = d_time / (double)(MY_CLOCKS_PER_SEC);
	d_kbs = (d_secs > 0.001) ? (d_bytes / d_secs) / 1000.0 : 0;
	t_secs = t_time / (double)(MY_CLOCKS_PER_SEC);
	t_kbs = (t_secs > 0.001) ? (t_bytes / t_secs) / 1000.0 : 0;

	total_n++;
	total_c_len += c_len;
	total_d_len += d_len;
	total_blocks += blocks;
	total_c_kbs += c_kbs;
	total_d_kbs += d_kbs;

	if (opt_verbose >= 2)
	{
		printf("  compressed into %lu bytes, %.2f%%   %.3f\n",
			(long) c_len, perc, bits);

		printf("%-15s %5d: ","overall", t_loops);
		printf("%10lu bytes, %8.2f secs, %10.2f KB/sec\n",
		        t_bytes, t_secs, t_kbs);
		printf("%-15s %5d: ","compress", c_loops);
		printf("%10lu bytes, %8.2f secs, %10.2f KB/sec\n",
		        c_bytes, c_secs, c_kbs);
		printf("%-15s %5d: ","decompress", d_loops);
		printf("%10lu bytes, %8.2f secs, %10.2f KB/sec\n",
		        d_bytes, d_secs, d_kbs);
		printf("\n");
	}

	/* create a line for util/table.pl */
	if (opt_verbose >= 1)
	{
		/* get basename */
		const char *n, *nn, *b;
		for (nn = n = b = file_name; *nn; nn++)
			if (*nn == '/' || *nn == '\\' || *nn == ':')
				b = nn + 1;
			else
				n = b;

		printf("%-12s | %-14s %7ld %4ld %7ld %6.1f %9.2f %9.2f |\n",
			method_name, n,
			(long) d_len, (long) blocks, (long) c_len, perc,
			c_kbs, d_kbs);
	}

	if (opt_verbose >= 2)
		printf("\n");
}


static
void print_totals ( void )
{
	double perc;

	perc = (total_d_len > 0) ? total_c_len * 100.0 / total_d_len : 0;

	if (opt_verbose >= 1 && total_n > 1)
	{
		printf("%-12s   %-14s %7ld %4ld %7ld %6.1f %9.2f %9.2f\n",
			"-------", "***TOTALS***",
			total_d_len, total_blocks, total_c_len, perc,
			total_c_kbs / total_n, total_d_kbs / total_n);
	}
}


/*************************************************************************
// compress and decompress a file
**************************************************************************/

static
int process_file ( const compress_t *c, lzo_decompress_t decompress,
                   const char *method_name,
                   const char *file_name, lzo_uint l,
                   int t_loops, int c_loops, int d_loops )
{
	int i;
	unsigned blocks = 0;
	unsigned long compressed_len = 0;
	my_clock_t t_time = 0, c_time = 0, d_time = 0;
	my_clock_t t_start, c_start, d_start;
#ifdef USE_DUMP
	FILE *dump = NULL;

	if (opt_dump_compressed_data)
		dump = fopen(opt_dump_compressed_data,"wb");
#endif

/* process the file */

	t_start = my_clock();
	for (i = 0; i < t_loops; i++)
	{
		lzo_uint len, c_len, c_len_max, d_len = 0;
		lzo_byte *d = data;

		len = l;
		c_len = 0;
		blocks = 0;

		/* process blocks */
		if (len > 0 || opt_try_to_compress_0_bytes) do
		{
			int j;
			int r;

			const lzo_uint bl = len > opt_block_size ? opt_block_size : len;
			lzo_uint bl_overwrite = bl;
#if defined(__LZO_CHECKER)
			lzo_byte *dd = NULL;
			lzo_byte *b1 = NULL;
			lzo_byte *b2 = NULL;
			const lzo_uint b1_len = bl + bl / 64 + 16 + 3;
#else
			lzo_byte * const dd = d;
			lzo_byte * const b1 = block1;
			lzo_byte * const b2 = block2;
			const lzo_uint b1_len = sizeof(_block1) - 256;
			unsigned char random_byte;
			random_byte = (unsigned char) my_clock();
#endif

			blocks++;

			/* may overwrite 3 bytes past the end of the decompressed block */
			if (opt_use_asm_fast_decompressor)
				bl_overwrite += (lzo_uint) sizeof(int) - 1;

#if defined(__LZO_CHECKER)
			/* malloc a block of the exact size to detect any overrun */
			dd = malloc(bl_overwrite > 0 ? bl_overwrite : 1);
			b1 = malloc(b1_len);
			b2 = dd;
			if (dd == NULL || b1 == NULL)
			{
				perror("malloc");
				return EXIT_MEM;
			}
			if (bl > 0)
				memcpy(dd,d,bl);
#endif

		/* compress the block */
			c_len = c_len_max = 0;
			c_start = my_clock();
			for (j = r = 0; r == 0 && j < c_loops; j++)
			{
				c_len = b1_len;
				r = do_compress(c,dd,bl,b1,&c_len);
				if (r == 0 && c_len > c_len_max)
					c_len_max = c_len;
			}
			c_time += my_clock() - c_start;
			if (r != 0)
			{
				printf("  compression failed in block %d (%d) (%lu %lu)\n",
					blocks, r, (long)c_len, (long)bl);
				return EXIT_LZO_ERROR;
			}

		/* optimize the block */
			if (opt_optimize_compressed_data)
			{
				d_len = bl;
				r = do_optimize(c,b1,c_len,b2,&d_len);
				if (r != 0 || d_len != bl)
				{
					printf("  optimization failed in block %d (%d) "
						"(%lu %lu %lu)\n", blocks, r,
						(long)c_len, (long)d_len, (long)bl);
					return EXIT_LZO_ERROR;
				}
			}

#ifdef USE_DUMP
			/* dump compressed data to disk */
			if (dump)
			{
				lzo_fwrite(dump,b1,c_len);
				fflush(dump);
			}
#endif

		/* decompress the block and verify */
#if defined(__LZO_CHECKER)
			lzo_memset(b2,0,bl_overwrite);
#else
			init_mem_checker(b2,_block2,bl_overwrite,random_byte);
#endif
			d_start = my_clock();
			for (j = r = 0; r == 0 && j < d_loops; j++)
			{
				d_len = bl;
				r = do_decompress(c,decompress,b1,c_len,b2,&d_len);
				if (d_len != bl)
					break;
			}
			d_time += my_clock() - d_start;
			if (r != 0)
			{
				printf("  decompression failed in block %d (%d) "
					"(%lu %lu %lu)\n", blocks, r,
					(long)c_len, (long)d_len, (long)bl);
				return EXIT_LZO_ERROR;
			}
			if (d_len != bl)
			{
				printf("  decompression size error in block %d (%lu %lu %lu)\n",
					blocks, (long)c_len, (long)d_len, (long)bl);
				return EXIT_LZO_ERROR;
			}
			if (is_compressor(c))
			{
				if (lzo_memcmp(d,b2,bl) != 0)
				{
					lzo_uint x = 0;
					while (x < bl && b2[x] == d[x])
						x++;
					printf("  decompression data error in block %d at offset "
						"%lu (%lu %lu)\n", blocks, (long)x,
						(long)c_len, (long)d_len);
					if (opt_compute_adler32)
						printf("      checksum: 0x%08lx 0x%08lx\n",
							(long)adler_in, (long)adler_out);
#if 0
					printf("Orig:  ");
					r = (x >= 10) ? -10 : 0 - (int) x;
					for (j = r; j <= 10 && x + j < bl; j++)
						printf(" %02x", (int)d[x+j]);
					printf("\nDecomp:");
					for (j = r; j <= 10 && x + j < bl; j++)
						printf(" %02x", (int)b2[x+j]);
					printf("\n");
#endif
					return EXIT_LZO_ERROR;
				}
				if ((opt_compute_adler32 && adler_in != adler_out) ||
				    (opt_compute_crc32 && crc_in != crc_out))
				{
					printf("  checksum error in block %d (%lu %lu)\n",
						blocks, (long)c_len, (long)d_len);
					printf("      adler32: 0x%08lx 0x%08lx\n",
						(long)adler_in, (long)adler_out);
					printf("      crc32: 0x%08lx 0x%08lx\n",
						(long)crc_in, (long)crc_out);
					return EXIT_LZO_ERROR;
				}
			}

#if defined(__LZO_CHECKER)
			/* free in reverse order of allocations */
			free(b1);
			free(dd);
#else
			if (check_mem(b2,_block2,bl_overwrite,random_byte) != 0)
			{
				printf("  decompression overwrite error in block %d "
					"(%lu %lu %lu)\n",
					blocks, (long)c_len, (long)d_len, (long)bl);
				return EXIT_LZO_ERROR;
			}
#endif

			d += bl;
			len -= bl;
			compressed_len += c_len_max;
		}
		while (len > 0);
	}
	t_time += my_clock() - t_start;

#ifdef USE_DUMP
	if (dump)
		fclose(dump);
	opt_dump_compressed_data = NULL;	/* only dump the first file */
#endif

	print_stats(method_name, file_name,
	            t_loops, c_loops, d_loops,
	            t_time, c_time, d_time,
	            compressed_len, l, blocks);

	return EXIT_OK;
}




static
int do_file ( int method, const char *file_name,
              int c_loops, int d_loops,
              lzo_uint32 *p_adler, lzo_uint32 *p_crc )
{
	int r;
	const compress_t *c;
	lzo_decompress_t decompress;
	lzo_uint l;
	lzo_uint32 adler, crc;
	char method_name[32];
	const char *n;
	const int t_loops = 1;
	lzo_byte *saved_dict = dict;
	lzo_uint saved_dict_len = dict_len;

	adler_in = adler_out = 0;
	crc_in = crc_out = 0;
	if (p_adler)
		*p_adler = 0;
	if (p_crc)
		*p_crc = 0;

	c = info(method,NULL);
	if (c == NULL || c->name == NULL || c->compress == NULL)
		return EXIT_INTERNAL;
	decompress = get_decomp_info(c,&n);
	if (!decompress || n == NULL || WRK_LEN < c->mem_decompress)
		return EXIT_INTERNAL;
	strcpy(method_name,c->name);
	strcat(method_name,n);

	if (c_loops < 1)  c_loops = 1;
	if (d_loops < 1)  d_loops = 1;

	fflush(stdout); fflush(stderr);

	/* read the whole file */
	r = load_file(file_name,&l);
	if (r != 0)
		return r;

	/* compute some checksums */
	adler = lzo_adler32(0, NULL, 0);
	adler = lzo_adler32(adler, data, l);
	if (p_adler)
		*p_adler = adler;
	crc = lzo_crc32(0, NULL, 0);
	crc = lzo_crc32(crc, data, l);
	if (p_crc)
		*p_crc = crc;

#if 0 && defined(ALG_ZLIB)
	{
		uLong x;
		x = adler32(0, Z_NULL, 0);
		x = adler32(x, data, l);
		if (x != adler)
			return EXIT_LZO_ERROR;
		x = crc32(0, Z_NULL, 0);
		x = crc32(x, data, l);
		if (x != crc)
			return EXIT_LZO_ERROR;
	}
#endif

	if (opt_dict && dict)
		if (opt_max_dict_len <= 0 && dict_len > l)
		{
			if (opt_max_dict_len == -1)
				dict += dict_len - l;		/* use end of dictionary */
			dict_len = l;
		}

	if (opt_verbose >= 2)
	{
		printf("File %s: %lu bytes   (0x%08lx, 0x%08lx)\n",
		        file_name, (long) l, (long) adler, (long) crc);
		printf("  compressing %lu bytes (%d/%d/%d loops, %lu block-size)\n",
		        (long) l, t_loops, c_loops, d_loops, (long) opt_block_size);
		printf("  %s\n", method_name);
	}

	r = process_file(c, decompress, method_name, file_name, l,
	                 t_loops, c_loops, d_loops);

	dict = saved_dict;
	dict_len = saved_dict_len;

	return r;
}


/*************************************************************************
// Calgary Corpus test suite driver
**************************************************************************/

#ifdef USE_CORPUS

typedef struct
{
	const char *name;
	int loops;
	lzo_uint32 adler;
	lzo_uint32 crc;
}
corpus_t;

static const corpus_t calgary_corpus[] =
{
	{ "bib",       8,  0x4bd09e98L, 0xb856ebe8L },
	{ "book1",     1,  0xd4d3613eL, 0x24e19972L },
	{ "book2",     1,  0x6fe14cc3L, 0xba0f3f26L },
	{ "geo",       6,  0xf3cc5be0L, 0x4d3a6ed0L },
	{ "news",      2,  0x2ed405b8L, 0xcafac853L },
	{ "obj1",     35,  0x3887dd2cL, 0xc7b0cd26L },
	{ "obj2",      4,  0xf89407c4L, 0x3ae33007L },
	{ "paper1",   17,  0xfe65ce62L, 0x2b6baca0L },
	{ "paper2",   11,  0x1238b7c2L, 0xf76cba72L },
	{ "pic",       4,  0xf61a5702L, 0x4b17e59cL },
	{ "progc",    25,  0x4c00ba45L, 0x6fb16094L },
	{ "progl",    20,  0x4cba738eL, 0xddbf6baaL },
	{ "progp",    28,  0x7495b92bL, 0x493a1809L },
	{ "trans",    15,  0x52a2cec8L, 0xcdec06a6L },
	{ NULL,        0,  0x00000000L, 0x00000000L },
	{ "paper3",   30,  0x50b727a9L, 0x00000000L },
	{ "paper4",   30,  0xcb4a305fL, 0x00000000L },
	{ "paper5",   30,  0x2ca8a6f3L, 0x00000000L },
	{ "paper6",   30,  0x9ddbcfa4L, 0x00000000L },
	{ NULL,        0,  0x00000000L, 0x00000000L }
};


static
int do_corpus ( const corpus_t *corpus, int method, const char *path,
                int c_loops, int d_loops )
{
	size_t i, n;
	char name[256];

	if (path == NULL || strlen(path) >= sizeof(name) - 12)
		return EXIT_USAGE;

	strcpy(name,path);
	n = strlen(name);
	if (n > 0 && name[n-1] != '/' && name[n-1] != '\\' && name[n-1] != ':')
	{
		strcat(name,"/");
		n++;
	}

	for (i = 0; corpus[i].name != NULL; i++)
	{
		lzo_uint32 adler, crc;
		int c = c_loops * corpus[i].loops;
		int d = d_loops * corpus[i].loops;
		int r;

		strcpy(name+n,corpus[i].name);
		r = do_file(method, name, c, d, &adler, &crc);
		if (r != 0)
			return r;
		if (adler != corpus[i].adler)
		{
			printf("  invalid test suite\n");
			return EXIT_ADLER;
		}
		if (corpus[i].crc && crc != corpus[i].crc)
		{
			printf("  internal checksum error !!  (0x%08lx 0x%08lx)\n",
			        (long) crc, (long) corpus[i].crc);
			return EXIT_INTERNAL;
		}
	}
	return EXIT_OK;
}

#endif


/*************************************************************************
// usage
**************************************************************************/

static
void usage ( const char *name, int exit_code, lzo_bool show_methods )
{
	FILE *f;
	int i;

	f = stdout;

	fflush(stdout); fflush(stderr);

	fprintf(f,"Usage: %s [option..] file...\n", name);
	fprintf(f,"\n");
	fprintf(f,"Options:\n");
	fprintf(f,"  -m#     compression method\n");
	fprintf(f,"  -b#     set input block size (default %ld, max %ld)\n",
		(long) opt_block_size, (long) MAX_BLOCK_SIZE);
	fprintf(f,"  -n#     number of compression/decompression runs\n");
	fprintf(f,"  -c#     number of compression runs\n");
	fprintf(f,"  -d#     number of decompression runs\n");
	fprintf(f,"  -S      use safe decompressor (if available)\n");
	fprintf(f,"  -A      use assembler decompressor (if available)\n");
	fprintf(f,"  -F      use fast assembler decompressor (if available)\n");
	fprintf(f,"  -O      optimize compressed data (if available)\n");
#ifdef USE_CORPUS
	fprintf(f,"  -s DIR  process Calgary Corpus test suite in directory `DIR'\n");
#endif
	fprintf(f,"  -@      read list of files to compress from stdin\n");
	fprintf(f,"  -q      be quiet\n");
	fprintf(f,"  -Q      be very quiet\n");
	fprintf(f,"  -v      be verbose\n");
	fprintf(f,"  -L      display software license\n");

	if (show_methods)
	{
#if defined(my_clock_desc)
		fprintf(f,"\nAll timings are recorded using %s.\n",my_clock_desc);
#endif
		fprintf(f,"\n\n");
		fprintf(f,"The following compression methods are available:\n");
		fprintf(f,"\n");
		fprintf(f,"  usage   name           memory          available extras\n");
		fprintf(f,"  -----   ----           ------          ----------------\n");

		for (i = 0; i <= M_LAST_COMPRESSOR; i++)
		{
			const compress_t *c;
			c = info(i,NULL);
			if (c)
			{
				char n[16];
				static const char * const s[3] = {"          ", ", ", ""};
				int j = 0;
				unsigned long m = c->mem_compress;

				sprintf(n,"-m%d",i);
				fprintf(f,"  %-6s  %-12s",n,c->name);
#if 1
				fprintf(f,"%9ld", m);
#else
				m = (m + 1023) / 1024;
				fprintf(f,"%6ld kB", m);
#endif

				if (c->decompress_safe)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "safe");
				if (c->decompress_asm)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "asm");
				if (c->decompress_asm_safe)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "asm+safe");
				if (c->decompress_asm_fast)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "fastasm");
				if (c->decompress_asm_fast_safe)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "fastasm+safe");
				if (c->optimize)
					fprintf(f, "%s%s", j++ == 0 ? s[0] : s[1], "optimize");
				if (j > 0)
					fprintf(f, s[2]);
				fprintf(f,"\n");
			}
		}
	}
	else
	{
		fprintf(f,"\n");
		fprintf(f,"Type '%s -m' to list all available methods.\n", name);
	}

	fflush(f);
	if (exit_code < 0)
		exit_code = EXIT_USAGE;
	exit(exit_code);
}


static
void license(void)
{
	FILE *f;

	f = stdout;

	fflush(stdout); fflush(stderr);

fprintf(f,
"   The LZO library is free software; you can redistribute it and/or\n"
"   modify it under the terms of the GNU General Public License as\n"
"   published by the Free Software Foundation; either version 2 of\n"
"   the License, or (at your option) any later version.\n"
"\n"
"   The LZO library is distributed in the hope that it will be useful,\n"
"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"   GNU General Public License for more details.\n"
"\n"
"   You should have received a copy of the GNU General Public License\n"
"   along with the LZO library; see the file COPYING.\n"
"   If not, write to the Free Software Foundation, Inc.,\n"
"   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n"
"\n"
"   Markus F.X.J. Oberhumer\n"
"   <markus.oberhumer@jk.uni-linz.ac.at>\n"
"   http://www.oberhumer.com/opensource/lzo/\n"
"\n"
	);

	fflush(f);
	exit(EXIT_OK);
}


/*************************************************************************
// parse method option '-m'
**************************************************************************/

static int methods[256+1];
static int methods_n = 0;

static void add_method(int m)
{
	int i;

	if (m > 0)
	{
		if (!info(m,NULL))
			info(m,stdout);

		for (i = 0; i < methods_n; i++)
			if (methods[i] == m)
				return;

		if (methods_n >= 256)
		{
			fprintf(stderr,"%s: too many methods\n",argv0);
			exit(EXIT_USAGE);
		}

		methods[methods_n++] = m;
		methods[methods_n] = 0;
	}
}


static void add_methods(const int *ml)
{
	while (*ml != 0)
		add_method(*ml++);
}


static void add_all_methods(int first, int last)
{
	int m;

	for (m = first; m <= last; m++)
		if (info(m,NULL) != NULL)
			add_method(m);
}


static int m_strcmp(const char *a, const char *b)
{
	size_t n;

	if (a[0] == 0 || b[0] == 0)
		return 1;
	n = strlen(b);
	if (strncmp(a,b,n) == 0 && (a[n] == 0 || a[n] == ','))
		return 0;
	return 1;
}


static lzo_bool m_strisdigit(const char *s)
{
	for (;;)
	{
		if (!is_digit(*s))
			return 0;
		s++;
		if (*s == 0 || *s == ',')
			return 1;
	}
}


static void parse_methods(const char *p)
{
	const compress_t *c;

	for (;;)
	{
		if (p == NULL || p[0] == 0)
			usage(argv0,-1,1);
		else if ((c = find_method(p)) != NULL)
			add_method(c->id);
		else if (m_strcmp(p,"all") == 0 || m_strcmp(p,"avail") == 0)
			add_all_methods(1,M_LAST_COMPRESSOR);
		else if (m_strcmp(p,"ALL") == 0)
		{
			add_all_methods(1,M_LAST_COMPRESSOR);
			add_all_methods(9721,9729);
			add_all_methods(9781,9789);
		}
		else if (m_strcmp(p,"lzo") == 0)
			add_all_methods(1,M_MEMCPY);
		else if (m_strcmp(p,"bench") == 0)
			add_methods(benchmark_methods);
		else if (m_strcmp(p,"m1") == 0)
			add_methods(x1_methods);
		else if (m_strcmp(p,"m99") == 0)
			add_methods(x99_methods);
		else if (m_strcmp(p,"m999") == 0)
			add_methods(x999_methods);
		else if (m_strcmp(p,"1x999") == 0)
			add_all_methods(9721,9729);
		else if (m_strcmp(p,"1y999") == 0)
			add_all_methods(9821,9829);
#if defined(ALG_ZLIB)
		else if (m_strcmp(p,"zlib") == 0)
			add_all_methods(M_ZLIB_8_1,M_ZLIB_8_9);
#endif
#if defined(MFX)
#  include "maint/t_opt_m.ch"
#endif
		else if (m_strisdigit(p))
			add_method(atoi(p));
		else
		{
			printf("%s: invalid method '%s'\n\n",argv0,p);
			exit(EXIT_USAGE);
		}

		while (*p && *p != ',')
			p++;
		while (*p == ',')
			p++;
		if (*p == 0)
			return;
	}
}


/*************************************************************************
// options
**************************************************************************/

enum {
	OPT_LONGOPT_ONLY = 512,
	OPT_ADLER32,
	OPT_CALGARY_CORPUS,
	OPT_CRC32,
	OPT_DICT,
	OPT_DUMP,
	OPT_EXECUTION_TIME,
	OPT_MAX_DATA_LEN,
	OPT_MAX_DICT_LEN,
	OPT_QUERY,
	OPT_UNUSED
};

static const struct mfx_option longopts[] =
{
 /* { name  has_arg  *flag  val } */
    {"help",             0, 0, 'h'+256}, /* give help */
    {"license",          0, 0, 'L'},     /* display software license */
    {"quiet",            0, 0, 'q'},     /* quiet mode */
    {"verbose",          0, 0, 'v'},     /* verbose mode */
    {"version",          0, 0, 'V'+256}, /* display version number */

    {"adler32",          0, 0, OPT_ADLER32},
#ifdef USE_CORPUS
    {"calgary-corpus",   1, 0, OPT_CALGARY_CORPUS},
    {"corpus",           1, 0, OPT_CALGARY_CORPUS},
#endif
    {"crc32",            0, 0, OPT_CRC32},
#ifdef USE_DICT
    {"dict",             1, 0, OPT_DICT},
#endif
#ifdef USE_DUMP
    {"dump-compressed",  1, 0, OPT_DUMP},
#endif
    {"execution-time",   0, 0, OPT_EXECUTION_TIME},
    {"max-data-length",  1, 0, OPT_MAX_DATA_LEN},
#ifdef USE_DICT
    {"max-dict-length",  1, 0, OPT_MAX_DICT_LEN},
#endif
    {"methods",          1, 0, 'm'},
    {"query",            0, 0, OPT_QUERY},
    {"totals",           0, 0, 'T'},

    { 0, 0, 0, 0 }
};


static int do_option(int optc)
{
	switch (optc)
	{
	case 'A':
		opt_use_asm_decompressor = 1;
		break;
	case 'b':
		opt_block_size = MAX_BLOCK_SIZE;
		if (mfx_optarg)
		{
			if (!mfx_optarg || !is_digit(mfx_optarg[0]))
				return optc;
			opt_block_size = atoi(mfx_optarg);
		}
		break;
	case 'c':
	case 'C':
		if (!mfx_optarg || !is_digit(mfx_optarg[0]))
			return optc;
		opt_c_loops = atoi(mfx_optarg);
		break;
	case 'd':
	case 'D':
		if (!mfx_optarg || !is_digit(mfx_optarg[0]))
			return optc;
		opt_d_loops = atoi(mfx_optarg);
		break;
	case 'F':
		opt_use_asm_fast_decompressor = 1;
		break;
	case 'h':
	case 'H':
	case '?':
	case 'h'+256:
		usage(argv0,EXIT_OK,0);
		break;
	case 'L':
		license();
		break;
	case 'm':
		parse_methods(mfx_optarg);
		break;
	case 'n':
		if (!mfx_optarg || !is_digit(mfx_optarg[0]))
			return optc;
		opt_c_loops = opt_d_loops = atoi(mfx_optarg);
		break;
	case 'O':
		opt_optimize_compressed_data = 1;
		break;
	case 'q':
		opt_verbose = (opt_verbose > 1) ? opt_verbose - 1 : 1;
		break;
	case 'Q':
		opt_verbose = 0;
		break;
#ifdef USE_CORPUS
	case 's':
	case OPT_CALGARY_CORPUS:
		if (!mfx_optarg || !mfx_optarg[0])
			return optc;
		opt_calgary_corpus_path = mfx_optarg;
		break;
#endif
	case 'S':
		opt_use_safe_decompressor = 1;
		break;
	case 'T':
		opt_totals = 1;
		break;
	case 'v':
		opt_verbose = (opt_verbose < 2) ? 2 : opt_verbose + 1;
		break;
	case 'V':
	case 'V'+256:
		exit(EXIT_OK);
		break;
	case '@':
		opt_read_from_stdin = 1;
		break;

	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		/* this is a dirty hack... */
		parse_methods(nextchar-1);
		if (nextchar[0])
		{
			nextchar = NULL;
			mfx_optind++;
		}
		break;

	case OPT_ADLER32:
		opt_compute_adler32 = 1;
		break;
	case OPT_CRC32:
		opt_compute_crc32 = 1;
		break;
#ifdef USE_DICT
	case OPT_DICT:
		opt_dict = 1;
		opt_dictionary_file = mfx_optarg;
		break;
#endif
	case OPT_EXECUTION_TIME:
		opt_execution_time = 1;
		break;
#ifdef USE_DUMP
	case OPT_DUMP:
		opt_dump_compressed_data = mfx_optarg;
		break;
#endif
	case OPT_MAX_DATA_LEN:
		if (!mfx_optarg || !is_digit(mfx_optarg[0]))
			return optc;
		opt_max_data_len = atoi(mfx_optarg);
		break;
#ifdef USE_DICT
	case OPT_MAX_DICT_LEN:
		if (!mfx_optarg || !(is_digit(mfx_optarg[0]) || mfx_optarg[0] == '-'))
			return optc;
		opt_max_dict_len = atoi(mfx_optarg);
		break;
#endif
	case OPT_QUERY:
		opt_query = 1;
		break;

	case '\0':
		return -1;
	case ':':
		return -2;
	default:
		fprintf(stderr,"%s: internal error in getopt (%d)\n",argv0,optc);
		return -3;
	}
	return 0;
}


static int get_options(int argc, char **argv)
{
	int optc;

	mfx_optind = 0;
	mfx_opterr = 1;
	while ((optc = mfx_getopt_long (argc, argv,
#ifdef USE_CORPUS
	                  "Ab::c:C:d:D:FhHLm::n:OqQs:STvV@123456789",
#else
	                  "Ab::c:C:d:D:FhHLm::n:OqQSTvV@123456789",
#endif
	                  longopts, (int *)0)) >= 0)
	{
		if (do_option(optc) != 0)
			exit(EXIT_USAGE);
	}

	return mfx_optind;
}


/*************************************************************************
// main
**************************************************************************/

int main(int argc, char *argv[])
{
	int r = EXIT_OK;
	int i, ii;
	int m;
	time_t t_total;

#if defined(__EMX__)
	_response(&argc,&argv);
	_wildcard(&argc,&argv);
#endif

	if (argv[0])
		argv0 = argv[0];
	align_mem();
	(void) my_clock();

	printf("\nLZO real-time data compression library (v%s, %s).\n",
	        LZO_VERSION_STRING, LZO_VERSION_DATE);
	printf("Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer\n\n");

	if (lzo_init() != LZO_E_OK)
	{
		printf("lzo_init() failed !!!\n");
		exit(EXIT_LZO_INIT);
	}

	if (argc < 2)
		usage(argv0,-1,0);
	i = get_options(argc,argv);

	if (methods_n == 0)
		add_method(default_method);
	if (methods_n > 1 && opt_read_from_stdin)
	{
		printf("%s: cannot use multiple methods and '-@'\n", argv0);
		exit(EXIT_USAGE);
	}

	if (opt_block_size < 16)
		opt_block_size = 16;
	if (opt_block_size > MAX_BLOCK_SIZE)
		opt_block_size = MAX_BLOCK_SIZE;

	dict_len = 0;
#ifndef USE_DICT
	opt_dict = 0;
#else
	if (opt_dict)
	{
		opt_optimize_compressed_data = 0;
		if (opt_dictionary_file)
		{
			read_dict(opt_dictionary_file);
			if (opt_max_dict_len > 0 && dict_len > (lzo_uint) opt_max_dict_len)
				dict_len = opt_max_dict_len;
			if (dict_len > 0)
				printf("Using dictionary '%s', %ld bytes, ID 0x%08lx.\n",
				        opt_dictionary_file,
			            (long) dict_len, (long) dict_adler32);
		}
		if (dict_len <= 0)
		{
			init_default_dict();
			if (opt_max_dict_len > 0 && dict_len > (lzo_uint) opt_max_dict_len)
				dict_len = opt_max_dict_len;
			printf("Using default dictionary, %ld bytes, ID 0x%08lx.\n",
			        (long) dict_len, (long) dict_adler32);
		}
		if (opt_max_dict_len == -1)
			printf("Dictionary size will be adjusted to file size.\n");
		else if (opt_max_dict_len <= 0)
			printf("Dictionary size will be adjusted to file size.\n");
	}
#endif

	t_total = time(NULL);
	(void) my_clock();
	ii = i;
	for (m = 0; m < methods_n && r == EXIT_OK; m++)
	{
		int method = methods[m];

		i = ii;
		if (i >= argc && opt_calgary_corpus_path == NULL && !opt_read_from_stdin)
			usage(argv0,-1,0);
		if (m == 0 && opt_verbose >= 1)
			printf("%lu block-size\n\n", (long) opt_block_size);

		if (!info(method,NULL))
			info(method,stdout);

#ifdef USE_CORPUS
		if (opt_calgary_corpus_path != NULL)
			r = do_corpus(calgary_corpus,method,opt_calgary_corpus_path,
			              opt_c_loops,opt_d_loops);
		else
#endif
		{
			for ( ; i < argc && r == EXIT_OK; i++)
			{
				r = do_file(method,argv[i],opt_c_loops,opt_d_loops,NULL,NULL);
				if (r == EXIT_FILE)		/* ignore file errors */
					r = EXIT_OK;
			}
			if (opt_read_from_stdin)
			{
				char buf[512], *p;

				while (r == EXIT_OK && fgets(buf,sizeof(buf)-1,stdin) != NULL)
				{
					buf[sizeof(buf)-1] = 0;
					p = buf + strlen(buf);
					while (p > buf && is_space(p[-1]))
							*--p = 0;
					p = buf;
					while (*p && is_space(*p))
						p++;
					if (*p)
						r = do_file(method,p,opt_c_loops,opt_d_loops,NULL,NULL);
					if (r == EXIT_FILE)		/* ignore file errors */
						r = EXIT_OK;
				}
				opt_read_from_stdin = 0;
			}
		}
	}
	t_total = time(NULL) - t_total;

	if (opt_totals)
		print_totals();
	if (opt_execution_time || (methods_n > 1 && opt_verbose >= 1))
		printf("\n%s: execution time: %lu seconds\n", argv0, (long) t_total);
	if (r != EXIT_OK)
		printf("\n%s: exit code: %d\n", argv0, r);

	return r;
}


/*
vi:ts=4:et
*/

