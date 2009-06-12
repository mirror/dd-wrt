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
// buildoptionsdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/buildoptionsdlg.h#3 $
// Purpose:
// Description: Header file for ecBuildOptionsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_BUILDOPTIONSDLG_H_
#define _ECOS_BUILDOPTIONSDLG_H_

#ifdef __GNUG__
#pragma interface "buildoptionsdlg.cpp"
#endif

#include "wx/treectrl.h"
#include "ecutils.h"

class ecBuildOptionsDialog : public ecDialog
{
public:
// Ctor(s)
    ecBuildOptionsDialog(wxWindow* parent);
    ~ecBuildOptionsDialog();

//// Event handlers

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnSelCategory(wxCommandEvent& event);
    void OnSelTree(wxTreeEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);
	void Redisplay (wxTreeItemId item);
    void CreateItems(ecConfigItem *pti, wxTreeItemId hParent);

protected:
    wxTreeCtrl*     m_treeCtrl;
    wxImageList     m_imageList;

    typedef std::vector<CdlBuildInfo_Loadable> EntriesArray;
    const EntriesArray& m_arEntries;
private:
    DECLARE_EVENT_TABLE()
};

class ecBuildOptionsData : public wxTreeItemData
{
public:
    ecBuildOptionsData(ecConfigItem* item) : m_configItem(item) { }

    ecConfigItem *GetConfigItem() const { return m_configItem; }
    void SetConfigItem(ecConfigItem *item) { m_configItem = item; }

private:
    ecConfigItem*   m_configItem;
};


#define ecID_BUILD_OPTIONS_CATEGORY 10054
#define ecID_BUILD_OPTIONS_PACKAGES_TREE 11000
#define ecID_BUILD_OPTIONS_FLAGS 10055

#endif
        // _ECOS_BUILDOPTIONSDLG_H_
