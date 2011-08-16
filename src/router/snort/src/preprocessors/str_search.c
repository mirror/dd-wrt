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
 


#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

#include "str_search.h"
#include "mpse.h"

typedef struct tag_search
{
    void *mpse;
    unsigned int max_len;
    int in_use;
} t_search;

static t_search *_mpse = NULL;
static unsigned int  _num_mpse=0;
       
void SearchFreeId(unsigned int id);

int SearchInit(unsigned int num)
{
    unsigned int i;

    _num_mpse = num;

    _mpse = malloc(sizeof(t_search) * num);
    if ( _mpse == NULL )
        return -1;

    for ( i = 0; i < num; i++ )
    {
        _mpse[i].mpse = mpseNew(MPSE_AC_BNFA, MPSE_DONT_INCREMENT_GLOBAL_COUNT,
                                NULL, NULL, NULL);
        if ( !_mpse[i].mpse )
            return -1;
        _mpse[i].max_len = 0;
        _mpse[i].in_use=1;
    }
    return 0;
}

int SearchReInit(unsigned int i)
{
    if ( _mpse[i].mpse != NULL )
        mpseFree(_mpse[i].mpse);

    _mpse[i].mpse = mpseNew(MPSE_AC_BNFA, MPSE_DONT_INCREMENT_GLOBAL_COUNT,
                            NULL, NULL, NULL);
    _mpse[i].max_len = 0;
    _mpse[i].in_use=1;
    
    if ( !_mpse[i].mpse )
        return -1;

    return 0;
}


void SearchFree(void)
{
    unsigned int i;

    if ( _mpse != NULL )
    {
        for ( i = 0; i < _num_mpse; i++ )
        {
            if ( _mpse[i].mpse != NULL )
                mpseFree(_mpse[i].mpse);
            _mpse[i].in_use=0;
         }
        free(_mpse);
    }
}

void SearchFreeId(unsigned int id)
{
    if (  id < _num_mpse && _mpse != NULL )
    {
       if ( _mpse[id].mpse != NULL )
            mpseFree(_mpse[id].mpse);
       _mpse[id].mpse=NULL;
    }
}

int SearchGetHandle(void)
{
    unsigned int i;

    for ( i = 0; i < _num_mpse; i++ )
    {
        if ( !_mpse[i].mpse && !_mpse[i].in_use )
        {
            _mpse[i].in_use=1;
            
            return i;
        }
    }

    return -1;
    
}

int SearchPutHandle(unsigned int id)
{
    if( (id > 0 && id < _num_mpse) && _mpse[id].in_use )
    {
       SearchFreeId(id);

       _mpse[id].in_use=0;

       return 0;
    }
    return -1;
}


/*  
    Do efficient search of data 
    @param   mpse_id    specify which engine to use to search
    @param   str        string to search
    @param   str_len    length of string to search
    @param   confine   1 means only search at beginning of string (confine to length of max search string)
    @param   Match      function callback when string match found
 */
int SearchFindString(unsigned int mpse_id,
                     const char *str,
                     unsigned int str_len,
                     int confine,
                     int (*Match) (void *, void *, int, void *, void *))
{
    int num;
    int start_state;

    if ( confine && _mpse[mpse_id].max_len != 0 )
    {
        if ( _mpse[mpse_id].max_len < str_len )
        {
            str_len = _mpse[mpse_id].max_len;
        }
    }

    start_state = 0;
    num = mpseSearch(_mpse[mpse_id].mpse, (unsigned char*)str, str_len, Match, (void *) str, 
		    	&start_state );
    
    return num;
}


void SearchAdd(unsigned int mpse_id, const char *pat, unsigned int pat_len, int id)
{
    mpseAddPattern(_mpse[mpse_id].mpse, (void *)pat, pat_len, 1, 0, 0, 0, (void *)(long) id, 0);

    if ( pat_len > _mpse[mpse_id].max_len )
        _mpse[mpse_id].max_len = pat_len;
}

void SearchPrepPatterns(unsigned int mpse_id)
{
    mpsePrepPatterns(_mpse[mpse_id].mpse, NULL, NULL);
}


/*
 * Instance Functions
 *
 * max_len is not handled by 
 */
void *  SearchInstanceNew(void)
{
    t_search * search = malloc(sizeof(t_search));
    if( !search ) 
        return NULL;

    search->mpse  = mpseNew(MPSE_AC_BNFA, MPSE_DONT_INCREMENT_GLOBAL_COUNT,
                            NULL, NULL, NULL);
    if (search-> mpse == NULL )
    {
        free(search);
        return NULL;
    }
    search->in_use=1;
    search->max_len=0;

    return search;
}
void SearchInstanceFree( void * instance )
{
    t_search * search = (t_search*)instance;
    if( instance )
    {
        mpseFree( search->mpse );
        free( instance );
    }
}

void SearchInstanceAdd( void*instance, const char *pat, unsigned int pat_len, int id)
{
    t_search * search = (t_search*)instance;
   
    if( search && search->mpse )
        mpseAddPattern( search->mpse, (void *)pat, pat_len, 1, 0, 0, 0, (void *)(long) id, 0);
    
    if ( search && pat_len > search->max_len )
         search->max_len = pat_len;
    
}
void SearchInstancePrepPatterns(void * instance)
{
    t_search * search = (t_search*)instance;
    if( search && search->mpse )
    {
        mpsePrepPatterns( search->mpse, NULL, NULL);
    }
}

int  SearchInstanceFindString(void * instance,
                              const char *str,
                              unsigned int str_len, 
                              int confine, 
                              int (*Match) (void *, void *, int, void *, void *))
{
    int num;
    int start_state = 0;
    t_search * search = (t_search*)instance;
    
    if ( confine && (search->max_len > 0) )
    {
        if ( search->max_len < str_len )
        {
            str_len = search->max_len;
        }
    }
    num = mpseSearch( search->mpse, (unsigned char*)str, str_len, Match, (void *) str,
            &start_state);
    
    return num;
    
}


/* API exported by this module */
SearchAPI searchAPI =
{
    SearchInit,
    SearchReInit,
    SearchFree,
    SearchAdd,
    SearchPrepPatterns,
    SearchFindString,
    SearchFreeId,
    SearchGetHandle,
    SearchPutHandle,
    SearchInstanceNew,
    SearchInstanceFree,
    SearchInstanceAdd,
    SearchInstancePrepPatterns,
    SearchInstanceFindString,
};

SearchAPI *search_api = &searchAPI;

