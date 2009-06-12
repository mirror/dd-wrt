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
// folderdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/12/20
// Version:     $Id: folderdlg.cpp,v 1.2 2001/03/23 13:38:04 julians Exp $
// Purpose:
// Description: Implementation of ecFolderDialog
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
    #pragma implementation "folderdlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "configtool.h"
#include "folderdlg.h"

//----------------------------------------------------------------------------
// ecFolderDialog
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ecFolderDialog, ecDialog)
    EVT_BUTTON( wxID_OK, ecFolderDialog::OnOK )
    EVT_BUTTON( wxID_CANCEL, ecFolderDialog::OnCancel )
    EVT_BUTTON( ecID_FOLDER_DIALOG_BROWSE, ecFolderDialog::OnBrowse )
    EVT_INIT_DIALOG(ecFolderDialog::OnInitDialog)
END_EVENT_TABLE()

ecFolderDialog::ecFolderDialog( const wxString& defaultPath, const wxArrayString& paths,
    const wxString& msg, wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style )
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    m_defaultPath = defaultPath;
    m_paths = paths;
    m_message = msg;

    wxDialog::Create( parent, id, title, position, size, style );

    CreateControls();
    
    Centre(wxBOTH);
}

void ecFolderDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxComboBox* comboBox = (wxComboBox*) FindWindow(ecID_FOLDER_DIALOG_PATHS);

    wxASSERT (comboBox != NULL) ;

    size_t i;
    for (i = (size_t) 0; i < m_paths.GetCount(); i++)
    {
        comboBox->Append(m_paths[i]);
        if (m_paths[i] == m_defaultPath)
            comboBox->SetSelection(i);
    }
    if (comboBox->FindString(m_defaultPath) == -1)
        comboBox->Append(m_defaultPath);

    if (comboBox->GetSelection() == -1 && comboBox->Number() > 0)
        comboBox->SetSelection(0);

    comboBox->SetFocus();

    wxStaticText* staticText = (wxStaticText*) FindWindow(ecID_FOLDER_DIALOG_MSG);

    wxASSERT ( staticText != NULL );

    staticText->SetLabel(m_message);
}

void ecFolderDialog::OnOK(wxCommandEvent &event)
{
    wxComboBox* comboBox = (wxComboBox*) FindWindow(ecID_FOLDER_DIALOG_PATHS);
    m_defaultPath = comboBox->GetValue();

    event.Skip();
}

void ecFolderDialog::OnCancel(wxCommandEvent &event)
{
    event.Skip();
}

void ecFolderDialog::OnBrowse(wxCommandEvent &event)
{
    wxComboBox* comboBox = (wxComboBox*) FindWindow(ecID_FOLDER_DIALOG_PATHS);

    wxString value = comboBox->GetValue();

    wxDirDialog dirDialog(this, wxT("Choose a directory"), value);
    if (dirDialog.ShowModal() == wxID_OK)
    {
        comboBox->SetValue(dirDialog.GetPath());
    }
}

void ecFolderDialog::CreateControls()
{
    wxWindow* parent = this;

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item2 = new wxStaticText( parent, ecID_FOLDER_DIALOG_MSG, _("text"), wxDefaultPosition, wxSize(-1,70), wxST_NO_AUTORESIZE );
    item1->Add( item2, 10, wxALIGN_CENTER_HORIZONTAL|wxALL, 10 );

    wxSizer *item3 = new wxBoxSizer( wxVERTICAL );

    wxButton *item4 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->SetDefault();
    item3->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item5 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item3->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, ecID_FOLDER_DIALOG_BROWSE, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item3->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString *strs7 = (wxString*) NULL;
    wxComboBox *item7 = new wxComboBox( parent, ecID_FOLDER_DIALOG_PATHS, "", wxDefaultPosition, wxSize(470,-1), 0, strs7, wxCB_DROPDOWN );
    item0->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    // Add validators
    //parent->FindWindow( ecID_LICENSE_TEXT )->SetValidator(wxGenericValidator(& m_licenseText));

    // Add context-sensitive help text
    parent->FindWindow( wxID_OK )->SetHelpText(_("Confirms your folder selection."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Cancels the operation."));

#if __WXGTK__
    //parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif
}
