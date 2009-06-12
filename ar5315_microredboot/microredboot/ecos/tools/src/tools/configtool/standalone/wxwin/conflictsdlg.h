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
// conflictsdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/conflictsdlg.h#3 $
// Purpose:
// Description: Header file for ecResolveConflictsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFLICTSDLG_H_
#define _ECOS_CONFLICTSDLG_H_

#ifdef __GNUG__
#pragma interface "conflictsdlg.h"
#endif

#include "wx/wx.h"
#include "wx/listctrl.h"

#include "ecutils.h"

// Forward declarations
class ecConflictListCtrl;
class ecSolutionListCtrl;

class ecResolveConflictsDialog : public ecDialog
{
public:
// Ctor(s)
	ecResolveConflictsDialog(wxWindow* parent, std::list<CdlConflict> conflicts, CdlTransaction=NULL, wxList *parConflictsOfInterest=NULL);
    ~ecResolveConflictsDialog();

//// Event handlers

    void OnContinue(wxCommandEvent& event);
    void OnAll(wxCommandEvent& event);
    void OnNone(wxCommandEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);
    void OnUpdateAll(wxUpdateUIEvent& event);
    void OnUpdateNone(wxUpdateUIEvent& event);
    void OnConflictSelected(wxListEvent& event) ;
    void OnConflictDeselected(wxListEvent& event) ;

//// Operations
    void CreateControls(wxWindow* parent);

protected:
    struct SolutionInfo {
        int nCount;
        enum {CHECKED=-1,UNCHECKED=-2};
        int arItem[1]; // real size==nCount.
        // Each element of nItem==item index (if selected) or accept bool (if not)
    };

    void SetButtons();
    void AddConflictSolutions (CdlConflict conflict);
    void RemoveConflictSolutions (CdlConflict conflict);
    void OnLocate();
    SolutionInfo & Info (const CdlConflict conflict);
    void SetAll (bool bOnOff);

//// Member variables
protected:
    //UINT m_idTimer;
    wxHashTable             m_Map; // maps conflicts to bool array representing fixes
    wxArrayString           m_arValues;
    const std::list<CdlConflict> m_conflicts;
    CdlTransactionBody*     m_Transaction;
    wxList*                 m_parConflictsOfInterest;
    int                     m_nContextItem;
    int                     m_nContextRow;
    

    ecConflictListCtrl* m_conflictsCtrl;
    ecSolutionListCtrl* m_solutionsCtrl;

private:
    DECLARE_EVENT_TABLE()
};

#define ecID_RESOLVE_CONFLICTS_DIALOG   2031
#define ecID_CONFLICTS_CONTINUE     3000
#define ecID_CONFLICTS_CONFLICTS    3005
#define ecID_CONFLICTS_NONE         3001
#define ecID_CONFLICTS_ALL          3002
#define ecID_CONFLICTS_SOLUTIONS    3004
#define ecID_CONFLICTS_MSG          3005


#endif
        // _ECOS_CONFLICTSDLG_H_
