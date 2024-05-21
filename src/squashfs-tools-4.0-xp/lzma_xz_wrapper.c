/*
 * Copyright (c) 2010, 2013
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
 * lzma_xz_wrapper.c
 *
 * Support for LZMA1 compression using XZ Utils liblzma http://tukaani.org/xz/
 */

#include <stdio.h>
#include <string.h>
#include <lzma.h>

#include "squashfs_fs.h"
#include "compressor.h"
#include "lzma_xz_options.h"

#define LZMA_PROPS_SIZE 5
#define LZMA_UNCOMP_SIZE 8
#define LZMA_HEADER_SIZE (LZMA_PROPS_SIZE + LZMA_UNCOMP_SIZE)

#define LZMA_OPTIONS 5
#define MEMLIMIT (32 * 1024 * 1024)

static int lzma_compress_2(void *dest, void *src,  int size,
	int block_size, int *error,int lc, int lp, int pb)
{
	uint32_t preset;
	unsigned char *d = (unsigned char *) dest;
	struct lzma_xz_options *opts = lzma_xz_get_options();

	lzma_options_lzma opt;
	lzma_stream strm = LZMA_STREAM_INIT;
	int res;

	preset = opts->preset;

	preset |= LZMA_PRESET_EXTREME;

	lzma_lzma_preset(&opt, opts->preset);
	opt.lc = lc;
	opt.lp = lp;
	opt.pb = pb;
//	opt.nice_len = 64;
	opt.dict_size = opts->dict_size;

	res = lzma_alone_encoder(&strm, &opt);
	if(res != LZMA_OK) {
		lzma_end(&strm);
		goto failed;
	}

	strm.next_out = dest;
	strm.avail_out = block_size;
	strm.next_in = src;
	strm.avail_in = size;

	res = lzma_code(&strm, LZMA_FINISH);
	lzma_end(&strm);

	if(res == LZMA_STREAM_END) {
		/*
	 	 * Fill in the 8 byte little endian uncompressed size field in
		 * the LZMA header.  8 bytes is excessively large for squashfs
		 * but this is the standard LZMA header and which is expected by
		 * the kernel code
	 	 */

		d[LZMA_PROPS_SIZE] = size & 255;
		d[LZMA_PROPS_SIZE + 1] = (size >> 8) & 255;
		d[LZMA_PROPS_SIZE + 2] = (size >> 16) & 255;
		d[LZMA_PROPS_SIZE + 3] = (size >> 24) & 255;
		d[LZMA_PROPS_SIZE + 4] = lc;
		d[LZMA_PROPS_SIZE + 5] = lp;
		d[LZMA_PROPS_SIZE + 6] = pb;
		d[LZMA_PROPS_SIZE + 7] = opt.nice_len;

		return (int) strm.total_out;
	}

	if(res == LZMA_OK)
		/*
	 	 * Output buffer overflow.  Return out of buffer space
	 	 */
		return 0;

failed:
	/*
	 * All other errors return failure, with the compressor
	 * specific error code in *error
	 */
	*error = res;
	return -1;
}



#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

unsigned char pbmatrix[3] = { 0, 1, 2 };
unsigned char lcmatrix[4] = { 0, 1, 2, 3 };
unsigned char lpmatrix[4] = { 0, 1, 2, 3 };

struct MATRIXENTRY {
	int pb;
	int lc;
	int lp;
};

struct MATRIXENTRY matrix[] = {
	{2, 0, 0},
	{2, 0, 1},
	{2, 0, 2},
	{2, 1, 0},
	{2, 1, 2},
	{2, 2, 0},
	{2, 3, 0},
	{0, 2, 0},
	{0, 2, 1},
	{0, 3, 0},
	{0, 0, 0},
	{0, 0, 2},
	{1, 0, 1},
	{1, 2, 0},
	{1, 3, 0}
};

int pbsave = -1;
int lcsave = -1;
int lpsave = -1;
int s_avail;

