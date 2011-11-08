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
#ifndef __XR_AVL64_H__
#define __XR_AVL64_H__

#include <sys/types.h>

typedef struct	avl64node {
	struct	avl64node	*avl_forw;	/* pointer to right child  (> parent) */
	struct	avl64node *avl_back;	/* pointer to left child  (< parent) */
	struct	avl64node *avl_parent;	/* parent pointer */
	struct	avl64node *avl_nextino;	/* next in-order; NULL terminated list*/
	char		 avl_balance;	/* tree balance */
} avl64node_t;

/*
 * avl-tree operations
 */
typedef struct avl64ops {
	__uint64_t	(*avl_start)(avl64node_t *);
	__uint64_t	(*avl_end)(avl64node_t *);
} avl64ops_t;

/*
 * avoid complaints about multiple def's since these are only used by
 * the avl code internally
 */
#ifndef AVL_START
#define	AVL_START(tree, n)	(*(tree)->avl_ops->avl_start)(n)
#define	AVL_END(tree, n)	(*(tree)->avl_ops->avl_end)(n)
#endif

/*
 * tree descriptor:
 *	root points to the root of the tree.
 *	firstino points to the first in the ordered list.
 */
typedef struct avl64tree_desc {
	avl64node_t	*avl_root;
	avl64node_t	*avl_firstino;
	avl64ops_t	*avl_ops;
} avl64tree_desc_t;

/* possible values for avl_balance */

#define AVL_BACK	1
#define AVL_BALANCE	0
#define AVL_FORW	2

/*
 * 'Exported' avl tree routines
 */
avl64node_t
*avl64_insert(
	avl64tree_desc_t *tree,
	avl64node_t *newnode);

void
avl64_delete(
	avl64tree_desc_t *tree,
	avl64node_t *np);

void
avl64_insert_immediate(
	avl64tree_desc_t *tree,
	avl64node_t *afterp,
	avl64node_t *newnode);

void
avl64_init_tree(
	avl64tree_desc_t  *tree,
	avl64ops_t *ops);

avl64node_t *
avl64_findrange(
	avl64tree_desc_t *tree,
	__uint64_t value);

avl64node_t *
avl64_find(
	avl64tree_desc_t *tree,
	__uint64_t value);

avl64node_t *
avl64_findanyrange(
	avl64tree_desc_t *tree,
	__uint64_t	start,
	__uint64_t	end,
	int     checklen);


avl64node_t *
avl64_findadjacent(
	avl64tree_desc_t *tree,
	__uint64_t	value,
	int		dir);

void
avl64_findranges(
	register avl64tree_desc_t *tree,
	register __uint64_t	start,
	register __uint64_t	end,
	avl64node_t	        **startp,
	avl64node_t		**endp);

/*
 * avoid complaints about multiple def's since these are only used by
 * the avl code internally
 */
#ifndef AVL_PRECEED
#define AVL_PRECEED	0x1
#define AVL_SUCCEED	0x2

#define AVL_INCLUDE_ZEROLEN	0x0000
#define AVL_EXCLUDE_ZEROLEN	0x0001
#endif

#endif /* __XR_AVL64_H__ */
