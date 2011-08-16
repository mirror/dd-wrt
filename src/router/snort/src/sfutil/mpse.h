/*
** $Id$
**
**  mpse.h       
**
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Marc Norton <mnorton@sourcefire.com>
**
** Multi-Pattern Search Engine
**
**  Supports:
**
**    Modified Wu-Manber mwm.c/.h
**    Aho-Corasick - Deterministic Finite Automatum   
**    Keyword Trie with Boyer Moore Bad Character Shifts
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
** GNU Gener*
**
**
** Updates:
**
** man - 7/25/2002 - modified #defines for WIN32, and added uint64
**
*/

#ifndef _MPSE_H
#define _MPSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "bitop.h"

/*
*   Move these defines to a generic Win32/Unix compatability file, 
*   there must be one somewhere...
*/
#ifndef CDECL 
#define CDECL 
#endif


/*
*  Pattern Matching Methods 
*/
//#define MPSE_MWM       1
#define MPSE_AC        2
//#define MPSE_KTBM      3
#define MPSE_LOWMEM    4
//#define MPSE_AUTO      5
#define MPSE_ACF       6
#define MPSE_ACS       7
#define MPSE_ACB       8
#define MPSE_ACSB      9
#define MPSE_AC_BNFA   10 
#define MPSE_AC_BNFA_Q 11 
#define MPSE_ACF_Q     12 
#define MPSE_LOWMEM_Q  13 

#ifdef INTEL_SOFT_CPM
#define MPSE_INTEL_CPM 14 
#endif /* INTEL_SOFT_CPM */

#define MPSE_INCREMENT_GLOBAL_CNT 1
#define MPSE_DONT_INCREMENT_GLOBAL_COUNT 0

/*
** PROTOTYPES
*/
void * mpseNew( int method, int use_global_counter_flag,
                void (*userfree)(void *p),
                void (*optiontreefree)(void **p),
                void (*neg_list_free)(void **p));
void   mpseFree( void * pv );

int  mpseAddPattern  ( void * pv, void * P, int m, 
                       unsigned noCase, unsigned offset, unsigned depth,
                       unsigned negative, void* ID, int IID );

void mpseLargeShifts   ( void * pvoid, int flag );

int  mpsePrepPatterns  ( void * pvoid,
                         int ( *build_tree )(void *id, void **existing_tree),
                         int ( *neg_list_func )(void *id, void **list) );

void mpseSetRuleMask   ( void *pv, BITOP * rm );

int  mpseSearch( void *pv, const unsigned char * T, int n, 
                 int ( *action )(void* id, void * tree, int index, void *data, void *neg_list), 
                 void * data, int* current_state ); 

int mpseGetPatternCount(void *pv);

uint64_t mpseGetPatByteCount(void);
void   mpseResetByteCount(void);

int mpsePrintInfo( void * obj );
int mpsePrintSummary(int);
  
void   mpseVerbose( void * pvoid );
void   mpseSetOpt( void * pvoid,int flag);

void mpse_print_qinfo(void);
void mpseInitSummary(void);

#endif