int testlevel;
//int testfb;
static pthread_spinlock_t pos_mutex;
static pthread_spinlock_t p_mutex;
unsigned char *test1;
unsigned char *testsource;


typedef unsigned long uLongf;

uLongf test2len;
uLongf test1len;
uLongf testsourcelen;
int running = 0;
void *brute(void *arg)
{
	int oldstate;
	uLongf test3len = test2len;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
	int *count = (int *)arg;
	int testcount = count[0];
	int takelcvalue = matrix[testcount].lc;
	int takepbvalue = matrix[testcount].pb;
	int takelpvalue = matrix[testcount].lp;
	unsigned char *test2 = (unsigned char *) malloc(test2len * 2);
//fprintf(stderr,"try method [pb:%d lc:%d lp:%d]\n",takepbvalue,takelcvalue,takelpvalue);

	//static int lzma_compress_2(void *dest, void *src,  int size,int block_size, int *error,int lc, int lp, int pb)

	int error;
	test3len = lzma_compress_2(test2, testsource, testsourcelen, s_avail, &error, takelcvalue, takelpvalue, takepbvalue);
//fprintf(stderr,"test return %d\n",test3len);
	pthread_spin_lock(&pos_mutex);
	if (test3len < test1len) {
		test1len = test3len;
		memcpy(test1, test2, test3len);
		pbsave = takepbvalue;
		lcsave = takelcvalue;
		lpsave = takelpvalue;
	}
	pthread_spin_unlock(&pos_mutex);
//fprintf(stderr,"finished %d running\n",running);
	free(test2);
	running = 0;
	return NULL;
}
int count=0;

//int (*compress)(void *, void *, void *, int, int, int *);
///	int (*uncompress)(void *, void *, int, int, int *);
//static int lzma_compress(char *dummy, void *dest, void *src,  int size,
//	int block_size, int *error)
static int lzma_compress(void *dummy, void *dst,
			      void *src, int sourceLen,
			      int block_size, int *error, int special)
{
//printf("in %s %d\n",__func__,count);

	pthread_spin_lock(&p_mutex);
	count++;
	unsigned char *source = (unsigned char*)src;
	unsigned char *dest = (unsigned char*)dst;
	int i, a;
	pthread_t *thread;
	cpu_set_t cpuset[4];
	test1 = (unsigned char *) malloc(block_size);
	test1len = block_size*2;
	test2len = block_size;
	testsource = source;
	//testfb = fb;
	testsourcelen = sourceLen;
	//testlevel = level;
	s_avail = block_size;
	
	pthread_spin_init(&pos_mutex, 0);
	if ((thread = (pthread_t *) malloc((16) * sizeof(pthread_t))) == NULL)
		fprintf(stderr,
			"Out of memory allocating thread descriptors\n");
//for (a=0;a<2;a++)
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64 * 1024);

	int threadcount = 0;
#define MAX_THREADS 8
	int *argument = (int *)malloc(16 * 4);
	for (a = 0; a < 15 / MAX_THREADS; a++) {
		for (i = 0; i < MAX_THREADS; i++) {
			argument[i] = threadcount++;
//              fprintf(stderr,"start thread %d\n",threadcount);
			if (pthread_create
			    (&thread[i], &attr, brute, &argument[i]) != 0)
				fprintf(stderr, "Failed to create thread\n");
		}
		for (i = 0; i < MAX_THREADS; i++) {
			pthread_join(thread[i], NULL);
		}
	}

	for (i = 0; i < 15 % MAX_THREADS; i++) {
		argument[i] = threadcount++;
//              fprintf(stderr,"start thread %d\n",threadcount);
		if (pthread_create(&thread[i], NULL, brute, &argument[i]) != 0)
			fprintf(stderr, "Failed to create thread\n");
	}
	for (i = 0; i < 15 % MAX_THREADS; i++) {
		pthread_join(thread[i], NULL);
	}
	free(argument);
	pthread_attr_destroy(&attr);
