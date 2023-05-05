/*
** bnfa_search.c
**
** Basic multi-pattern search engine using Aho-Corasick NFA construction.
**
** Version 3.0  (based on acsmx.c and acsmx2.c)
**
** author: marc norton
** date:   started 12/21/05
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** General Design
**  Aho-Corasick based NFA state machine.
**  Compacted sparse storage mode for better performance.
**  Up to 16 Million states + transitions (combined) in compacted sparse mode.
**
**  ** Compacted sparse array storage **
**
**  The primary data is held in one array.
**  The patterns themselves are stored separately.
**  The matching lists of patterns for each state are stored separately as well.
**  The compacted sparse format improves caching/performance.
**
**   word 1 : state  ( only low 24 bits are used )
**   word 2 : control word = cb << 24 | fs
**   cb: control byte
**       cb = mb | fb | nt
**   mb : 8th bit - if set state has matching patterns bit
**   fb : 7th bit - if set full storage array bit (256 entries used),
                    else sparse
**   nt : 0-63= number of transitions (more than 63 requires full storage)
**   fs: 24 bits for failure state transition index.
**   word 3+ : transition word =  input<<24 |  next-state-index
**   input : 8 bit character, input to state machine from search text
**   next-state-index: 24 bits for index of next state
**     (if we reallly need 16M states, we can add a state->index lookup array)
**     ...repeat for each state ...
**
**   * if a state is empty it has words 1 and 2, but no transition words.
**
**   Construction:
**
**   Patterns are added to a list based trie.
**   The list based trie is compiled into a list based NFA with failure states.
**   The list based NFA is converted to full or sparse format NFA.
**   The Zero'th state sparse transitions may be stored in full format for
**      performance.
**   Sparse transition arrays are searched using linear and binary search
**      strategies depending on the number of entries to search through in
**      each state.
**   The state machine in sparse mode is compacted into a single vector for
**      better performance.
**
** Notes:
**
** The NFA can require twice the state transitions that a DFA uses. However,
** the construction of a DFA generates many additional transitions in each
** state which consumes significant additional memory. This particular
** implementation is best suited to environments where the very large memory
** requirements of a full state table implementation is not possible and/or
** the speed trade off is warranted to maintain a small memory footprint.
**
** Each state of an NFA usually has very few transitions but can have up to
** 256.  It is important to not degenerate into a linear search so we utilize
** a binary search if there are more than 5 elements in the state to test for
** a match.  This allows us to use a simple sparse memory design with an
** acceptable worst case search scenario.  The binary search over 256 elements
** is limtied to a max of 8 tests.  The zero'th state may use a full 256 state
** array, so a quick index lookup provides the next state transition.  The
** zero'th state is generally visited much more than other states.
**
** Compiling : gcc, Intel C/C++, Microsoft C/C++, each optimize differently.
** My studies have shown Intel C/C++ 9,8,7 to be the fastest, Microsoft 8,7,6
** is next fastest, and gcc 4.x,3.x,2.x is the slowest of the three.  My
** testing has been mainly on x86.  In general gcc does a poor job with
** optimizing this state machine for performance, compared to other less cache
** and prefetch sensitive algorithms.  I've documented this behavior in a
** paper 'Optimizing Pattern Matching for IDS' (www.sourcefire.com,
** www.idsresearch.org).
**
** The code is sensitive to cache optimization and prefetching, as well as
** instruction pipelining.  Aren't we all.  To this end, the number of
** patterns, length of search text, and cpu cache L1,L2,L3 all affect
** performance. The relative performance of the sparse and full format NFA and
** DFA varies as you vary the pattern charactersitics,and search text length,
** but strong performance trends are present and stable.
**
**
**  BNFA API SUMMARY
**
**  bnfa=bnfaNew();             create a state machine
**  bnfaAddPattern(bnfa,..);    add a pattern to the state machine
**  bnfaCompile (bnfa,..)       compile the state machine
**  bnfaPrintInfo(bnfa);        print memory usage and state info
**  bnfaPrint(bnfa);            print the state machine in total
**  state=bnfaSearch(bnfa, ...,state);  search a data buffer for a pattern match
**  bnfaFree (bnfa);            free the bnfa
**
**
** Reference - Efficient String matching: An Aid to Bibliographic Search
**             Alfred V Aho and Margaret J Corasick
**             Bell Labratories
**             Copyright(C) 1975 Association for Computing Machinery,Inc
**
** 12/4/06 - man - modified summary
** 6/26/07 - man - Added last_match tracking, and accounted for nocase/case by
**                 preseting the last match state, and reverting if we fail the
**                 case memcmp test for any rule in the states matching rule
**                 list.  The states in the defaul matcher represent either
**                 case or nocase states, so they are dual mode, that makes
**                 this a bit tricky.  When we sue the pure exact match, or
**                 pure don't care matching routines, we just track the last
**                 state, and never need to revert.  This only tracks the
**                 single repeated states and repeated data.
** 01/2008 - man - added 2 phase pattern matcher using a pattern match queue.
**                 Text is scanned and matching states are queued, duplicate
**                 matches are dropped, and after the complete buffer scan the
**                 queued matches are processed.  This improves cacheing
**                 performance, and reduces duplicate rule processing.  The
**                 queue is limited in size and is flushed if it becomes full
**                 during the scan.  This allows simple insertions.  Tracking
**                 queue ops is optional, as this can impose a modest
**                 performance hit of a few percent.
**
** LICENSE (GPL)
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
**
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"

#ifndef DYNAMIC_PREPROC_CONTEXT
#define BNFA_TRACK_Q

#ifdef BNFA_TRACK_Q
# include "snort.h"
#endif
#endif //DYNAMIC_PREPROC_CONTEXT

#include "bnfa_search.h"
#include "snort_debug.h"
#include "util.h"
#include "sf_dynamic_preprocessor.h"

/*
 * Used to initialize last state, states are limited to 0-16M
 * so this will not conflict.
 */
#define LAST_STATE_INIT  0xffffffff

#define printf LogMessage
/*
* Case Translation Table - his guarantees we use
* indexed lookups for case conversion
*/
static
unsigned char xlatcase[BNFA_MAX_ALPHABET_SIZE];
static
void init_xlatcase(void)
{
  int i;
  static int first=1;

  if( !first )
      return;

  for(i=0; i<BNFA_MAX_ALPHABET_SIZE; i++)
  {
      xlatcase[i] = (unsigned char)toupper(i);
  }

  first=0;
}

/*
* Custom memory allocator
*/
static
void * bnfa_alloc( int n, int * m )
{
   void * p = calloc(1,n);
   if( p )
   {
     if(m)
     {
         m[0] += n;
     }
   }
   return p;
}
static
void bnfa_free( void *p, int n, int * m )
{
   if( p )
   {
       free(p);
       if(m)
       {
          m[0] -= n;
       }
   }
}
#define BNFA_MALLOC(n,memory) bnfa_alloc(n,&(memory))
#define BNFA_FREE(p,n,memory) bnfa_free(p,n,&(memory))


/* queue memory traker */
static int queue_memory=0;

/*
*    simple queue node
*/
typedef struct _qnode
{
   unsigned state;
   struct _qnode *next;
 }
QNODE;
/*
*    simple fifo queue structure
*/
typedef struct _queue
{
  QNODE * head, *tail;
  int count;
  int maxcnt;
}
QUEUE;
/*
*   Initialize the fifo queue
*/
static
void queue_init (QUEUE * s)
{
  s->head = s->tail = 0;
  s->count= 0;
  s->maxcnt=0;
}
/*
*  Add items to tail of queue (fifo)
*/
static
int queue_add (QUEUE * s, int state)
{
  QNODE * q;
  if (!s->head)
  {
      q = s->tail = s->head = (QNODE *) BNFA_MALLOC (sizeof(QNODE),queue_memory);
      if(!q) return -1;
      q->state = state;
      q->next = 0;
  }
  else
  {
      q = (QNODE *) BNFA_MALLOC (sizeof(QNODE),queue_memory);
      if(!q) return -1;
      q->state = state;
      q->next = 0;
      s->tail->next = q;
      s->tail = q;
  }
  s->count++;

  if( s->count > s->maxcnt )
      s->maxcnt = s->count;

  return 0;
}
/*
*  Remove items from head of queue (fifo)
*/
static
int queue_remove (QUEUE * s)
{
  int state = 0;
  QNODE * q;
  if (s->head)
  {
      q       = s->head;
      state   = q->state;
      s->head = s->head->next;
      s->count--;

      if( !s->head )
      {
        s->tail = 0;
        s->count = 0;
      }
      BNFA_FREE (q,sizeof(QNODE),queue_memory);
  }
  return state;
}
/*
*   Return count of items in the queue
*/
static
int queue_count (QUEUE * s)
{
  return s->count;
}
/*
*  Free the queue
*/
static
void queue_free (QUEUE * s)
{
  while (queue_count (s))
    {
      queue_remove (s);
    }
}

/*
*  Get next state from transition list
*/
static
int _bnfa_list_get_next_state( bnfa_struct_t * bnfa, int state, int input )
{
  if ( state == 0 ) /* Full set of states  always */
  {
       bnfa_state_t * p = (bnfa_state_t*)bnfa->bnfaTransTable[0];
       if(!p)
       {
           return 0;
       }
       return p[input];
  }
  else
  {
    bnfa_trans_node_t * t = bnfa->bnfaTransTable[state];
    while( t )
    {
      if( t->key == (unsigned)input )
      {
        return t->next_state;
      }
      t=t->next;
    }
    return BNFA_FAIL_STATE; /* Fail state */
  }
}

