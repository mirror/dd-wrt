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
// folderdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/12/20
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/folderdlg.h#3 $
// Purpose:
// Description: Header for ecFolderDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_FOLDERDLG_H_
#define _ECOS_FOLDERDLG_H_

#ifdef __GNUG__
    #pragma interface "folderdlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "ecutils.h"

//----------------------------------------------------------------------------
// ecFolderDialog
//----------------------------------------------------------------------------

class ecFolderDialog: public ecDialog
{
public:
    // constructors and destructors
    ecFolderDialog( const wxString& defaultPath, const wxArrayString& paths,
        const wxString& msg, wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

/// Operations    
    void CreateControls();

//// Event handlers
    void OnOK( wxCommandEvent &event );
    void OnCancel( wxCommandEvent& event );
    void OnBrowse( wxCommandEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);

//// Accessors
    wxString GetPath() const { return m_defaultPath; }

private:
	wxString        m_defaultPath;
	wxArrayString   m_paths;
    wxString        m_message;
    
    DECLARE_EVENT_TABLE()
};


#endif
    // _ECOS_FOLDERDLG_H_
