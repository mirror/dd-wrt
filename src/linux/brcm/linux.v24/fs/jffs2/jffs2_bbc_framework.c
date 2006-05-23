/* 
 * JFFS2-BBC: Compression Framework
 *
 * $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
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

/* USE JFFS2_BBC_STANDALONE define if you don't want to compile without JFFS2 */

//#define DEBUG_COMPRESSORS
//#define DEBUG_SHOW_BLOCK_SIZES

#define JFFS2_BBC_STAT_BUFF_SIZE   8000

#ifndef __KERNEL__

#include <stdio.h>
#include <malloc.h>
typedef unsigned long uint32_t;

#else

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#endif

#define JFFS2_BBC_ZLIB_BLOCK_SIGN_0 (120)
#define JFFS2_BBC_ZLIB_BLOCK_SIGN_1 (94)

#define JFFS2_BBC_DUMMY_BLOCKSIGN_0 (0x54)
#define JFFS2_BBC_DUMMY_BLOCKSIGN_1 (0x01)

#ifndef NULL
#define NULL ((void*)(0))
#endif

#include "jffs2_bbc_framework.h"

/*********************************************************************
 *                      Global data                                  *
 *********************************************************************/

static int jffs2_bbc_compression_mode = JFFS2_BBC_ZLIB_MODE;
static struct jffs2_bbc_compressor_type *jffs2_bbc_manual_compressor = NULL;
static struct jffs2_bbc_compressor_type *jffs2_bbc_compressors = NULL;
static struct jffs2_bbc_model_list_node *jffs2_bbc_model_list = NULL;
static void *last_sb = NULL;	/* previously activated sb */

/*********************************************************************
 *               Compressor initialization                           *
 *********************************************************************/

#ifndef JFFS2_BBC_STANDALONE

#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_ARMLIB)
struct jffs2_bbc_compressor_type *jffs2_bbc_armlib_init(void);
void jffs2_bbc_armlib_deinit(void);
#endif

#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZO)
struct jffs2_bbc_compressor_type *jffs2_bbc_lzo_init(void);
void jffs2_bbc_lzo_deinit(void);
#endif

#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZSS)
struct jffs2_bbc_compressor_type *jffs2_bbc_lzss_init(void);
void jffs2_bbc_lzss_deinit(void);
#endif

#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZARI)
struct jffs2_bbc_compressor_type *jffs2_bbc_lzari_init(void);
void jffs2_bbc_lzari_deinit(void);
#endif

#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZHD)
struct jffs2_bbc_compressor_type *jffs2_bbc_lzhd_init(void);
void jffs2_bbc_lzhd_deinit(void);
#endif

void jffs2_bbc_compressor_init()
{
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_ARMLIB)
	jffs2_bbc_armlib_init();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZO)
	jffs2_bbc_lzo_init();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZSS)
	jffs2_bbc_lzss_init();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZARI)
	jffs2_bbc_lzari_init();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZHD)
	jffs2_bbc_lzhd_init();
#endif
}

void jffs2_bbc_compressor_deinit()
{
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZHD)
	jffs2_bbc_lzhd_deinit();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZARI)
	jffs2_bbc_lzari_deinit();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZSS)
	jffs2_bbc_lzss_deinit();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_LZO)
	jffs2_bbc_lzo_deinit();
#endif
#if !defined(__KERNEL__) || defined(CONFIG_JFFS2_BBC_ARMLIB)
	jffs2_bbc_armlib_deinit();
#endif
}

#endif

#ifndef JFFS2_BBC_STANDALONE

/*********************************************************************
 *                          ZLIB COMPRESSOR                          *
 *********************************************************************/

extern int jffs2_zlib_compress2(unsigned char *data_in, unsigned char *cpage_out, uint32_t * sourcelen, uint32_t * dstlen);
extern void jffs2_zlib_decompress2(unsigned char *data_in, unsigned char *cpage_out, uint32_t srclen, uint32_t destlen);

static int jffs2_bbc_zlib_compress(void *model, unsigned char *input, unsigned char *output, unsigned long *sourcelen, unsigned long *dstlen) 
{
        return jffs2_zlib_compress2(input, output, sourcelen, dstlen);
}

static int jffs2_bbc_zlib_decompress(void *model, unsigned char *input, unsigned char *output, unsigned long sourcelen, unsigned long dstlen) 
{
        jffs2_zlib_decompress2(input, output, sourcelen, dstlen);
        return 0;
}

static int jffs2_bbc_zlib_estimate(void *model, unsigned char *input, unsigned long sourcelen, unsigned long *dstlen, unsigned long *readtime, unsigned long *writetime)
{
	*dstlen    = sourcelen * 65 / 100;
	*readtime  = JFFS2_BBC_ZLIB_READ_TIME;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME;
	return 0;
}

static struct jffs2_bbc_compressor_type jffs2_bbc_zlib = {
        "zlib",
        0,
        {JFFS2_BBC_ZLIB_BLOCK_SIGN_0, JFFS2_BBC_ZLIB_BLOCK_SIGN_1, 0, 0},
        NULL,
        NULL,
        NULL,
        NULL,
        jffs2_bbc_zlib_compress,
        jffs2_bbc_zlib_estimate,
        jffs2_bbc_zlib_decompress,
        NULL,
        NULL,
        1
};

static struct jffs2_bbc_compressor_type *jffs2_bbc_original_compressor = &jffs2_bbc_zlib;

#endif

/*********************************************************************
 *               Compression mode handling                           *
 *********************************************************************/

