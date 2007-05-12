/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
 * IPv4 performance optimization (c) 2006, sven-ola(gmx.de)
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
 * $Id: lq_avl.c,v 1.9 2007/04/20 14:23:41 bernd67 Exp $
 */

#include <stddef.h>
#include <time.h>

#include "lq_avl.h"

#define AVLMAX(x, y) ((x > y) ? x : y)
#define AVLMIN(x, y) ((x < y) ? x : y)

void avl_init(struct avl_tree *tree, int (*comp)(void *, void *))
{
  tree->root = NULL;
  tree->comp = comp;
}

static struct avl_node *find_rec_ipv4(struct avl_node *node, void *key)
{
  if (*(unsigned int *)key < *(unsigned int *)node->key)
  {
    if (node->left != NULL)
      return find_rec_ipv4(node->left, key);
  }

  else if (*(unsigned int *)key > *(unsigned int *)node->key)
  {
    if (node->right != NULL)
      return find_rec_ipv4(node->right, key);
  }

  return node;
}

static struct avl_node *find_rec(struct avl_node *node, void *key,
                                 int (*comp)(void *, void *))
{
  int diff;

  if (0 == comp)
    return find_rec_ipv4(node, key);

  diff = (*comp)(key, node->key);

  if (diff < 0)
  {
    if (node->left != NULL)
      return find_rec(node->left, key, comp);

    return node;
  }

  if (diff > 0)
  {
    if (node->right != NULL)
      return find_rec(node->right, key, comp);

    return node;
  }

  return node;
}

struct avl_node *avl_find(struct avl_tree *tree, void *key)
{
  struct avl_node *node;

  if (tree->root == NULL)
    return NULL;

  node = find_rec(tree->root, key, tree->comp);

  if (0 == tree->comp)
  {
    if (0 != inline_avl_comp_ipv4(node->key, key))
      return NULL;
  }

  else
  {
    if ((*tree->comp)(node->key, key) != 0)
      return NULL;
  }

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

static void insert_before(struct avl_node *pos_node, struct avl_node *node)
{
  if (pos_node->prev != NULL)
    pos_node->prev->next = node;

  node->prev = pos_node->prev;
  node->next = pos_node;

  pos_node->prev = node;
}

static void insert_after(struct avl_node *pos_node, struct avl_node *node)
{
  if (pos_node->next != NULL)
    pos_node->next->prev = node;

  node->prev = pos_node;
  node->next = pos_node->next;

  pos_node->next = node;
}

static void remove(struct avl_node *node)
{
  if (node->prev != NULL)
    node->prev->next = node->next;

  if (node->next != NULL)
    node->next->prev = node->prev;
}

int avl_insert(struct avl_tree *tree, struct avl_node *new, int allow_duplicates)
{
  struct avl_node *node;
  struct avl_node *last;
  int diff;

  new->parent = NULL;

  new->left = NULL;
  new->right = NULL;

  new->next = NULL;
  new->prev = NULL;

  new->balance = 0;
  new->leader = 1;

  if (tree->root == NULL)
  {
    tree->root = new;
    return 0;
  }

  node = find_rec(tree->root, new->key, tree->comp);

  last = node;

  while (last->next != NULL && last->next->leader == 0)
    last = last->next;

  if (0 == tree->comp)
    diff = inline_avl_comp_ipv4(new->key, node->key);

  else
    diff = (*tree->comp)(new->key, node->key);

  if (diff == 0)
  {
    if (allow_duplicates == 0)
      return -1;

    new->leader = 0;

    insert_after(last, new);
    return 0;
  }

  if (node->balance == 1)
  {
    insert_before(node, new);

    node->balance = 0;
    new->parent = node;
    node->left = new;
    return 0;
  }
  
  if (node->balance == -1)
  {
    insert_after(last, new);

    node->balance = 0;
    new->parent = node;
    node->right = new;
    return 0;
  }

  if (diff < 0)
  {
    insert_before(node, new);

    node->balance = -1;
    new->parent = node;
    node->left = new;
    post_insert(tree, node);
    return 0;
  }

  insert_after(last, new);

  node->balance = 1;
  new->parent = node;
  node->right = new;
  post_insert(tree, node);
  return 0;
}

static void post_delete(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent;

  if ((parent = node->parent) == NULL)
    return;

  if (node == parent->left)
  {
    parent->balance++;

    if (parent->balance == 0)
    {
      post_delete(tree, parent);
      return;
    }
    
    if (parent->balance == 1)
      return;

    if (parent->right->balance == 0)
    {
      rotate_left(tree, parent);
      return;
    }

    if (parent->right->balance == 1)
    {
      rotate_left(tree, parent);
      post_delete(tree, parent->parent);
      return;
    }

    rotate_right(tree, parent->right);
    rotate_left(tree, parent);
    post_delete(tree, parent->parent);
    return;
  }

  parent->balance--;

  if (parent->balance == 0)
  {
    post_delete(tree, parent);
    return;
  }
    
  if (parent->balance == -1)
    return;

  if (parent->left->balance == 0)
  {
    rotate_right(tree, parent);
    return;
  }

  if (parent->left->balance == -1)
  {
    rotate_right(tree, parent);
    post_delete(tree, parent->parent);
    return;
  }

  rotate_left(tree, parent->left);
  rotate_right(tree, parent);
  post_delete(tree, parent->parent);
}

static struct avl_node *local_min(struct avl_node *node)
{
  while (node->left != NULL)
    node = node->left;

