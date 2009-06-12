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
// platformsdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/platformsdlg.h#3 $
// Purpose:
// Description: Header file for ecPlatformsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_PLATFORMSDLG_H_
#define _ECOS_PLATFORMSDLG_H_

#ifdef __GNUG__
#pragma interface "platformsdlg.cpp"
#endif

#include "wx/wx.h"
#include "wx/listctrl.h"

#include "eCosTest.h"
#include "ecutils.h"

/*
 * ecPlatformsListCtrl
 *
 * A list control for the platforms dialog
 */

class ecPlatformsListCtrl : public wxListCtrl
{
public:
// Ctor(s)
    ecPlatformsListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);

//// Event handlers
    void OnChar(wxKeyEvent& event);
    void OnDoubleLClick(wxMouseEvent& event);

//// Operations

protected:
private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecPlatformsListCtrl)
};

/*
 * ecPlatformsDialog
 *
 */

class ecPlatformsDialog : public ecDialog
{
public:
// Ctor(s)
    ecPlatformsDialog(wxWindow* parent);
    ~ecPlatformsDialog();

//// Event handlers

    void OnModify(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnUpdateAny(wxUpdateUIEvent& event);
    void OnDoubleLClick();
    void OnDeleteKey();

//// Operations
    void CreateControls(wxWindow* parent);

    CeCosTestPlatform *Platform (int i) { return (CeCosTestPlatform *) m_arTargetInfo[i]; }
    unsigned int PlatformCount() const { return m_arTargetInfo.Number(); }
    void Add(const CeCosTestPlatform &ti);   
    void Clear();

protected:

    ecPlatformsListCtrl*    m_listCtrl;
    wxList                  m_arTargetInfo;
    static const wxChar*    sm_arpszTypes[];

private:
    DECLARE_EVENT_TABLE()
};

#define ecID_PLATFORMS_ADD          4001
#define ecID_PLATFORMS_DELETE       4002
#define ecID_PLATFORMS_MODIFY       4003
#define ecID_PLATFORMS_LIST         4004


#endif
        // _ECOS_PLATFORMSDLG_H_