/*
*  Put next state - head insertion, and transition updates
*/
static
int _bnfa_list_put_next_state( bnfa_struct_t * bnfa, int state, int input, int next_state )
{
  if( state >= bnfa->bnfaMaxStates )
  {
      return -1;
  }

  if( input >= bnfa->bnfaAlphabetSize )
  {
      return -1;
  }

  if( state == 0 )
  {
    bnfa_state_t * p;

    p = (bnfa_state_t*)bnfa->bnfaTransTable[0];
    if( !p )
    {
       p = (bnfa_state_t*)BNFA_MALLOC(sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize,bnfa->list_memory);
       if( !p )
       {
           return -1;
       }

       bnfa->bnfaTransTable[0] = (bnfa_trans_node_t*)p;
    }
    if( p[input] )
    {
        p[input] =  next_state;
        return 0;
    }
    p[input] =  next_state;
  }
  else
  {
    bnfa_trans_node_t * p;
    bnfa_trans_node_t * tnew;

    /* Check if the transition already exists, if so just update the next_state */
    p = bnfa->bnfaTransTable[state];
    while( p )
    {
      if( p->key == (unsigned)input )  /* transition already exists- reset the next state */
      {
          p->next_state = next_state;
          return 0;
      }
      p=p->next;
    }

    /* Definitely not an existing transition - add it */
    tnew = (bnfa_trans_node_t*)BNFA_MALLOC(sizeof(bnfa_trans_node_t),bnfa->list_memory);
    if( !tnew )
    {
      return -1;
    }

    tnew->key        = input;
    tnew->next_state = next_state;
    tnew->next       = bnfa->bnfaTransTable[state];

    bnfa->bnfaTransTable[state] = tnew;
  }

  bnfa->bnfaNumTrans++;

  return 0;
}

/*
*   Free the entire transition list table
*/
static
int _bnfa_list_free_table( bnfa_struct_t * bnfa )
{
  int i;
  bnfa_trans_node_t * t, *p;

  if( !bnfa->bnfaTransTable ) return 0;

  if( bnfa->bnfaTransTable[0] )
  {
      BNFA_FREE(bnfa->bnfaTransTable[0],sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize,bnfa->list_memory);
  }

  for(i=1; i<bnfa->bnfaMaxStates; i++)
  {
     t = bnfa->bnfaTransTable[i];

     while( t )
     {
       p = t;
       t = t->next;
       BNFA_FREE(p,sizeof(bnfa_trans_node_t),bnfa->list_memory);
     }
   }

   if( bnfa->bnfaTransTable )
   {
      BNFA_FREE(bnfa->bnfaTransTable,sizeof(bnfa_trans_node_t*)*bnfa->bnfaMaxStates,bnfa->list_memory);
      bnfa->bnfaTransTable = 0;
   }

   return 0;
}

static
int bnfaBuildMatchStateTrees(bnfa_struct_t *bnfa,
                             int (*build_tree)(void *id, void **existing_tree),
                             int (*neg_list_func)(void *id, void **list))
{
    int i,cnt = 0;
    bnfa_match_node_t  * mn;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_pattern_t     * patrn;

    for (i=0;i<bnfa->bnfaNumStates;i++)
    {
        for(mn = MatchList[i];
            mn!= NULL;
            mn = mn->next )
        {
            patrn = (bnfa_pattern_t *)mn->data;
            if (patrn->userdata)
            {
                if (patrn->negative)
                {
                    neg_list_func(patrn->userdata, &MatchList[i]->neg_list);
                }
                else
                {
                    build_tree(patrn->userdata, &MatchList[i]->rule_option_tree);
                }
            }

            cnt++;
        }

        /* Last call to finalize the tree */
        if (MatchList[i])
        {
            build_tree(NULL, &MatchList[i]->rule_option_tree);
        }
    }

    return cnt;
}

#ifndef DYNAMIC_PREPROCESSOR_CONTEXT
static
int bnfaBuildMatchStateTreesWithSnortConf(struct _SnortConfig *sc, bnfa_struct_t *bnfa,
                                          int (*build_tree)(struct _SnortConfig *, void *id, void **existing_tree),
                                          int (*neg_list_func)(void *id, void **list))
{
    int i,cnt = 0;
    bnfa_match_node_t  * mn;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_pattern_t     * patrn;

    for (i=0;i<bnfa->bnfaNumStates;i++)
    {
        for(mn = MatchList[i];
            mn!= NULL;
            mn = mn->next )
        {
            patrn = (bnfa_pattern_t *)mn->data;
            if (patrn->userdata)
            {
                if (patrn->negative)
                {
                    neg_list_func(patrn->userdata, &MatchList[i]->neg_list);
                }
                else
                {
                    build_tree(sc, patrn->userdata, &MatchList[i]->rule_option_tree);
                }
            }

            cnt++;
        }

        /* Last call to finalize the tree */
        if (MatchList[i])
        {
            build_tree(sc, NULL, &MatchList[i]->rule_option_tree);
        }
    }

    return cnt;
}
#endif //DYNAMIC_PREPROCESSOR_CONTEXT

#ifdef ALLOW_LIST_PRINT
/*
* Print the transition list table to stdout
*/
static
int _bnfa_list_print_table( bnfa_struct_t * bnfa )
{
  int i;
  bnfa_trans_node_t * t;
  bnfa_match_node_t * mn;
  bnfa_pattern_t * patrn;

  if( !bnfa->bnfaTransTable )
  {
      return 0;
  }

  printf("Print Transition Table- %d active states\n",bnfa->bnfaNumStates);

  for(i=0;i< bnfa->bnfaNumStates;i++)
  {
     printf("state %3d: ",i);

     if( i == 0 )
     {
        int k;
        bnfa_state_t * p = (bnfa_state_t*)bnfa->bnfaTransTable[0];
        if(!p) continue;

        for(k=0;k<bnfa->bnfaAlphabetSize;k++)
        {
          if( p[k] == 0 ) continue;

          if( isascii((int)p[k]) && isprint((int)p[k]) )
             printf("%3c->%-5d\t",k,p[k]);
          else
             printf("%3d->%-5d\t",k,p[k]);
        }
     }
     else
     {
       t = bnfa->bnfaTransTable[i];
       while( t )
       {
         if( isascii((int)t->key) && isprint((int)t->key) )
           printf("%3c->%-5d\t",t->key,t->next_state);
         else
           printf("%3d->%-5d\t",t->key,t->next_state);
         t = t->next;
       }
     }

     mn =bnfa->bnfaMatchList[i];
     while( mn )
     {
       patrn =(bnfa_pattern_t *)mn->data;
       printf("%.*s ",patrn->n,patrn->casepatrn);
       mn = mn->next;
     }
     printf("\n");
   }
   return 0;
}
#endif
/*
* Converts a single row of states from list format to a full format
*/
static
int _bnfa_list_conv_row_to_full(bnfa_struct_t * bnfa, bnfa_state_t state, bnfa_state_t * full )
{
    if( (int)state >= bnfa->bnfaMaxStates ) /* protects 'full' against overflow */
    {
    return -1;
    }

    if( state == 0 )
    {
       if( bnfa->bnfaTransTable[0] )
          memcpy(full,bnfa->bnfaTransTable[0],sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize);
       else
          memset(full,0,sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize);

       return bnfa->bnfaAlphabetSize;
    }
    else
    {
       int tcnt = 0;

       bnfa_trans_node_t * t = bnfa->bnfaTransTable[ state ];

       memset(full,0,sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize);

       if( !t )
       {
           return 0;
       }

       while(t && (t->key < BNFA_MAX_ALPHABET_SIZE ) )
       {
         full[ t->key ] = t->next_state;
         tcnt++;
         t = t->next;
       }
       return tcnt;
    }
}

/*
*  Add pattern characters to the initial upper case trie
*  unless Exact has been specified, in  which case all patterns
*  are assumed to be case specific.
*/
static
int _bnfa_add_pattern_states (bnfa_struct_t * bnfa, bnfa_pattern_t * p)
{
  int             state, next, n;
  unsigned char * pattern;
  bnfa_match_node_t  * pmn;

  n       = p->n;
  pattern = p->casepatrn;
  state   = 0;

  /*
  *  Match up pattern with existing states
  */
  for (; n > 0; pattern++, n--)
  {
      if( bnfa->bnfaCaseMode == BNFA_CASE )
        next = _bnfa_list_get_next_state(bnfa,state,*pattern);
      else
        next = _bnfa_list_get_next_state(bnfa,state,xlatcase[*pattern]);

      if( next == (int)BNFA_FAIL_STATE || next == 0 )
      {
         break;
      }
      state = next;
  }

  /*
  *   Add new states for the rest of the pattern bytes, 1 state per byte, uppercase
  */
  for (; n > 0; pattern++, n--)
  {
      bnfa->bnfaNumStates++;

      if( bnfa->bnfaCaseMode == BNFA_CASE )
      {
        if( _bnfa_list_put_next_state(bnfa,state,*pattern,bnfa->bnfaNumStates)  < 0 )
             return -1;
      }
      else
      {
        if( _bnfa_list_put_next_state(bnfa,state,xlatcase[*pattern],bnfa->bnfaNumStates)  < 0 )
             return -1;
      }
      state = bnfa->bnfaNumStates;

      if ( bnfa->bnfaNumStates >= bnfa->bnfaMaxStates )
      {
           return -1;
      }
  }

  /*  Add a pattern to the list of patterns terminated at this state */
  pmn = (bnfa_match_node_t*)BNFA_MALLOC(sizeof(bnfa_match_node_t),bnfa->matchlist_memory);
  if( !pmn )
  {
      return -1;
  }

  pmn->data = p;
  pmn->next = bnfa->bnfaMatchList[state];

  bnfa->bnfaMatchList[state] = pmn;

  return 0;
}

#ifdef XXXXX
int _bnfa_list_get_next_state( bnfa_struct_t * bnfa, int state, int input )
{
  if ( state == 0 ) /* Full set of states  always */
  {
       bnfa_state_t * p = (bnfa_state_t*)bnfa->bnfaTransTable[0];
       if(!p)
      {
          return 0;
      }
       return p[input];
  }
  else
  {
    bnfa_trans_node_t * t = bnfa->bnfaTransTable[state];
    while( t )
    {
      if( t->key == (unsigned)input )
      {
        return t->next_state;
      }
      t=t->next;
    }
    return BNFA_FAIL_STATE; /* Fail state */
  }
}
#endif
static /* used only by KcontainsJ() */
int
_bnfa_conv_node_to_full(bnfa_trans_node_t *t, bnfa_state_t * full )
{
       int tcnt = 0;

       memset(full,0,sizeof(bnfa_state_t)*BNFA_MAX_ALPHABET_SIZE);

       if( !t )
      {
          return 0;
      }

       while(t && (t->key < BNFA_MAX_ALPHABET_SIZE ) )
       {
         full[ t->key ] = t->next_state;
         tcnt++;
         t = t->next;
       }
       return tcnt;
}
/*
 *  containment test -
 *  test if all of tj transitions are in tk
 */