  return node;
}

static void delete_worker(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *parent, *min;

  parent = node->parent;

  if (node->left == NULL && node->right == NULL)
  {
    if (parent == NULL)
    {
      tree->root = NULL;
      return;
    }

    if (parent->left == node)
    {
      parent->left = NULL;
      parent->balance++;

      if (parent->balance == 1)
        return;

      if (parent->balance == 0)
      {
        post_delete(tree, parent);
        return;
      }

      if (parent->right->balance == 0)
      {
        rotate_left(tree, parent);
        return;
      }
      
      if (parent->right->balance == 1)
      {
        rotate_left(tree, parent);
        post_delete(tree, parent->parent);
        return;
      }

      rotate_right(tree, parent->right);
      rotate_left(tree, parent);
      post_delete(tree, parent->parent);
      return;
    }

    if (parent->right == node)
    {
      parent->right = NULL;
      parent->balance--;

      if (parent->balance == -1)
        return;

      if (parent->balance == 0)
      {
        post_delete(tree, parent);
        return;
      }

      if (parent->left->balance == 0)
      {
        rotate_right(tree, parent);
        return;
      }
      
      if (parent->left->balance == -1)
      {
        rotate_right(tree, parent);
        post_delete(tree, parent->parent);
        return;
      }

      rotate_left(tree, parent->left);
      rotate_right(tree, parent);
      post_delete(tree, parent->parent);
      return;
    }
  }

  if (node->left == NULL)
  {
    if (parent == NULL)
    {
      tree->root = node->right;
      node->right->parent = NULL;
      return;
    }

    node->right->parent = parent;

    if (parent->left == node)
      parent->left = node->right;
    
    else
      parent->right = node->right;

    post_delete(tree, node->right);
    return;
  }

  if (node->right == NULL)
  {
    if (parent == NULL)
    {
      tree->root = node->left;
      node->left->parent = NULL;
      return;
    }

    node->left->parent = parent;

    if (parent->left == node)
      parent->left = node->left;

    else
      parent->right = node->left;

    post_delete(tree, node->left);
    return;
  }

  min = local_min(node->right);
  delete_worker(tree, min);
  parent = node->parent;

  min->balance = node->balance;
  min->parent = parent;
  min->left = node->left;
  min->right = node->right;

  if (min->left != NULL)
    min->left->parent = min;

  if (min->right != NULL)
    min->right->parent = min;
    
  if (parent == NULL)
  {
    tree->root = min;
    return;
  }

  if (parent->left == node)
  {
    parent->left = min;
    return;
  }

  parent->right = min;
}

void avl_delete(struct avl_tree *tree, struct avl_node *node)
{
  struct avl_node *next;
  struct avl_node *parent;
  struct avl_node *left;
  struct avl_node *right;

  if (node->leader != 0)
  {
    next = node->next;

    if (next != NULL && next->leader == 0)
    {
      next->leader = 1;
      next->balance = node->balance;

      parent = node->parent;
      left = node->left;
      right = node->right;

      next->parent = parent;
      next->left = left;
      next->right = right;

      if (parent == NULL)
        tree->root = next;

      else
      {
        if (node == parent->left)
          parent->left = next;

        else
          parent->right = next;
      }

      if (left != NULL)
        left->parent = next;

      if (right != NULL)
        right->parent = next;
    }

    else
      delete_worker(tree, node);
  }

  remove(node);
}

struct avl_node *avl_walk_first(struct avl_tree *tree)
{
  struct avl_node *node = tree->root;

  if (node == NULL)
    return NULL;

  return local_min(node);
}

struct avl_node *avl_walk_next(struct avl_node *node)
{
  return node->next;
}
