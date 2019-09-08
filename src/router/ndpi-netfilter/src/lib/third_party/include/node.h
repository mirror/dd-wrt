/*
 * node.h: automata node header file
 * This file is part of multifast.
 *
 Copyright 2010-2012 Kamiar Kanani <kamiar.kanani@gmail.com>

 multifast is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 multifast is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with multifast.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NODE_H_
#define _NODE_H_

#include "actypes.h"

/* Forward Declaration */
struct edge;

/*
 * automata node
 * 3 pointers + 16 bytes : 40/28 bytes for 64/32 bit
 */
typedef struct ac_node
{
  AC_PATTERN_t   * matched_patterns;   /* Array of matched patterns */
  struct edge    * outgoing;           /* Array of outgoing edges */

  struct ac_node * failure_node;       /* The failure node of this node */

  int id;                              /* Node ID : set after finalize() */
  unsigned short final;                /* 0: no ; 1: yes, it is a final node */
  unsigned short depth;                /* depth: distance between this node and the root */

  unsigned short matched_patterns_num; /* Number of matched patterns at this node */
  unsigned short matched_patterns_max; /* Max capacity of allocated memory for matched_patterns */

  unsigned short outgoing_degree;      /* Number of outgoing edges */
  unsigned short outgoing_max;         /* Max capacity of allocated memory for outgoing */
} AC_NODE_t;

#ifndef __SIZEOF_POINTER__
#error SIZEOF_POINTER not defined!
#endif

/* The Edge of the Node
 * After changes this structure 
 * needed review node_edge_swap()
 * if __SIZEOF_POINTER__ == 8 then sizeof(struct edge) == 16
 * if __SIZEOF_POINTER__ == 4 then sizeof(struct edge) == 8
 */

struct edge
{
  struct ac_node * next; /* Target of the edge */
  AC_ALPHABET_t alpha; /* Edge alpha */
}
#if __SIZEOF_POINTER__ == 8
__attribute__((aligned(8)));
#else
__attribute__((aligned(4)));
#endif



AC_NODE_t * node_create            (void);
AC_NODE_t * node_create_next       (AC_NODE_t * thiz, AC_ALPHABET_t alpha);
void        node_register_matchstr (AC_NODE_t * thiz, AC_PATTERN_t * str);
void        node_register_outgoing (AC_NODE_t * thiz, AC_NODE_t * next, AC_ALPHABET_t alpha);
AC_NODE_t * node_find_next         (AC_NODE_t * thiz, AC_ALPHABET_t alpha);
AC_NODE_t * node_findbs_next       (AC_NODE_t * thiz, AC_ALPHABET_t alpha);
void        node_release           (AC_NODE_t * thiz);
void        node_sort_edges        (AC_NODE_t * thiz);

#endif