#ifdef XXXX
static
int KcontainsJx(bnfa_trans_node_t * tk, bnfa_trans_node_t *tj )
{
    bnfa_trans_node_t *t;
    int found;

    while( tj )
    {
      found=0;
      for( t=tk;t;t=t->next )
      {
          if( tj->key == t->key )
          {
              found=1;
              break;
          }
      }
      if( !found )
          return 0;

      tj=tj->next; /* get next tj key */
    }
  return 1;
}
#endif
static
int KcontainsJ(bnfa_trans_node_t * tk, bnfa_trans_node_t *tj )
{
    bnfa_state_t       full[BNFA_MAX_ALPHABET_SIZE];

    if( !_bnfa_conv_node_to_full(tk,full)  )
        return 1; /* emtpy state */

    while( tj )
    {
      if( !full[tj->key] )
          return 0;

      tj=tj->next; /* get next tj key */
    }
  return 1;
}
/*
 * 1st optimization - eliminate duplicate fail states
 *
 * check if a fail state is a subset of the current state,
 * if so recurse to the next fail state, and so on.
 */
static
int _bnfa_opt_nfa (bnfa_struct_t * bnfa)
{
    int            cnt=0;
    int            k, fs, fr;
    bnfa_state_t * FailState = bnfa->bnfaFailState;

    for(k=2;k<bnfa->bnfaNumStates;k++)
    {
        fr = fs = FailState[k];
        while( fs &&  KcontainsJ(bnfa->bnfaTransTable[k],bnfa->bnfaTransTable[fs]) )
        {
            fs = FailState[fs];
        }
        if( fr != fs )
        {
           cnt++;
           FailState[ k ] = fs;
        }
    }
#ifdef DEBUG
    if( cnt)LogMessage("ac-bnfa: %d nfa optimizations found in %d states\n",cnt,bnfa->bnfaNumStates);
#endif
    return 0;
}

/*
*   Build a non-deterministic finite automata using Aho-Corasick construction
*   The keyword trie must already be built via _bnfa_add_pattern_states()
*/
static
int _bnfa_build_nfa (bnfa_struct_t * bnfa)
{
    int             r, s, i;
    QUEUE           q, *queue = &q;
    bnfa_state_t     * FailState = bnfa->bnfaFailState;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_match_node_t  * mlist;
    bnfa_match_node_t  * px;

    /* Init a Queue */
    queue_init (queue);

    /* Add the state 0 transitions 1st,
    * the states at depth 1, fail to state 0
    */
    for (i = 0; i < bnfa->bnfaAlphabetSize; i++)
    {
        /* note that state zero deos not fail,
        *  it just returns 0..nstates-1
        */
        s = _bnfa_list_get_next_state(bnfa,0,i);
        if( s ) /* don't bother adding state zero */
        {
          if( queue_add (queue, s) )
          {
              return -1;
          }
          FailState[s] = 0;
        }
    }

    /* Build the fail state successive layer of transitions */
    while (queue_count (queue) > 0)
    {
        r = queue_remove (queue);

        /* Find Final States for any Failure */
        for(i = 0; i<bnfa->bnfaAlphabetSize; i++)
        {
            int fs, next;

            s = _bnfa_list_get_next_state(bnfa,r,i);

            if( s == (int)BNFA_FAIL_STATE )
                continue;

            if( queue_add (queue, s) )
            {
                return -1;
            }

            fs = FailState[r];

            /*
            *  Locate the next valid state for 'i' starting at fs
            */
            while( (next=_bnfa_list_get_next_state(bnfa,fs,i)) == (int)BNFA_FAIL_STATE )
            {
                fs = FailState[fs];
            }

            /*
            *  Update 's' state failure state to point to the next valid state
            */
            FailState[s] = next;

            /*
            *  Copy 'next'states MatchList into 's' states MatchList,
            *  we just create a new list nodes, the patterns are not copied.
            */
            for( mlist = MatchList[next];mlist;mlist = mlist->next)
            {
                /* Dup the node, don't copy the data */
                px = (bnfa_match_node_t*)BNFA_MALLOC(sizeof(bnfa_match_node_t),bnfa->matchlist_memory);
                if( !px )
                {
                    return 0;
                }

                px->data = mlist->data;

                px->next = MatchList[s]; /* insert at head */

                MatchList[s] = px;
            }
        }
    }

    /* Clean up the queue */
    queue_free (queue);

    /* optimize the failure states */
    if( bnfa->bnfaOpt )
        _bnfa_opt_nfa(bnfa);

    return 0;
}

#ifdef ALLOW_NFA_FULL
/*
*  Conver state machine to full format
*/
static
int _bnfa_conv_list_to_full(bnfa_struct_t * bnfa)
{
  int          k;
  bnfa_state_t  * p;
  bnfa_state_t ** NextState = bnfa->bnfaNextState;

  for(k=0;k<bnfa->bnfaNumStates;k++)
  {
    p = BNFA_MALLOC(sizeof(bnfa_state_t)*bnfa->bnfaAlphabetSize,bnfa->nextstate_memory);
    if(!p)
    {
      return -1;
    }
    _bnfa_list_conv_row_to_full( bnfa, (bnfa_state_t)k, p );

    NextState[k] = p; /* now we have a full format row vector */
  }

  return 0;
}
#endif

/*
*  Convert state machine to csparse format
*
*  Merges state/transition/failure arrays into one.
*
*  For each state we use a state-word followed by the transition list for
*  the state sw(state 0 )...tl(state 0) sw(state 1)...tl(state1) sw(state2)...
*  tl(state2) ....
*
*  The transition and failure states are replaced with the start index of
*  transition state, this eliminates the NextState[] lookup....
*
*  The compaction of multiple arays into a single array reduces the total
*  number of states that can be handled since the max index is 2^24-1,
*  whereas without compaction we had 2^24-1 states.
*/
static
int _bnfa_conv_list_to_csparse_array(bnfa_struct_t * bnfa)
{
  int            m, k, i, nc;
  bnfa_state_t      state;
  bnfa_state_t    * FailState = (bnfa_state_t  *)bnfa->bnfaFailState;
  bnfa_state_t    * ps; /* transition list */
  bnfa_state_t    * pi; /* state indexes into ps */
  bnfa_state_t      ps_index=0;
  unsigned       nps;
  bnfa_state_t      full[BNFA_MAX_ALPHABET_SIZE];


  /* count total state transitions, account for state and control words  */
  nps = 0;
  for(k=0;k<bnfa->bnfaNumStates;k++)
  {
    nps++; /* state word */
    nps++; /* control word */

    /* count transitions */
    nc = 0;
    _bnfa_list_conv_row_to_full(bnfa, (bnfa_state_t)k, full );
    for( i=0; i<bnfa->bnfaAlphabetSize; i++ )
    {
        state = full[i] & BNFA_SPARSE_MAX_STATE;
        if( state != 0 )
        {
            nc++;
        }
    }

    /* add in transition count */
       if( (k == 0 && bnfa->bnfaForceFullZeroState) || nc > BNFA_SPARSE_MAX_ROW_TRANSITIONS )
    {
        nps += BNFA_MAX_ALPHABET_SIZE;
    }
    else
    {
           for( i=0; i<bnfa->bnfaAlphabetSize; i++ )
        {
           state = full[i] & BNFA_SPARSE_MAX_STATE;
           if( state != 0 )
           {
               nps++;
           }
        }
    }
  }

  /* check if we have too many states + transitions */
  if( nps > BNFA_SPARSE_MAX_STATE )
  {
      /* Fatal */
      return -1;
  }

  /*
    Alloc The Transition List - we need an array of bnfa_state_t items of size 'nps'
  */
  ps = BNFA_MALLOC( nps*sizeof(bnfa_state_t),bnfa->nextstate_memory);
  if( !ps )
  {
      /* Fatal */
      return -1;
  }
  bnfa->bnfaTransList = ps;

  /*
     State Index list for pi - we need an array of bnfa_state_t items of size 'NumStates'
  */
  pi = BNFA_MALLOC( bnfa->bnfaNumStates*sizeof(bnfa_state_t),bnfa->nextstate_memory);
  if( !pi )
  {
      /* Fatal */
      return -1;
  }

  /*
      Build the Transition List Array
  */
  for(k=0;k<bnfa->bnfaNumStates;k++)
  {
    pi[k] = ps_index; /* save index of start of state 'k' */

    ps[ ps_index ] = k; /* save the state were in as the 1st word */

    ps_index++;  /* skip past state word */

    /* conver state 'k' to full format */
    _bnfa_list_conv_row_to_full(bnfa, (bnfa_state_t)k, full );

    /* count transitions */
    nc = 0;
    for( i=0; i<bnfa->bnfaAlphabetSize; i++ )
    {
        state = full[i] & BNFA_SPARSE_MAX_STATE;
        if( state != 0 )
        {
            nc++;
        }
    }

    /* add a full state or a sparse state  */
    if( (k == 0 && bnfa->bnfaForceFullZeroState) ||
        nc > BNFA_SPARSE_MAX_ROW_TRANSITIONS )
    {
        /* set the control word */
        ps[ps_index]  = BNFA_SPARSE_FULL_BIT;
        ps[ps_index] |= FailState[k] & BNFA_SPARSE_MAX_STATE;
        if( bnfa->bnfaMatchList[k] )
        {
            ps[ps_index] |= BNFA_SPARSE_MATCH_BIT;
        }
        ps_index++;

        /* copy the transitions */
        _bnfa_list_conv_row_to_full(bnfa, (bnfa_state_t)k, &ps[ps_index] );

         ps_index += BNFA_MAX_ALPHABET_SIZE;  /* add in 256 transitions */

    }
       else
    {
        /* set the control word */
           ps[ps_index]  = nc<<BNFA_SPARSE_COUNT_SHIFT ;
           ps[ps_index] |= FailState[k]&BNFA_SPARSE_MAX_STATE;
           if( bnfa->bnfaMatchList[k] )
          {
               ps[ps_index] |= BNFA_SPARSE_MATCH_BIT;
          }
        ps_index++;

        /* add in the transitions */
           for( m=0, i=0; i<bnfa->bnfaAlphabetSize && m<nc; i++ )
          {
               state = full[i] & BNFA_SPARSE_MAX_STATE;
               if( state != 0 )
             {
                   ps[ps_index++] = (i<<BNFA_SPARSE_VALUE_SHIFT) | state;
                m++;
             }
          }
    }
  }

  /* sanity check we have not overflowed our buffer */
  if( ps_index > nps )
  {
      /* Fatal */
      return -1;
  }

  /*
  Replace Transition states with Transition Indices.
  This allows us to skip using NextState[] to locate the next state
  This limits us to <16M transitions due to 24 bit state sizes, and the fact
  we have now converted next-state fields to next-index fields in this array,
  and we have merged the next-state and state arrays.
  */
  ps_index=0;
  for(k=0; k< bnfa->bnfaNumStates; k++ )
  {
     if( pi[k] >= nps )
     {
         /* Fatal */
         return -1;
     }

     //ps_index = pi[k];  /* get index of next state */
     ps_index++;        /* skip state id */

     /* Full Format */
     if( ps[ps_index] & BNFA_SPARSE_FULL_BIT )
     {
       /* Do the fail-state */
       ps[ps_index] = ( ps[ps_index] & 0xff000000 ) |
                      ( pi[ ps[ps_index] & BNFA_SPARSE_MAX_STATE ] ) ;
       ps_index++;

       /* Do the transition-states */
       for(i=0;i<BNFA_MAX_ALPHABET_SIZE;i++)
       {
         ps[ps_index] = ( ps[ps_index] & 0xff000000 ) |
                        ( pi[ ps[ps_index] & BNFA_SPARSE_MAX_STATE ] ) ;
         ps_index++;
       }
     }

     /* Sparse Format */
     else
     {
           nc = (ps[ps_index] & BNFA_SPARSE_COUNT_BITS)>>BNFA_SPARSE_COUNT_SHIFT;

           /* Do the cw = [cb | fail-state] */
           ps[ps_index] =  ( ps[ps_index] & 0xff000000 ) |
                        ( pi[ ps[ps_index] & BNFA_SPARSE_MAX_STATE ] );
           ps_index++;

           /* Do the transition-states */
           for(i=0;i<nc;i++)
           {
               ps[ps_index] = ( ps[ps_index] & 0xff000000 ) |
                           ( pi[ ps[ps_index] & BNFA_SPARSE_MAX_STATE ] );
             ps_index++;
           }
     }

     /* check for buffer overflow again */
      if( ps_index > nps )
     {
         /* Fatal */
         return -1;
     }

  }

  BNFA_FREE(pi,bnfa->bnfaNumStates*sizeof(bnfa_state_t),bnfa->nextstate_memory);

  return 0;
}

