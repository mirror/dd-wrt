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
// outputwin.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/2
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/outputwin.h#3 $
// Purpose:
// Description: Header file for ecOutputWindow
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_OUTPUTWIN_H_
#define _ECOS_OUTPUTWIN_H_

#ifdef __GNUG__
#pragma interface "outputwin.h"
#endif

#include "wx/textctrl.h"

class ecOutputWindow : public wxTextCtrl
{
public:
// Ctor(s)
    ecOutputWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
    ~ecOutputWindow();

//// Event handlers

    void OnMouseEvent(wxMouseEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnSelectAll(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);

//// Operations

//// Accessors
    wxMenu* GetPropertiesMenu() const { return m_propertiesMenu; }

protected:
    wxMenu*     m_propertiesMenu;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecOutputWindow)
};


#endif
        // _ECOS_OUTPUTWIN_H_
