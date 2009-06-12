//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
// Author(s):   julians, jld
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: platformeditordlg.cpp,v 1.4 2001/07/05 10:42:16 julians Exp $
// Purpose:
// Description: Implementation file for the ecPlatformEditorDialog
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
#pragma implementation "platformeditordlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/valgen.h"

#include "eCosTestPlatform.h"
#include "configtool.h"
#include "platformeditordlg.h"

#ifdef __WXMSW__
#include "wx/msw/winundef.h"
#endif

//#ifdef __WXMSW__
//static const wxChar* g_NewLineToReplace = wxT("\r\n");
//#else
static const wxChar* g_NewLineToReplace = wxT("\n");
//#endif

BEGIN_EVENT_TABLE(ecPlatformEditorDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecPlatformEditorDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecPlatformEditorDialog::OnCancel)
    EVT_COMBOBOX(ecID_MODIFY_PLATFORM_PREFIX, ecPlatformEditorDialog::OnChangeNewPlatformPrefix)
    EVT_TEXT(ecID_MODIFY_PLATFORM_NAME, ecPlatformEditorDialog::OnChangeNewPlatform)
    EVT_INIT_DIALOG(ecPlatformEditorDialog::OnInitDialog)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecPlatformEditorDialog::ecPlatformEditorDialog(wxWindow* parent)
{
	m_bServerSideGdb = FALSE;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_PLATFORM_EDITOR_DIALOG, _("Platforms"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    Centre(wxBOTH);
}

void ecPlatformEditorDialog::OnInitDialog(wxInitDialogEvent& event)
{
    m_strGDB.Replace(wxT(";"), g_NewLineToReplace);
    
    wxComboBox* comboBox = (wxComboBox*) FindWindow(ecID_MODIFY_PLATFORM_PREFIX);

    unsigned int i;
    for ( i=0 ; i < CeCosTestPlatform::Count() ; i++ )
    {
        if (wxNOT_FOUND == comboBox->FindString(CeCosTestPlatform::Get(i)->Prefix()))
            comboBox->Append(CeCosTestPlatform::Get(i)->Prefix());
    }

    wxDialog::OnInitDialog(event);
    
    SetTitle(m_strCaption);
    if( ! m_strPlatform.IsEmpty() )
    {
        ((wxTextCtrl*)FindWindow(ecID_MODIFY_PLATFORM_NAME))->SetValue(m_strPlatform);

        FindWindow(ecID_MODIFY_PLATFORM_NAME)->Enable(FALSE);
    }

    FindWindow(wxID_OK)->Enable(!m_strPlatform.IsEmpty() && !m_strPrefix.IsEmpty());
}

void ecPlatformEditorDialog::CreateControls(wxWindow* parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxFlexGridSizer( 2, 0, 0 );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("Platform &name:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxTextCtrl *item3 = new wxTextCtrl( parent, ecID_MODIFY_PLATFORM_NAME, _(""), wxDefaultPosition, wxSize(240,-1), 0 );
    item1->Add( item3, 0, wxGROW|wxALL, 5 );

    wxStaticText *item4 = new wxStaticText( parent, wxID_STATIC, _("Command &prefix:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString *strs5 = (wxString*) NULL;
    wxComboBox *item5 = new wxComboBox( parent, ecID_MODIFY_PLATFORM_PREFIX, "", wxDefaultPosition, wxDefaultSize, 0, strs5, wxCB_DROPDOWN );
    item1->Add( item5, 0, wxGROW|wxALL, 5 );

    wxStaticText *item6 = new wxStaticText( parent, wxID_STATIC, _("Arguments for &GDB:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item6, 0, wxALL, 5 );

    wxTextCtrl *item7 = new wxTextCtrl( parent, ecID_MODIFY_PLATFORM_ARGS, _(""), wxDefaultPosition, wxSize(-1,140), wxTE_MULTILINE );
    item1->Add( item7, 0, wxGROW|wxALL, 5 );

    wxStaticText *item8 = new wxStaticText( parent, wxID_STATIC, _("Inferior:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxTextCtrl *item9 = new wxTextCtrl( parent, ecID_MODIFY_PLATFORM_INFERIOR, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item9, 0, wxGROW|wxALL, 5 );

    wxStaticText *item10 = new wxStaticText( parent, wxID_STATIC, _("Prompt:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxTextCtrl *item11 = new wxTextCtrl( parent, ecID_MODIFY_PLATFORM_PROMPT, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item11, 0, wxGROW|wxALL, 5 );

#if USE_SS_GDB_CONTROL
    item1->Add( 20, 20, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxCheckBox *item12 = new wxCheckBox( parent, ecID_MODIFY_PLATFORM_SS_GDB, _("Server-side GDB"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item12, 0, wxGROW|wxALL, 5 );
#endif
    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item13 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item14 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item14->SetDefault();
    item13->Add( item14, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item15 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item13->Add( item15, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item13->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    // Can't do this from wxDesigner :-(
    ((wxFlexGridSizer*)item1)->AddGrowableCol(1);
    ((wxFlexGridSizer*)item1)->AddGrowableRow(2);

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    item3->SetFocus();
    item14->SetDefault();

    // Add context-sensitive help text
    parent->FindWindow( ecID_MODIFY_PLATFORM_NAME )->SetHelpText(_("Specifies the name of the platform. Platform names are arbitrary strings, but must be unique for a given user."));
    parent->FindWindow( ecID_MODIFY_PLATFORM_PREFIX )->SetHelpText(_("Specifies the prefix to be used when invoking tools (for example, 'arm-elf' is the correct prefix if the appropriate gdb executable is arm-elf-gdb)."));
    parent->FindWindow( ecID_MODIFY_PLATFORM_ARGS )->SetHelpText(_("Specifies any additional arguments to be used when invoking gdb."));
    parent->FindWindow( ecID_MODIFY_PLATFORM_INFERIOR )->SetHelpText(_("The gdb command to run."));
    parent->FindWindow( ecID_MODIFY_PLATFORM_PROMPT )->SetHelpText(_("The gdb prompt."));
#if USE_SS_GDB_CONTROL
    parent->FindWindow( ecID_MODIFY_PLATFORM_SS_GDB )->SetHelpText(_("TODO"));
#endif
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without saving any changes you have made."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

    // Add validators
    parent->FindWindow( ecID_MODIFY_PLATFORM_NAME )->SetValidator(wxGenericValidator(& m_strPlatform));
    parent->FindWindow( ecID_MODIFY_PLATFORM_PREFIX )->SetValidator(wxGenericValidator(& m_strPrefix));
    parent->FindWindow( ecID_MODIFY_PLATFORM_ARGS )->SetValidator(wxGenericValidator(& m_strGDB));
    parent->FindWindow( ecID_MODIFY_PLATFORM_INFERIOR )->SetValidator(wxGenericValidator(& m_strInferior));
    parent->FindWindow( ecID_MODIFY_PLATFORM_PROMPT )->SetValidator(wxGenericValidator(& m_strPrompt));
#if USE_SS_GDB_CONTROL
    parent->FindWindow( ecID_MODIFY_PLATFORM_SS_GDB )->SetValidator(wxGenericValidator(& m_bServerSideGdb));
#endif
}

void ecPlatformEditorDialog::OnChangeNewPlatformPrefix(wxCommandEvent& event)
{
    FindWindow(wxID_OK)->Enable(!((wxTextCtrl*)FindWindow(ecID_MODIFY_PLATFORM_NAME))->GetValue().IsEmpty() &&
        !((wxComboBox*)FindWindow(ecID_MODIFY_PLATFORM_PREFIX))->GetValue().IsEmpty());
}

void ecPlatformEditorDialog::OnChangeNewPlatform(wxCommandEvent& event)
{
    FindWindow(wxID_OK)->Enable(!((wxTextCtrl*)FindWindow(ecID_MODIFY_PLATFORM_NAME))->GetValue().IsEmpty() &&
        !((wxComboBox*)FindWindow(ecID_MODIFY_PLATFORM_PREFIX))->GetValue().IsEmpty());
}

void ecPlatformEditorDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecPlatformEditorDialog::OnOK(wxCommandEvent& event)
{
    TransferDataFromWindow();

    m_strGDB.Replace(g_NewLineToReplace, wxT(";"));

    EndModal(wxID_OK);
}
