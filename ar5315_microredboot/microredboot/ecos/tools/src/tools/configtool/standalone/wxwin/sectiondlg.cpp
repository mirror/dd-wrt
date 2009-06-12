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
// sectiondlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: sectiondlg.cpp,v 1.2 2001/03/15 17:33:45 julians Exp $
// Purpose:
// Description: Implementation file for ecSectionDialog
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
    #pragma implementation "sectiondlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/notebook.h"
#include "wx/cshelp.h"

#include "sectiondlg.h"
#include "configtool.h"

/*
 * Settings dialog
 */

IMPLEMENT_CLASS(ecSectionDialog, wxDialog)

BEGIN_EVENT_TABLE(ecSectionDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ecSectionDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecSectionDialog::OnCancel)
    EVT_BUTTON(wxID_HELP, ecSectionDialog::OnHelp)
    EVT_BUTTON(wxID_APPLY, ecSectionDialog::OnApply)
    EVT_NOTEBOOK_PAGE_CHANGED(-1, ecSectionDialog::OnPageChange)
END_EVENT_TABLE()

#define PROPERTY_DIALOG_WIDTH   400
#define PROPERTY_DIALOG_HEIGHT  380

// For 400x400 settings dialog, size your panels to about 375x325 in dialog editor
// (209 x 162 dialog units)