/*
*  Print the state machine - rather verbose
*/
void bnfaPrint(bnfa_struct_t * bnfa)
{
  int               k;
  bnfa_match_node_t  ** MatchList;
  bnfa_match_node_t   * mlist;
  int              ps_index=0;
  bnfa_state_t      * ps=0;

  if( !bnfa )
      return;

  MatchList = bnfa->bnfaMatchList;

  if( !bnfa->bnfaNumStates )
      return;

  if( bnfa->bnfaFormat ==BNFA_SPARSE )
  {
    printf("Print NFA-SPARSE state machine : %d active states\n", bnfa->bnfaNumStates);
    ps = bnfa->bnfaTransList;
    if( !ps )
        return;
  }

#ifdef ALLOW_NFA_FULL
  else if( bnfa->bnfaFormat ==BNFA_FULL )
  {
    printf("Print NFA-FULL state machine : %d active states\n", bnfa->bnfaNumStates);
  }
#endif


  for(k=0;k<bnfa->bnfaNumStates;k++)
  {
    printf(" state %-4d fmt=%d ",k,bnfa->bnfaFormat);

    if( bnfa->bnfaFormat == BNFA_SPARSE )
    {
       unsigned i,cw,fs,nt,fb,mb;

       ps_index++; /* skip state number */

       cw = ps[ps_index]; /* control word  */
       fb = (cw &  BNFA_SPARSE_FULL_BIT)>>BNFA_SPARSE_VALUE_SHIFT;  /* full storage bit */
       mb = (cw &  BNFA_SPARSE_MATCH_BIT)>>BNFA_SPARSE_VALUE_SHIFT; /* matching state bit */
       nt = (cw &  BNFA_SPARSE_COUNT_BITS)>>BNFA_SPARSE_VALUE_SHIFT;/* number of transitions 0-63 */
       fs = (cw &  BNFA_SPARSE_MAX_STATE)>>BNFA_SPARSE_VALUE_SHIFT; /* fail state */

       ps_index++;  /* skip control word */

       printf("mb=%3u fb=%3u fs=%-4u ",mb,fb,fs);

       if( fb )
       {
         printf(" nt=%-3d : ",bnfa->bnfaAlphabetSize);

         for( i=0; i<(unsigned)bnfa->bnfaAlphabetSize; i++, ps_index++  )
         {
            if( ps[ps_index] == 0  ) continue;

            if( isascii((int)i) && isprint((int)i) )
               printf("%3c->%-6d\t",i,ps[ps_index]);
            else
               printf("%3d->%-6d\t",i,ps[ps_index]);
         }
       }
       else
       {
          printf(" nt=%-3d : ",nt);

          for( i=0; i<nt; i++, ps_index++ )
          {
              if( isascii(ps[ps_index]>>BNFA_SPARSE_VALUE_SHIFT) &&
                  isprint(ps[ps_index]>>BNFA_SPARSE_VALUE_SHIFT) )
               printf("%3c->%-6d\t",ps[ps_index]>>BNFA_SPARSE_VALUE_SHIFT,ps[ps_index] & BNFA_SPARSE_MAX_STATE);
             else
               printf("%3d->%-6d\t",ps[ps_index]>>BNFA_SPARSE_VALUE_SHIFT,ps[ps_index] & BNFA_SPARSE_MAX_STATE);
          }
       }
    }
#ifdef ALLOW_NFA_FULL
    else if( bnfa->bnfaFormat == BNFA_FULL )
    {
       int          i;
       bnfa_state_t    state;
       bnfa_state_t  * p;
       bnfa_state_t ** NextState;

       NextState = (bnfa_state_t **)bnfa->bnfaNextState;
       if( !NextState )
           continue;

       p = NextState[k];

       printf("fs=%-4d nc=256 ",bnfa->bnfaFailState[k]);

       for( i=0; i<bnfa->bnfaAlphabetSize; i++ )
       {
          state = p[i];

          if( state != 0 && state != BNFA_FAIL_STATE )
          {
             if( isascii(i) && isprint(i) )
               printf("%3c->%-5d\t",i,state);
             else
               printf("%3d->%-5d\t",i,state);
          }
       }
    }
#endif

   printf("\n");

   if( MatchList[k] )
       printf("---MatchList For State %d\n",k);

    for( mlist = MatchList[k];
         mlist!= NULL;
         mlist = mlist->next )
    {
         bnfa_pattern_t * pat;
         pat = (bnfa_pattern_t*)mlist->data;
         printf("---pattern : %.*s\n",pat->n,pat->casepatrn);
    }
  }
}

/*
*  Create a new AC state machine
*/
bnfa_struct_t * bnfaNew(void (*userfree)(void *p),
                        void (*optiontreefree)(void **p),
                        void (*neg_list_free)(void **p))
{
  bnfa_struct_t * p;
  int bnfa_memory=0;

  init_xlatcase ();

  p = (bnfa_struct_t *) BNFA_MALLOC(sizeof(bnfa_struct_t),bnfa_memory);
  if(!p)
      return 0;

  if( p )
  {
     p->bnfaOpt                = 0;
     p->bnfaCaseMode           = BNFA_PER_PAT_CASE;
     p->bnfaFormat             = BNFA_SPARSE;
     p->bnfaAlphabetSize       = BNFA_MAX_ALPHABET_SIZE;
     p->bnfaForceFullZeroState = 1;
     p->bnfa_memory            = sizeof(bnfa_struct_t);
     p->userfree               = userfree;
     p->optiontreefree         = optiontreefree;
     p->neg_list_free          = neg_list_free;
  }

  queue_memory = 0;
  return p;
}

void bnfaSetOpt(bnfa_struct_t  * p, int flag)
{
   p->bnfaOpt=flag;
}

void bnfaSetCase(bnfa_struct_t  * p, int flag)
{
   if( flag == BNFA_PER_PAT_CASE ) p->bnfaCaseMode = flag;
   if( flag == BNFA_CASE    ) p->bnfaCaseMode = flag;
   if( flag == BNFA_NOCASE  ) p->bnfaCaseMode = flag;
}

/*
*   Fee all memory
*/
void bnfaFree (bnfa_struct_t * bnfa)
{
  int i;
  bnfa_pattern_t * patrn, *ipatrn;
  bnfa_match_node_t   * mlist, *ilist;

  for(i = 0; i < bnfa->bnfaNumStates; i++)
  {
      /* free match list entries */
      mlist = bnfa->bnfaMatchList[i];

      while (mlist)
      {
        ilist = mlist;
        mlist = mlist->next;
        if (ilist->rule_option_tree && bnfa->optiontreefree)
        {
            bnfa->optiontreefree(&(ilist->rule_option_tree));
        }

        if (ilist->neg_list && bnfa->neg_list_free)
        {
            bnfa->neg_list_free(&(ilist->neg_list));
        }

        BNFA_FREE(ilist,sizeof(bnfa_match_node_t),bnfa->matchlist_memory);
      }
      bnfa->bnfaMatchList[i] = 0;

#ifdef ALLOW_NFA_FULL
      /* free next state entries */
      if( bnfa->bnfaFormat==BNFA_FULL )/* Full format */
      {
         if( bnfa->bnfaNextState[i] )
         {
            BNFA_FREE(bnfa->bnfaNextState[i],bnfa->bnfaAlphabetSize*sizeof(bnfa_state_t),bnfa->nextstate_memory);
         }
      }
#endif
  }

  /* Free patterns */
  patrn = bnfa->bnfaPatterns;
  while(patrn)
  {
     ipatrn=patrn;
     patrn=patrn->next;
     BNFA_FREE(ipatrn->casepatrn,ipatrn->n,bnfa->pat_memory);
     if(bnfa->userfree && ipatrn->userdata)
         bnfa->userfree(ipatrn->userdata);
     BNFA_FREE(ipatrn,sizeof(bnfa_pattern_t),bnfa->pat_memory);
  }

  /* Free arrays */
  BNFA_FREE(bnfa->bnfaFailState,bnfa->bnfaNumStates*sizeof(bnfa_state_t),bnfa->failstate_memory);
  BNFA_FREE(bnfa->bnfaMatchList,bnfa->bnfaNumStates*sizeof(bnfa_pattern_t*),bnfa->matchlist_memory);
  BNFA_FREE(bnfa->bnfaNextState,bnfa->bnfaNumStates*sizeof(bnfa_state_t*),bnfa->nextstate_memory);
  BNFA_FREE(bnfa->bnfaTransList,(2*bnfa->bnfaNumStates+bnfa->bnfaNumTrans)*sizeof(bnfa_state_t*),bnfa->nextstate_memory);
  free( bnfa ); /* cannot update memory tracker when deleting bnfa so just 'free' it !*/
}

