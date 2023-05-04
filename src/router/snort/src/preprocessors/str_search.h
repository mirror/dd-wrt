/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 

#ifndef __STR_SEARCH_H__
#define __STR_SEARCH_H__

#include "mpse_methods.h"

/*search pattern case sensitivity */
#define STR_SEARCH_CASE_SENSITIVE 0
#define STR_SEARCH_CASE_INSENSITIVE 1

/* Function prototypes  */
typedef int (*MatchFunction)(void *, void *, int, void *, void *);

int  SearchInit(unsigned int num);
int  SearchGetHandle(void);
int  SearchPutHandle(unsigned int id);
int  SearchReInit(unsigned int i);
void SearchFree(void);
void SearchFreeId(unsigned id);
void SearchAdd(unsigned int mpse_id, const char *pat, unsigned int pat_len, int id);
void SearchPrepPatterns(unsigned int mpse_id);
int  SearchFindString(unsigned int mpse_id, const char *str, unsigned int str_len, int confine, MatchFunction);


void * SearchInstanceNew( void );
void * SearchInstanceNewEx( unsigned method );
void   SearchInstanceFree( void * insance );
void   SearchInstanceAdd( void * instance, const char *pat, unsigned int pat_len, int id);
void   SearchInstanceAddEx( void * instance, const char *pat, unsigned int pat_len, void* id, unsigned nocase);
void   SearchInstancePrepPatterns( void * instance );
int    SearchInstanceFindString( void * instance, const char *str, unsigned int str_len, int confine, MatchFunction);
int    SearchInstanceFindStringAll( void * instance, const char *str, unsigned int str_len, int confine, MatchFunction, void *userData);
int    SearchInstanceSFindString( void * instance, const char *str, unsigned int str_len, int confine, MatchFunction, int *state);

typedef struct _search_api
{
    int (*search_init)(unsigned int);

    int (*search_reinit)(unsigned int);

    void (*search_free)(void);

    void (*search_add)(unsigned int, const char *, unsigned int, int);

    void (*search_prep)(unsigned int);

    int (*search_find)(unsigned int, const char *, unsigned int, int, MatchFunction); 

    /* 6/1/06*/
    void (*search_free_id)(unsigned id);
    
    int (*search_get_handle)(void);
    int (*search_put_handle)(unsigned int);

    void * (*search_instance_new)(void);
    void * (*search_instance_new_ex)(unsigned method);
    void   (*search_instance_free)(void * instance);
    void   (*search_instance_add) (void * instance, const char *s, unsigned int s_len, int s_id);
    void   (*search_instance_add_ex) (void * instance, const char *s, unsigned int s_len, void* s_id, unsigned nocase);
    void   (*search_instance_prep)(void * instance );
    int    (*search_instance_find)(void * instance, const char *s, unsigned int s_len, int confine, MatchFunction); 
    int    (*search_instance_find_all)(void * instance, const char *s, unsigned int s_len, int confine, MatchFunction, void *userData); 
    char * (*search_instance_find_end)(char *match_ptr, int buflen, char *search_str, int search_len);  
    int    (*stateful_search_instance_find)(void * instance, const char *s, unsigned int s_len, int confine, MatchFunction, int *state); 
    
} SearchAPI;

extern SearchAPI *search_api;

#endif  /*  __STR_SEARCH_H__  */