int jffs2_bbc_get_compression_mode(void)
{
	return jffs2_bbc_compression_mode;
}

void jffs2_bbc_set_compression_mode(int mode)
{
	jffs2_bbc_compression_mode = mode;
}

void jffs2_bbc_set_manual_compressor(struct jffs2_bbc_compressor_type *c)
{
	jffs2_bbc_manual_compressor = c;
	jffs2_bbc_set_compression_mode(JFFS2_BBC_MANUAL_MODE);
}

int jffs2_bbc_set_manual_compressor_by_name(char *name)
{
	struct jffs2_bbc_compressor_type *l;
	int i;

	l = jffs2_bbc_compressors;
	while (l != NULL) {
		for (i = 0; i < 1000; i++) {
			if (l->name[i] == 0) {
				jffs2_bbc_set_manual_compressor(l);
				return 0;
			}
			else if (name[i] == 0)
				i = 1000;
			else if (name[i] != l->name[i])
				i = 1000;
		}
		l = l->next;
	}
	jffs2_bbc_set_manual_compressor(NULL);
	return 1;
}

static struct jffs2_bbc_compressor_type *jffs2_bbc_get_compressor_by_name(char *name)
{
	struct jffs2_bbc_compressor_type *l;
	int i;

#ifndef JFFS2_BBC_STANDALONE
	l = jffs2_bbc_original_compressor;
	for (i = 0; i < 1000; i++) {
		if (l->name[i] == 0) {
			return l;
		}
		else if (name[i] == 0)
			i = 1000;
		else if (name[i] != l->name[i])
			i = 1000;
	}
#endif

	l = jffs2_bbc_compressors;
	while (l != NULL) {
		for (i = 0; i < 1000; i++) {
			if (l->name[i] == 0) {
				return l;
			}
			else if (name[i] == 0)
				i = 1000;
			else if (name[i] != l->name[i])
				i = 1000;
		}
		l = l->next;
	}

	return NULL;
}

int jffs2_bbc_disable_compressor_by_name(char *name)
{
	struct jffs2_bbc_compressor_type *l;

        l = jffs2_bbc_get_compressor_by_name(name);
        if (l == NULL) return 1;
        l->enabled = 0;
        return 0;
}

int jffs2_bbc_enable_compressor_by_name(char *name)
{
	struct jffs2_bbc_compressor_type *l;

        l = jffs2_bbc_get_compressor_by_name(name);
        if (l == NULL) return 1;
        l->enabled = 1;
        return 0;
}

void jffs2_bbc_compressor_command_by_name(char *name_and_command)
{
	struct jffs2_bbc_compressor_type *l;
	int i;

	l = jffs2_bbc_compressors;
	while (l != NULL) {
		for (i = 0; i < 1000; i++) {
			if (l->name[i] == 0) {
				if (name_and_command[i] != ':') {
					jffs2_bbc_print1("jffs2.bbc: ':' missing after compressor name\n");
				}
				else {
					if (l->proc_command != NULL)
						l->proc_command(name_and_command + i + 1);
				}
				i = 1000;
				return;
			}
			else if (name_and_command[i] == 0) {
				i = 1000;
			}
			else if (name_and_command[i] != l->name[i]) {
				i = 1000;
			}
		}
		l = l->next;
	}
}

struct jffs2_bbc_compressor_type *jffs2_bbc_get_manual_compressor(void)
{
	if (jffs2_bbc_get_compression_mode() != JFFS2_BBC_MANUAL_MODE) {
		jffs2_bbc_manual_compressor = NULL;
	}
	return jffs2_bbc_manual_compressor;
}

/*********************************************************************
 *                  Compressor handling                              *
 *********************************************************************/

struct jffs2_bbc_compressor_type *jffs2_bbc_get_compressor_list(void)
{
	return jffs2_bbc_compressors;
}

struct jffs2_bbc_model_list_node *jffs2_bbc_get_model_list(void)
{
	return jffs2_bbc_model_list;
}

int jffs2_bbc_register_compressor(struct jffs2_bbc_compressor_type *c)
{
	struct jffs2_bbc_compressor_type *l;
	struct jffs2_bbc_model_list_node *l2;
	int model_found = 0;

	l = jffs2_bbc_compressors;
	/* Check for confilcts */
	while (l != NULL) {
		c->name[15] = 0;
		/*if (strcmp(c->name,l->name)==0) {
		   jffs2_bbc_print1("jffs2.bbc: compressor is already loaded.");
		   return -1;
		   } */
		if ((l->model_file_sign == c->model_file_sign) && (c->model_file_sign != 0)) {
			jffs2_bbc_print1("jffs2.bbc: already used model file sign. fail.");
			return -1;
		}
		l = l->next;
	}
	/* Search and initialize model */
	c->models = NULL;
	c->mounted = 0;
	if (c->init != NULL) {
		if (c->init() != 0) {
			jffs2_bbc_print2("jffs2.bbc: cannot initialize compressor %s.\n", c->name);
			return -1;
		}
	}
	if (c->model_file_sign != 0) {
		l2 = jffs2_bbc_model_list;
		while (1) {
			if (l2 == NULL)
				break;
			if (c->model_file_sign == l2->sign) {
				if (l2->compressor != NULL) {
					jffs2_bbc_print2("jffs2.bbc: register for %s: BUG, model file already reserved!!!!\n", c->name);
				}
				else {
					if (c->init_model(&(l2->model)) != 0) {
						jffs2_bbc_print2("jffs2.bbc: cannot initialize compressor %s for a model", c->name);
					}
					else {
						l2->compressor = c;
						l2->next_compr_model = c->models;
						c->models = l2;
						c->mounted++;
						model_found++;
					}
				}
			}
			l2 = l2->next_model;
		}
		/*if (model_found==0) {
		   jffs2_bbc_print2("jffs2.bbc: no macthing model file found for %s at this time (maybe later)\n",c->name);
		   } */
	}
	/* Insert to the end of the compressor list */
	c->enabled = 1;
	c->buffer = NULL;
	c->buffer_size = 0;
	c->stat_compr_orig = c->stat_compr_new = c->stat_decompr = 0;
	c->next = NULL;
	if (jffs2_bbc_compressors == NULL) {
		jffs2_bbc_compressors = c;
	}
	else {
		l = jffs2_bbc_compressors;
		while (l->next != NULL)
			l = l->next;
		l->next = c;
	}
	return 0;
}

