/* src/toolbox/avl.c - AVL tree implementation

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

#include "toolbox/avl.hpp"
#include <assert.h>                     // for assert
#include <stddef.h>                     // for NULL
#include "mm/memory.hpp"                // for NEW
#include "threads/mutex.hpp"            // for Mutex
#include "toolbox/logging.hpp"          // for log_print, log_finish, etc
#include "vm/types.hpp"                 // for s4
#include "vm/vm.hpp"                    // for vm_abort

/* define direction in an AVL node ********************************************/

#define AVL_LEFT     0
#define AVL_RIGHT    1


/* avl_tree_t *****************************************************************/

struct avl_tree_t {
   Mutex*          mutex;              ///< Mutex to lock the tree.
   avl_node_t     *root;               /* pointer to root node               */
   avl_comparator *comparator;         /* pointer to comparison function     */
   s4              entries;            /* contains number of entries         */
};


/* avl_node_t *****************************************************************/

struct avl_node_t {
   void       *data;                   /* pointer to data structure          */
   s4          balance;                /* the range of the field is -2...2   */
   avl_node_t *childs[2];              /* pointers to the child nodes        */
};


/* avl_create ******************************************************************

   Creates a AVL tree structure.

*******************************************************************************/

avl_tree_t *avl_create(avl_comparator *comparator)
{
	avl_tree_t *t;

	t = NEW(avl_tree_t);

	t->mutex      = new Mutex();
	t->root       = NULL;
	t->comparator = comparator;
	t->entries    = 0;

	return t;
}


/* avl_newnode *****************************************************************

   Creates a new AVL node and sets the pointers correctly.

*******************************************************************************/

static avl_node_t *avl_newnode(void *data)
{
	avl_node_t *n;

	n = NEW(avl_node_t);

	n->data      = data;

	/* ATTENTION: NEW allocates memory zeroed out */

/* 	n->balance   = 0; */
/* 	n->childs[0] = NULL; */
/* 	n->childs[1] = NULL; */

	return n;
}


/* avl_rotate_left *************************************************************

   Does a left rotation on an AVL node.

   A (node)         B
    \              / \
     B       -->  A   C
      \
       C

*******************************************************************************/

static void avl_rotate_left(avl_node_t **node)
{
	avl_node_t *tmp;
	avl_node_t *tmpnode;

	/* rotate the node */

	tmp                       = *node;
	tmpnode                   = tmp->childs[AVL_RIGHT];
	tmp->childs[AVL_RIGHT]    = tmpnode->childs[AVL_LEFT];
	tmpnode->childs[AVL_LEFT] = tmp;

	/* set new parent node */

	*node                     = tmpnode;
}


/* avl_rotate_right ************************************************************

   Does a right rotation on an AVL node.

       C (node)         B
      /                / \
     B           -->  A   C
    /
   A

*******************************************************************************/

static void avl_rotate_right(avl_node_t **node)
{
	avl_node_t *tmp;
	avl_node_t *tmpnode;

	/* rotate the node */

	tmp                        = *node;
	tmpnode                    = tmp->childs[AVL_LEFT];
	tmp->childs[AVL_LEFT]      = tmpnode->childs[AVL_RIGHT];
	tmpnode->childs[AVL_RIGHT] = tmp;

	/* set new parent node */

	*node                      = tmpnode;
}


/* avl_adjust_balance **********************************************************

   Does a balance adjustment after a double rotation.

*******************************************************************************/

static void avl_adjust_balance(avl_node_t *node)
{
	avl_node_t *left;
	avl_node_t *right;

	left  = node->childs[AVL_LEFT];
	right = node->childs[AVL_RIGHT];

	switch (node->balance) {
	case -1:
		left->balance  = 0;
		right->balance = 1;
		break;

	case 0:
		left->balance  = 0;
		right->balance = 0;
		break;

	case 1:
		left->balance  = -1;
		right->balance = 0;
		break;
	}

	node->balance = 0;
}


/* avl_insert_intern ***********************************************************

   Inserts a AVL node into a AVL tree.

*******************************************************************************/

