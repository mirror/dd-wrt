/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: lq_avl.c,v 1.1 2005/01/22 14:30:57 tlopatic Exp $
 */

#include <stddef.h>

#include "lq_avl.h"

#define AVLMAX(x, y) ((x > y) ? x : y)
#define AVLMIN(x, y) ((x < y) ? x : y)

void avl_init(struct avl_tree *tree, int (*comp)(void *, void *))
{
  tree->root = NULL;
  tree->comp = comp;
}

static struct avl_node *avl_find_rec(struct avl_node *node, void *key,
                                     int (*comp)(void *, void *))
{
  int diff;

  diff = (*comp)(key, node->key);

  if (diff < 0)
  {
    if (node->left != NULL)
      return avl_find_rec(node->left, key, comp);

    return node;
  }

  if (diff > 0)
  {
    if (node->right != NULL)
      return avl_find_rec(node->right, key, comp);

    return node;
  }

  return node;
}

struct avl_node *avl_find(struct avl_tree *tree, void *key)
{
  struct avl_node *node;

  if (tree->root == NULL)
    return NULL;

  node = avl_find_rec(tree->root, key, tree->comp);

  if ((*tree->comp)(node->key, key) != 0)
    return NULL;

  return node;
}

static void rotate_right(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *left, *parent;

  left = node->left;
  parent = node->parent;

  left->parent = parent;
  node->parent = left;

  if (parent == NULL)
    tree->root = left;

  else
  {
    if (parent->left == node)
      parent->left = left;

    else
      parent->right = left;
  }

  node->left = left->right;
  left->right = node;

  if (node->left != NULL)
    node->left->parent = node;

  node->balance += 1 - AVLMIN(left->balance, 0);
  left->balance += 1 + AVLMAX(node->balance, 0);
}

static void rotate_left(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *right, *parent;

  right = node->right;
  parent = node->parent;

  right->parent = parent;
  node->parent = right;

  if (parent == NULL)
    tree->root = right;

  else
  {
    if (parent->left == node)
      parent->left = right;

    else
      parent->right = right;
  }

  node->right = right->left;
  right->left = node;

  if (node->right != NULL)
    node->right->parent = node;

  node->balance -= 1 + AVLMAX(right->balance, 0);
  right->balance -= 1 - AVLMIN(node->balance, 0);
}

static void post_insert(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent;

  if ((parent = node->parent) == NULL)
    return;

  if (node == parent->left)
  {
    parent->balance--;

    if (parent->balance == 0)
      return;

    if (parent->balance == -1)
    {
      post_insert(tree, parent);
      return;
    }

    if (node->balance == -1)
    {
      rotate_right(tree, parent);
      return;
    }

    rotate_left(tree, node);
    rotate_right(tree, node->parent->parent);
    return;
  }

  parent->balance++;

  if (parent->balance == 0)
    return;

  if (parent->balance == 1)
  {
    post_insert(tree, parent);
    return;
  }

  if (node->balance == 1)
  {
    rotate_left(tree, parent);
    return;
  }

  rotate_right(tree, node);
  rotate_left(tree, node->parent->parent);
  return;
}

int avl_insert(struct avl_tree *tree, struct avl_node *new)
{
  struct avl_node *node;
  int diff;

  new->balance = 0;
  new->left = NULL;
  new->right = NULL;

  if (tree->root == NULL)
  {
    new->parent = NULL;
    tree->root = new;
    return 0;
  }

  node = avl_find_rec(tree->root, new->key, tree->comp);

  diff = (*tree->comp)(new->key, node->key);

  if (diff == 0)
    return -1;

  if (node->balance == 1)
  {
    node->balance = 0;
    new->parent = node;
    node->left = new;
    return 0;
  }
  
  if (node->balance == -1)
  {
    node->balance = 0;
    new->parent = node;
    node->right = new;
    return 0;
  }

  if (diff < 0)
  {
    node->balance = -1;
    new->parent = node;
    node->left = new;
    post_insert(tree, node);
    return 0;
  }

  node->balance = 1;
  new->parent = node;
  node->right = new;
  post_insert(tree, node);
  return 0;
}
