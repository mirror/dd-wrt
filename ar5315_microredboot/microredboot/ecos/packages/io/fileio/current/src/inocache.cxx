//=============================================================================
//
//      inocache.cxx
//
//      Implementation of inode cache
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2002-01-18
// Purpose:       
// Description:   
// Usage:
//              #include <cyg/fileio/inode.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <stdlib.h>
#include <cyg/fileio/inode.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/mutex.hxx>
# if 0
# define LOCK_ICACHE(_ic_)        \
  CYG_MACRO_START                 \
  (_ic_)->icmutex.lock();         \
  CYG_MACRO_END

# define UNLOCK_ICACHE(_ic_)      \
  CYG_MACRO_START                 \
  (_ic_)->icmutex.unlock();       \
  CYG_MACRO_END
# define LOCK_INO(_ic_,_ino_)     \
  CYG_MACRO_START                 \
  (_ic_)->icmutex.lock();         \
  CYG_MACRO_END
# define UNLOCK_INO(_ic_,_ino_)   \
  CYG_MACRO_START                 \
  (_ic_)->icmutex.unlock();       \
  CYG_MACRO_END
# endif
#endif

# define LOCK_ICACHE(_ic_)         CYG_EMPTY_STATEMENT
# define UNLOCK_ICACHE(_ic_)       CYG_EMPTY_STATEMENT
# define LOCK_INO(_ic_,_ino_)      CYG_EMPTY_STATEMENT
# define UNLOCK_INO(_ic_,_ino_)    CYG_EMPTY_STATEMENT

// Tried to make this implementation use tables, but the requirement to
// allow for extra space makes this difficult

__externC void
cyg_inodecache_destroy( cyg_inodecache *ic )
{
    cyg_inode *tmp;
    for ( tmp=ic->head; tmp != NULL; tmp=tmp->i_cache_next )
        free(tmp);
#if CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD > 0
    for ( tmp=ic->freeable; tmp != NULL; tmp=tmp->i_cache_next )
        free(tmp);
#endif    
} // cyg_inodecache_destroy()

static __inline__ void
insert_in_list( cyg_inode **i, cyg_inode *ino )
{   
    cyg_inode *here = *i;
    if ( here == NULL ) {
        ino->i_cache_prev = ino->i_cache_next = ino;
    } else {
        ino->i_cache_prev = here->i_cache_prev;
        ino->i_cache_next = here;
        here->i_cache_prev = ino;
        ino->i_cache_prev->i_cache_next = ino;
    }
    // put at start, as this is more likely to come off sooner than later
    *i = ino;
}

// Create an inode. Returns a negative error code on error.
__externC cyg_inode *
cyg_inode_create( cyg_inodecache *ic )
{
    cyg_inode *ni;

    ni = (cyg_inode *)malloc( sizeof(cyg_inode)+ic->privatespace );
    if ( !ni )
        return ni;
    ni->i_count = 1;

    LOCK_ICACHE(ic);
    insert_in_list( &ic->head, ni );
    UNLOCK_ICACHE(ic);
    
    return ni;
} // cyg_inode_create()

__externC cyg_inode *
cyg_inode_get( cyg_inodecache *ic, cyg_uint32 ino )
{
    cyg_inode *head = ic->head;
    // first try the (live) cache
    if (head) { 
        cyg_inode *tmp=head;
        while (1) {
            if ( tmp->i_ino == ino ) {
                tmp->i_count++;
                return tmp;
            }
            tmp = tmp->i_cache_next;
            if ( tmp == head )
                break;
        }
    }
#if CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD > 0
    // now try the cache of dead inodes
    head = ic->freeable;
    if (head) { 
        cyg_inode *tmp=head;
        while (1) {
            if ( tmp->i_ino == ino ) {
                tmp->i_count++;
                LOCK_ICACHE(ic);
                ic->freeablelistlen--;
                if ( ic->freeablelistlen ) {
                    tmp->i_cache_prev->i_cache_next = tmp->i_cache_next;
                    tmp->i_cache_next->i_cache_prev = tmp->i_cache_prev;
                } else
                    ic->freeable = NULL;
                insert_in_list( &ic->head, tmp );
                UNLOCK_ICACHE(ic);
                return tmp;
            }
            tmp = tmp->i_cache_next;
            if ( tmp == head )
                break;
        }
    }
#endif
    // not found so make it
    return cyg_inode_create( ic );
    
} // cyg_inode_get()

__externC void
cyg_inode_put( cyg_inodecache *ic, cyg_inode *ino )
{
    if ( --ino->i_count == 0 )
    {
        LOCK_ICACHE(ic);
        if ( ino->i_cache_next == ino ) {
            ic->head = NULL;
        } else {
            ino->i_cache_prev->i_cache_next = ino->i_cache_next;
            ino->i_cache_next->i_cache_prev = ino->i_cache_prev;
        }
#if CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD > 0
        ic->freeablelistlen++;
        insert_in_list( &ic->freeable, ino );
        if ( ic->freeablelistlen > CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD ) {
            cyg_inode *prev = ino->i_cache_prev;
            prev->i_cache_prev->i_cache_next = ino;
            ino->i_cache_prev = prev->i_cache_prev;
            ic->freecallback(prev);
            free(prev);
        }
#else
        ic->freecallback(ino);
        free(ino);
#endif
    }
} // cyg_inode_put()

// EOF inode.h