/*
*   Add a pattern to the pattern list
*/
int
bnfaAddPattern (bnfa_struct_t * p,
                unsigned char *pat,
                int n,
                int nocase,
                int negative,
                void * userdata )
{
  bnfa_pattern_t * plist;

  plist = (bnfa_pattern_t *)BNFA_MALLOC(sizeof(bnfa_pattern_t),p->pat_memory);
  if(!plist) return -1;

  plist->casepatrn = (unsigned char *)BNFA_MALLOC(n,p->pat_memory );
  if(!plist->casepatrn)
  {
      BNFA_FREE(plist,sizeof(bnfa_pattern_t),p->pat_memory);
      return -1;
  }

  memcpy (plist->casepatrn, pat, n);

  plist->n        = n;
  plist->nocase   = nocase;
  plist->negative = negative;
  plist->userdata = userdata;

  plist->next     = p->bnfaPatterns; /* insert at front of list */
  p->bnfaPatterns = plist;

  p->bnfaPatternCnt++;

  return 0;
}

/*
*   Compile the patterns into an nfa state machine
*/
static inline int
_bnfaCompile (bnfa_struct_t * bnfa)
{
    bnfa_pattern_t  * plist;
    bnfa_match_node_t   ** tmpMatchList;
    unsigned          cntMatchStates;
    int               i;

    queue_memory =0;

    /* Count number of states */
    for(plist = bnfa->bnfaPatterns; plist != NULL; plist = plist->next)
    {
       bnfa->bnfaMaxStates += plist->n;
    }
    bnfa->bnfaMaxStates++; /* one extra */

    /* Alloc a List based State Transition table */
    bnfa->bnfaTransTable =(bnfa_trans_node_t**) BNFA_MALLOC(sizeof(bnfa_trans_node_t*) * bnfa->bnfaMaxStates,bnfa->list_memory );
    if(!bnfa->bnfaTransTable)
    {
        return -1;
    }

    /* Alloc a MatchList table - this has a list of pattern matches for each state */
    bnfa->bnfaMatchList=(bnfa_match_node_t**) BNFA_MALLOC(sizeof(void*)*bnfa->bnfaMaxStates,bnfa->matchlist_memory );
    if(!bnfa->bnfaMatchList)
    {
        return -1;
    }

    /* Add each Pattern to the State Table - This forms a keyword trie using lists */
    bnfa->bnfaNumStates = 0;
    for (plist = bnfa->bnfaPatterns; plist != NULL; plist = plist->next)
    {
        _bnfa_add_pattern_states (bnfa, plist);
    }
    bnfa->bnfaNumStates++;

    if( bnfa->bnfaNumStates > BNFA_SPARSE_MAX_STATE )
    {
        return -1;  /* Call bnfaFree to clean up */
    }

    /* ReAlloc a smaller MatchList table -  only need NumStates  */
    tmpMatchList=bnfa->bnfaMatchList;

    bnfa->bnfaMatchList=(bnfa_match_node_t**)BNFA_MALLOC(sizeof(void*) * bnfa->bnfaNumStates,bnfa->matchlist_memory);
    if(!bnfa->bnfaMatchList)
    {
        return -1;
    }

    memcpy(bnfa->bnfaMatchList,tmpMatchList,sizeof(void*) * bnfa->bnfaNumStates);

    BNFA_FREE(tmpMatchList,sizeof(void*) * bnfa->bnfaMaxStates,bnfa->matchlist_memory);

#ifdef MATCH_LIST_CNT
    bnfa->bnfaMatchListCnt=(unsigned*)calloc(sizeof(unsigned) * bnfa->bnfaNumStates);
    if(!bnfa->bnfaMatchListCnt)
    {
        return -1;
    }
#endif
    /* Alloc a failure state table -  only need NumStates */
    bnfa->bnfaFailState =(bnfa_state_t*)BNFA_MALLOC(sizeof(bnfa_state_t) * bnfa->bnfaNumStates,bnfa->failstate_memory);
    if(!bnfa->bnfaFailState)
    {
        return -1;
    }

#ifdef ALLOW_NFA_FULL
    if( bnfa->bnfaFormat == BNFA_FULL )
    {
      /* Alloc a state transition table -  only need NumStates  */
      bnfa->bnfaNextState=(bnfa_state_t**)BNFA_MALLOC(sizeof(bnfa_state_t*) * bnfa->bnfaNumStates,bnfa->nextstate_memory);
      if(!bnfa->bnfaNextState)
      {
          return -1;
      }
    }
#endif

    /* Build the nfa w/failure states - time the nfa construction */
    if( _bnfa_build_nfa (bnfa) )
    {
        return -1;
    }

    /* Convert nfa storage format from list to full or sparse */
    if( bnfa->bnfaFormat == BNFA_SPARSE )
    {
      if( _bnfa_conv_list_to_csparse_array(bnfa)  )
      {
          return -1;
      }
      BNFA_FREE(bnfa->bnfaFailState,sizeof(bnfa_state_t)*bnfa->bnfaNumStates,bnfa->failstate_memory);
      bnfa->bnfaFailState=0;
    }
#ifdef ALLOW_NFA_FULL
    else if( bnfa->bnfaFormat == BNFA_FULL )
    {
      if( _bnfa_conv_list_to_full( bnfa ) )
      {
            return -1;
      }
    }
#endif
    else
    {
        return -1;
    }

    /* Free up the Table Of Transition Lists */
    _bnfa_list_free_table( bnfa );

    /* Count states with Pattern Matches */
    cntMatchStates=0;
    for(i=0;i<bnfa->bnfaNumStates;i++)
    {
        if( bnfa->bnfaMatchList[i] )
            cntMatchStates++;
    }

    bnfa->bnfaMatchStates = cntMatchStates;
    bnfa->queue_memory    = queue_memory;

    bnfaAccumInfo( bnfa  );

    return 0;
}

int
bnfaCompile (bnfa_struct_t * bnfa,
             int (*build_tree)(void * id, void **existing_tree),
             int (*neg_list_func )(void *id, void **list))
{
    int rval;

    if ((rval = _bnfaCompile (bnfa)))
        return rval;

    if (build_tree && neg_list_func)
    {
        bnfaBuildMatchStateTrees( bnfa, build_tree, neg_list_func );
    }
    return 0;
}

#ifndef DYNAMIC_PREPROCESSOR_CONTEXT
int
bnfaCompileWithSnortConf (struct _SnortConfig *sc, bnfa_struct_t * bnfa,
                          int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                          int (*neg_list_func )(void *id, void **list))
{
    int rval;

    if ((rval = _bnfaCompile (bnfa)))
        return rval;

    if (build_tree && neg_list_func)
    {
        bnfaBuildMatchStateTreesWithSnortConf( sc, bnfa, build_tree, neg_list_func );
    }
    return 0;
}
#endif //DYNAMIC_PREPROCESSOR_CONTEXT

#ifdef ALLOW_NFA_FULL

