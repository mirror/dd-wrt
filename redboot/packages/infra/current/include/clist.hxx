#ifndef CYGONCE_INFRA_CLIST_HXX
#define CYGONCE_INFRA_CLIST_HXX

//==========================================================================
//
//      clist.hxx
//
//      Standard types, and some useful coding macros.
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
// Author(s):     nickg
// Contributors:  nickg
// Date:        2000-11-08
// Purpose:     Simple circular list implementation
// Description: A simple implementation of circular lists.
// Usage:       #include "cyg/infra/clist.hxx"
//              ...
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>

// -------------------------------------------------------------------------
// Class and structure conversion macros.
// CYG_CLASSFROMFIELD translates a pointer to a field of a struct or
// class into a pointer to the class.
// CYG_OFFSETOFBASE yields the offset of a base class of a derived
// class.
// CYG_CLASSFROMBASE translates a pointer to a base class into a pointer
// to a selected derived class. The base class object _must_ be part of
// the specified derived class. This is essentially a poor mans version
// of the RTTI dynamic_cast operator.
// Caveat: These macros do not work for virtual base classes.

// Note: These definitions are exact duplicates of definitions in
// ktypes.h. If either definition is changed, the other should be too
// to avoid compiler messages.

#define CYG_CLASSFROMFIELD(_type_,_member_,_ptr_)\
    ((_type_ *)((char *)(_ptr_)-((char *)&(((_type_ *)0)->_member_))))

#define CYG_OFFSETOFBASE(_type_,_base_)\
    ((char *)((_base_ *)((_type_ *)4)) - (char *)4)

# define CYG_CLASSFROMBASE(_class_,_base_,_ptr_)\
    ((_class_ *)((char *)(_ptr_) - CYG_OFFSETOFBASE(_class_,_base_)))


// -------------------------------------------------------------------------
// Cyg_DNode class.
// This simply represents a double linked node that is intended to
// be a base member of the class that is being managed.

class Cyg_DNode
{
    friend class Cyg_CList;
    
    Cyg_DNode   *next;
    Cyg_DNode   *prev;

public:

    Cyg_DNode()
    {
        // Initialize pointers to point here
        next = prev = this;
    };

    // Accessor and test functions
    Cyg_DNode *get_next() { return next; };
    Cyg_DNode *get_prev() { return prev; };
    cyg_bool  in_list() { return next != this; };
    
    // Insert a node into the list before this one,
    // so that it becomes this nodes predecessor in
    // the list.
    void insert( Cyg_DNode *node )
    {
        node->next = this;
        node->prev = prev;
        prev->next = node;
        prev = node;
    };

    // Append a node after this one so that it become
    // this nodes sucessor in the list.
    void append( Cyg_DNode *node )
    {
        node->prev = this;
        node->next = next;
        next->prev = node;
        next = node;
    };

    // Unlink this node from it's list. It is safe to apply this to an
    // already unlinked node.
    void unlink()
    {
        next->prev = prev;
        prev->next = next;
        next = prev = this;
    };
    
    ~Cyg_DNode()
    {
        // If this node is still linked, unlink it.
        if( next != this )
            unlink();
    };
    
};

// -------------------------------------------------------------------------
// Cyg_CList class.

// This is a simple class that manages a circular list of DNodes. This
// object points to the head of the list and provides functions to
// manipulate the head and tail of the list.

class Cyg_CList
{
    Cyg_DNode   *head;                  // list head pointer

public:

    Cyg_CList()
    {
        head = NULL;
    };

    // Accessor and test functions
    Cyg_DNode *get_head() { return head; };
    Cyg_DNode *get_tail() { return head?head->prev:NULL; };
    cyg_bool empty() { return head == NULL; };
    
    // Add a node at the head of the list
    void add_head( Cyg_DNode *node )
    {
        if( head == NULL )
            head = node;
        else
        {
            head->insert( node );
            head = node;
        }
    };