int jffs2_bbc_unregister_compressor(struct jffs2_bbc_compressor_type *c)
{
	struct jffs2_bbc_compressor_type *l;
	struct jffs2_bbc_model_list_node *l2;

	if (c->mounted != 0) {
		jffs2_bbc_print1("jffs2.bbc: Compressor is in use. Sorry.");
		return -1;
	}
	if (jffs2_bbc_compressors == NULL) {
		jffs2_bbc_print1("jffs2.bbc: unregister: empty list.");
		return -1;
	}
	else if (jffs2_bbc_compressors == c) {
		if (c->deinit != NULL)
			c->deinit();
		jffs2_bbc_compressors = c->next;
	}
	else {
		l = jffs2_bbc_compressors;
		while (l->next != c) {
			if (l->next == NULL) {
				jffs2_bbc_print2("jffs2.bbc: unregister: cannot find compressor %s in the list.", c->name);
				return -1;
			}
			l = l->next;
		}
		if (c->deinit != NULL)
			c->deinit();
		l->next = c->next;
	}
	if (c->buffer != NULL) {
		jffs2_bbc_free(c->buffer);
		c->buffer = NULL;
		c->buffer_size = 0;
	}

	l2 = jffs2_bbc_model_list;
	while (l2 != NULL) {
		if (l2->compressor == c) {
			jffs2_bbc_print1("jffs2.bbc: unregister: BUG: model found!!!");
			l2->compressor = NULL;
			l2->next_compr_model = NULL;
		}
		l2 = l2->next_model;
	}

	return 0;
}

int jffs2_bbc_model_new(void *sb, int i_num, void *model)
{
	struct jffs2_bbc_model_list_node *node;
	struct jffs2_bbc_compressor_type *l;
	char block_sign[2];

	int sign;

	/* check for conflicts... */
	sign = *((int *) model);
	block_sign[0] = *(((char *) model) + 4);
	block_sign[1] = *(((char *) model) + 5);
	node = jffs2_bbc_model_list;
	while (node != NULL) {
		if ((node->block_sign[0] == block_sign[0]) && (node->block_sign[1] == block_sign[1]) && (node->sb == sb)) {
			//jffs2_bbc_print2("jffs2.bbc: model_new: model conflict (inode=%d)!\n",i_num);
			return -1;
		}
		node = node->next_model;
	}

	/* insertion */
	node = jffs2_bbc_malloc_small((long)sizeof(struct jffs2_bbc_model_list_node));
	node->sb = sb;
	node->model = model;
	node->sign = *((int *) model);
	node->block_sign[0] = *(((char *) model) + 4);
	node->block_sign[1] = *(((char *) model) + 5);
	node->inode = i_num;
	node->next_model = jffs2_bbc_model_list;
	node->compressor = NULL;
	node->stat_decompr = 0;
	node->next_compr_model = NULL;
	jffs2_bbc_model_list = node;

	/* search for matching compressor */
	l = jffs2_bbc_compressors;
	while (l != NULL) {
		if (l->model_file_sign == sign) {
			//jffs2_bbc_print2("jffs2.bbc: compressor for model found: %s ",l->name);
			if (l->init_model(&(node->model)) != 0) {
				jffs2_bbc_print1("jffs2.bbc: cannot initialize compressor for a model");
			}
			else {
				l->mounted++;
				node->compressor = l;
				node->next_compr_model = l->models;
				l->models = node;
			}
			break;
		}
		l = l->next;
	}
	return 0;
}

static void jffs2_bbc_model_del_from_compressor(struct jffs2_bbc_model_list_node *node)
{
	struct jffs2_bbc_model_list_node *l;

	if (node->model != NULL) {
		if (node->compressor != NULL) {
			if (node->compressor->destroy_model == NULL) {
				jffs2_bbc_free(node->model);
				node->model = NULL;
			}
			else {
				node->compressor->destroy_model(&(node->model));
				if (node->model != NULL)
					jffs2_bbc_print1("jffs2.bbc: warning: not NULL model after destroying!\n");
			}
		}
	}

	if (node->compressor == NULL) {
		jffs2_bbc_print1("jffs2.bbc: jffs2_bbc_model_del_from_compressor: no compressor!\n");
		return;
	}
	l = node->compressor->models;
	if (l == NULL) {
		jffs2_bbc_print1("jffs2.bbc: jffs2_bbc_model_del_from_compressor error, models=NULL!\n");
		return;
	}
	if (l == node) {
		node->compressor->models = node->next_compr_model;
		node->compressor->mounted--;
		return;
	}
	while (1) {
		if (l->next_compr_model == node) {
			l->next_compr_model = node->next_compr_model;
			node->compressor->mounted--;
			return;
		}
		l = l->next_compr_model;
		if (l == NULL) {
			jffs2_bbc_print1("jffs2.bbc: jffs2_bbc_model_del_from_compressor: not found\n");
			return;
		}
	}
}

