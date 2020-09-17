// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __SYS_AVL_H__
#define __SYS_AVL_H__


typedef struct	avlnode {
	struct	avlnode	*avl_forw;	/* pointer to right child  (> parent) */
	struct	avlnode *avl_back;	/* pointer to left child  (< parent) */
	struct	avlnode *avl_parent;	/* parent pointer */
	struct	avlnode *avl_nextino;	/* next in-order; NULL terminated list*/
	char		 avl_balance;	/* tree balance */
} avlnode_t;

/*
 * avl-tree operations
 */
typedef struct avlops {
	uintptr_t	(*avl_start)(avlnode_t *);
	uintptr_t	(*avl_end)(avlnode_t *);
} avlops_t;

#define	AVL_START(tree, n)	(*(tree)->avl_ops->avl_start)(n)
#define	AVL_END(tree, n)	(*(tree)->avl_ops->avl_end)(n)

/*
 * tree descriptor:
 *	root points to the root of the tree.
 *	firstino points to the first in the ordered list.
 */
typedef struct avltree_desc {
	avlnode_t	*avl_root;
	avlnode_t	*avl_firstino;
	avlops_t	*avl_ops;
	short		 avl_flags;
} avltree_desc_t;

/* possible values for avl_balance */

#define AVL_BACK	1
#define AVL_BALANCE	0
#define AVL_FORW	2

/* possible values for avl_flags */

#define AVLF_DUPLICITY	0x0001		/* no warnings on insert dups */

/*
 * 'Exported' avl tree routines
 */
avlnode_t
*avl_insert(
	avltree_desc_t *tree,
	avlnode_t *newnode);

void
avl_delete(
	avltree_desc_t *tree,
	avlnode_t *np);

void
avl_insert_immediate(
	avltree_desc_t *tree,
	avlnode_t *afterp,
	avlnode_t *newnode);

void
avl_init_tree(
	avltree_desc_t  *tree,
	avlops_t *ops);

static inline avlnode_t *
avl_findrange(
	avltree_desc_t *tree,
	uintptr_t value)
{
	avlnode_t *np = tree->avl_root;

	while (np) {
		if (value < AVL_START(tree, np)) {
			np = np->avl_back;
			continue;
		}
		if (value >= AVL_END(tree, np)) {
			np = np->avl_forw;
			continue;
		}
		ASSERT(AVL_START(tree, np) <= value &&
		       value < AVL_END(tree, np));
		return np;
	}
	return NULL;
}

avlnode_t *
avl_find(
	avltree_desc_t *tree,
	uintptr_t value);

avlnode_t *
avl_findanyrange(
	avltree_desc_t *tree,
	uintptr_t start,
	uintptr_t end,
	int     checklen);


avlnode_t *
avl_findadjacent(
	avltree_desc_t *tree,
	uintptr_t value,
	int		dir);

void
avl_findranges(
	avltree_desc_t *tree,
	uintptr_t start,
	uintptr_t end,
	avlnode_t	        **startp,
	avlnode_t		**endp);

avlnode_t *
avl_firstino(
	avlnode_t		*root);

avlnode_t *
avl_lastino(
	avlnode_t		*root);


#define AVL_PRECEED	0x1
#define AVL_SUCCEED	0x2

#define AVL_INCLUDE_ZEROLEN	0x0000
#define AVL_EXCLUDE_ZEROLEN	0x0001

#endif /* __SYS_AVL_H__ */
