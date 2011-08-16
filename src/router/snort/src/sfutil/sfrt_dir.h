/****************************************************************************
 *
 * Copyright (C) 2006-2011 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/*
 * @file    sfdir.h
 * @author  Adam Keeton <akeeton@sourcefire.com>
 * @date    Thu July 20 10:16:26 EDT 2006
 *
 * The implementation uses an multibit-trie that is similar to Gupta et-al's 
 * DIR-n-m.
*/

#ifndef SFRT_DIR_H_
#define SFRT_DIR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
/*******************************************************************/
/* DIR-n-m data structures 
 * Each table in the DIR-n-m method is represented by a 
 * dir_sub_table_t.  They are managed by a dir_table_t. */
typedef struct
{
    word *entries;
    char *lengths;
    int num_entries; /* Number of entries in this table */
    int width;       /* width of this table. */
                     /* While one determines the other, this way fewer 
                      * calculations are needed at runtime, since both
                      * are used. */
    int cur_num;     /* Present number of used nodes */
} dir_sub_table_t;

/* Master data structure for the DIR-n-m derivative */
typedef struct
{
    int *dimensions;    /* DIR-n-m will consist of any number of arbitrarily
                         * long tables. This variable keeps track of the
                         * dimensions */
    int dim_size;       /* And this variable keeps track of 'dimensions''s 
                         * dimensions! */
    uint32_t mem_cap;  /* User-defined maximum memory that can be allocated
                         * for the DIR-n-m derivative */

    int cur_num;        /* Present number of used nodes */

    uint32_t allocated;

    dir_sub_table_t *sub_table;
} dir_table_t;

/*******************************************************************/
/* DIR-n-m functions, these are not intended to be called directly */
dir_table_t *  sfrt_dir_new(uint32_t mem_cap, int count,...);
void           sfrt_dir_free(void *);
tuple_t        sfrt_dir_lookup(IP ip, void *table);
int            sfrt_dir_insert(IP ip, int len, word data_index,
                               int behavior, void *table);
uint32_t      sfrt_dir_usage(void *table);

#endif /* SFRT_DIR_H_ */