ecSectionDialog::ecSectionDialog(wxWindow* parent):
    wxDialog()
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    wxDialog::Create(parent, ecID_SECTION_DIALOG, _("Section Properties"), wxPoint(0, 0), wxSize(PROPERTY_DIALOG_WIDTH, PROPERTY_DIALOG_HEIGHT));

    // Under MSW, we don't seem to be able to react to a click on the dialog background (no
    // event is generated).
    SetHelpText(_("TODO"));

    wxScreenDC dc;
    wxSize ppi = dc.GetPPI();

    //double scaleFactor = ((double) charH) / 13.0;
    double scaleFactor = ((double) ppi.y) / 96.0;
    // Fudge the scale factor to make the dialog slightly smaller,
    // otherwise it's a bit big. (We're assuming that most displays
    // are 96 or 120 ppi).
    if (ppi.y == 120)
        scaleFactor = 1.14;
    int dialogWidth = (int)(PROPERTY_DIALOG_WIDTH * scaleFactor);
    int dialogHeight = (int)(PROPERTY_DIALOG_HEIGHT * scaleFactor);
    SetSize(dialogWidth, dialogHeight);
        
    m_notebook = new wxNotebook(this, ecID_SETTINGS_NOTEBOOK,
         wxPoint(2, 2), wxSize(PROPERTY_DIALOG_WIDTH - 4, PROPERTY_DIALOG_HEIGHT - 4));

    m_general = new ecSectionGeneralDialog(m_notebook);
    m_notebook->AddPage(m_general, wxT("General"));
    m_general->TransferDataToWindow();

    m_relocation = new ecSectionRelocationDialog(m_notebook);
    m_notebook->AddPage(m_relocation, wxT("Relocation"));
    m_relocation->TransferDataToWindow();

    m_note = new ecSectionNoteDialog(m_notebook);
    m_notebook->AddPage(m_note, wxT("Note"));
    m_note->TransferDataToWindow();

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    item0->Add( m_notebook, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *okButton = new wxButton( this, wxID_OK, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( okButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *cancelButton = new wxButton( this, wxID_CANCEL, "&Cancel", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( cancelButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *applyButton = new wxButton( this, wxID_APPLY, "&Apply", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( applyButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *helpButton = new wxButton( this, wxID_HELP, "&Help", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( helpButton, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    // Context-sensitive help button (question mark)
    wxButton *contextButton = new wxContextHelpButton( this );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    // Necessary to add a spacer or the button highlight interferes with the notebook under wxGTK
    item0->Add( 4, 4, 0, wxALIGN_CENTRE|wxALL, 0 );

    item0->Add( item1, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    this->SetAutoLayout( TRUE );
    this->SetSizer( item0 );

    okButton->SetDefault();
    okButton->SetFocus();

    Layout();

    m_general->Layout();
    m_relocation->Layout();
    m_note->Layout();

    okButton->SetHelpText(_("Closes the dialog and saves any changes you may have made."));
    cancelButton->SetHelpText(_("Closes the dialog without saving any changes you have made."));
    helpButton->SetHelpText(_("Invokes help for the selected dialog."));
    applyButton->SetHelpText(_("Immediately applies any changes you may have made."));

    Centre(wxBOTH);
}

void ecSectionDialog::OnOK(wxCommandEvent& event)
{
    wxDialog::OnOK(event);
}

void ecSectionDialog::OnCancel(wxCommandEvent& event)
{
    wxDialog::OnCancel(event);
}

void ecSectionDialog::OnApply(wxCommandEvent& event)
{
}

void ecSectionDialog::OnHelp(wxCommandEvent& event)
{
    int sel = m_notebook->GetSelection();

    wxASSERT_MSG( (sel != -1), wxT("A notebook tab should always be selected."));

    wxWindow* page = (wxWindow*) m_notebook->GetPage(sel);

    wxString helpTopic;
    if (page == m_general)
    {
        helpTopic = wxT("General section dialog");
    }

    if (!helpTopic.IsEmpty())
    {
        wxGetApp().GetHelpController().KeywordSearch(helpTopic);
    }
}

// This sets the text for the selected page, but doesn't help
// when trying to click on a tab: we would expect the appropriate help
// for that tab. We would need to look at the tabs to do this, from within OnContextHelp -
// probably not worth it.
void ecSectionDialog::OnPageChange(wxNotebookEvent& event)
{
    event.Skip();

    int sel = m_notebook->GetSelection();
    if (sel < 0)
        return;

    wxWindow* page = m_notebook->GetPage(sel);
    if (page)
    {
        wxString helpText;
#if 0
        if (page == m_displayOptions)
            helpText = _("The display options dialog allows you to change display-related options.");
        else if (page == m_viewerOptions)
            helpText = _("The viewer options dialog allows you to configure viewers.");
        else if (page == m_pathOptions)
            helpText = _("The path options dialog allows you to change tool paths.");
        else if (page == m_conflictResolutionOptions)
            helpText = _("The conflict resolution options dialog allows you to change options related to conflict resolution.");
        m_notebook->SetHelpText(helpText);
#endif
    }
}

bool ecSectionDialog::TransferDataToWindow()
{
    m_general->TransferDataToWindow();
    m_relocation->TransferDataToWindow();
    m_note->TransferDataToWindow();
    return TRUE;
}

bool ecSectionDialog::TransferDataFromWindow()
{
    m_general->TransferDataFromWindow();
    m_relocation->TransferDataFromWindow();
    m_note->TransferDataFromWindow();
    return TRUE;
}

/* General page
 */

IMPLEMENT_CLASS(ecSectionGeneralDialog, wxPanel)

ecSectionGeneralDialog::ecSectionGeneralDialog(wxWindow* parent):
    wxPanel(parent, ecID_SECTION_GENERAL)
{
    CreateControls(this);    

    SetHelpText(_("TODO"));
}

void ecSectionGeneralDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, "Name" );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxHORIZONTAL );

    wxSizer *item3 = new wxBoxSizer( wxVERTICAL );

    wxRadioButton *item4 = new wxRadioButton( parent, ecID_SECTION_GENERAL_LINKER, "&Linker-defined:", wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item3->Add( item4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxRadioButton *item5 = new wxRadioButton( parent, ecID_SECTION_GENERAL_USER, "&User-defined:", wxDefaultPosition, wxDefaultSize, 0 );
    item3->Add( item5, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item1->Add( item3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    item1->Add( 5, 5, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxSizer *item6 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item7 = new wxTextCtrl( parent, ecID_SECTION_GENERAL_LINKER_TEXT, "", wxDefaultPosition, wxSize(200,-1), 0 );
    item6->Add( item7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString strs8[] = 
    {
        "ChoiceItem"
    };
    wxChoice *item8 = new wxChoice( parent, ecID_SECTION_GENERAL_USER_TEXT, wxDefaultPosition, wxSize(100,-1), 1, strs8, 0 );
    item6->Add( item8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    wxSizer *item9 = new wxBoxSizer( wxHORIZONTAL );

    wxCheckBox *item10 = new wxCheckBox( parent, ecID_SECTION_GENERAL_KNOWN_SIZE, "&Known size:", wxDefaultPosition, wxDefaultSize, 0 );
    item9->Add( item10, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs11[] = 
    {
        "ChoiceItem"
    };
    wxChoice *item11 = new wxChoice( parent, ecID_SECTION_GENERAL_KNOWN_SIZE_CHOICE, wxDefaultPosition, wxSize(100,-1), 1, strs11, 0 );
    item9->Add( item11, 0, wxALIGN_CENTRE|wxALL, 5 );

    item6->Add( item9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    item1->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    //item0->Add( 20, 20, 0, wxALIGN_CENTRE|wxALL, 5 );
    item0->Add( 1, 1, 20, wxALIGN_CENTRE|wxALL, 0 );

    wxStaticBox *item13 = new wxStaticBox( parent, -1, "Find Location (VMA)" );
    wxSizer *item12 = new wxStaticBoxSizer( item13, wxHORIZONTAL );

    wxSizer *item14 = new wxBoxSizer( wxVERTICAL );

    wxRadioButton *item15 = new wxRadioButton( parent, ecID_SECTION_GENERAL_ABSOLUTE, "A&bsolute:", wxDefaultPosition, wxDefaultSize, 0 );
    item14->Add( item15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxRadioButton *item16 = new wxRadioButton( parent, ecID_SECTION_GENERAL_FOLLOWING, "&Following:", wxDefaultPosition, wxDefaultSize, 0 );
    item14->Add( item16, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item12->Add( item14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    item12->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxSizer *item17 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item18 = new wxTextCtrl( parent, ecID_SECTION_GENERAL_ABSOLUTE_TEXT, "", wxDefaultPosition, wxSize(200,-1), 0 );
    item17->Add( item18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString strs19[] = 
    {
        "ChoiceItem"
    };
    wxChoice *item19 = new wxChoice( parent, ecID_SECTION_GENERAL_FOLLOWING_TEXT, wxDefaultPosition, wxSize(100,-1), 1, strs19, 0 );
    item17->Add( item19, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    wxSizer *item20 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item21 = new wxStaticText( parent, wxID_STATIC, "&Alignment:", wxDefaultPosition, wxDefaultSize, 0 );
    item20->Add( item21, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs22[] = 
    {
        "ChoiceItem"
    };
    wxChoice *item22 = new wxChoice( parent, ecID_SECTION_GENERAL_ALIGNMENT, wxDefaultPosition, wxSize(100,-1), 1, strs22, 0 );
    item20->Add( item22, 0, wxALIGN_CENTRE|wxALL, 5 );

    item17->Add( item20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    item12->Add( item17, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    item0->Add( item12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );


    // Add context-sensitive help
    //item2->SetHelpText(_(""));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

bool ecSectionGeneralDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();
    return TRUE;
}

bool ecSectionGeneralDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();
    return TRUE;
}

/* Relocation page
 */

IMPLEMENT_CLASS(ecSectionRelocationDialog, wxPanel)

ecSectionRelocationDialog::ecSectionRelocationDialog(wxWindow* parent):
    wxPanel(parent, ecID_SECTION_RELOCATION)
{
    CreateControls(this);    

    SetHelpText(_("TODO"));
}

void ecSectionRelocationDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    item0->Add( 20, 20, 5, wxALIGN_CENTRE|wxALL, 5 );

    wxCheckBox *item1 = new wxCheckBox( parent, ecID_SECTION_RELOCATION_RELOCATE, "&Relocate section", wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 10 );

    item0->Add( 20, 20, 20, wxALIGN_CENTRE|wxALL, 0 );

    wxStaticBox *item3 = new wxStaticBox( parent, -1, "Initial Location (LMA)" );
    wxSizer *item2 = new wxStaticBoxSizer( item3, wxVERTICAL );

    wxSizer *item4 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item5 = new wxRadioButton( parent, ecID_SECTION_RELOCATION_ABSOLUTE, "A&bsolute:", wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    item4->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxTextCtrl *item6 = new wxTextCtrl( parent, ecID_SECTION_RELOCATION_ABSOLUTE_TEXT, "", wxDefaultPosition, wxSize(80,-1), 0 );
    item4->Add( item6, 20, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    item2->Add( item4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item7 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item8 = new wxRadioButton( parent, ecID_SECTION_RELOCATION_FOLLOWING, "&Following:", wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add( item8, 0, wxALIGN_CENTRE|wxALL, 5 );

    item7->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs9[] = 
    {
        "ChoiceItem"
    };
    wxChoice *item9 = new wxChoice( parent, ecID_SECTION_RELOCATION_FOLLOWING_CHOICE, wxDefaultPosition, wxSize(100,-1), 1, strs9, 0 );
    item7->Add( item9, 20, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    item2->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item2, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( 20, 20, 20, wxALIGN_CENTRE|wxALL, 5 );

    // Add context-sensitive help
    //item2->SetHelpText(_(""));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

bool ecSectionRelocationDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();
    return TRUE;
}

bool ecSectionRelocationDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();
    return TRUE;
}

/* Note page
 */

IMPLEMENT_CLASS(ecSectionNoteDialog, wxPanel)

ecSectionNoteDialog::ecSectionNoteDialog(wxWindow* parent):
    wxPanel(parent, ecID_SECTION_NOTE)
{
    CreateControls(this);    

    SetHelpText(_("TODO"));
}

void ecSectionNoteDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxTextCtrl *item1 = new wxTextCtrl( parent, ecID_SECTION_NOTE_TEXT, "", wxDefaultPosition, wxSize(80,40), wxTE_MULTILINE );
    item0->Add( item1, 20, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );


    // Add context-sensitive help
    //item2->SetHelpText(_(""));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

bool ecSectionNoteDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();
    return TRUE;
}

bool ecSectionNoteDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();
    return TRUE;
}
