/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001, 2002, 2003 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Memory allocation/anti-leak system.  Yes, this *IS* stolen from Apache
 * also.  What can I say?  It makes sense, and it's safe (more overhead
 * though)
 * $Id: pool.h,v 1.1 2006/04/24 11:39:28 honor Exp $
 */

#ifndef PR_POOL_H
#define PR_POOL_H

typedef struct pool pool;

extern pool *permanent_pool;

void init_pools(void);
void free_pools(void);
pool *make_sub_pool(pool *);		/* All pools are sub-pools of perm */
pool *pr_pool_create_sz(pool *, int);

/* The make_named_sub_pool() function, which was never really supported,
 * has been deprecated.  This macro is for supporting those contrib modules
 * that still may make use of this old function.  Newer code should simply
 * use make_sub_pool() directly.
 */
#define make_named_sub_pool(p, s)       make_sub_pool((p))

/* Clears out _everything_ in a pool, destroying any sub-pools */
void destroy_pool(struct pool *);

/* Allocate memory from a pool */
void *palloc(struct pool *, int);
void *pallocsz(struct pool *, int);
void *pcalloc(struct pool *, int);
void *pcallocsz(struct pool *, int);
extern char *pstrdup(struct pool *, const char *);
extern char *pstrndup(struct pool *, const char *, int);
char *pstrcat(struct pool *, ...);       /* Must be char * */
char *pdircat(struct pool *, ...);	/* Must be char * */

/* MM debugging */
void pr_pool_debug_memory(void (*)(const char *, ...));
void pr_pool_tag(struct pool *, const char *);

/* Array management */

typedef struct {
  pool *pool;
  int elt_size;
  int nelts;
  int nalloc;
  void *elts;
} array_header;

array_header *make_array(pool *, int, int);
void *push_array(array_header *);
void array_cat(array_header *, const array_header *);
array_header *append_arrays(pool *, const array_header *, const array_header *);
array_header *copy_array(pool *, const array_header *);
array_header *copy_array_str(pool *, const array_header *);
array_header *copy_array_hdr(pool *, const array_header *);

/* Alarm signals can easily interfere with the pooled memory operations, thus
 * pr_alarms_block() and pr_alarms_unblock() provide for re-entrant security.
 */
extern void pr_alarms_block(void);
extern void pr_alarms_unblock(void);

FILE *pfopen(struct pool *, const char *, const char *);
FILE *pfdopen(struct pool *, int, const char *);
int popenf(struct pool *, const char *, int, int);

int pfclose(struct pool *, FILE *);
int pclosef(struct pool *, int);

/* Functions for cleanup handlers */
void register_cleanup(pool *, void *, void (*)(void *), void (*)(void *));
void register_file_cleanups(pool *, FILE *);
void unregister_cleanup(pool *, void *, void (*)(void *));

/* minimum free bytes in a new block pool */

#define BLOCK_MINFREE	PR_TUNABLE_NEW_POOL_SIZE

#endif /* PR_POOL_H */
