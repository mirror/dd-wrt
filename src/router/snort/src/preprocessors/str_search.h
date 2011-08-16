/****************************************************************************
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 

#ifndef __STR_SEARCH_H__
#define __STR_SEARCH_H__

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
void   SearchInstanceFree( void * insance );
void   SearchInstanceAdd( void * instance, const char *pat, unsigned int pat_len, int id);
void   SearchInstancePrepPatterns( void * instance );
int    SearchInstanceFindString( void * instance, const char *str, unsigned int str_len, int confine, MatchFunction);

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
    void   (*search_instance_free)(void * instance);
    void   (*search_instance_add) (void * instance, const char *s, unsigned int s_len, int s_id);
    void   (*search_instance_prep)(void * instance );
    int    (*search_instance_find)(void * instance, const char *s, unsigned int s_len, int confine, MatchFunction); 
    
} SearchAPI;

extern SearchAPI *search_api;

#endif  /*  __STR_SEARCH_H__  */

