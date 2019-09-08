/*
 * node.c: implementation of automata node
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

#ifndef __KERNEL__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#endif
#include "ndpi_api.h"
#include "../include/node.h"
#include "sort.h"

/* reallocation step for AC_NODE_t.matched_patterns */
#define REALLOC_CHUNK_MATCHSTR 8

/* reallocation step for AC_NODE_t.outgoing array */
#define REALLOC_CHUNK_OUTGOING 8
/* TODO: For different depth of node, number of outgoing edges differs
   considerably, It is efficient to use different chunk size for 
   different depths */

/* Private function prototype */
AC_NODE_t *  node_init (AC_NODE_t * thiz);
int  node_edge_compare (const void * l, const void * r);
int  node_has_matchstr (AC_NODE_t * thiz, AC_PATTERN_t * newstr);


/******************************************************************************
 * FUNCTION: node_create
 * Create the node
 ******************************************************************************/
AC_NODE_t * node_create(void)
{
  AC_NODE_t * thiz =  (AC_NODE_t *) ndpi_malloc (sizeof(AC_NODE_t));
  if(!node_init(thiz)) {
	  ndpi_free(thiz);
	  return NULL;
  }
  return thiz;
}

/******************************************************************************
 * FUNCTION: node_init
 * Initialize node
 ******************************************************************************/
AC_NODE_t * node_init(AC_NODE_t * thiz)
{
  if(!thiz) return thiz;
  memset(thiz, 0, sizeof(AC_NODE_t));

  thiz->outgoing_max = REALLOC_CHUNK_OUTGOING;
  thiz->outgoing = (struct edge *) ndpi_malloc
    (thiz->outgoing_max*sizeof(struct edge));
  if(!thiz->outgoing) {
	return NULL;
  }

  thiz->matched_patterns_max = REALLOC_CHUNK_MATCHSTR;
  thiz->matched_patterns = (AC_PATTERN_t *) ndpi_malloc
    (thiz->matched_patterns_max*sizeof(AC_PATTERN_t));
  if(!thiz->matched_patterns) {
	ndpi_free(thiz->outgoing);
	return NULL;
  }
  return thiz;
}

/******************************************************************************
 * FUNCTION: node_release
 * Release node
 ******************************************************************************/
void node_release(AC_NODE_t * thiz)
{
  if(thiz->matched_patterns) {
	ndpi_free(thiz->matched_patterns);
	thiz->matched_patterns = NULL;
  }
  if(thiz->outgoing) {
	ndpi_free(thiz->outgoing);
	thiz->outgoing = NULL;
  }
  memset(thiz, 0, sizeof(AC_NODE_t));
  ndpi_free(thiz);
}

/******************************************************************************
 * FUNCTION: node_find_next
 * Find out the next node for a given Alpha to move. this function is used in
 * the pre-processing stage in which edge array is not sorted. so it uses
 * linear search.
 ******************************************************************************/
AC_NODE_t * node_find_next(AC_NODE_t * thiz, AC_ALPHABET_t alpha)
{
  int i;

  for (i=0; i < thiz->outgoing_degree; i++)
    {
      if(thiz->outgoing[i].alpha == alpha) {
	return (thiz->outgoing[i].next);
      }
    }
  return NULL;
}

/******************************************************************************
 * FUNCTION: node_findbs_next
 * Find out the next node for a given Alpha. this function is used after the
 * pre-processing stage in which we sort edges. so it uses Binary Search.
 ******************************************************************************/
AC_NODE_t * node_findbs_next (AC_NODE_t * thiz, AC_ALPHABET_t alpha)
{
  int min, max, mid;
  AC_ALPHABET_t amid;

  min = 0;
  max = thiz->outgoing_degree - 1;

  while (min <= max)
    {
      mid = (min+max) >> 1;
      amid = thiz->outgoing[mid].alpha;
      if (alpha > amid)
	min = mid + 1;
      else if (alpha < amid)
	max = mid - 1;
      else
	return (thiz->outgoing[mid].next);
    }
  return NULL;
}

/******************************************************************************
 * FUNCTION: node_has_matchstr
 * Determine if a final node contains a pattern in its accepted pattern list
 * or not. return values: 1 = it has, 0 = it hasn't
 ******************************************************************************/
int node_has_matchstr (AC_NODE_t * thiz, AC_PATTERN_t * newstr)
{
  int i, j;
  AC_PATTERN_t * str;

  for (i=0; i < thiz->matched_patterns_num; i++)
    {
      str = &thiz->matched_patterns[i];

      if (str->length != newstr->length)
	continue;

      for (j=0; j<(int)str->length; j++)
	if(str->astring[j] != newstr->astring[j])
	  continue;

      if (j == str->length)
	return 1;
    }
  return 0;
}

