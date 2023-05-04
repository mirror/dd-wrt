/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdbool.h>
#include "fw_avltree.h"
#include <string.h>
#include <stdlib.h>

static inline int is_root(struct FwAvlNode* node) { return (node->parent == NULL); }
static inline int get_balance(struct FwAvlNode* node) { return node->balance; }
static inline void set_balance(int balance, struct FwAvlNode* node) { node->balance = balance; }
static inline int inc_balance(struct FwAvlNode* node) { return ++node->balance; }
static inline int dec_balance(struct FwAvlNode* node) { return --node->balance; }
static inline struct FwAvlNode* get_parent(struct FwAvlNode* node) { return node->parent; }
static inline void set_parent(struct FwAvlNode* parent, struct FwAvlNode* node) { node->parent = parent; }

static inline struct FwAvlNode* new_node(uint32_t key, void* data)
{
    struct FwAvlNode* node = calloc(1, sizeof(struct FwAvlNode));
    if(node != NULL)
    {
        node->key = key;
        node->data = data;
    }
    return node;
}

static inline struct FwAvlNode* get_first(struct FwAvlNode* node)
{
    while(node->left != NULL)
        node = node->left;
    return node;
}

static inline struct FwAvlNode* get_last(struct FwAvlNode* node)
{
    while(node->right != NULL)
        node = node->right;
    return node;
}

struct FwAvlNode* fwAvlFirst(const struct FwAvlTree* tree)
{
    if((tree != 0) && (tree->root != 0))
        return get_first(tree->root);
    else
        return NULL;
}

struct FwAvlNode* fwAvlLast(const struct FwAvlTree* tree)
{
    if((tree != 0) && (tree->root != 0))
        return get_last(tree->root);
    else
        return NULL;
}

struct FwAvlNode* fwAvlNext(struct FwAvlNode* node)
{
    struct FwAvlNode* parent = NULL;
    struct FwAvlNode* tmp;

    if(node->right != NULL)
    {
        return get_first(node->right);
    }
    else
    {
        tmp = node;
        while( ((parent = get_parent(tmp)) != NULL) && (parent->right == tmp) )
            tmp = parent;
    }
    return parent;
}

struct FwAvlNode* fwAvlPrev(struct FwAvlNode* node)
{
    struct FwAvlNode* parent;
    struct FwAvlNode* tmp;

    if(node->left != NULL)
    {
        tmp = get_first(node->left);
    }
    else
    {
        tmp = node;
        while( ((parent = get_parent(tmp)) != NULL) && (parent->left == tmp) )
            tmp = parent;
    }
    return tmp;
}

