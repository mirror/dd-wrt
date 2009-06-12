/////////////////////////////////////////////////////////////////////////////
// Name:        msgdlgex.h
// Purpose:     wxMessageDialogEx
// Author:      Julian Smart
// Modified by:
// Created:     12/12/2000
// RCS-ID:      $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/msgdlgex.h#3 $
// Copyright:   (c) Julian Smart
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
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSGDLGEX_H_
#define _WX_MSGDLGEX_H_

#ifdef __GNUG__
    #pragma interface "msgdlgex.cpp"
#endif

#include "wx/dialog.h"

// Styles
#define wxMD_OK                 0x00000004
#define wxMD_YES_NO             0x00000008
#define wxMD_CANCEL             0x00000010
#define wxMD_YES                0x00000020
#define wxMD_NO                 0x00000040
#define wxMD_HELP               0x00008000

#define wxMD_YESTOALL           0x00010000
#define wxMD_NOTOALL            0x00020000
#define wxMD_ABORT              0x00040000
#define wxMD_RETRY              0x00080000
#define wxMD_IGNORE             0x00100000

// Use same numbers as wxICON_... equivalents
#define wxMD_ICON_EXCLAMATION   0x00000100
#define wxMD_ICON_HAND          0x00000200
#define wxMD_ICON_WARNING       wxICON_EXCLAMATION
#define wxMD_ICON_ERROR         wxICON_HAND
#define wxMD_ICON_QUESTION      0x00000400
#define wxMD_ICON_INFORMATION   0x00000800
#define wxMD_ICON_STOP          wxICON_HAND
#define wxMD_ICON_ASTERISK      wxICON_INFORMATION
#define wxMD_ICON_MASK          (0x00000100|0x00000200|0x00000400|0x00000800)

#define wxMD_NO_DEFAULT         0x00000080
#define wxMD_YES_DEFAULT        0x00200000
// OK and YES are equivalent
#define wxMD_OK_DEFAULT         0x00200000
#define wxMD_YESTOALL_DEFAULT   0x00400000
#define wxMD_NOTOALL_DEFAULT    0x00800000
#define wxMD_ABORT_DEFAULT      0x01000000
#define wxMD_RETRY_DEFAULT      0x02000000
#define wxMD_IGNORE_DEFAULT     0x04000000

#ifndef wxID_YESTOALL
#define wxID_YESTOALL           5113
#define wxID_NOTOALL            5114
#define wxID_ABORT              5115
#define wxID_RETRY              5116
#define wxID_IGNORE             5117
#endif

//----------------------------------------------------------------------------
// wxMessageDialogEx
//----------------------------------------------------------------------------

class wxMessageDialogEx: public wxDialog
{
public:
    // constructors and destructors
    wxMessageDialogEx();
    wxMessageDialogEx( wxWindow *parent, const wxString& message, const wxString& caption,
        long style = wxOK|wxCANCEL,
        const wxPoint& pos = wxDefaultPosition
    );
    virtual ~wxMessageDialogEx();
    
protected:

    //void CreateControls(wxWindow* parent, long style);
    wxSizer *CreateButtonSizer( long flags );
    void OnCommand( wxCommandEvent &event );

    long    m_dialogStyle;

private:
    DECLARE_CLASS(wxMessageDialogEx)
    DECLARE_EVENT_TABLE()
};

#endif
    // _WX_MSGDLGEX_H_