void jffs2_bbc_model_del(void *sb)
{
	struct jffs2_bbc_model_list_node *l, *l2;

	l = jffs2_bbc_model_list;
	if (l == NULL)
		return;
	if (l->sb == sb) {
		jffs2_bbc_model_list = l->next_model;
		jffs2_bbc_model_del_from_compressor(l);
		jffs2_bbc_free_small(l);
		jffs2_bbc_model_del(sb);
		return;
	}
	while (1) {
		if (l->next_model == NULL) {
			break;
		}
		if (l->next_model->sb == sb) {
			l2 = l->next_model;
			l->next_model = l->next_model->next_model;
			jffs2_bbc_model_del_from_compressor(l2);
			jffs2_bbc_free_small(l2);
			jffs2_bbc_model_del(sb);
			return;
		}
		l = l->next_model;
	}
	last_sb = NULL;
}

void jffs2_bbc_model_set_act_sb(void *sb)
{
	last_sb = sb;
}

void *jffs2_bbc_model_get_act_sb(void)
{
	return last_sb;
}

void *jffs2_bbc_model_get_newest(struct jffs2_bbc_compressor_type *compressor)
{
	struct jffs2_bbc_model_list_node *m, *best_m;
	int max_sign, sign;

	if (compressor == NULL) {
		jffs2_bbc_print1("jffs2.bbc: jffs2_bbc_model_get: NULL!!\n");
		return NULL;
	}

	best_m = NULL;
	max_sign = -1;
	m = compressor->models;
	while (m != NULL) {
		if (m->sb == last_sb) {
			sign = (int) (m->block_sign[0]) * 256 + (int) (m->block_sign[1]);
			if (sign > max_sign) {
				max_sign = sign;
				best_m = m;
			}
		}
		m = m->next_compr_model;
	}
	if (best_m != NULL)
		return best_m->model;
	else
		return NULL;
}

/*********************************************************************          
 *                        Statistics                                 *          
 *********************************************************************/

static char *jffs2_bbc_stat_buff = NULL;

char *jffs2_bbc_get_model_stats(void)
{
	char *b;
	struct jffs2_bbc_model_list_node *m;
	struct jffs2_bbc_compressor_type *c;

	if (jffs2_bbc_stat_buff == NULL)
		jffs2_bbc_stat_buff = jffs2_bbc_malloc(8000);

	b = jffs2_bbc_stat_buff;

	b += sprintf(b, "Loaded compressors:");
	c = jffs2_bbc_compressors;
	while (c != NULL) {
		b += sprintf(b, "\n  %s (%d) ", c->name, c->enabled);
		if (c->model_file_sign != 0) {
			b += sprintf(b, "m_sign=%d ", c->model_file_sign);
			b += sprintf(b, "models=");
			m = c->models;
			while (m != NULL) {
				b += sprintf(b, "(inode=%d)", m->inode);
				m = m->next_compr_model;
			}
		}
		else {
			b += sprintf(b, "b_sign=(%d,%d) nomodel", (int) (c->block_sign[0]), (int) (c->block_sign[1]));
		}
		if (c->proc_info != NULL) {
			b += sprintf(b, "\n    %s", c->proc_info());
		}
		c = c->next;
	}

	m = jffs2_bbc_model_list;
	
	if (m == NULL) {
		b += sprintf(b, "\nPresent models: NONE\n");
	}
	else {
		b += sprintf(b, "\nPresent models:\n");
		while (m != NULL) {
			b += sprintf(b, "  b_sign=(%d,%d),inode=%d,decompr=%d", (int) (m->block_sign[0]), (int) (m->block_sign[1]), m->inode, m->stat_decompr);
			if (m->compressor == NULL)
				b += sprintf(b, ",compressor=NULL\n");
			else
				b += sprintf(b, ",compressor=%s\n", m->compressor->name);
			m = m->next_model;
		}
	}

	return jffs2_bbc_stat_buff;
}

/*********************************************************************
 *                  Memory handling, debug                           *
 *********************************************************************/

static int jffs2_bbc_mem_counter = 0;

#ifdef __KERNEL__

void *jffs2_bbc_malloc(long size)
{
	void *addr = vmalloc(size);
	if (addr != NULL)
		jffs2_bbc_mem_counter++;
	else {
		jffs2_bbc_print2("DEBUG: not enough memory (%ld)\n", size);
	}
	return addr;
}

void jffs2_bbc_free(void *addr)
{
	jffs2_bbc_mem_counter--;
	vfree(addr);
}

void *jffs2_bbc_malloc_small(long size)
{
	void *addr;
	addr = kmalloc(size, 0);
	if (addr != NULL)
		jffs2_bbc_mem_counter++;
	return addr;
}

void jffs2_bbc_free_small(void *addr)
{
	jffs2_bbc_mem_counter--;
	kfree(addr);
}

#else

void *jffs2_bbc_malloc(long size)
{
	void *addr = malloc(size);
	if (addr != NULL)
		jffs2_bbc_mem_counter++;
	return addr;
}

void jffs2_bbc_free(void *addr)
{
	jffs2_bbc_mem_counter--;
	free(addr);
}

void *jffs2_bbc_malloc_small(long size)
{
	return jffs2_bbc_malloc(size);
}