    // Remove the node at the head of the list
    Cyg_DNode *rem_head()
    {
        Cyg_DNode *node = head;
        if( node != NULL )
        {
            // There is a node available
            Cyg_DNode *next = node->next;
            if( next == node )
            {
                // Only node on list
                head = NULL;
            }
            else
            {
                // remove head node and move head to next.
                node->unlink();
                head = next;
            }
        }
        return node;
    };


    // Add a node at the tail of the list
    void add_tail( Cyg_DNode *node )
    {
        if( head == NULL )
            head = node;
        else
            head->insert( node );
    };

    // Remove the node at the tail of the list
    Cyg_DNode *rem_tail()
    {
        if( head == NULL )
            return NULL;

        Cyg_DNode *node = head->prev;

        if( node == head )
            head = NULL;
        else node->unlink();
        
        return node;
    };

    // Merge the supplied list into this one, at the tail.
    void merge( Cyg_CList& list )
    {
        if( list.head == NULL )
            return;                     // Nothing to do
        else if( head == NULL )
            head = list.head;           // this is empty, just move it
        else
        {
            // We have a real merge to do. Adjust the pointers
            // on the two lists so that they become one.

            Cyg_DNode *lh = list.head;
            Cyg_DNode *lt = lh->prev;
            Cyg_DNode *tail = head->prev;

            head->prev = lt;
            lt->next = head;
            tail->next = lh;
            lh->prev = tail;
        }
        list.head = NULL;
    };
    
    // General removal. Deals with what happend if this is only
    // object on list, or is the head.
    void remove( Cyg_DNode *node )
    {
        if( node == head )
            rem_head();
        else node->unlink();
    };

    // Rotation - move the head to the next node in the list.
    void rotate()
    {
        if( head )
            head = head->next;
    };

    // Move a node to the head of the list. Assumes that the
    // node is in this list.
    void to_head( Cyg_DNode *node )
    {
        head = node;
    };

    // Insert a node before one in this list, and deal with what
    // happens if the node happens to be at the head of the list.
    void insert( Cyg_DNode *list_node, Cyg_DNode *node )
    {
        if( list_node == head )
        {
            head->insert( node );
            head = node;
        }
        else list_node->insert( node );
    };
    
    ~Cyg_CList()
    {
        while( head != NULL )
            rem_head();
    };

};

// -------------------------------------------------------------------------
// Cyg_CList_T
// Template class that allows us to make use of the CList class in a
// type-specific way.

template <class T> class Cyg_CList_T
    : public Cyg_CList
{
public:

    Cyg_CList_T() {};
    ~Cyg_CList_T() {};

    T *get_head()
    {
        Cyg_DNode *node = Cyg_CList::get_head();
        if( node ) return CYG_CLASSFROMBASE( T, Cyg_DNode, node );
        return NULL;
    };
    T *get_tail()
    {
        Cyg_DNode *node = Cyg_CList::get_tail();
        if( node ) return CYG_CLASSFROMBASE( T, Cyg_DNode, node );
        return NULL;
    };
    
    T *rem_head()
    {
        Cyg_DNode *node = Cyg_CList::rem_head();
        if( node ) return CYG_CLASSFROMBASE( T, Cyg_DNode, node );
        return NULL;
    };

    T *rem_tail()
    {
        Cyg_DNode *node = Cyg_CList::rem_tail();
        if( node ) return CYG_CLASSFROMBASE( T, Cyg_DNode, node );
        return NULL;
    };

    // The rest just default to the Cyg_CList class operations.
};

// -------------------------------------------------------------------------
// Cyg_DNode_T
// Template class that allows us to make use of the DNode class in a
// type-specific way.

template <class T> class Cyg_DNode_T
    : public Cyg_DNode
{
public:

    Cyg_DNode_T() {};
    ~Cyg_DNode_T() {};

    T *get_next() { return CYG_CLASSFROMBASE( T, Cyg_DNode, Cyg_DNode::get_next() ); };
    T *get_prev() { return CYG_CLASSFROMBASE( T, Cyg_DNode, Cyg_DNode::get_prev() ); };

    // The rest just default to the Cyg_DNode class operations.
};

// -------------------------------------------------------------------------
#endif // CYGONCE_INFRA_CLIST_HXX multiple inclusion protection
// EOF clist.hxx

