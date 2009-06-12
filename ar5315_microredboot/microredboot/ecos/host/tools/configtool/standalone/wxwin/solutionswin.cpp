//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//
//####COPYRIGHTEND####
// configtree.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/08
// Version:     $Id: solutionswin.cpp,v 1.1 2001/02/08 18:06:52 julians Exp $
// Purpose:
// Description: Implementation file for ecSolutionListCtrl
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
    #pragma implementation "solutionswin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "solutionswin.h"

#include "bitmaps/checked.xpm"
#include "bitmaps/unchecked.xpm"

/*
 * ecSolutionListCtrl
 */

IMPLEMENT_CLASS(ecSolutionListCtrl, wxListCtrl)

BEGIN_EVENT_TABLE(ecSolutionListCtrl, wxListCtrl)
    EVT_LEFT_DOWN(ecSolutionListCtrl::OnMouseEvent)
END_EVENT_TABLE()

ecSolutionListCtrl::ecSolutionListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxListCtrl(parent, id, pt, sz, style), m_imageList(16, 16, TRUE)

{
    SetImageList(& m_imageList, wxIMAGE_LIST_SMALL);

    m_imageList.Add(wxICON(unchecked));
    m_imageList.Add(wxICON(checked));

    InsertColumn(0, _("Item"), wxLIST_FORMAT_LEFT, 200);
    InsertColumn(1, _("Value"), wxLIST_FORMAT_LEFT, 80);

#if 0
    int i = 0;
    int j;

    for (j = 0; j < 10; j++)
    {
        wxListItem info;
        info.m_text = _("CYGPKG_HAL_EXCEPTIONS");
        info.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE ; // | wxLIST_MASK_DATA;
        info.m_itemId = i;
        info.m_image = 0;
        //info.m_data = (long) doc;
        
        long item = InsertItem(info);
        
        SetItem(i, 1, _("Disabled"));   
        i ++;
        
        info.m_text = _("CYGPKG_KERNEL_EXCEPTIONS");
        info.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE ; // | wxLIST_MASK_DATA;
        info.m_itemId = i;
        info.m_image = 1;
        //info.m_data = (long) doc;
        item = InsertItem(info);
        
        SetItem(item, 1, _("Disabled"));
        i ++;
    }
#endif
}

void ecSolutionListCtrl::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown())
    {
        int flags;
        long item = HitTest(event.GetPosition(), flags);
        if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON))
        {
            SetChecked(item, !IsChecked(item));
        }
        else
            event.Skip();
    }
    else
    {
        event.Skip();
    }
}

bool ecSolutionListCtrl::IsChecked(long item) const
{
    wxListItem info;
    info.m_mask = wxLIST_MASK_IMAGE ;
    info.m_itemId = item;

    if (GetItem(info))
    {
        return (info.m_image == 1);
    }
    else
        return FALSE;
}

void ecSolutionListCtrl::SetChecked(long item, bool checked)
{
    SetItemImage(item, (checked ? 1 : 0), -1);
}