//	fprintf(stderr, "use method [pb:%d lc:%d lp:%d] (len %d/%d)\n",
//		pbsave, lcsave, lpsave, test1len,block_size);
	memcpy(dest, test1, test1len);
	//dest[0] = pbsave;
	//dest[1] = lcsave;
	//dest[2] = lpsave;
	//dest[3] = fb;
	
	//*destLen = test1len //+ 4;
	free(thread);
	pthread_spin_destroy(&pos_mutex);
	free(test1);
	count--;
	pthread_spin_unlock(&p_mutex);
//printf("out %s %d\n",__func__,count);
	return test1len;
}



static int lzma_uncompress(void *dest, void *src, int size, int outsize,
	int *error)
{
	lzma_stream strm = LZMA_STREAM_INIT;
	int uncompressed_size = 0, res;
	unsigned char lzma_header[LZMA_HEADER_SIZE];
	fprintf(stderr, "uncompress\n");
	res = lzma_alone_decoder(&strm, MEMLIMIT);
	if(res != LZMA_OK) {
		lzma_end(&strm);
		goto failed;
	}

	memcpy(lzma_header, src, LZMA_HEADER_SIZE);
	uncompressed_size = lzma_header[LZMA_PROPS_SIZE] |
		(lzma_header[LZMA_PROPS_SIZE + 1] << 8) |
		(lzma_header[LZMA_PROPS_SIZE + 2] << 16) |
		(lzma_header[LZMA_PROPS_SIZE + 3] << 24);

	if(uncompressed_size > outsize) {
		res = 0;
		goto failed;
	}

	memset(lzma_header + LZMA_PROPS_SIZE, 255, LZMA_UNCOMP_SIZE);

	strm.next_out = dest;
	strm.avail_out = outsize;
	strm.next_in = lzma_header;
	strm.avail_in = LZMA_HEADER_SIZE;

	res = lzma_code(&strm, LZMA_RUN);

	if(res != LZMA_OK || strm.avail_in != 0) {
		lzma_end(&strm);
		goto failed;
	}

	strm.next_in = src + LZMA_HEADER_SIZE;
	strm.avail_in = size - LZMA_HEADER_SIZE;

	res = lzma_code(&strm, LZMA_FINISH);
	lzma_end(&strm);

	if(res == LZMA_STREAM_END || (res == LZMA_OK &&
		strm.total_out >= uncompressed_size && strm.avail_in == 0))
		return uncompressed_size;

failed:
	*error = res;
	return -1;
}

static int lzma_options(char *argv[], int argc)
{
	return lzma_xz_options(argv, argc, LZMA_OPT_LZMA);
}


static int lzma_options_post(int block_size)
{
	return lzma_xz_options_post(block_size, LZMA_OPT_LZMA);
}


static void *lzma_dump_options(int block_size, int *size)
{
	return lzma_xz_dump_options(block_size, size, 0);
}


static int lzma_extract_options(int block_size, void *buffer, int size)
{
	return lzma_xz_extract_options(block_size, buffer, size, LZMA_OPT_LZMA);
}


void lzma_usage()
{
	lzma_xz_usage(LZMA_OPT_LZMA);
}


int lzma_init(void **dummy, int a, int b)
{
	static int hasinit = 0;
	if (hasinit)
	    return 0;
	hasinit=1;
	
	pthread_spin_init(&p_mutex, 0);
	return 0;
}

int lzma_deinit(void)
{
	pthread_spin_destroy(&p_mutex);
	return 0;
}

struct compressor lzma_comp_ops = {
	.init = lzma_init,
	.deinit = lzma_deinit,
	.compress = lzma_compress,
	.uncompress = lzma_uncompress,
	.options = lzma_options,
	.options_post = lzma_options_post,
	.dump_options = lzma_dump_options,
	.extract_options = lzma_extract_options,
	.usage = lzma_usage,
	.id = LZMA_COMPRESSION,
	.name = "lzma",
	.supported = 1
};