/******************************************************************************
 * FUNCTION: node_create_next
 * Create the next node for the given alpha.
 ******************************************************************************/
AC_NODE_t * node_create_next (AC_NODE_t * thiz, AC_ALPHABET_t alpha)
{
  AC_NODE_t * next;
  next = node_find_next (thiz, alpha);
  if (next)
    /* The edge already exists */
    return NULL;
  /* Otherwise register new edge */
  next = node_create ();
  if(next) {
	node_register_outgoing(thiz, next, alpha);
	next->depth = thiz->depth+1;
  }

  return next;
}

/******************************************************************************
 * FUNCTION: node_register_matchstr
 * Adds the pattern to the list of accepted pattern.
 ******************************************************************************/
void node_register_matchstr (AC_NODE_t * thiz, AC_PATTERN_t * str)
{
  /* Check if the new pattern already exists in the node list */
  if (node_has_matchstr(thiz, str))
    return;

  /* Manage memory */
  if (thiz->matched_patterns_num >= thiz->matched_patterns_max)
    {
      thiz->matched_patterns = (AC_PATTERN_t *) ndpi_realloc 
	(thiz->matched_patterns, thiz->matched_patterns_max*sizeof(AC_PATTERN_t),
	 (REALLOC_CHUNK_MATCHSTR+thiz->matched_patterns_max)*sizeof(AC_PATTERN_t));

      thiz->matched_patterns_max += REALLOC_CHUNK_MATCHSTR;
    }

  thiz->matched_patterns[thiz->matched_patterns_num].astring = str->astring;
  thiz->matched_patterns[thiz->matched_patterns_num].length = str->length;
  memcpy(&thiz->matched_patterns[thiz->matched_patterns_num].rep, &str->rep, sizeof(AC_REP_t));
  thiz->matched_patterns_num++;
}

/******************************************************************************
 * FUNCTION: node_register_outgoing
 * Establish an edge between two nodes
 ******************************************************************************/
void node_register_outgoing
(AC_NODE_t * thiz, AC_NODE_t * next, AC_ALPHABET_t alpha)
{
  if(thiz->outgoing_degree >= thiz->outgoing_max)
    {
      thiz->outgoing = (struct edge *) ndpi_realloc 
	(thiz->outgoing, thiz->outgoing_max*sizeof(struct edge),
	 (REALLOC_CHUNK_OUTGOING+thiz->outgoing_max)*sizeof(struct edge));
      thiz->outgoing_max += REALLOC_CHUNK_OUTGOING;
    }

  thiz->outgoing[thiz->outgoing_degree].alpha = alpha;
  thiz->outgoing[thiz->outgoing_degree++].next = next;
}

/******************************************************************************
 * FUNCTION: node_edge_compare
 * Comparison function for qsort. see man qsort.
 ******************************************************************************/
int node_edge_compare (const void * l, const void * r)
{
  /* According to man page:
   * The comparison function must return an integer less than, equal to, or
   * greater than zero if the first argument is considered to be
   * respectively less than, equal to, or greater than the second. if  two
   * members compare as equal, their order in the sorted array is undefined.
   *
   * NOTE: Because edge alphabets are unique in every node we ignore
   * equivalence case.
   **/
  if ( ((struct edge *)l)->alpha >= ((struct edge *)r)->alpha )
    return 1;
  else
    return -1;
}

static void node_edge_swap (void * l, void * r, int s)
{
/* We hope for an optimizer that will leave only 1 option. */
switch(sizeof(struct edge)) {
case 16: // for 64bit
	{
	uint64_t t1,t2,*p1 = l,*p2 = r;

	t1  = *p1; t2    = p1[1];
	*p1 = *p2; p1[1] = p2[1];
	*p2 = t1;  p2[1] = t2;
	}
	break;
case 8: // for 32bit
	{
	uint32_t t1,t2,*p1 = l,*p2 = r;

	t1  = *p1; t2    = p1[1];
	*p1 = *p2; p1[1] = p2[1];
	*p2 = t1;  p2[1] = t2;
	}
	break;
default: { // What is ???
	  uint8_t a;
	  struct edge *le = l, *re = r;
	  void *p = le->next;  le->next  = re->next;  re->next  = p;
		a = le->alpha; le->alpha = re->alpha; re->alpha = a;
	 }
}
}

/******************************************************************************
 * FUNCTION: node_sort_edges
 * sorts edges alphabets.
 ******************************************************************************/
void node_sort_edges (AC_NODE_t * thiz)
{
  sort ((void *)thiz->outgoing, thiz->outgoing_degree, 
		sizeof(struct edge), node_edge_compare, node_edge_swap);
}
