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
// Date:        2000/09/04
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/conflictwin.h#3 $
// Purpose:
// Description: Header file for ecConflictListCtrl
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFLICTWIN_H_
#define _ECOS_CONFLICTWIN_H_

#ifdef __GNUG__
#pragma interface "conflictwin.h"
#endif

#include "wx/listctrl.h"

class ecConfigItem;

/*
 * ecConflictListCtrl
 *
 * Displays conflicts. Equivalent to original configtool's CRulesList class.
 */

class ecConflictListCtrl : public wxListCtrl
{
public:
// Ctor(s)
    ecConflictListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
    ~ecConflictListCtrl();
    bool IsSelected (int nIndex);

//// Event handlers
    void OnRightClick(wxMouseEvent& event);
    void OnLocate(wxCommandEvent& event);
    void OnResolve(wxCommandEvent& event);
    void OnLeftDClick(wxMouseEvent& event);

//// Operations
    void AddConflict (const CdlConflict& conflict);
    void AddConflicts (const std::list<CdlConflict>& conflicts);
    void AddConflicts (const std::vector<CdlConflict>& conflicts);
    ecConfigItem *AssociatedItem (int nRow,int nCol);
    void FillRules();

    wxMenu* GetContextMenu() const { return m_contextMenu; }

protected:
private:
    wxMenu* m_contextMenu;
    long    m_contextItem; // Item right-clicked over
    long    m_contextCol;  // Column clicked over

    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecConflictListCtrl)
};


#endif
        // _ECOS_CONFLICTWIN_H_