void jffs2_bbc_free_small(void *addr)
{
	jffs2_bbc_free(addr);
}

#endif

int jffs2_bbc_test_memory_counter(int verbose)
{
	if (verbose > 0) {
		jffs2_bbc_print2("jffs2.bbc: mem_counter=%d!\n", jffs2_bbc_mem_counter);
	}
	return jffs2_bbc_mem_counter;
}

int jffs2_bbc_get_memory_counter(void)
{
	return jffs2_bbc_mem_counter;
}

static char mem_stat[200];

char *jffs2_bbc_get_mem_stats(void)
{
	sprintf(mem_stat, "Memcounter=%d\n", jffs2_bbc_mem_counter);
	return mem_stat;
}

void jffs2_bbc_print_flush(void)
{
#ifdef __KERNEL__
	return;
#else
	fflush(stdout);
	fflush(stderr);
#endif
}

/*********************************************************************
 *                FRAMEWORK - ZLIB REPLACEMENT                       *
 *********************************************************************/

#ifndef JFFS2_BBC_STANDALONE

/* Temorary buffers */
static char stat_str[JFFS2_BBC_STAT_BUFF_SIZE];
static int tmp_buffer_size = 0;
static char *tmp_buffer = NULL;

/* Statistic - used by /proc/jffs2_bbc and mkfs.jffs2 */
char *jffs2_bbc_get_compr_stats(void)
{
	struct jffs2_bbc_compressor_type *l;
	char *s = stat_str;

	s += sprintf(s, "Compression statistics:\n");
        l = jffs2_bbc_original_compressor;
	//s += sprintf(s, " zlib: compr=%d/%d decompr=%d\n", stat_zlib_compr_new, stat_zlib_compr_orig, stat_zlib_decompr);
	s += sprintf(s, " %s: compr=%d/%d decompr=%d\n", l->name, l->stat_compr_new, l->stat_compr_orig, l->stat_decompr);
	l = jffs2_bbc_get_compressor_list();
	while (l != NULL) {
		s += sprintf(s, " %s: compr=%d/%d decompr=%d\n", l->name, l->stat_compr_new, l->stat_compr_orig, l->stat_decompr);
		l = l->next;
	}
	return stat_str;
}

static void jffs2_bbc_buffer_fill(unsigned char *buff, int size)
{
	for (; size > 0; size--, buff++)
		*buff = 255;
}


static int jffs2_bbc_update_compr_buf(unsigned long size)
{
	struct jffs2_bbc_compressor_type *l;

	if (size < 5000)
		size = 5000;
	if (tmp_buffer == NULL) {
		tmp_buffer = jffs2_bbc_malloc(size);
		jffs2_bbc_buffer_fill(tmp_buffer, size);
		tmp_buffer_size = size;
	}
	else if (tmp_buffer_size < size) {
		jffs2_bbc_free(tmp_buffer);
		tmp_buffer = jffs2_bbc_malloc(size);
		jffs2_bbc_buffer_fill(tmp_buffer, size);
		tmp_buffer_size = size;
	}
	l = jffs2_bbc_get_compressor_list();
	while (l != NULL) {
		if (l->buffer == NULL) {
			l->buffer_size = size;
			l->buffer = jffs2_bbc_malloc(size);
			jffs2_bbc_buffer_fill(l->buffer, size);
		}
		else if (l->buffer_size < size) {
			jffs2_bbc_free(l->buffer);
			l->buffer_size = size;
			l->buffer = jffs2_bbc_malloc(size);
			jffs2_bbc_buffer_fill(l->buffer, size);
		}
		l = l->next;
	}
	return 0;
}

#ifdef DEBUG_COMPRESSORS

static unsigned char *debug_tmp_buff = NULL;
static long debug_orig_srclen = -1;
static long debug_orig_dstlen = -1;
static int debug_mem_counter = -1;


void debug_before_compress(struct jffs2_bbc_compressor_type *c, void *model, unsigned char *input, unsigned char *output, long *sourcelen, long *dstlen)
{

	debug_orig_srclen = *sourcelen;	// for buffer overflow test
	debug_orig_dstlen = *dstlen;	// for buffer overflow test
	output[debug_orig_dstlen + 1] = 255;

	debug_mem_counter = jffs2_bbc_get_memory_counter();	// for memory guard
}

void debug_after_compress(struct jffs2_bbc_compressor_type *c, int back, void *model, unsigned char *input, unsigned char *output, long *sourcelen, long *dstlen)
{
	long dst_len = *dstlen;
	long src_len = *sourcelen;
	int i;

	// Memory guard
	if (debug_mem_counter != jffs2_bbc_get_memory_counter()) {
		jffs2_bbc_print4("!!!!!!!! %s error: possible COMPRESSOR MEMORY LEAK: %d->%d\n", c->name, debug_mem_counter, jffs2_bbc_get_memory_counter());
		debug_mem_counter = jffs2_bbc_get_memory_counter();
	}

	// Buffer overflow test  
	if (output[debug_orig_dstlen + 1] != 255) {
		jffs2_bbc_print7("!!!!!!!! %s error: BUFFER OVERFLOW !!!!!!!!!!!! b[%d]=%d (srclen=%d dstlen=%d, back=%d)\n", c->name, (int) (debug_orig_dstlen + 1), (int) (output[debug_orig_dstlen + 1]), (int) (debug_orig_srclen), (int) (*dstlen), back);
	}

	// Decompression check
	if (back == 0) {
		if (debug_tmp_buff == NULL)
			debug_tmp_buff = jffs2_bbc_malloc(17000);
		for (i = 0; i < src_len; i++) debug_tmp_buff[i] = 0xf6;
		c->decompress(model, output, debug_tmp_buff, dst_len, src_len);
		// Memory guard for decompressor
		if (debug_mem_counter != jffs2_bbc_get_memory_counter()) {
			jffs2_bbc_print4("!!!!!!!! %s error: possible DECOMPRESSOR MEMORY LEAK: %d->%d\n", c->name, debug_mem_counter, jffs2_bbc_get_memory_counter());
			debug_mem_counter = jffs2_bbc_get_memory_counter();
		}

		for (i = 0; i < src_len; i++)
			if (input[i] != debug_tmp_buff[i]) {
				jffs2_bbc_print7("!!!!!!!! %s error: BLOCK DECOMPRESSED BADLY (first bad: %d in %d: %d!=%d (compressed size=%d)) !!!!!!!!!!!!\n", c->name, i, src_len, (int)input[i], (int)debug_tmp_buff[i], dst_len);
				break;
			}
		return;
	}

	// Return value test
	//jffs2_bbc_print3("!!!!!!!! %s error: %d !!!!!!!!!!!!\n", c->name, back);
}

