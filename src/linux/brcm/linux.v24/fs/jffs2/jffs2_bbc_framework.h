/*
 * JFFS2-BBC: Compression Framework - headers
 *
 * $Id: jffs2_bbc_framework.h,v 1.1 2005/09/01 11:59:15 seg Exp $
 *
 * Copyright (C) 2004, Ferenc Havasi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __JFFS2_BBC_FRAMEWORK_H__

#define __JFFS2_BBC_FRAMEWORK_H__

#define JFFS2_BBC_VERSION "0.54.3"

#define JFFS2_BBC_CONFIG_FILE "bbc.conf"

/********************************************************************* 
 *               Compression mode handling                           *
 *********************************************************************/

#define JFFS2_BBC_ZLIB_MODE            1
#define JFFS2_BBC_SIZE_MODE            2
#define JFFS2_BBC_FASTR_MODE           3
#define JFFS2_BBC_FASTW_MODE           4
#define JFFS2_BBC_FASTS_MODE           5
#define JFFS2_BBC_MANUAL_MODE          6
#define JFFS2_BBC_DUMMY_MODE           7

int jffs2_bbc_get_compression_mode(void);
void jffs2_bbc_set_compression_mode(int mode);

/********************************************************************* 
 *                  Read/write speed unit                            *
 *         everything is relative to the speed of zlib               * 
 *             bigger number means slower speed!                     *
 *********************************************************************/
 
#define JFFS2_BBC_ZLIB_READ_TIME  	10000
#define JFFS2_BBC_ZLIB_WRITE_TIME 	10000

/********************************************************************* 
 *                  Compressor handling                              *
 *********************************************************************/

struct jffs2_bbc_compressor_type
{
	char name[16];
	int model_file_sign;	/* 0 for no model file needed */
	char block_sign[4];	/* only nomodel compressors, and only the first 2 _bytes are used! */
	int (*init)(void);
	int (*init_model)(void **model);
	void (*destroy_model)(void **model);
	void (*deinit)(void);
	/* Compress block
	 *   *dstlen bytes are allocated.
	 *   if it is not enough write *sourcelen over to the processed amount of data
	 *   returns non zero if fails
	 */
	int (*compress)(void *model, unsigned char *input, unsigned char *output, unsigned long *sourcelen, unsigned long *dstlen);
	int (*estimate)(void *model, unsigned char *input, unsigned long sourcelen, 
	                unsigned long *dstlen, unsigned long *readtime, unsigned long *writetime);
	/* Decompress block 
	 *   returns non zero if fails
	 */
	int (*decompress)(void *model, unsigned char *input, unsigned char *output, unsigned long sourcelen, unsigned long dstlen);
	char *(*proc_info)(void);
	int (*proc_command)(char *command);
	int enabled;		/* filled by BBC */
	int mounted;		/* filled by BBC */
	void *models;		/* filled by BBC */
	char *buffer;		/* filled by BBC */
	int buffer_size;	/* filled by BBC */
	int buffer_cnt;		/* filled by BBC */
	int buffer_tmp;		/* filled by BBC */
	int stat_compr_orig;	/* filled by BBC */
	int stat_compr_new;	/* filled by BBC */
	int stat_decompr;	/* filled by BBC */
	struct jffs2_bbc_compressor_type *next;	/* filled by BBC */
};

/* It sets the compression mode to JFFS2_BBC_MANUAL_MODE */

void jffs2_bbc_set_manual_compressor(struct jffs2_bbc_compressor_type *c);	/* NULL = ZLIB */
int jffs2_bbc_set_manual_compressor_by_name(char *name);
int jffs2_bbc_disable_compressor_by_name(char *name);
int jffs2_bbc_enable_compressor_by_name(char *name);
void jffs2_bbc_compressor_command_by_name(char *name_and_command);

/* If the compression mode is JFFS2_BCC_MANUAL_MODE the manually setted
   compressor can be get using it. Otherwise it returns with NULL. */

struct jffs2_bbc_compressor_type *jffs2_bbc_get_manual_compressor(void);

