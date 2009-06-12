#ifndef CYGONCE_KERNEL_LLISTT_HXX
#define CYGONCE_KERNEL_LLISTT_HXX

//==========================================================================
//
//      llistt.hxx
//
//      Llistt linked list template class declarations
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-02-10
// Purpose:     Define Llistt template class
// Description: The classes defined here provide the APIs for llistts.
// Usage:       #include <cyg/kernel/llistt.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros
#include <cyg/kernel/thread.hxx>

// -------------------------------------------------------------------------
// A simple linked list template; each item also contains a pointer of type
// T, and you can search for a particular T* in a list.
// 
// It is intended that this list be amenable to the trick of using the
// address of the pointer that is the list head, cast to an item pointer,
// as the "zeroth" element of the list; prev of the first item is the
// address of the head pointer, and inserting before the first item works
// correctly.  For this reason, a "getprev" is not provided; iteration may
// only be forwards, until a NULL is found.
//
// It is expected that derived classes will be used to hold other
// information than just the T* but that is beyond our discussion here;
// only the T* can be searched for using code provided here.
//
// This module is NOT thread-safe; it is expected that all clients will be
// seeing that that themselves.

template <class T>
class Cyg_Llistt
{
private:
    Cyg_Llistt<T> *next, *prev;
    T *tptr;

private:
    // make initialisation _without_ a T* impossible.
    Cyg_Llistt<T> &operator=(Cyg_Llistt<T> &);
    Cyg_Llistt(Cyg_Llistt<T> &);
    Cyg_Llistt();

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Llistt( T *tvalue ) // Constructor
    {
        tptr = tvalue;
        next = prev = NULL;
    }

    ~Cyg_Llistt()                       // Destructor
    {
        CYG_ASSERT( NULL == next, "bad item next - still in list" );
        CYG_ASSERT( NULL == prev, "bad item prev - still in list" );
    }

    // iterator, basically.
    Cyg_Llistt<T> * getnext() { return next; }

    // get the value
    T * getitem() { return tptr; }

    // look up a particular T value in the llist
    static Cyg_Llistt<T> *
    find( Cyg_Llistt<T> *list, T *tvalue )
    {
        for ( ; list ; list = list->next ) {
            if ( list->tptr == tvalue )
                break;
        }
        return list;
    }

    // unlink an item from the list
    void
    unlink()
    {
        CYG_ASSERT( prev, "not in a list" );
        prev->next = next;
        if ( next ) {
            next->prev = prev;
        }
        next = prev = NULL;
    }

    // insert a new item in the list after "this"
    void
    insertafter( Cyg_Llistt<T> *item )
    {
        CYG_ASSERT( item, "null item" );
        CYG_ASSERT( NULL == item->next, "bad item next - already linked" );
        CYG_ASSERT( NULL == item->prev, "bad item prev - already linked" );
        item->next = next;
        item->prev = this;
        if ( next )
            next->prev = item;
        next = item;
    }   

    // insert a new item in the list before "this"
    void
    insertbefore( Cyg_Llistt<T> *item )
    {
        CYG_ASSERT( prev, "this not in a list" );
        CYG_ASSERT( item, "null item" );
        CYG_ASSERT( NULL == item->next, "bad item next - already linked" );
        CYG_ASSERT( NULL == item->prev, "bad item prev - already linked" );
        item->prev = prev;
        item->next = this;
        prev->next = item;
              prev = item;
    }   
};



// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_LLISTT_HXX
// EOF llistt.hxx