static s4 avl_insert_intern(avl_tree_t *tree, avl_node_t **node, void *data)
{
	avl_node_t *tmpnode;
	s4          res;
	s4          direction;
	s4          insert;
	s4          balance;

	/* set temporary variable */

	tmpnode = *node;

	/* per default no node was inserted */

	insert = 0;

	/* compare the current node */

	res = tree->comparator(tmpnode->data, data);

	/* is this node already in the tree? */

	if (res == 0)
		vm_abort("avl_insert_intern: node already in the tree");

	/* goto left or right child */

	direction = (res < 0) ? AVL_LEFT : AVL_RIGHT;

	/* there is a child, go recursive */

	if (tmpnode->childs[direction]) {
		balance = avl_insert_intern(tree, &tmpnode->childs[direction], data);
	}
	else {
		avl_node_t *newnode;

		/* no child, create this node */

		newnode = avl_newnode(data);

		/* insert into parent node */

		tmpnode->childs[direction] = newnode;

		/* node was inserted, but don't set insert to 1, since this
		   insertion is handled in this recursion depth */

		balance = 1;
	}

	/* add insertion value to node balance, value depends on the
	   direction */

	tmpnode->balance += (direction == AVL_LEFT) ? -balance : balance;

	if ((balance != 0) && (tmpnode->balance != 0)) {
		if (tmpnode->balance < -1) {
			/* left subtree too tall: right rotation needed */

			if (tmpnode->childs[AVL_LEFT]->balance < 0) {
				avl_rotate_right(&tmpnode);

				/* simple balance adjustments */

				tmpnode->balance = 0;
				tmpnode->childs[AVL_RIGHT]->balance = 0;
			}
			else {
				avl_rotate_left(&tmpnode->childs[AVL_LEFT]);
				avl_rotate_right(&tmpnode);
				avl_adjust_balance(tmpnode);
			}
		}
		else if (tmpnode->balance > 1) {
			/* right subtree too tall: left rotation needed */

			if (tmpnode->childs[AVL_RIGHT]->balance > 0) {
				avl_rotate_left(&tmpnode);

				/* simple balance adjustments */

				tmpnode->balance = 0;
				tmpnode->childs[AVL_LEFT]->balance = 0;
			}
			else {
				avl_rotate_right(&tmpnode->childs[AVL_RIGHT]);
				avl_rotate_left(&tmpnode);
				avl_adjust_balance(tmpnode);
			}
		}
		else {
			insert = 1;
		}
	}

	/* set back node */

	*node = tmpnode;

	/* insertion was ok */

	return insert;
}


/* avl_insert ******************************************************************

   Inserts a AVL node into a AVL tree.

*******************************************************************************/

bool avl_insert(avl_tree_t *tree, void *data)
{
	assert(tree);
	assert(data);

	tree->mutex->lock();

	/* if we don't have a root node, create one */

	if (tree->root == NULL)
		tree->root = avl_newnode(data);
	else
		avl_insert_intern(tree, &tree->root, data);

	/* increase entries count */

	tree->entries++;

	tree->mutex->unlock();

	/* insertion was ok */

	return true;
}


/* avl_find ********************************************************************

   Find a given data structure in the AVL tree, with the comparision
   function of the tree.

*******************************************************************************/

void *avl_find(avl_tree_t *tree, void *data)
{
	avl_node_t *node;
	s4          res;

	assert(tree);
	assert(data);

	tree->mutex->lock();

	/* search the tree for the given node */

	for (node = tree->root; node != NULL; ) {
		/* compare the current node */

		res = tree->comparator(node->data, data);

		/* was the entry found? return it */

		if (res == 0) {
			tree->mutex->unlock();

			return node->data;
		}

		/* goto left or right child */

		node = node->childs[(res < 0) ? AVL_LEFT : AVL_RIGHT];
	}

	tree->mutex->unlock();

	/* entry was not found, returning NULL */

	return NULL;
}


/* avl_dump ********************************************************************

   Dumps the AVL tree starting with node.

*******************************************************************************/

#if !defined(NDEBUG)
void avl_dump(avl_node_t* node, s4 indent)
{
	s4 tmp;

	tmp = indent;

	if (node == NULL)
			return;

	if (node->childs[AVL_RIGHT])
		avl_dump(node->childs[AVL_RIGHT], tmp + 1);

	log_start();

	while(indent--)
		log_print("   ");

	log_print("%p (%d)", node->data, node->balance);
	log_finish();

	if (node->childs[AVL_LEFT])
		avl_dump(node->childs[AVL_LEFT], tmp + 1);
}
#endif


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