#endif

int jffs2_zlib_compress(unsigned char *data_in, unsigned char *cpage_out, uint32_t * sourcelen, uint32_t * dstlen)
{
	struct jffs2_bbc_compressor_type *c;
	int back, back_zlib, mode, min, i, i2;
	long tmp = 0, tmp_read_time = 1000, tmp_write_time = 1000, orig_src, orig_dest, src, dest;
	struct jffs2_bbc_model_list_node *m;
	void *sb;
	unsigned char *tmp_p = NULL;

	sb = jffs2_bbc_model_get_act_sb();

	orig_src = *sourcelen;
	orig_dest = *dstlen;

	mode = jffs2_bbc_get_compression_mode();

	if (mode == JFFS2_BBC_DUMMY_MODE) {
		i=0; i2=0;
		if (*dstlen>2) {
			cpage_out[i++]=JFFS2_BBC_DUMMY_BLOCKSIGN_0;
			cpage_out[i++]=JFFS2_BBC_DUMMY_BLOCKSIGN_1;
			i2=i;
		}		
		for (;((i < *dstlen) && (i < (*sourcelen)+i2));i++) {
			cpage_out[i] = data_in[i-i2];
		}
		*sourcelen=i-i2;
		*dstlen=i;
		return 0;
	}

	if (mode == JFFS2_BBC_ZLIB_MODE) {
                /*if (!jffs2_bbc_original_compressor->enabled) {
                        jffs2_bbc_print2("jffs2.bbc: WARNING: ZLIB mode but %s disabled! Enabling for this procedure...\n",jffs2_bbc_original_compressor->name);
                }*/
		back = jffs2_bbc_original_compressor->compress(NULL, data_in, cpage_out, sourcelen, dstlen);
		jffs2_bbc_original_compressor->stat_compr_orig += *sourcelen;
		jffs2_bbc_original_compressor->stat_compr_new += *dstlen;
		return back;
	}

	jffs2_bbc_update_compr_buf(orig_dest);

	if (mode == JFFS2_BBC_SIZE_MODE) {
		// Testing all compressors
                if (!jffs2_bbc_original_compressor->enabled) {
			min = -1;
                }
                else {
			back_zlib = jffs2_bbc_original_compressor->compress(NULL, data_in, cpage_out, sourcelen, dstlen);
			min = *dstlen;
                }
		c = jffs2_bbc_get_compressor_list();
		while (c != NULL) {
			c->buffer_cnt = -1;
			if (c->enabled == 0) {
				c = c->next;
				continue;
			}
			if (c->model_file_sign == 0) {
				src = orig_src;
				dest = orig_dest;
#ifdef DEBUG_COMPRESSORS
				debug_before_compress(c, NULL, data_in, c->buffer, &src, &dest);
#endif
				back = c->compress(NULL, data_in, c->buffer, &src, &dest);
#ifdef DEBUG_COMPRESSORS
				debug_after_compress(c, back, NULL, data_in, c->buffer, &src, &dest);
#endif
				if (back == 0) {
					c->buffer_cnt = dest;
					if ((min < 0) || (min > dest))
						min = dest;
				}
			}
			else {
				m = c->models;
				while (m != NULL) {
					src = orig_src;
					dest = orig_dest;
					if (m->sb == sb) {
						if (c->buffer_cnt == -1) {
#ifdef DEBUG_COMPRESSORS
							debug_before_compress(c, m->model, data_in, c->buffer, (long *) (&src), (long *) (&dest));
#endif
							back = c->compress(m->model, data_in, c->buffer, (long *) (&src), (long *) (&dest));
#ifdef DEBUG_COMPRESSORS
							debug_after_compress(c, back, m->model, data_in, c->buffer, (long *) (&src), (long *) (&dest));
#endif
							if (back == 0) {
								c->buffer_cnt = dest;
								if ((min < 0) || (min > dest))
									min = dest;
							}
						}
						else {
#ifdef DEBUG_COMPRESSORS
							debug_before_compress(c, m->model, data_in, tmp_buffer, &src, &dest);
#endif
							back = c->compress(m->model, data_in, tmp_buffer, &src, &dest);
#ifdef DEBUG_COMPRESSORS
							debug_after_compress(c, back, m->model, data_in, tmp_buffer, &src, &dest);
#endif
							if (back == 0) {
								if (c->buffer_cnt > dest) {
									c->buffer_cnt = dest;
									tmp_p = c->buffer;
									c->buffer = tmp_buffer;
									tmp_buffer = tmp_p;
									if ((min < 0) || (min > dest))
										min = dest;
								}
							}
						}
					}
					m = m->next_compr_model;
				}
			}
			c = c->next;
		}
		//Finding the best and copy its result

#ifdef DEBUG_SHOW_BLOCK_SIZES
		jffs2_bbc_print1("\n");
                if (jffs2_bbc_original_compressor->enabled) {
			if (min == *dstlen) {
				jffs2_bbc_print3("%s:%d* ", jffs2_bbc_original_compressor->name, (int) (*dstlen));
                        }
			else {
				jffs2_bbc_print3("%s:%d ", jffs2_bbc_original_compressor->name, (int) (*dstlen));
                        }
		} 
		c = jffs2_bbc_get_compressor_list();
		while (c != NULL) {
			if (c->enabled == 0) {
				c = c->next;
				continue;
			}
			if (c->buffer_cnt == min)
				jffs2_bbc_print3("%s:%d* ", c->name, c->buffer_cnt);
			else
				jffs2_bbc_print3("%s:%d ", c->name, c->buffer_cnt);
			c = c->next;
		}
#endif

		if (min == -1) {
			return -1; // none of compressors work (maybe too short output buffer)
		}

		if (jffs2_bbc_original_compressor->enabled) {
			if (min == *dstlen) {
		                jffs2_bbc_original_compressor->stat_compr_orig += *sourcelen;
		                jffs2_bbc_original_compressor->stat_compr_new += *dstlen;
				return back_zlib;
			}
		}

		c = jffs2_bbc_get_compressor_list();
		while (c != NULL) {
			if (c->enabled == 0) {
				c = c->next;
				continue;
			}
			if (c->buffer_cnt == min) {
				*dstlen = c->buffer_cnt;
				*sourcelen = orig_src;
				for (i = 0; i < *dstlen; i++) {
					cpage_out[i] = c->buffer[i];
				}
				c->stat_compr_orig += *sourcelen;
				c->stat_compr_new += *dstlen;
				return 0;
			}
			c = c->next;
		}
		jffs2_bbc_print1("jffs2.bbc: compr (full): BUG!!!\n");
		return 0;
	}

	if ((mode == JFFS2_BBC_FASTR_MODE)||(mode == JFFS2_BBC_FASTW_MODE)||(mode == JFFS2_BBC_FASTS_MODE)) {
		// Estimating all compressors
                if (jffs2_bbc_original_compressor->enabled) {
		        back = jffs2_bbc_original_compressor->estimate(NULL, data_in, *sourcelen, &tmp, &tmp_read_time, &tmp_write_time);
		} 
                else {
			tmp = -1;
			tmp_read_time = -1;
			tmp_write_time = -1;
		}
		if (mode == JFFS2_BBC_FASTR_MODE) tmp = tmp_read_time;
		if (mode == JFFS2_BBC_FASTW_MODE) tmp = tmp_write_time;
		min = tmp;
		c = jffs2_bbc_get_compressor_list();
		while (c != NULL) {
			src = orig_src;
			dest = orig_dest;
			c->buffer_cnt = -1;
			if (c->enabled == 0) {
				c = c->next;
				continue;
			}
			if ((c->model_file_sign == 0) || (jffs2_bbc_model_get_newest(c) != NULL)) {
				back = c->estimate(jffs2_bbc_model_get_newest(c), data_in, src, &dest, &tmp_read_time, &tmp_write_time);
				if (mode == JFFS2_BBC_FASTR_MODE) dest = tmp_read_time;
				if (mode == JFFS2_BBC_FASTW_MODE) dest = tmp_write_time;
				if (back == 0) {
					c->buffer_cnt = dest;
					if ((min < 0) || (min > dest))
						min = dest;
				}
				else {
					c->buffer_cnt = -1;
				}
			}
			c = c->next;
		}
		// Finding the best and compress with it
		if (min == -1) {
			return -1;
		}
                if (jffs2_bbc_original_compressor->enabled) {
    		        if (min == tmp) {
				back = jffs2_bbc_original_compressor->compress(NULL, data_in, cpage_out, sourcelen, dstlen);
				jffs2_bbc_original_compressor->stat_compr_orig += *sourcelen;
				jffs2_bbc_original_compressor->stat_compr_new += *dstlen;
				return back;
			}
	        }
		c = jffs2_bbc_get_compressor_list();
		while (c != NULL) {
			if (c->enabled == 0) {
				c = c->next;
				continue;
			}
			if (c->buffer_cnt == min) {
				back = c->compress(jffs2_bbc_model_get_newest(c), data_in, cpage_out, (unsigned long*)sourcelen, (unsigned long*)dstlen);
				if ((back == 0) && (*dstlen < orig_dest) && (*dstlen > 4)) {
					c->stat_compr_orig += *sourcelen;
					c->stat_compr_new += *dstlen;
				}
				else { // fallback will always be available
					*sourcelen = orig_src;
					*dstlen = orig_dest;
					back = jffs2_bbc_original_compressor->compress(NULL, data_in, cpage_out, sourcelen, dstlen);
					jffs2_bbc_original_compressor->stat_compr_orig += *sourcelen;
					jffs2_bbc_original_compressor->stat_compr_new += *dstlen;
					return back;
				}
				return 0;
			}
			c = c->next;
		}
		jffs2_bbc_print1("jffs2.bbc: compress (fastX mode): BUG!!!\n");
		return 0;
	}

	if (mode == JFFS2_BBC_MANUAL_MODE) {
		c = jffs2_bbc_get_manual_compressor();
		if (c != NULL) {
			if (c->model_file_sign == 0) {
				src = orig_src;
				dest = orig_dest;
				back = c->compress(NULL, data_in, cpage_out, &src, &dest);
				if (back == 0) {
					*dstlen = dest;
					*sourcelen = src;
					c->stat_compr_orig += *sourcelen;
					c->stat_compr_new += *dstlen;
					return 0;
				}
			}
			else {
				c->buffer_cnt = -1;
				m = c->models;
				min = -1;
				while (m != NULL) {
					src = orig_src;
					dest = orig_dest;
					if (m->sb == sb) {
						if (min == -1) {
							back = c->compress(m->model, data_in, cpage_out, (unsigned long*)sourcelen, (unsigned long*)dstlen);
							if ((back == 0) && (*dstlen < orig_dest) && (*dstlen > 4)) {
								min = dest;
								tmp_p = cpage_out;
							}
						}
						else {
							back = c->compress(m->model, data_in, tmp_buffer, &src, &dest);
							if ((back == 0) && (dest < orig_dest) && (dest > 4)) {
								if (c->buffer_cnt > dest) {
									if (min > dest) {
										min = dest;
										tmp_p = tmp_buffer;
									}
								}
							}
						}
					}
					m = m->next_compr_model;
				}
				if (min != -1) {
					if (tmp_p != cpage_out) {
						for (i = 0; i < min; i++)
							cpage_out[i] = tmp_p[i];
						*sourcelen = orig_src;
						*dstlen = min;
					}
					c->stat_compr_orig += *sourcelen;
					c->stat_compr_new += *dstlen;
					return 0;
				}
			}
		}
		/*else {
		   jffs2_bbc_print1("iPack: manual mode without selected compressor!\n");
		   } */
                
                /*if (!jffs2_bbc_original_compressor->enabled) {
                        jffs2_bbc_print2("jffs2.bbc: WARNING: %s must be enabled! Enabling for this procedure...\n",jffs2_bbc_original_compressor->name);
                }*/
		back = jffs2_bbc_original_compressor->compress(NULL, data_in, cpage_out, sourcelen, dstlen);
		jffs2_bbc_original_compressor->stat_compr_orig += *sourcelen;
		jffs2_bbc_original_compressor->stat_compr_new += *dstlen;
		return back;


	}

	jffs2_bbc_print1("jffs2.bbc: compress: unimlemented compress mode!!!\n");
	return 0;
}

