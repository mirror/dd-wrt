/*
 *	Filters: utility functions
 *
 *	Copyright 1998 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "conf/conf.h"
#include "filter/filter.h"

/*
 * find_nth - finds n-th element in linked list. Don't be confused by types, it is really a linked list.
 */
static struct f_tree *
find_nth(struct f_tree *from, int nth)
{
  struct f_tree *pivot;
  int lcount = 0, rcount = 0;
  struct f_tree *left, *right, *next;

  pivot = from;

  left = right = NULL;
  next = from->right;
  while (from = next) {
    next = from->right;
    if (val_compare(pivot->from, from->from)==1) {
      from->right = left;
      left = from;
      lcount++;
    } else {
      from->right = right;
      right = from;
      rcount++;
    }
  }
  if (lcount == nth) 
    return pivot;
  if (lcount < nth)
    return find_nth(right, nth-lcount-1);
  return find_nth(left, nth);
}

/*
 * find_median - Gets list linked by @left, finds its median, trashes pointers in @right.
 */
static struct f_tree *
find_median(struct f_tree *from)
{
  struct f_tree *t = from;
  int cnt = 0;

  if (!from)
    return NULL;
  do {
    t->right = t->left;
    cnt++;
  } while (t = t->left);
  return find_nth(from, cnt/2);
}

/**
 * find_tree
 * @t: tree to search in
 * @val: value to find
 *
 * Search for given value in the tree. I relies on fact that sorted tree is populated
 * by &f_val structures (that can be compared by val_compare()). In each node of tree, 
 * either single value (then t->from==t->to) or range is present.
 *
 * Both set matching and |switch() { }| construction is implemented using this function,
 * thus both are as fast as they can be.
 */
struct f_tree *
find_tree(struct f_tree *t, struct f_val val)
{
  if (!t)
    return NULL;
  if ((val_compare(t->from, val) != 1) &&
      (val_compare(t->to, val) != -1))
    return t;
  if (val_compare(t->from, val) == -1)
    return find_tree(t->right, val);
  else
    return find_tree(t->left, val);
}

/**
 * build_tree
 * @from: degenerated tree (linked by @tree->left) to be transformed into form suitable for find_tree()
 *
 * Transforms denerated tree into balanced tree.
 */
struct f_tree *
build_tree(struct f_tree *from)
{
  struct f_tree *median, *t = from, *next, *left = NULL, *right = NULL;

  median = find_median(from);
  if (!median)
    return NULL;

  do {
    next = t->left;
    if (t == median)
      continue;

    if (val_compare(median->from, t->from)==1) {
      t->left = left;
      left = t;
    } else {
      t->left = right;
      right = t;
    }
  } while(t = next);

  median->left = build_tree(left);
  median->right = build_tree(right);
  return median;
}

struct f_tree *
f_new_tree(void)
{
  struct f_tree * ret;
  ret = cfg_alloc(sizeof(struct f_tree));
  ret->left = ret->right = NULL;
  ret->from.type = ret->to.type = T_VOID;
  ret->from.val.i = ret->to.val.i = 0;
  ret->data = NULL;
  return ret;
}

/**
 * same_tree
 * @t1: first tree to be compared
 * @t2: second one
 *
 * Compares two trees and returns 1 if they are same
 */
int
same_tree(struct f_tree *t1, struct f_tree *t2)
{
  if ((!!t1) != (!!t2))
    return 0;
  if (!t1)
    return 1;
  if (val_compare(t1->from, t2->from))
    return 0;
  if (val_compare(t1->to, t2->to))
    return 0;
  if (!same_tree(t1->left, t2->left))
    return 0;
  if (!same_tree(t1->right, t2->right))
    return 0;
  if (!i_same(t1->data, t2->data))
    return 0;
  return 1;
}