struct jffs2_bbc_model_list_node
{
	void *sb;		/* FS idendifier (JFFS2_SB_INFO(sb) at this moment) */
	void *model;		/* model data */
	int sign;		/* sign of the model (first 4 bytes) */
	char block_sign[4];	/* block sign - only the first 2 bytes are used! */
	int inode;		/* inode number of the model file */
	int stat_decompr;
	struct jffs2_bbc_compressor_type *compressor;
	struct jffs2_bbc_model_list_node *next_model;
	struct jffs2_bbc_model_list_node *next_compr_model;
};

struct jffs2_bbc_compressor_type *jffs2_bbc_get_compressor_list(void);
struct jffs2_bbc_model_list_node *jffs2_bbc_get_model_list(void);

int jffs2_bbc_register_compressor(struct jffs2_bbc_compressor_type *c);
int jffs2_bbc_unregister_compressor(struct jffs2_bbc_compressor_type *c);

int jffs2_bbc_model_new(void *sb, int i_num, void *model);
void jffs2_bbc_model_del(void *sb);
void jffs2_bbc_model_set_act_sb(void *sb);
void *jffs2_bbc_model_get_act_sb(void);
void *jffs2_bbc_model_get_newest(struct jffs2_bbc_compressor_type *compressor);
 
/********************************************************************* 
 *                 Compressor init function                          *
 *********************************************************************/

void jffs2_bbc_compressor_init(void);
void jffs2_bbc_compressor_deinit(void);

/********************************************************************* 
 *                        Statistics                                 *
 *********************************************************************/

char *jffs2_bbc_get_compr_stats(void);
char *jffs2_bbc_get_model_stats(void);

/********************************************************************* 
 *                          Other                                    *
 *********************************************************************/


void jffs2_bbc_print_flush(void);

#ifdef __KERNEL__
#include <linux/kernel.h>
#define jffs2_bbc_print1(a) printk(a)
#define jffs2_bbc_print2(a,b) printk(a,b)
#define jffs2_bbc_print3(a,b,c) printk(a,b,c)
#define jffs2_bbc_print4(a,b,c,d) printk(a,b,c,d)
#define jffs2_bbc_print5(a,b,c,d,e) printk(a,b,c,d,e)
#define jffs2_bbc_print6(a,b,c,d,e,f) printk(a,b,c,d,e,f)
#define jffs2_bbc_print7(a,b,c,d,e,f,g) printk(a,b,c,d,e,f,g)
#define jffs2_bbc_print8(a,b,c,d,e,f,g,h) printk(a,b,c,d,e,f,g,h)
#define jffs2_bbc_print9(a,b,c,d,e,f,g,h,i) printk(a,b,c,d,e,f,g,h,i)
#else
#include <stdio.h>
#define jffs2_bbc_print1(a) fprintf(stderr,a)
#define jffs2_bbc_print2(a,b) fprintf(stderr,a,b)
#define jffs2_bbc_print3(a,b,c) fprintf(stderr,a,b,c)
#define jffs2_bbc_print4(a,b,c,d) fprintf(stderr,a,b,c,d)
#define jffs2_bbc_print5(a,b,c,d,e) fprintf(stderr,a,b,c,d,e)
#define jffs2_bbc_print6(a,b,c,d,e,f) fprintf(stderr,a,b,c,d,e,f)
#define jffs2_bbc_print7(a,b,c,d,e,f,g) fprintf(stderr,a,b,c,d,e,f,g)
#define jffs2_bbc_print8(a,b,c,d,e,f,g,h) fprintf(stderr,a,b,c,d,e,f,g,h)
#define jffs2_bbc_print9(a,b,c,d,e,f,g,h,i) fprintf(stderr,a,b,c,d,e,f,g,h,i)
#endif

/* Handle endianness */
#ifndef __KERNEL__

#define ENDIAN_HOST_AND_TARGET_SAME      0
#define ENDIAN_HOST_AND_TARGET_DIFFERENT 1

extern int jffs2_bbc_glb_endian_X;

#endif

/* Allocating more than one page (tip. 4096 byte) */
void *jffs2_bbc_malloc(long size);
void jffs2_bbc_free(void *addr);

/* Allocating less than one page (tip. 4096 byte) */
void *jffs2_bbc_malloc_small(long size);
void jffs2_bbc_free_small(void *addr);

/* Memory guarding */
int jffs2_bbc_test_memory_counter(int verbose);
char *jffs2_bbc_get_mem_stats(void);
int jffs2_bbc_get_memory_counter(void);

#endif