void jffs2_zlib_decompress(unsigned char *data_in, unsigned char *cpage_out, uint32_t srclen, uint32_t destlen)
{
	struct jffs2_bbc_model_list_node *m;
	struct jffs2_bbc_compressor_type *c;
	char d[2];
	void *sb;
	int i;

	/* If the input too small... */
	if (destlen<=2) {
		cpage_out[0]=data_in[0];
		if (destlen==2) cpage_out[1]=data_in[1];
		return;
	}
	
	sb = jffs2_bbc_model_get_act_sb();
	d[0] = *(data_in);
	d[1] = *(data_in + 1);

	d[0] &= 0x7f;		// Variants support...

	/* Search for model based decompressors... */
	m = jffs2_bbc_get_model_list();
	while (m != NULL) {
		if ((d[0] == m->block_sign[0]) && (d[1] == m->block_sign[1]) && (sb == m->sb)) {
			if (m->compressor == NULL) {
				jffs2_bbc_print3("jffs2.bbc: decompressor for block_sign (%d,%d) not loaded!\n", (int) (d[0]), (int) (d[1]));
			}
			else {
				m->compressor->decompress(m->model, data_in, cpage_out, srclen, destlen);
				m->compressor->stat_decompr++;
				m->stat_decompr++;
			}
			return;
		}
		m = m->next_model;
	}
	/* Is it ZLIB? */
	if ((((int) d[0]) == (int)(jffs2_bbc_original_compressor->block_sign[0])) && (((int) d[1]) == (int)(jffs2_bbc_original_compressor->block_sign[1]))) {
		jffs2_bbc_original_compressor->decompress(NULL, data_in, cpage_out, srclen, destlen);
		jffs2_bbc_original_compressor->stat_decompr++;
		return;
	}
	/* Search for non model based decompressors... */
	c = jffs2_bbc_get_compressor_list();
	while (c != NULL) {
		if (c->model_file_sign == 0) {
			if (((int) (d[0]) == (int) (c->block_sign[0])) && ((int) (d[1]) == (int) (c->block_sign[1]))) {
				c->decompress(NULL, data_in, cpage_out, srclen, destlen);
				c->stat_decompr++;
				return;
			}
		}
		c = c->next;
	}
	/* Is it DUMMY? */
	if ((((int) d[0]) == JFFS2_BBC_DUMMY_BLOCKSIGN_0) && (((int) d[1]) == JFFS2_BBC_DUMMY_BLOCKSIGN_1)) {
		for (i=0;i<destlen;i++) {
			cpage_out[i]=data_in[i+2];
		}
		return;
	}
	/* No matching decompressor found... */
	jffs2_bbc_print4("jffs2.bbc: cannot find model for decompress: bsign=(%d,%d),sb=%d. Using zlib.\n", (int) d[0], (int) d[1], (int) sb);
	jffs2_bbc_original_compressor->decompress(NULL, data_in, cpage_out, srclen, destlen);
	jffs2_bbc_original_compressor->stat_decompr++;
}

#endif
