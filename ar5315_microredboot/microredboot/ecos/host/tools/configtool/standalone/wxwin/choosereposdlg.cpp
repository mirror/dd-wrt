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
// choosereposdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/02
// Version:     $Id: choosereposdlg.cpp,v 1.3 2002/02/28 18:30:35 julians Exp $
// Purpose:
// Description: Implementation file for ecChooseRepositoryDialog
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
#pragma implementation "choosereposdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/dirdlg.h"
#include "wx/valgen.h"

#include "configtool.h"
#include "configtooldoc.h"
#include "choosereposdlg.h"

BEGIN_EVENT_TABLE(ecChooseRepositoryDialog, ecDialog)
    EVT_BUTTON(ecID_CHOOSE_REPOSITORY_BROWSE, ecChooseRepositoryDialog::OnBrowse)
    EVT_BUTTON(wxID_OK, ecChooseRepositoryDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecChooseRepositoryDialog::OnCancel)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecChooseRepositoryDialog::ecChooseRepositoryDialog(wxWindow* parent)
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_CHOOSE_REPOSITORY_DIALOG, _("Choose folder for eCos repository"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    Centre(wxBOTH);
}

void ecChooseRepositoryDialog::CreateControls(wxWindow* parent)
{
    wxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item1 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("Please specify the root of the eCos repository tree."), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item1->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxTextCtrl *item3 = new wxTextCtrl( parent, ecID_CHOOSE_REPOSITORY_TEXT, _(""), wxDefaultPosition, wxSize(320,-1), 0 );
    item1->Add( item3, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    item0->Add( item1, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxSizer *item4 = new wxBoxSizer( wxVERTICAL );

    wxButton *item5 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item7 = new wxButton( parent, ecID_CHOOSE_REPOSITORY_BROWSE, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

#if 0
    wxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item1 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, "Please specify the root of the eCos repository tree.", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item1->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxTextCtrl *item3 = new wxTextCtrl( parent, ecID_CHOOSE_REPOSITORY_TEXT, "", wxDefaultPosition, wxSize(320,-1), 0 );
    item1->Add( item3, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxSizer *item4 = new wxBoxSizer( wxVERTICAL );

    wxButton *item5 = new wxButton( parent, wxID_OK, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );
    item5->SetDefault();

    wxButton *item6 = new wxButton( parent, wxID_CANCEL, "&Cancel", wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item7 = new wxButton( parent, ecID_CHOOSE_REPOSITORY_BROWSE, "&Browse...", wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item4->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item4->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    parent->FindWindow( ecID_CHOOSE_REPOSITORY_TEXT)->SetFocus();

    // Add context-sensitive help text
    parent->FindWindow( ecID_CHOOSE_REPOSITORY_TEXT)->SetHelpText(_("Selects the repository folder."));
    parent->FindWindow( ecID_CHOOSE_REPOSITORY_BROWSE)->SetHelpText(_("Browse for the folder."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and loads the selected repository."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without loading a repository."));

    // Add validators
    parent->FindWindow( ecID_CHOOSE_REPOSITORY_TEXT)->SetValidator(wxGenericValidator(& wxGetApp().GetConfigToolDoc()->m_strRepository));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif
}

void ecChooseRepositoryDialog::OnBrowse(wxCommandEvent& event)
{
    wxDirDialog dialog(this, wxT("Repository path"));
    if (wxID_OK == dialog.ShowModal())
    {
        wxString path = dialog.GetPath();
        ((wxTextCtrl*) FindWindow( ecID_CHOOSE_REPOSITORY_TEXT))->SetValue(path);
    }
}

void ecChooseRepositoryDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecChooseRepositoryDialog::OnOK(wxCommandEvent& event)
{
    wxString folder(GetFolder());
    if (!wxDirExists(folder))
    {
        wxMessageBox(_("This is not a valid folder."));
        return; // Don't Skip so we don't dismiss the dialog
    }
    if (FALSE)
    {
        wxMessageBox(_("This does not like a valid eCos repository."));
        return; // Don't Skip so we don't dismiss the dialog
    }
    event.Skip();
}

wxString ecChooseRepositoryDialog::GetFolder()
{
    return ((wxTextCtrl*) FindWindow( ecID_CHOOSE_REPOSITORY_TEXT))->GetValue();
}
