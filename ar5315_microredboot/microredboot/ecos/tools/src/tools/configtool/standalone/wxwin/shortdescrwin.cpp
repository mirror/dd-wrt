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
// shortdescrwin.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/02
// Version:     $Id: shortdescrwin.cpp,v 1.4 2001/05/16 16:08:24 julians Exp $
// Purpose:
// Description: Implementation file for ecShortDescriptionWindow
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
    #pragma implementation "shortdescrwin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "configtool.h"
#include "shortdescrwin.h"

/*
 * ecShortDescriptionWindow
 */

IMPLEMENT_CLASS(ecShortDescriptionWindow, wxTextCtrl)

BEGIN_EVENT_TABLE(ecShortDescriptionWindow, wxTextCtrl)
    EVT_MOUSE_EVENTS(ecShortDescriptionWindow::OnMouseEvent)
END_EVENT_TABLE()

ecShortDescriptionWindow::ecShortDescriptionWindow(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxTextCtrl(parent, id, wxEmptyString, pt, sz, style)
{
/*
#ifdef __WXGTK__
    SetThemeEnabled(FALSE);
#endif
*/
    
    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Short Description")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Short Description")));
    }

    m_propertiesMenu = new wxMenu;

    m_propertiesMenu->Append(ecID_WHATS_THIS, _("&What's This?"));
    m_propertiesMenu->AppendSeparator();
    m_propertiesMenu->Append(wxID_COPY, _("&Copy"));

    //SetValue(_("This is a short description, to give you helpful remarks about the selected item."));
}

ecShortDescriptionWindow::~ecShortDescriptionWindow()
{
    delete m_propertiesMenu;
}

void ecShortDescriptionWindow::OnMouseEvent(wxMouseEvent& event)
{
    if (event.RightDown())
    {
        PopupMenu(GetPropertiesMenu(), event.GetX(), event.GetY());
    }
    else
    {
        event.Skip();
    }
}