/*
*   Full Matrix Format Search
*/
static
inline
unsigned
_bnfa_search_full_nfa(    bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                    int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                    void *data, bnfa_state_t state, int *current_state )
{
  unsigned char      * Tend;
  unsigned char      * T;
  unsigned char        Tchar;
  unsigned            index;
  bnfa_state_t      ** NextState= bnfa->bnfaNextState;
  bnfa_state_t       * FailState= bnfa->bnfaFailState;
  bnfa_match_node_t ** MatchList= bnfa->bnfaMatchList;
  bnfa_state_t       * pcs;
  bnfa_match_node_t  * mlist;
  bnfa_pattern_t     * patrn;
  unsigned             nfound = 0;
  int                  res;
  unsigned             last_match=LAST_STATE_INIT;
  unsigned             last_match_saved=LAST_STATE_INIT;

  T    = Tx;
  Tend = T + n;

  for( ; T < Tend; T++ )
  {
    Tchar = xlatcase[ *T ];

    for(;;)
    {
        pcs = NextState[state];
        if( pcs[Tchar] == 0 && state > 0 )
        {
            state = FailState[state];
        }
        else
        {
            state = pcs[Tchar];
            break;
        }
    }

    if( state )
    {
        if( state == last_match )
            continue;

        last_match_saved=last_match;
        last_match = state;

        {
            mlist = MatchList[state];
            if (!mlist)
            {
                continue;
            }
            patrn = (bnfa_pattern_t*)mlist->data;
            if( ( T - Tx) < patrn->n )
                index = 0;
            else
                index = T - Tx - patrn->n + 1;
            nfound++;
            /* Don't do anything specific for case sensitive patterns and not,
             * since that will be covered by the rule tree itself.  Each tree
             * might have both case sensitive & case insensitive patterns.
             */
            res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
            if ( res > 0 )
            {
              *current_state = state;
              return nfound;
            }
            else if( res < 0 )
            {
              last_match = last_match_saved;
            }
        }
    }
  }
  *current_state = state;
  return nfound;
}
/*
*   Full Matrix Format Search - Exact matching patterns only
*/
static
inline
unsigned
_bnfa_search_full_nfa_case(    bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                    int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                    void *data, bnfa_state_t state, int *current_state )
{
  unsigned char      * Tend;
  unsigned char      * T;
  unsigned char        Tchar;
  unsigned        index;
  bnfa_state_t      ** NextState= bnfa->bnfaNextState;
  bnfa_state_t       * FailState= bnfa->bnfaFailState;
  bnfa_match_node_t ** MatchList= bnfa->bnfaMatchList;
  bnfa_state_t       * pcs;
  bnfa_match_node_t  * mlist;
  bnfa_pattern_t     * patrn;
  unsigned             nfound = 0;
  unsigned             last_match=LAST_STATE_INIT;
  unsigned             last_match_saved=LAST_STATE_INIT;
  int                  res;

  T    = Tx;
  Tend = T + n;

  for( ; T < Tend; T++ )
  {
    Tchar = *T ;

    for(;;)
    {
        pcs = NextState[state];
        if( pcs[Tchar] == 0 && state > 0 )
        {
            state = FailState[state];
        }
        else
        {
            state = pcs[Tchar];
            break;
        }
    }

    if( state )
    {
        if( state == last_match )
            continue;

        last_match_saved=last_match;
        last_match = state;

        {
            mlist = MatchList[state];
            if (!mlist)
            {
                continue;
            }
            patrn = (bnfa_pattern_t*)mlist->data;
            if( ( T - Tx) < patrn->n )
                index = 0;
            else
                index = T - Tx - patrn->n + 1;
            nfound++;
            /* Don't do anything specific for case (in)sensitive patterns
             * since that will be covered by the rule tree itself.  Each
             * tree might have both case sensitive & case insensitive patterns.
             */
            res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
            if ( res > 0 )
            {
              *current_state = state;
              return nfound;
            }
            else if( res < 0 )
            {
              last_match = last_match_saved;
            }
        }
    }
  }
  *current_state = state;
  return nfound;
}
/*
*   Full Matrix Format Search - no case
*/
static
inline
unsigned
_bnfa_search_full_nfa_nocase(    bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                    int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                    void *data, bnfa_state_t state, int *current_state )
{
  unsigned char      * Tend;
  unsigned char      * T;
  unsigned char        Tchar;
  unsigned        index;
  bnfa_state_t      ** NextState= bnfa->bnfaNextState;
  bnfa_state_t       * FailState= bnfa->bnfaFailState;
  bnfa_match_node_t ** MatchList= bnfa->bnfaMatchList;
  bnfa_state_t       * pcs;
  bnfa_match_node_t  * mlist;
  bnfa_pattern_t     * patrn;
  unsigned             nfound = 0;
  unsigned             last_match=LAST_STATE_INIT;
  unsigned             last_match_saved=LAST_STATE_INIT;
  int                  res;

  T    = Tx;
  Tend = T + n;

  for( ; T < Tend; T++ )
  {
    Tchar = xlatcase[ *T ];

    for(;;)
    {
        pcs = NextState[state];
        if( pcs[Tchar] == 0 && state > 0 )
        {
            state = FailState[state];
        }
        else
        {
            state = pcs[Tchar];
            break;
        }
    }

    if( state )
    {
        if( state == last_match )
            continue;

        last_match_saved=last_match;
        last_match = state;

        {
            mlist = MatchList[state];
            if (!mlist)
            {
                continue;
            }
            patrn = (bnfa_pattern_t*)mlist->data;
            if( ( T - Tx) < patrn->n )
                index = 0;
            else
                index = T - Tx - patrn->n + 1;
            /* Don't do anything specific for case sensitive patterns and not,
             * since that will be covered by the rule tree itself.  Each tree
             * might have both case sensitive & case insensitive patterns.
             */
            res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
            if ( res > 0 )
            {
              *current_state = state;
              return nfound;
            }
            else if( res < 0 )
            {
              last_match = last_match_saved;
            }
        }
    }
  }
  *current_state = state;
  return nfound;
}
#endif

/*
   binary array search on sparse transition array

   O(logN) search times..same as a binary tree.
   data must be in sorted order in the array.

   return:  = -1 => not found
           >= 0  => index of element 'val'

  notes:
    val is tested against the high 8 bits of the 'a' array entry,
    this is particular to the storage format we are using.
*/
static
inline
int _bnfa_binearch( bnfa_state_t * a, int a_len, int val )
{
   int m, l, r;
   int c;

   l = 0;
   r = a_len - 1;

   while( r >= l )
   {
      m = ( r + l ) >> 1;

      c = a[m] >> BNFA_SPARSE_VALUE_SHIFT;

      if( val == c )
      {
          return m;
      }

      else if( val <  c )
      {
          r = m - 1;
      }

      else /* val > c */
      {
          l = m + 1;
      }
   }
   return -1;
}

/*
*   Sparse format for state table using single array storage
*
*   word 1: state
*   word 2: control-word = cb<<24| fs
*           cb    : control-byte
*                : mb | fb | nt
*                mb : bit 8 set if match state, zero otherwise
*                fb : bit 7 set if using full format, zero otherwise
*                nt : number of transitions 0..63 (more than 63 requires full format)
*            fs: failure-transition-state
*   word 3+: byte-value(0-255) << 24 | transition-state
*/
static
inline
unsigned
_bnfa_get_next_state_csparse_nfa_qx(bnfa_state_t * pcx, unsigned sindex, unsigned  input)
{
    int k;
    int nc;
    int index;
    register bnfa_state_t * pcs;

    for(;;)
    {
        pcs = pcx + sindex + 1; /* skip state-id == 1st word */

        if( pcs[0] & BNFA_SPARSE_FULL_BIT )
        {
            if( sindex == 0 )
            {
                return pcs[1+input] & BNFA_SPARSE_MAX_STATE;
            }
            else
            {
                if( pcs[1+input] & BNFA_SPARSE_MAX_STATE )
                    return pcs[1+input] & BNFA_SPARSE_MAX_STATE;
            }
        }
        else
        {
            nc = (pcs[0]>>BNFA_SPARSE_COUNT_SHIFT) & BNFA_SPARSE_MAX_ROW_TRANSITIONS;
            if( nc > BNFA_SPARSE_LINEAR_SEARCH_LIMIT )
            {
                /* binary search... */
                index = _bnfa_binearch( pcs+1, nc, input );
                if( index >= 0 )
                {
                    return pcs[index+1] & BNFA_SPARSE_MAX_STATE;
                }
            }
            else
            {
                /* linear search... */
                for( k=0; k<nc; k++ )
                {
                    if( (pcs[k+1]>>BNFA_SPARSE_VALUE_SHIFT) == input )
                    {
                        return pcs[k+1] & BNFA_SPARSE_MAX_STATE;
                    }
                }
            }
        }

        return 0; /* no transition keyword match failed */
    }
}

/*
*   Sparse format for state table using single array storage
*
*   word 1: state
*   word 2: control-word = cb<<24| fs
*           cb    : control-byte
*                : mb | fb | nt
*                mb : bit 8 set if match state, zero otherwise
*                fb : bit 7 set if using full format, zero otherwise
*                nt : number of transitions 0..63 (more than 63 requires full format)
*            fs: failure-transition-state
*   word 3+: byte-value(0-255) << 24 | transition-state
*/
static
inline
unsigned
_bnfa_get_next_state_csparse_nfa(bnfa_state_t * pcx, unsigned sindex, unsigned  input)
{
   int k;
   int nc;
   int index;
   register bnfa_state_t * pcs;

    for(;;)
   {
      pcs = pcx + sindex + 1; /* skip state-id == 1st word */

      if( pcs[0] & BNFA_SPARSE_FULL_BIT )
     {
       if( sindex == 0 )
       {
         return pcs[1+input] & BNFA_SPARSE_MAX_STATE;
       }
       else
       {
         if( pcs[1+input] & BNFA_SPARSE_MAX_STATE )
             return pcs[1+input] & BNFA_SPARSE_MAX_STATE;
       }
     }
      else
     {
         nc = (pcs[0]>>BNFA_SPARSE_COUNT_SHIFT) & BNFA_SPARSE_MAX_ROW_TRANSITIONS;
         if( nc > BNFA_SPARSE_LINEAR_SEARCH_LIMIT )
         {
           /* binary search... */
           index = _bnfa_binearch( pcs+1, nc, input );
           if( index >= 0 )
           {
              return pcs[index+1] & BNFA_SPARSE_MAX_STATE;
           }
         }
         else
         {
           /* linear search... */
           for( k=0; k<nc; k++ )
           {
             if( (pcs[k+1]>>BNFA_SPARSE_VALUE_SHIFT) == input )
             {
                return pcs[k+1] & BNFA_SPARSE_MAX_STATE;
             }
           }
         }
      }

      /* no transition found ... get the failure state and try again  */
      sindex = pcs[0] & BNFA_SPARSE_MAX_STATE;
    }
}

/*
 *  Per Pattern case search, case is on per pattern basis
 *  standard snort search
 *  note: index is not used by snort, so it's commented
 *  TRACK_Q can impose a modest couple % performance difference in the
 *  pattern matching rate.
 */

/* Queue whole pattern groups at end states in AC */
void bnfa_print_qinfo(void)
{
#ifdef BNFA_TRACK_Q
    if( snort_conf->max_inq )
    {
        LogMessage("ac-bnfa: queue size     = %d, max = %d\n",snort_conf->max_inq, MAX_INQ );
        LogMessage("ac-bnfa: queue flushes  = "STDu64"\n", snort_conf->tot_inq_flush );
        LogMessage("ac-bnfa: queue inserts  = "STDu64"\n", snort_conf->tot_inq_inserts );
        LogMessage("ac-bnfa: queue uinserts = "STDu64"\n", snort_conf->tot_inq_uinserts );
    }
#endif
}
static
inline
void
_init_queue(bnfa_struct_t * b)
{
    b->inq=0;
    b->inq_flush=0;
}
/* uniquely insert into q, should splay elements for performance */
static
inline
int
_add_queue(bnfa_struct_t* b, bnfa_match_node_t * p  )
{
    int i;

#ifdef BNFA_TRACK_Q
    snort_conf->tot_inq_inserts++;
#endif

    for(i=(int)(b->inq)-1;i>=0;i--)
        if( p == b->q[i] )
            return 0;

#ifdef BNFA_TRACK_Q
    snort_conf->tot_inq_uinserts++;
#endif

    if( b->inq < MAX_INQ )
    {
        b->q[ b->inq++ ] = p;
    }

    if( b->inq == MAX_INQ )
    {
#ifdef BNFA_TRACK_Q
        b->inq_flush++;
#endif
        return 1;
    }

    return 0;
}

