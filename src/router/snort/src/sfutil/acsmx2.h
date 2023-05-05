/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/*
**   ACSMX2.H
**
**   Version 2.0
**
**   Author: Marc Norton
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ACSMX2S_H
#define ACSMX2S_H

/*
*   DEFINES and Typedef's
*/
#define MAX_ALPHABET_SIZE 256

/*
   FAIL STATE for 1,2,or 4 bytes for state transitions

   Uncomment this define to use 32 bit state values
   #define AC32
*/

#define AC32

#ifdef AC32

typedef  unsigned int   acstate_t;
#define ACSM_FAIL_STATE2  0xffffffff

#else

typedef    unsigned short acstate_t;
#define ACSM_FAIL_STATE2 0xffff

#endif

/*
*
*/
typedef
struct _acsm_pattern2
{
    struct  _acsm_pattern2 *next;

    unsigned char         *patrn;
    unsigned char         *casepatrn;
    int      n;
    int      nocase;
    int      offset;
    int      depth;
    int      negative;
    void *udata;
    int      iid;
    void   * rule_option_tree;
    void   * neg_list;

} ACSM_PATTERN2;

/*
*    transition nodes  - either 8 or 12 bytes
*/
typedef
struct trans_node_s {

  acstate_t    key;           /* The character that got us here - sized to keep structure aligned on 4 bytes */
                              /* to better the caching opportunities. A value that crosses the cache line */
                              /* forces an expensive reconstruction, typing this as acstate_t stops that. */
  acstate_t    next_state;    /*  */
  struct trans_node_s * next; /* next transition for this state */

} trans_node_t;


/*
*  User specified final storage type for the state transitions
*/
enum {
  ACF_FULL,
  ACF_SPARSE,
  ACF_BANDED,
  ACF_SPARSEBANDS,
  ACF_FULLQ
};

/*
*   User specified machine types
*
*   TRIE : Keyword trie
*   NFA  :
*   DFA  :
*/
enum {
  FSA_TRIE,
  FSA_NFA,
  FSA_DFA
};

#define AC_MAX_INQ 32
typedef struct
{
    unsigned inq;
    unsigned inq_flush;
    void * q[AC_MAX_INQ];
} PMQ;

/*
*   Aho-Corasick State Machine Struct - one per group of pattterns
*/
typedef struct {

    int acsmMaxStates;
    int acsmNumStates;

    ACSM_PATTERN2    * acsmPatterns;
    acstate_t        * acsmFailState;
    ACSM_PATTERN2   ** acsmMatchList;

    /* list of transitions in each state, this is used to build the nfa & dfa */
    /* after construction we convert to sparse or full format matrix and free */
    /* the transition lists */
    trans_node_t ** acsmTransTable;

    acstate_t ** acsmNextState;
    int          acsmFormat;
    int          acsmSparseMaxRowNodes;
    int          acsmSparseMaxZcnt;

    int          acsmNumTrans;
    int          acsmAlphabetSize;
    int          acsmFSA;
    int          numPatterns;
    void         (*userfree)(void *p);
    void         (*optiontreefree)(void **p);
    void         (*neg_list_free)(void **p);
    PMQ q;
    int sizeofstate;
    int compress_states;

}ACSM_STRUCT2;

/*
*   Prototypes
*/
ACSM_STRUCT2 * acsmNew2 (void (*userfree)(void *p),
                         void (*optiontreefree)(void **p),
                         void (*neg_list_free)(void **p));
int acsmAddPattern2( ACSM_STRUCT2 * p, unsigned char * pat, int n,
                    int nocase, int offset, int depth, int negative, void * id, int iid );
int acsmCompile2 ( ACSM_STRUCT2 * acsm,
                   int (*build_tree)(void * id, void **existing_tree),
                   int (*neg_list_func)(void *id, void **list));
struct _SnortConfig;
int acsmCompile2WithSnortConf ( struct _SnortConfig *, ACSM_STRUCT2 * acsm,
                                int (*build_tree)(struct _SnortConfig *, void * id, void **existing_tree),
                                int (*neg_list_func)(void *id, void **list));
int acsmSearch2 ( ACSM_STRUCT2 * acsm,unsigned char * T, int n,
                  int (*Match)(void * id, void *tree, int index, void *data, void *neg_list),
                  void * data, int* current_state );
int acsmSearchAll2 ( ACSM_STRUCT2 * acsm,unsigned char * T, int n,
                  int (*Match)(void * id, void *tree, int index, void *data, void *neg_list),
                  void * data, int* current_state );
void acsmFree2 ( ACSM_STRUCT2 * acsm );
int acsmPatternCount2 ( ACSM_STRUCT2 * acsm );

void acsmCompressStates(ACSM_STRUCT2 *, int);

int  acsmSelectFormat2( ACSM_STRUCT2 * acsm, int format );
int  acsmSelectFSA2( ACSM_STRUCT2 * acsm, int fsa );

void acsmSetMaxSparseBandZeros2( ACSM_STRUCT2 * acsm, int n );
void acsmSetMaxSparseElements2( ACSM_STRUCT2 * acsm, int n );
int  acsmSetAlphabetSize2( ACSM_STRUCT2 * acsm, int n );
void acsmSetVerbose2(void);

void acsmPrintInfo2( ACSM_STRUCT2 * p);

int acsmPrintDetailInfo2(ACSM_STRUCT2*);
int acsmPrintSummaryInfo2(void);
void acsmx2_print_qinfo(void);
void acsm_init_summary(void);

#endif