static void rotate_left(struct FwAvlNode* node, struct FwAvlTree* tree)
{
    struct FwAvlNode* p = node;
    struct FwAvlNode* q = node->right;
    struct FwAvlNode* parent = get_parent(node);

    if(!is_root(p))
    {
        if(parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    }
    else
    {
        tree->root = q;
    }

    set_parent(parent, q);
    set_parent(q, p);

    p->right = q->left;
    if(p->right != NULL)
    {
        set_parent(p, p->right);
    }
    q->left = p;
}

static void rotate_right(struct FwAvlNode* node, struct FwAvlTree* tree)
{
    struct FwAvlNode* p = node;
    struct FwAvlNode* q = node->left;
    struct FwAvlNode* parent = get_parent(node);

    if(!is_root(p))
    {
        if(parent->left == p)
            parent->left = q;
        else
            parent->right = q;
    }
    else
    {
        tree->root = q;
    }

    set_parent(parent, q);
    set_parent(q, p);

    p->left = q->right;
    if(p->left != NULL)
    {
        set_parent(p, p->left);
    }
    q->right = p;
}

static inline struct FwAvlNode* do_lookup(const uint32_t key,
        const struct FwAvlTree* tree, struct FwAvlNode** pparent,
        struct FwAvlNode** unbalanced, int* is_left)
{
    struct FwAvlNode* node = tree->root;

    *pparent = NULL;
    *unbalanced = node;
    *is_left = 0;

    while (node != NULL)
    {
        if(get_balance(node) != 0)
        {
            *unbalanced = node;
        }

        *pparent = node;

        if(key == node->key)
        {
            return node;
        }
        else
        {
            if((*is_left = node->key > key) != 0)
                node = node->left;
            else
                node = node->right;
        }
    }
    return NULL;
}

void* fwAvlLookup(const uint32_t key, const struct FwAvlTree* tree)
{
    struct FwAvlNode* node = NULL;
    struct FwAvlNode* pparent;
    struct FwAvlNode* unbalanced;
    int is_left;

    if(tree == 0)
    {
        return NULL;
    }

    node = do_lookup(key, tree, &pparent, &unbalanced, &is_left);

    if(node != NULL)
    {
        return node->data;
    }
    else
    {
        return NULL;
    }
}

static inline void set_child(struct FwAvlNode* child, struct FwAvlNode* node,
                        int left)
{
    if(left != 0)
        node->left = child;
    else
        node->right = child;
}

int fwAvlInsert(uint32_t key, void* data, struct FwAvlTree* tree)
{
    int is_left;
    struct FwAvlNode* parent;
    struct FwAvlNode* right;
    struct FwAvlNode* left;
    struct FwAvlNode* unbalanced;
    struct FwAvlNode* node;

    if(do_lookup(key, tree, &parent, &unbalanced, &is_left) != NULL)
        return 1;

    if(!(node = new_node(key, data)))
        return -1;

    tree->count++;
    if(parent == NULL)
    {
        tree->root = node;
        tree->first = node;
        tree->last = node;
        return 0;
    }

    if(is_left != 0)
    {
        if(parent == tree->first)
            tree->first = node;
    }
    else
    {
        if(parent == tree->last)
            tree->last = node;
    }
    set_parent(parent, node);
    set_child(node, parent, is_left);

    while(1)
    {
        if(parent->left == node)
            dec_balance(parent);
        else
            inc_balance(parent);

        if(parent == unbalanced)
            break;

        node = parent;
        parent = get_parent(node);
    }

    switch(get_balance(unbalanced))
    {
        case 1:
        case -1:
            tree->height++;
            break;
        case 0:
            break;
        case 2:
            right = unbalanced->right;

            if(get_balance(right) == 1)
            {
                set_balance(0, unbalanced);
                set_balance(0, right);
            }
            else
            {
                switch(get_balance(right->left))
                {
                    case 1:
                        set_balance(-1, unbalanced);
                        set_balance(0, right);
                        break;
                    case 0:
                        set_balance(0, unbalanced);
                        set_balance(0, right);
                        break;
                    case -1:
                        set_balance(0, unbalanced);
                        set_balance(1, right);
                        break;
                }
                set_balance(0, right->left);
                rotate_right(right, tree);
            }
            rotate_left(unbalanced, tree);
            break;
        case -2:
            left = unbalanced->left;

            if(get_balance(left) == -1)
            {
                set_balance(0, unbalanced);
                set_balance(0, left);
            }
            else
            {
                switch(get_balance(left->right))
                {
                    case 1:
                        set_balance(0, unbalanced);
                        set_balance(-1, left);
                        break;
                    case 0:
                        set_balance(0, unbalanced);
                        set_balance(0, left);
                        break;
                    case -1:
                        set_balance(1, unbalanced);
                        set_balance(0, left);
                        break;
                }
                set_balance(0, left->right);
                rotate_left(left, tree);
            }
            rotate_right(unbalanced, tree);
            break;
    }
    return 0;
}

struct FwAvlTree*  fwAvlInit(void)
{
    struct FwAvlTree* tree = calloc(1, sizeof(struct FwAvlTree));
    return tree; /* The calling function checks for NULL */
}

struct FwQNode* newFwQNode(struct FwAvlNode* treeNode)
{
    struct FwQNode* q_node = calloc(1, sizeof(struct FwQNode));

    if(q_node != NULL)
    {
        q_node->treeNode = treeNode;
        q_node->next = NULL;
    }
    return(q_node);
}

struct FwQNode* fwAvlSerialize(struct FwAvlTree* tree)
{
    struct FwQNode* head;
    struct FwQNode* node;
    struct FwQNode* tail;

    if((tree == NULL) || (tree->root == NULL)) return NULL;

    head = newFwQNode(tree->root);
    node = head;
    tail = head;

    while(node)
    {
        if(node->treeNode->left != NULL)
        {
            tail->next = newFwQNode(node->treeNode->left);
            tail = tail->next;
        }

        if(node->treeNode->right != NULL)
        {
            tail->next = newFwQNode(node->treeNode->right);
            tail = tail->next;
        }

        node = node->next;
    }
    return head;
}

void fwAvlDeleteTree(struct FwAvlTree* tree, void (*dataDelete)(void* data))
{
    struct FwQNode* node = fwAvlSerialize(tree);
    struct FwQNode* tmp;

    while(node != NULL)
    {
        if(dataDelete)
            dataDelete(node->treeNode->data);
        free(node->treeNode);
        tmp = node;
        node = node->next;
        free(tmp);
    }
    free(tree);
}
