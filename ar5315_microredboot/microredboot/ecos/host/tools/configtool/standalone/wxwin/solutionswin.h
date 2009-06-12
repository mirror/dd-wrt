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
// propertywin.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/08
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/solutionswin.h#3 $
// Purpose:
// Description: Header file for ecSolutionListCtrl
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_SOLUTIONSWIN_H_
#define _ECOS_SOLUTIONSWIN_H_

#ifdef __GNUG__
#pragma interface "solutionswin.h"
#endif

#include "wx/listctrl.h"

class ecSolutionListCtrl : public wxListCtrl
{
public:
// Ctor(s)
    ecSolutionListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);

//// Event handlers
    void OnMouseEvent(wxMouseEvent& event);

//// Operations

//// Accessors
    bool IsChecked(long item) const;
    void SetChecked(long item, bool checked) ;

protected:
    wxImageList m_imageList;
private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecSolutionListCtrl)
};


#endif
        // _ECOS_SOLUTIONSWIN_H_
