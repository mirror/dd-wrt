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
// platformeditordlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/platformeditordlg.h#3 $
// Purpose:
// Description: Header file for ecPlatformEditorDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_PLATFORMEDITORDLG_H_
#define _ECOS_PLATFORMEDITORDLG_H_

#ifdef __GNUG__
#pragma interface "platformeditordlg.cpp"
#endif

#include "ecutils.h"

class ecPlatformEditorDialog : public ecDialog
{
public:
// Ctor(s)
    ecPlatformEditorDialog(wxWindow* parent);

//// Event handlers

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
	void OnChangeNewPlatformPrefix(wxCommandEvent& event);
	void OnChangeNewPlatform(wxCommandEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);

    wxString    m_strCaption;
    wxString    m_strPlatform;
    wxString    m_strPrefix;
    wxString    m_strGDB;
    wxString    m_strInferior;
    wxString    m_strPrompt;
    bool        m_bServerSideGdb;
protected:

private:
    DECLARE_EVENT_TABLE()
};

#define ecID_MODIFY_PLATFORM_NAME           10031
#define ecID_MODIFY_PLATFORM_PREFIX         10032
#define ecID_MODIFY_PLATFORM_ARGS           10033
#define ecID_MODIFY_PLATFORM_INFERIOR       10034
#define ecID_MODIFY_PLATFORM_PROMPT         10035
#define ecID_MODIFY_PLATFORM_SS_GDB         10036

#endif
        // _ECOS_PLATFORMEDITORDLG_H_
