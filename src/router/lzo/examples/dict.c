/* dict.c -- example program: how to use preset dictionaries

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


/*************************************************************************
// This program shows how to use preset dictionaries.
//
// Please study LZO.FAQ and simple.c first.
**************************************************************************/

#include <lzo1x.h>
#include "lutil.h"
#include <time.h>

static const char *progname = NULL;

#define DICT_LEN	0xbfff
static lzo_byte		dict [ DICT_LEN ];
static lzo_uint		dict_len = 0;
static lzo_uint32	dict_adler32;


/*************************************************************************
//
**************************************************************************/

static lzo_uint total_n = 0;
static lzo_uint total_c_len = 0;
static lzo_uint total_d_len = 0;

static void print_file ( const char *name, lzo_uint d_len, lzo_uint c_len )
{
	double perc;

	perc = (d_len > 0) ? c_len * 100.0 / d_len : 0;
	printf("  |  %-30s   %9ld -> %9ld   %7.2f%%  |\n",
	        name, (long) d_len, (long) c_len, perc);

	total_n++;
	total_c_len += c_len;
	total_d_len += d_len;
}


/*************************************************************************
//
**************************************************************************/

int do_file ( const char *in_name, int level )
{
	int r;
	lzo_byte *in;
	lzo_byte *out;
	lzo_byte *newb;
	lzo_byte *wrkmem;
	lzo_uint in_len;
	lzo_uint out_len;
	lzo_uint new_len;
	long l;
	FILE *f;

/*
 * Step 1: open the input file
 */
 	f = fopen(in_name,"rb");
	if (f == 0)
	{
		printf("%s: cannot open file %s\n", progname, in_name);
		return 0;	/* no error */
	}
	fseek(f,0,SEEK_END);
	l = ftell(f);
	fseek(f,0,SEEK_SET);
	if (l <= 0)
	{
		printf("%s: %s: empty file -- skipping\n", progname, in_name);
		fclose(f);
		return 0;
	}
	in_len = (lzo_uint) l;

/*
 * Step 2: allocate compression buffers and read the file
 */
	in = (lzo_bytep) lzo_malloc(in_len);
	out = (lzo_bytep) lzo_malloc(in_len + in_len / 64 + 16 + 3);
	newb = (lzo_bytep) lzo_malloc(in_len);
	wrkmem = (lzo_bytep) lzo_malloc(LZO1X_999_MEM_COMPRESS);
	if (in == NULL || out == NULL || newb == NULL || wrkmem == NULL)
	{
		printf("%s: out of memory\n", progname);
		exit(1);
	}
	in_len = lzo_fread(f,in,in_len);
	fclose(f);

/*
 * Step 3: compress from `in' to `out' with LZO1X-999
 */
	r = lzo1x_999_compress_level(in,in_len,out,&out_len,wrkmem,
	                             dict, dict_len, 0, level);
	if (r != LZO_E_OK)
	{
		/* this should NEVER happen */
		printf("internal error - compression failed: %d\n", r);
		return 1;
	}

	print_file(in_name,in_len,out_len);

/*
 * Step 4: decompress again, now going from `out' to `newb'
 */
 	new_len = in_len;
	r = lzo1x_decompress_dict_safe(out,out_len,newb,&new_len,NULL,dict,dict_len);
	if (r != LZO_E_OK)
	{
		/* this should NEVER happen */
		printf("internal error - decompression failed: %d\n", r);
		return 1;
	}

/*
 * Step 5: verify decompression
 */
	if (new_len != in_len || lzo_memcmp(in,newb,in_len) != 0)
	{
		/* this should NEVER happen */
		printf("internal error - decompression data error\n");
		return 1;
	}

	/* free buffers in reverse order to help malloc() */
	lzo_free(wrkmem);
	lzo_free(newb);
	lzo_free(out);
	lzo_free(in);
	return 0;
}


/*************************************************************************
//
**************************************************************************/

int main(int argc, char *argv[])
{
	int i = 1;
	int r;
	const char *dict_name;
	FILE *f;
	time_t t_total;
	int level = 7;

#if defined(__EMX__)
	_response(&argc,&argv);
	_wildcard(&argc,&argv);
#endif

	printf("\nLZO real-time data compression library (v%s, %s).\n",
	        lzo_version_string(), lzo_version_date());
	printf("Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer\n\n");

	progname = argv[0];

	if (i < argc && argv[i][0] == '-' && isdigit(argv[i][1]))
		level = atoi(&argv[i++][1]);

	if (i + 1 >= argc || level < 1 || level > 9)
	{
		printf("usage: %s [-level] [ dictionary-file | -n ]  file...\n", progname);
		exit(1);
	}
	printf("Compression level is LZO1X-999/%d\n", level);

/*
 * Step 1: initialize the LZO library
 */
	if (lzo_init() != LZO_E_OK)
	{
		printf("lzo_init() failed !!!\n");
		exit(1);
	}

/*
 * Step 2: prepare the dictionary
 */
 	dict_name = argv[i++];
	if (strcmp(dict_name,"-n") == 0)
	{
		dict_name = "empty";
		dict_len = 0;
	}
	else
	{
 		f = fopen(dict_name,"rb");
		if (!f)
		{
			printf("%s: cannot open dictionary file %s\n", progname, dict_name);
			exit(1);
		}
		dict_len = lzo_fread(f,dict,DICT_LEN);
		fclose(f);
	}

	dict_adler32 = lzo_adler32(0,NULL,0);
	dict_adler32 = lzo_adler32(dict_adler32,dict,dict_len);
	printf("Using dictionary '%s', %ld bytes, ID 0x%08lx.\n",
	        dict_name, (long) dict_len, (long) dict_adler32);

/*
 * Step 3: process files
 */
	t_total = time(NULL);
	for (r = 0; r == 0 && i < argc; i++)
		r = do_file(argv[i], level);
	t_total = time(NULL) - t_total;


	if (total_n > 1)
		print_file("***TOTALS***",total_d_len,total_c_len);

	printf("Dictionary compression test %s, execution time %lu seconds.\n",
	        r == 0 ? "passed" : "FAILED", (long) t_total);
	return r;
}

/*
vi:ts=4
*/