static
inline
unsigned
_process_queue( bnfa_struct_t * bnfa,
              int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
              void *data )
{
    bnfa_match_node_t  * mlist;
    bnfa_pattern_t     * patrn;
    int                  res;
    unsigned int         i;

#ifdef BNFA_TRACK_Q
    if( bnfa->inq > snort_conf->max_inq )
        snort_conf->max_inq = bnfa->inq;
    snort_conf->tot_inq_flush += bnfa->inq_flush;
#endif

    for( i=0; i<bnfa->inq; i++ )
    {
        mlist = bnfa->q[i];
        if (mlist)
        {
            patrn = (bnfa_pattern_t*)mlist->data;
            /*process a pattern -  case is handled by otn processing */
            res = Match (patrn->userdata, mlist->rule_option_tree, 0, data, mlist->neg_list);
            if ( res > 0 )
            {    /* terminate matching */
                bnfa->inq=0;/* clear the q */
                return 1;
            }
        }
    }
    bnfa->inq=0;/* clear the q */
    return 0;
}

static
inline
unsigned
_bnfa_search_csparse_nfa_qx(bnfa_struct_t * bnfa, unsigned char *T, int n,
                            int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                            void *data )
{
    bnfa_match_node_t  * mlist;
    unsigned char      * Tend;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_state_t       * transList = bnfa->bnfaTransList;
    unsigned             sindex=0;

    Tend = T + n;

    for(; T<Tend; T++)
    {
        /* Transition to next state index */
        sindex = _bnfa_get_next_state_csparse_nfa_qx(transList,sindex,xlatcase[*T]);

        /* Log matches in this state - if any */
        if( sindex )
        {
            if( transList[sindex+1] & BNFA_SPARSE_MATCH_BIT )
            {
                mlist = MatchList[ transList[sindex] ];
                if( mlist )
                {
                    if( _add_queue(bnfa,mlist) )
                    {
                        if( _process_queue( bnfa, Match, data ) )
                        {
                            return 1;
                        }
                    }
                }
            }
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static
inline
unsigned
_bnfa_search_csparse_nfa_q(   bnfa_struct_t * bnfa, unsigned char *T, int n,
                 int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                 void *data, unsigned sindex, int *current_state )
{
    bnfa_match_node_t  * mlist;
    unsigned char      * Tend;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_state_t       * transList = bnfa->bnfaTransList;
    unsigned             last_sindex;

    Tend = T + n;

    _init_queue(bnfa);

    for(; T<Tend; T++)
    {
        last_sindex = sindex;

        /* Transition to next state index */
        sindex = _bnfa_get_next_state_csparse_nfa(transList,sindex,xlatcase[*T]);

        /* Log matches in this state - if any */
        if(sindex &&  (transList[sindex+1] & BNFA_SPARSE_MATCH_BIT) )
        {
            /* Test for same as last state */
            if( sindex == last_sindex )
                continue;

            mlist = MatchList[ transList[sindex] ];
            if( mlist )
            {
                if( _add_queue(bnfa,mlist) )
                {
                    if( _process_queue( bnfa, Match, data ) )
                    {
                        *current_state = sindex;
                        return 1;
                    }
                }
            }
        }
    }
    *current_state = sindex;

    return _process_queue( bnfa, Match, data );
}

/*
 *  Per Pattern case search, case is on per pattern basis
 *  standard snort search
 *
 *  note: index is not used by snort, so it's commented
 */
static
inline
unsigned
_bnfa_search_csparse_nfa( bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                          int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                          void *data, unsigned sindex, int *current_state )
{
    bnfa_match_node_t  * mlist;
    unsigned char      * Tend;
    unsigned char      * T;
    unsigned char        Tchar;
    unsigned             index;
    bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
    bnfa_pattern_t     * patrn;
    bnfa_state_t       * transList = bnfa->bnfaTransList;
    unsigned             nfound = 0;
    unsigned             last_match=LAST_STATE_INIT;
    unsigned             last_match_saved=LAST_STATE_INIT;
    int                  res;
#ifdef MATCH_LIST_CNT
    unsigned           * MatchTestCnt = bnfa->bnfaMatchTestCnt;
#endif
    T    = Tx;
    Tend = T + n;

    for(; T<Tend; T++)
    {
        Tchar = xlatcase[ *T ];

        /* Transition to next state index */
        sindex = _bnfa_get_next_state_csparse_nfa(transList,sindex,Tchar);

        /* Log matches in this state - if any */
        if( sindex && (transList[sindex+1] & BNFA_SPARSE_MATCH_BIT) )
        {
            if( sindex == last_match )
                continue;

            last_match_saved = last_match;
            last_match = sindex;

#ifdef MATCH_LIST_CNT
            if( MatchList[ transList[sindex] ] )
                MatchTestCnt[ transList[index] ]++;
#endif

            {
                mlist = MatchList[ transList[sindex] ];
                patrn = (bnfa_pattern_t*)mlist->data;
                if( ( T - Tx) < patrn->n )
                    index = 0;
                else
                    index = T - Tx - patrn->n + 1;
                nfound++;
                /* Don't do anything specific for case sensitive patterns and not,
                 * since that will be covered by the rule tree itself.  Each tree
                 * might have both case sensitive & case insensitive patterns.
                 */
                res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
                if ( res > 0 )
                {
                    *current_state = sindex;
                    return nfound;
                }
                else if( res < 0 )
                {
                    last_match = last_match_saved;
                }
            }
        }
    }
    *current_state = sindex;
    return nfound;
}
/*
 * Case specific search, global to all patterns
 *
 *  note: index is not used by snort, so it's commented
 */
static
inline
unsigned
_bnfa_search_csparse_nfa_case(   bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                        int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                        void *data, unsigned sindex, int *current_state )
{
  bnfa_match_node_t  * mlist;
  unsigned char      * Tend;
  unsigned char      * T;
  unsigned             index;
  bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
  bnfa_pattern_t     * patrn;
  bnfa_state_t       * transList = bnfa->bnfaTransList;
  unsigned             nfound = 0;
  unsigned             last_match=LAST_STATE_INIT;
  unsigned             last_match_saved=LAST_STATE_INIT;
  int                  res;

  T    = Tx;
  Tend = T + n;

  for(; T<Tend; T++)
  {
       /* Transition to next state index */
       sindex = _bnfa_get_next_state_csparse_nfa(transList,sindex,*T);

       /* Log matches in this state - if any */
    if( sindex && (transList[sindex+1] & BNFA_SPARSE_MATCH_BIT) )
    {
        if( sindex == last_match )
            continue;

        last_match_saved = last_match;
        last_match = sindex;

        {
            mlist = MatchList[ transList[sindex] ];
            patrn = (bnfa_pattern_t*)mlist->data;
            if( ( T - Tx) < patrn->n )
                index = 0;
            else
                index = T - Tx - patrn->n + 1;
            nfound++;
            /* Don't do anything specific for case sensitive patterns and not,
             * since that will be covered by the rule tree itself.  Each tree
             * might have both case sensitive & case insensitive patterns.
             */
            res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
            if ( res > 0 )
            {
              *current_state = sindex;
              return nfound;
            }
            else if( res < 0 )
            {
              last_match = last_match_saved;
            }
        }
    }
  }
  *current_state = sindex;
  return nfound;
}
/*
 *  NoCase search - global to all patterns
 *
 *  note: index is not used by snort, so it's commented
 */
static
inline
unsigned
_bnfa_search_csparse_nfa_nocase(   bnfa_struct_t * bnfa, unsigned char *Tx, int n,
                        int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
                        void *data, unsigned sindex, int *current_state )
{
  bnfa_match_node_t  * mlist;
  unsigned char      * Tend;
  unsigned char      * T;
  unsigned char        Tchar;
  unsigned             index;
  bnfa_match_node_t ** MatchList = bnfa->bnfaMatchList;
  bnfa_pattern_t     * patrn;
  bnfa_state_t       * transList = bnfa->bnfaTransList;
  unsigned             nfound = 0;
  unsigned             last_match=LAST_STATE_INIT;
  unsigned             last_match_saved=LAST_STATE_INIT;
  int                  res;

  T    = Tx;
  Tend = T + n;

  for(; T<Tend; T++)
  {
       Tchar = xlatcase[ *T ];

       /* Transition to next state index */
       sindex = _bnfa_get_next_state_csparse_nfa(transList,sindex,Tchar);

       /* Log matches in this state - if any */
    if( sindex && (transList[sindex+1] & BNFA_SPARSE_MATCH_BIT) )
    {
        if( sindex == last_match )
            continue;

        last_match_saved = last_match;
        last_match = sindex;

        {
            mlist = MatchList[ transList[sindex] ];
            patrn = (bnfa_pattern_t*)mlist->data;
            if( ( T - Tx) < patrn->n )
                index = 0;
            else
                index = T - Tx - patrn->n + 1;
            nfound++;
            /* Don't do anything specific for case sensitive patterns and not,
             * since that will be covered by the rule tree itself.  Each tree
             * might have both case sensitive & case insensitive patterns.
             */
            res = Match (patrn->userdata, mlist->rule_option_tree, index, data, mlist->neg_list);
            if ( res > 0 )
            {
              *current_state = sindex;
              return nfound;
            }
            else if( res < 0 )
            {
              last_match = last_match_saved;
            }
        }
      }
  }
  *current_state = sindex;
  return nfound;
}

/*
*  BNFA Search Function
*
*  bnfa   - state machine
*  Tx     - text buffer to search
*  n      - number of bytes in Tx
*  Match  - function to call when a match is found
*  data   - user supplied data that is passed to the Match function
*  sindex - state tracker, set value to zero to reset the state machine,
*            zero should be the value passed in on the 1st buffer or each buffer
*           that is to be analyzed on its own, the state machine updates this
*            during searches. This allows for sequential buffer searchs without
*            reseting the state machine. Save this value as returned from the
*            previous search for the next search.
*
*  returns
*    The state or sindex of the state machine. This can than be passed back
*   in on the next search, if desired.
*/
unsigned
bnfaSearchX( bnfa_struct_t * bnfa, unsigned char *T, int n,
            int (*Match)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list),
            void *data, unsigned sindex, int* current_state )
{
    int ret;

    _init_queue(bnfa);
    while( n > 0)
    {
        ret = _bnfa_search_csparse_nfa_qx( bnfa, T++, n--, Match, data );

        if( ret )
            return 0;
    }
    return _process_queue( bnfa, Match, data );
}

unsigned
bnfaSearch( bnfa_struct_t * bnfa, unsigned char *Tx, int n,
            int (*Match)(void * id, void *tree, int index, void *data, void *neg_list),
            void *data, unsigned sindex, int* current_state )
{
    int ret = 0;

    if (current_state)
    {
        sindex = (unsigned)*current_state;
    }

#ifdef ALLOW_NFA_FULL
    if( bnfa->bnfaFormat == BNFA_SPARSE )
    {
        if( bnfa->bnfaCaseMode == BNFA_PER_PAT_CASE )
        {
            if (bnfa->bnfaMethod)
            {
                ret = _bnfa_search_csparse_nfa( bnfa, Tx, n,
                    (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
                    Match, data, sindex, current_state );
            }
            else
            {
                ret = _bnfa_search_csparse_nfa_q( bnfa, Tx, n,
                    (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
                    Match, data, sindex, current_state );
            }
        }
        else  if( bnfa->bnfaCaseMode == BNFA_CASE )
        {
          ret = _bnfa_search_csparse_nfa_case( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, sindex, current_state );
        }
        else /* NOCASE */
        {
          ret = _bnfa_search_csparse_nfa_nocase( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, sindex, current_state );
        }
    }
    else if( bnfa->bnfaFormat == BNFA_FULL )
    {
        if( bnfa->bnfaCaseMode == BNFA_PER_PAT_CASE  )
        {
            ret = _bnfa_search_full_nfa( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, (bnfa_state_t) sindex, current_state );
        }
        else if( bnfa->bnfaCaseMode == BNFA_CASE  )
        {
            ret = _bnfa_search_full_nfa_case( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, (bnfa_state_t) sindex, current_state );
        }
        else
        {
            ret = _bnfa_search_full_nfa_nocase( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, (bnfa_state_t) sindex, current_state );
        }
    }
#else
    if( bnfa->bnfaCaseMode == BNFA_PER_PAT_CASE )
    {
        if (bnfa->bnfaMethod)
        {
            ret = _bnfa_search_csparse_nfa( bnfa, Tx, n,
                (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
                Match, data, sindex, current_state );
        }
        else
        {
            ret = _bnfa_search_csparse_nfa_q( bnfa, Tx, n,
                (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
                Match, data, sindex, current_state );
        }
    }
    else if( bnfa->bnfaCaseMode == BNFA_CASE )
    {
           ret = _bnfa_search_csparse_nfa_case( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, sindex, current_state );
    }
    else/* NOCASE */
    {
           ret = _bnfa_search_csparse_nfa_nocase( bnfa, Tx, n,
              (int (*)(bnfa_pattern_t * id, void *tree, int index, void *data, void *neg_list))
              Match, data, sindex, current_state );
    }
#endif
    return ret;
}

int bnfaPatternCount( bnfa_struct_t * p)
{
    return p->bnfaPatternCnt;
}

/*
 *  Summary Info Data
 */
static bnfa_struct_t summary;
static int summary_cnt=0;

/*
*  Info: Print info a particular state machine.
*/
void bnfaPrintInfoEx( bnfa_struct_t * p, char * text )
{
    unsigned max_memory;

    if( !p->bnfaNumStates )
    {
        return;
    }
    max_memory = p->bnfa_memory + p->pat_memory + p->list_memory +
                 p->matchlist_memory + p->failstate_memory + p->nextstate_memory;

    if( text && summary_cnt )
    {
    LogMessage("+-[AC-BNFA Search Info%s]------------------------------\n",text);
    LogMessage("| Instances        : %d\n",summary_cnt);
    }
    else
    {
    LogMessage("+-[AC-BNFA Search Info]------------------------------\n");
    }
    LogMessage("| Patterns         : %d\n",p->bnfaPatternCnt);
    LogMessage("| Pattern Chars    : %d\n",p->bnfaMaxStates);
    LogMessage("| Num States       : %d\n",p->bnfaNumStates);
    LogMessage("| Num Match States : %d\n",p->bnfaMatchStates);
    if( max_memory < 1024*1024 )
    {
        LogMessage("| Memory           :   %.2fKbytes\n", (double)max_memory/1024 );
        LogMessage("|   Patterns       :   %.2fK\n",(double)p->pat_memory/1024 );
        LogMessage("|   Match Lists    :   %.2fK\n",(double)p->matchlist_memory/1024 );
        LogMessage("|   Transitions    :   %.2fK\n",(double)p->nextstate_memory/1024 );
    }
    else
    {
        LogMessage("| Memory           :   %.2fMbytes\n", (double)max_memory/(1024*1024) );
        LogMessage("|   Patterns       :   %.2fM\n",(double)p->pat_memory/(1024*1024) );
        LogMessage("|   Match Lists    :   %.2fM\n",(double)p->matchlist_memory/(1024*1024) );
        LogMessage("|   Transitions    :   %.2fM\n",(double)p->nextstate_memory/(1024*1024) );
    }
    LogMessage("+-------------------------------------------------\n");
}
void bnfaPrintInfo( bnfa_struct_t * p )
{
     bnfaPrintInfoEx( p, 0 );
}

void bnfaPrintSummary( void )
{
     bnfaPrintInfoEx( &summary, " Summary" );
}
void bnfaInitSummary( void )
{
    summary_cnt=0;
    memset(&summary,0,sizeof(bnfa_struct_t));
}
void bnfaAccumInfo( bnfa_struct_t * p )
{
    bnfa_struct_t * px = &summary;

    summary_cnt++;

    px->bnfaAlphabetSize  = p->bnfaAlphabetSize;
    px->bnfaPatternCnt   += p->bnfaPatternCnt;
    px->bnfaMaxStates    += p->bnfaMaxStates;
    px->bnfaNumStates    += p->bnfaNumStates;
    px->bnfaNumTrans     += p->bnfaNumTrans;
    px->bnfaMatchStates  += p->bnfaMatchStates;
    px->bnfa_memory      += p->bnfa_memory;
    px->pat_memory       += p->pat_memory;
    px->list_memory      += p->list_memory;
    px->matchlist_memory += p->matchlist_memory;
    px->nextstate_memory += p->nextstate_memory;
    px->failstate_memory += p->failstate_memory;
}

#ifdef MATCH_LIST_CNT
void bnfaPrintMatchListCnt( bnfa_struct_t * p )
{
    unsigned * cnt = p->bnfaMatchListCnt;
    int i;
    bnfa_match_node_t  * mn;
    bnfa_pattern_t     * patrn;

    printf("[ MatchListCnt for ac-bnfa state machine\n ]");

    for(i=0;i<bnfa->bnfaNumStates;i++)
    {
        if( cnt[i] )
        {
            printf("state[%d] cnt=%d",i,cnt[i]);
            mn = bnfa->MatchList[i] ;
            if( mn )
            {
            patrn  =(bnfa_pattern_t *)mn->data;
            //xprintOTNSidGid(cnt,patrn->userdata);
            }
            printf("\n");
            fflush(stdout);
        }
    }
}
#endif

#ifdef BNFA_MAIN
#include <stdarg.h>
/*
*  Text Data Buffer
*/
unsigned char text[512];
unsigned char text2[512];
static int s_verbose=0;

/*
*    A Match is found
*/
 int
MatchFound (void* id, void *tree, int index, void *data, void *neglist)
{
  fprintf (stdout, "%s\n", (char *) data);
  return 0;
}

void objfree(void **obj)
{
    return;
}

int buildtree(void *id, void **existing)
{
    return 1;
}

int neglist(void *id, void **list)
{
    return 1;
}

void LogMessage(const char *format,...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

SnortConfig sc;
SnortConfig *snort_conf;

/*
*
*/
int
main (int argc, char **argv)
{
  int i, nc, nocase = 0;
  bnfa_struct_t * bnfa;
  int current_state = 0;
  bool split_search = false;
  char * p;

  if (argc < 3)

    {
      fprintf (stderr,"Usage: %s search-text pattern +pattern... [flags]\n",argv[0]);
      fprintf (stderr,"  flags: -q -nocase -splitsearch -v\n");
      exit (0);
    }

  memset(&sc, 0, sizeof(SnortConfig));
  snort_conf = &sc;

  bnfa = bnfaNew(free, objfree, objfree);
  if( !bnfa )
  {
     printf("bnfa-no memory\n");
     exit(0);
  }

  strncpy (text, argv[1], sizeof(text) - 1);
  text[sizeof(text) - 1] = '\0';

  bnfa->bnfaMethod = 1;

  for (i = 1; i < argc; i++)
  {
    if (strcmp (argv[i], "-nocase") == 0){
      nocase = 1;
    }
    if (strcmp (argv[i], "-v") == 0){
      s_verbose=1;
    }
    if (strcmp (argv[i], "-splitsearch") == 0){
      int len2 = strlen(text)/2;
      split_search =true;
      strncpy(text2, &text[len2], sizeof(text2) -1 );
      text[len2] = '\0';
      text2[len2] = '\0';
    }

    if (strcmp (argv[i], "-q") == 0){
        bnfa->bnfaMethod = 0;
    }
  }

  for (i = 2; i < argc; i++)
  {
      if (argv[i][0] == '-')
          continue;

      p = argv[i];

      if ( *p == '+')
      {
          nc=1;
          p++;
      }
      else
      {
          nc = nocase;
      }

      bnfaAddPattern (bnfa, p, strlen(p), nc, 0, (void*)NULL);
  }

  if(s_verbose)printf("Patterns added\n");

  //Print_DFA (acsm);

  bnfaCompile (bnfa, buildtree, neglist);

  //Write_DFA(acsm, "bnfa-snort.dfa") ;

  if(s_verbose) printf("Patterns compiled--written to file.\n");

  bnfaPrintInfo ( bnfa );
  bnfaPrintSummary ( );

  bnfaSearch (bnfa, text, strlen (text), MatchFound, NULL, current_state, &current_state);

  if (split_search)
      bnfaSearch (bnfa, text2, strlen (text2), MatchFound, NULL, current_state, &current_state);

  bnfaFree (bnfa);

  printf ("normal pgm end\n");

  return (0);
}
#endif /*  */

