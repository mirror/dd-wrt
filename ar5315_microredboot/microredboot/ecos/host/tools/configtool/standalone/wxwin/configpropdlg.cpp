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
// platformsdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: configpropdlg.cpp,v 1.2 2001/03/01 15:54:38 julians Exp $
// Purpose:
// Description: Implementation file for the ecConfigPropertiesDialog
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
#pragma implementation "configpropdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"

#include "configtool.h"
#include "configpropdlg.h"
#include "propertywin.h"

BEGIN_EVENT_TABLE(ecConfigPropertiesDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ecConfigPropertiesDialog::OnClose)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecConfigPropertiesDialog::ecConfigPropertiesDialog(wxWindow* parent, ecConfigItem* item)
{
    m_listCtrl = NULL;
    m_item = item;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    wxDialog::Create(parent, ecID_CONFIG_PROPERTIES_DIALOG, _("Configuration Item Properties"));

    CreateControls(this);

    m_listCtrl->Fill(m_item);

    Centre(wxBOTH);
}

void ecConfigPropertiesDialog::CreateControls(wxWindow* parent)
{
    // Create custom windows first
    m_listCtrl = new ecPropertyListCtrl(parent, ecID_CONFIG_PROPERTIES_LIST, wxDefaultPosition, wxSize(450, 300), wxLC_REPORT|wxCLIP_CHILDREN|wxSUNKEN_BORDER|wxLC_VRULES|wxLC_HRULES);
    //m_listCtrl->InsertColumn(0, _("Property"), wxLIST_FORMAT_LEFT, 100);
    //m_listCtrl->InsertColumn(1, _("Value"), wxLIST_FORMAT_LEFT, 100);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxWindow *item1 = parent->FindWindow( ecID_CONFIG_PROPERTIES_LIST );
    wxASSERT( item1 );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item2 = new wxBoxSizer( wxHORIZONTAL );

    item2->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item3 = new wxButton( parent, wxID_OK, "&Close", wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item2, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item2->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    ((wxButton*) parent->FindWindow(wxID_OK))->SetDefault();

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    // Add context-sensitive help text
    parent->FindWindow( ecID_CONFIG_PROPERTIES_LIST )->SetHelpText(_("Displays the properties of the currently selected configuration item. You can double-click on a URL property to navigate to that documentation page or on a File property to view that header file."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

}

void ecConfigPropertiesDialog::OnClose(wxCommandEvent& event)
{
    event.Skip();
}
