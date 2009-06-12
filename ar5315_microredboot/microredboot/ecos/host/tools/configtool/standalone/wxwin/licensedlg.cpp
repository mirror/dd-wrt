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
// appsettings.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/12/18
// Version:     $Id: licensedlg.cpp,v 1.2 2001/03/23 13:38:04 julians Exp $
// Purpose:
// Description: Implementation file for ecLicenseDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifdef __GNUG__
    #pragma implementation "licensedlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/valgen.h"

#include "licensedlg.h"
#include "configtool.h"

//----------------------------------------------------------------------------
// ecLicenseDialog
//----------------------------------------------------------------------------

// WDR: event table for ecLicenseDialog

BEGIN_EVENT_TABLE(ecLicenseDialog, ecDialog)
    EVT_BUTTON( wxID_OK, ecLicenseDialog::OnOK )
    EVT_BUTTON( wxID_CANCEL, ecLicenseDialog::OnCancel )
END_EVENT_TABLE()

ecLicenseDialog::ecLicenseDialog( const wxString& licenseText, wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style )
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
    m_licenseText = licenseText;

    wxDialog::Create( parent, id, title, position, size, style );

    CreateControls();
    
    Centre(wxBOTH);
}

// WDR: handler implementations for ecLicenseDialog

void ecLicenseDialog::OnOK(wxCommandEvent &event)
{
    event.Skip();
}

void ecLicenseDialog::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}

void ecLicenseDialog::CreateControls()
{
    wxWindow* parent = this;

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item1 = new wxTextCtrl( parent, ecID_LICENSE_TEXT, _(""), wxDefaultPosition, wxSize(590,260), wxTE_MULTILINE|wxTE_READONLY );
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, 
        _("Do you accept all the terms of the preceding license agreement?"),
        wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item2, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item3 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item4 = new wxButton( parent, wxID_OK, _("&Yes"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->SetDefault();
    item3->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item5 = new wxButton( parent, wxID_CANCEL, _("&No"), wxDefaultPosition, wxDefaultSize, 0 );
    item3->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item3, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    // Add validators
    parent->FindWindow( ecID_LICENSE_TEXT )->SetValidator(wxGenericValidator(& m_licenseText));

    // Add context-sensitive help text
    parent->FindWindow( ecID_LICENSE_TEXT )->SetHelpText(_("Displays the license for this package."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Confirms that you accept the license."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Confirms that you do NOT accept the license."));

#if __WXGTK__
    // parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

}


