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
// admindlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/28
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/admindlg.h#3 $
// Purpose:
// Description: Header file for ecAdminDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_ADMINDLG_H_
#define _ECOS_ADMINDLG_H_

#ifdef __GNUG__
#pragma interface "admindlg.cpp"
#endif

#include "ecutils.h"

class ecAdminDialog : public ecDialog
{
public:
// Ctor(s)
    ecAdminDialog(wxWindow* parent, const wxString& repository = wxEmptyString, const wxString& userTools = wxEmptyString);
    ~ecAdminDialog();

//// Event handlers

    void OnClose(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);
	bool FindUserToolsPath();
	bool RemovePackageVersion (wxTreeItemId hTreeItem);
	bool EvalTclFile (int nargc, const wxString& argv, const wxString& msg);
	void ClearPackageTree ();
	bool PopulatePackageTree (const wxString& packageDatabase);

protected:
    wxTreeCtrl* m_treeCtrl;
    wxImageList m_imageList;

private:
    DECLARE_EVENT_TABLE()

	wxString m_strRepository;
	wxString m_strUserTools;
	CdlPackagesDatabase m_CdlPkgData;
};

// Data to associate with each item
class ecAdminItemData : public wxTreeItemData
{
public:
    ecAdminItemData(const wxString& str) : m_string(str) { }

    wxString m_string;
};


#define ecID_ADMIN_DIALOG_TREE 10400
#define ecID_ADMIN_DIALOG_ADD 10064
#define ecID_ADMIN_DIALOG_REMOVE 10065

#endif
        // _ECOS_ADMINDLG_H_
