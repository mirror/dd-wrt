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


#ifndef _FW_AVL_TREE_H_
#define _FW_AVL_TREE_H_

#include <stdint.h>
#include <stdlib.h>

struct FwAvlNode
{
    uint32_t            key;
    void*               data;
    int                 balance;
    struct FwAvlNode*   left;
    struct FwAvlNode*   right;
    struct FwAvlNode*   parent;
};

struct FwAvlTree
{
    unsigned            count;
    size_t              height;
    struct FwAvlNode*   root;
    struct FwAvlNode*   first;
    struct FwAvlNode*   last;
};

struct FwQNode
{
    struct FwAvlNode*   treeNode;
    struct FwQNode*     next;
};

struct FwAvlTree*   fwAvlInit(void);
int                 fwAvlInsert(uint32_t key, void* data, struct FwAvlTree* tree);
void*               fwAvlLookup(const uint32_t key, const struct FwAvlTree* tree);
struct FwAvlNode*   fwAvlFirst(const struct FwAvlTree* tree);
struct FwAvlNode*   fwAvlLast(const struct FwAvlTree* tree);
struct FwAvlNode*   fwAvlNext(struct FwAvlNode* node);
struct FwAvlNode*   fwAvlPrev(struct FwAvlNode* node);
struct FwQNode*     fwAvlSerialize(struct FwAvlTree* tree);
void                fwAvlDeleteTree(struct FwAvlTree* tree, void (*dataDelete)(void* data));

#endif
