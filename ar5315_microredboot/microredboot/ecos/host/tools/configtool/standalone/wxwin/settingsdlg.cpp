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
// Date:        2000/09/11
// Version:     $Id: settingsdlg.cpp,v 1.16 2002/02/15 17:40:01 julians Exp $
// Purpose:
// Description: Implementation file for ecSettingsDialog
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
    #pragma implementation "settingsdlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/spinctrl.h"
#include "wx/notebook.h"
#include "wx/cshelp.h"
#include "wx/valgen.h"
#include "wx/fontdlg.h"
#include "wx/mimetype.h"

#include "settingsdlg.h"
#include "configtool.h"
#include "configtooldoc.h"
#include "configtoolview.h"

// For now, we're not implementing the paths page on the settings dialog,
// but probably we should eventually for consistency and ease of use
#define ecUSE_PATHS_PAGE 0

#define ecUSE_RUN_PAGE 1

/*
 * Settings dialog
 */

IMPLEMENT_CLASS(ecSettingsDialog, wxDialog)

BEGIN_EVENT_TABLE(ecSettingsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ecSettingsDialog::OnOK)
    EVT_BUTTON(wxID_HELP, ecSettingsDialog::OnHelp)
    EVT_NOTEBOOK_PAGE_CHANGED(-1, ecSettingsDialog::OnPageChange)
END_EVENT_TABLE()

// GTK+ widgets seem to be chunkier than WIN32's, so give 'em more elbow-room

#ifdef __WXMSW__
#define PROPERTY_DIALOG_WIDTH   380
#define PROPERTY_DIALOG_HEIGHT  420
#else
#define PROPERTY_DIALOG_WIDTH   550 // 460
#define PROPERTY_DIALOG_HEIGHT  480
#endif

// For 400x400 settings dialog, size your panels to about 375x325 in dialog editor
// (209 x 162 dialog units)

ecSettingsDialog::ecSettingsDialog(wxWindow* parent):
    wxDialog()
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    wxDialog::Create(parent, ecID_SETTINGS_DIALOG, wxGetApp().GetSettings().GetAppName() + wxT(" Settings"), wxPoint(0, 0), wxSize(PROPERTY_DIALOG_WIDTH, PROPERTY_DIALOG_HEIGHT));

    // Under MSW, we don't seem to be able to react to a click on the dialog background (no
    // event is generated).
    SetHelpText(_("The settings dialog provides the ability to change a variety of settings."));

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
        
    m_displayOptions = NULL;

    m_notebook = new wxNotebook(this, ecID_SETTINGS_NOTEBOOK,
         wxPoint(2, 2), wxSize(PROPERTY_DIALOG_WIDTH - 4, PROPERTY_DIALOG_HEIGHT - 4));

    m_displayOptions = new ecDisplayOptionsDialog(m_notebook);
    m_notebook->AddPage(m_displayOptions, wxT("Display"));
    m_displayOptions->TransferDataToWindow();

    m_viewerOptions = new ecViewerOptionsDialog(m_notebook);
    m_notebook->AddPage(m_viewerOptions, wxT("Viewers"));
    m_viewerOptions->TransferDataToWindow();

#if ecUSE_PATHS_PAGE
    m_pathOptions = new ecPathOptionsDialog(m_notebook);
    m_notebook->AddPage(m_pathOptions, wxT("Paths"));
    m_pathOptions->TransferDataToWindow();
#endif

    m_conflictResolutionOptions = new ecConflictResolutionOptionsDialog(m_notebook);
    m_notebook->AddPage(m_conflictResolutionOptions, wxT("Conflict Resolution"));
    m_conflictResolutionOptions->TransferDataToWindow();

#if ecUSE_RUN_PAGE
    m_runOptions = new ecRunOptionsDialog(m_notebook);
    m_notebook->AddPage(m_runOptions, wxT("Run Tests"));
    m_runOptions->TransferDataToWindow();
#endif

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    item0->Add( m_notebook, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *okButton = new wxButton( this, wxID_OK, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( okButton, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *cancelButton = new wxButton( this, wxID_CANCEL, "&Cancel", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( cancelButton, 0, wxALIGN_CENTRE|wxALL, 5 );

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

    m_displayOptions->Layout();
    m_viewerOptions->Layout();
#if ecUSE_PATHS_PAGE
    m_pathOptions->Layout();
#endif
    m_conflictResolutionOptions->Layout();
#if ecUSE_RUN_PAGE
    m_runOptions->Layout();
#endif
    
    okButton->SetHelpText(_("Closes the dialog and saves any changes you may have made."));
    cancelButton->SetHelpText(_("Closes the dialog without saving any changes you have made."));
    helpButton->SetHelpText(_("Invokes help for the selected dialog."));

    Centre(wxBOTH);
}

void ecSettingsDialog::SetSelection(int sel)
{
    m_notebook->SetSelection(sel);
}

void ecSettingsDialog::OnOK(wxCommandEvent& event)
{
    ecSettings oldSettings(wxGetApp().GetSettings());

    wxDialog::OnOK(event);

    if (wxGetApp().GetSettings().m_bHex != oldSettings.m_bHex)
    {
        // Refresh the values window and currently selected properties
        ecConfigToolHint hint(NULL, ecAllSaved);
        if (wxGetApp().GetConfigToolDoc())
            wxGetApp().GetConfigToolDoc()->UpdateAllViews (NULL, & hint);        
    }

    if (wxGetApp().GetSettings().m_showMacroNames != oldSettings.m_showMacroNames)
    {
        ecConfigToolHint hint(NULL, ecNameFormatChanged);
        if (wxGetApp().GetConfigToolDoc())
            wxGetApp().GetConfigToolDoc()->UpdateAllViews (NULL, & hint);        
    }    
}

void ecSettingsDialog::OnHelp(wxCommandEvent& event)
{
    int sel = m_notebook->GetSelection();

    wxASSERT_MSG( (sel != -1), wxT("A notebook tab should always be selected."));

    wxWindow* page = (wxWindow*) m_notebook->GetPage(sel);

    wxString helpTopic;
    if (page == m_displayOptions)
    {
        helpTopic = wxT("Display options dialog");
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
void ecSettingsDialog::OnPageChange(wxNotebookEvent& event)
{
    event.Skip();

    int sel = m_notebook->GetSelection();
    if (sel < 0)
        return;

    wxWindow* page = m_notebook->GetPage(sel);
    if (page)
    {
        wxString helpText;

        if (page == m_displayOptions)
            helpText = _("The display options dialog allows you to change display-related options.");
        else if (page == m_viewerOptions)
            helpText = _("The viewer options dialog allows you to configure viewers.");
#if ecUSE_PATHS_PAGE
        else if (page == m_pathOptions)
            helpText = _("The path options dialog allows you to change tool paths.");
#endif
        else if (page == m_conflictResolutionOptions)
            helpText = _("The conflict resolution options dialog allows you to change options related to conflict resolution.");
#if ecUSE_RUN_PAGE
        else if (page == m_runOptions)
            helpText = _("The run options dialog allows you to change options related to running tests.");
#endif
        m_notebook->SetHelpText(helpText);
    }
}

bool ecSettingsDialog::TransferDataToWindow()
{
    m_displayOptions->TransferDataToWindow();
    m_viewerOptions->TransferDataToWindow();
#if ecUSE_PATHS_PAGE
    m_pathOptions->TransferDataToWindow();
#endif
    m_conflictResolutionOptions->TransferDataToWindow();
#if ecUSE_RUN_PAGE
    m_runOptions->TransferDataToWindow();
#endif
    return TRUE;
}

bool ecSettingsDialog::TransferDataFromWindow()
{
    m_displayOptions->TransferDataFromWindow();
    m_viewerOptions->TransferDataFromWindow();
#if ecUSE_PATHS_PAGE
    m_pathOptions->TransferDataFromWindow();
#endif
    m_conflictResolutionOptions->TransferDataFromWindow();
#if ecUSE_RUN_PAGE
    m_runOptions->TransferDataFromWindow();
#endif
    return TRUE;
}

/* Display options dialog
 */

// For now, disable some unnecessary features
#define ecUSE_FONT_SELECTION 1

IMPLEMENT_CLASS(ecDisplayOptionsDialog, wxPanel)

BEGIN_EVENT_TABLE(ecDisplayOptionsDialog, wxPanel)
    EVT_BUTTON(ecID_DISPLAY_OPTIONS_CHANGE_FONT, ecDisplayOptionsDialog::OnChangeFont)
END_EVENT_TABLE()

ecDisplayOptionsDialog::ecDisplayOptionsDialog(wxWindow* parent):
    wxPanel(parent, ecID_SETTINGS_DISPLAY)
{
    CreateControls(this);    

    SetHelpText(_("The display options dialog allows you to change display-related options"));
}

void ecDisplayOptionsDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("Configuration Pane") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxHORIZONTAL );

    wxString strs3[] = 
    {
        _("Use &macro names"),
        _("Use descriptive &names")
    };
    wxRadioBox *item3 = new wxRadioBox( parent, ecID_DISPLAY_OPTIONS_LABELS, _("Labels"), wxDefaultPosition, wxDefaultSize, 2, strs3, 1, wxRA_SPECIFY_COLS );
    item1->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item1->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs4[] = 
    {
        _("&Decimal"),
        _("He&xadecimal")
    };
    wxRadioBox *item4 = new wxRadioBox( parent, ecID_DISPLAY_OPTIONS_INTEGER_ITEMS, _("Integer items"), wxDefaultPosition, wxDefaultSize, 2, strs4, 1, wxRA_SPECIFY_COLS );
    item1->Add( item4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#if ecUSE_FONT_SELECTION
    wxStaticBox *item6 = new wxStaticBox( parent, -1, _("Font") );
    wxSizer *item5 = new wxStaticBoxSizer( item6, wxHORIZONTAL );

    wxStaticText *item7 = new wxStaticText( parent, wxID_STATIC, _("&Window:"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxChoice *item8 = new wxChoice( parent, ecID_DISPLAY_OPTIONS_FONT_CHOICE, wxDefaultPosition, wxSize(120,-1), 0, NULL, 0 );
    item5->Add( item8, 0, wxALIGN_CENTRE|wxALL, 5 );

    item5->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item9 = new wxButton( parent, ecID_DISPLAY_OPTIONS_CHANGE_FONT, _("Change &Font..."), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item9, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
#endif

    wxStaticBox *item11 = new wxStaticBox( parent, -1, _("Miscellaneous") );
    wxSizer *item10 = new wxStaticBoxSizer( item11, wxVERTICAL );

    wxCheckBox *item12 = new wxCheckBox( parent, ecID_DISPLAY_OPTIONS_SHOW_SPLASH, _("Show initial &splash screen"), wxDefaultPosition, wxDefaultSize, 0 );
    item10->Add( item12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    // Add validators
    FindWindow(ecID_DISPLAY_OPTIONS_SHOW_SPLASH)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_showSplashScreen));

    // Add context-sensitive help
    item2->SetHelpText(_("Options relating to the configuration pane."));
    FindWindow(ecID_DISPLAY_OPTIONS_LABELS)->SetHelpText(_("Display macro or descriptive names in the configuration pane."));
    FindWindow(ecID_DISPLAY_OPTIONS_INTEGER_ITEMS)->SetHelpText(_("View integer items in the configuration pane in either decimal or hexadecimal."));
    
#if ecUSE_FONT_SELECTION
    FindWindow(ecID_DISPLAY_OPTIONS_FONT_CHOICE)->SetHelpText(_("Selects a window for which the font is to be changed."));
    FindWindow(ecID_DISPLAY_OPTIONS_CHANGE_FONT)->SetHelpText(_("Changes the font of the chosen window."));
    // Init the choice control
    unsigned int i;
    for (i = 0; i < wxGetApp().GetSettings().GetWindowSettings().GetCount(); i++)
    {
        ((wxChoice*) FindWindow(ecID_DISPLAY_OPTIONS_FONT_CHOICE))->Append(wxGetApp().GetSettings().GetWindowSettings().GetName(i));
    }

    ((wxChoice*) FindWindow(ecID_DISPLAY_OPTIONS_FONT_CHOICE))->SetSelection(0);
#endif

    FindWindow(ecID_DISPLAY_OPTIONS_SHOW_SPLASH)->SetHelpText(_("Selects whether a splash screen will be shown as the application starts."));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

bool ecDisplayOptionsDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();

    ((wxRadioBox*) FindWindow(ecID_DISPLAY_OPTIONS_INTEGER_ITEMS))->SetSelection( wxGetApp().GetSettings().m_bHex ? 1 : 0 ) ;
    ((wxRadioBox*) FindWindow(ecID_DISPLAY_OPTIONS_LABELS))->SetSelection( wxGetApp().GetSettings().m_showMacroNames ? 0 : 1 ) ;
    return TRUE;
}

bool ecDisplayOptionsDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxGetApp().GetSettings().m_bHex = ((wxRadioBox*) FindWindow(ecID_DISPLAY_OPTIONS_INTEGER_ITEMS))->GetSelection() == 1;
    wxGetApp().GetSettings().m_showMacroNames = ((wxRadioBox*) FindWindow(ecID_DISPLAY_OPTIONS_LABELS))->GetSelection() == 0 ;

    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults())
        wxGetApp().GetSettings().GetWindowSettings().ApplyFontsToWindows();

    return TRUE;
}

void ecDisplayOptionsDialog::OnChangeFont(wxCommandEvent& event)
{
    wxChoice* choice = (wxChoice*) FindWindow(ecID_DISPLAY_OPTIONS_FONT_CHOICE);

    wxString str = choice->GetStringSelection();
    if (!str.IsEmpty())
    {
        wxFontData data;
        data.SetInitialFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(str));

        wxFontDialog dlg(this, & data);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxGetApp().GetSettings().GetWindowSettings().SetFont(str, dlg.GetFontData().GetChosenFont()) ;

            // Changed a font, so start using specified fonts.
            wxGetApp().GetSettings().GetWindowSettings().SetUseDefaults(FALSE) ;
        }
    }
}


/* Viewer options dialog
 */

IMPLEMENT_CLASS(ecViewerOptionsDialog, wxPanel)

BEGIN_EVENT_TABLE(ecViewerOptionsDialog, wxPanel)
    EVT_BUTTON(ecID_VIEWER_DIALOG_BROWSE_HEADER, ecViewerOptionsDialog::OnBrowseForViewer)
    EVT_BUTTON(ecID_VIEWER_DIALOG_BROWSE_DOC, ecViewerOptionsDialog::OnBrowseForBrowser)
    EVT_BUTTON(ecID_VIEWER_DIALOG_ASSOC_INFO, ecViewerOptionsDialog::OnShowAssociatedViewerInfo)

    EVT_UPDATE_UI(ecID_VIEWER_DIALOG_HEADER_TEXT, ecViewerOptionsDialog::OnUpdateViewerText)
    EVT_UPDATE_UI(ecID_VIEWER_DIALOG_BROWSE_HEADER, ecViewerOptionsDialog::OnUpdateViewerText)
    EVT_UPDATE_UI(ecID_VIEWER_DIALOG_DOC_TEXT, ecViewerOptionsDialog::OnUpdateBrowserText)
    EVT_UPDATE_UI(ecID_VIEWER_DIALOG_BROWSE_DOC, ecViewerOptionsDialog::OnUpdateBrowserText)
END_EVENT_TABLE()

ecViewerOptionsDialog::ecViewerOptionsDialog(wxWindow* parent):
    wxPanel(parent, ecID_SETTINGS_VIEWER)
{
    CreateControls(this);    

    SetHelpText(_("The viewer options dialog allows you to configure viewers."));
}

bool ecViewerOptionsDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();

    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_HEADER_ASSOCIATED))->SetValue(! wxGetApp().GetSettings().m_bUseCustomViewer);
    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_HEADER_THIS))->SetValue(wxGetApp().GetSettings().m_bUseCustomViewer);

    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_BUILTIN))->SetValue(wxGetApp().GetSettings().m_eUseCustomBrowser == ecInternal);
    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_THIS))->SetValue(wxGetApp().GetSettings().m_eUseCustomBrowser == ecCustomExternal);
    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_ASSOCIATED))->SetValue(wxGetApp().GetSettings().m_eUseCustomBrowser == ecAssociatedExternal);

    return TRUE;
}

bool ecViewerOptionsDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxGetApp().GetSettings().m_bUseCustomViewer = ! ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_HEADER_ASSOCIATED))->GetValue();

    if (((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_BUILTIN))->GetValue())
        wxGetApp().GetSettings().m_eUseCustomBrowser = ecInternal;
    else if (((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_THIS))->GetValue())
        wxGetApp().GetSettings().m_eUseCustomBrowser = ecCustomExternal;
    else if (((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_ASSOCIATED))->GetValue())
        wxGetApp().GetSettings().m_eUseCustomBrowser = ecAssociatedExternal;

    return TRUE;
}

void ecViewerOptionsDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("View header files using") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxVERTICAL );

    wxRadioButton *item3 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_HEADER_ASSOCIATED, _("Associated viewer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item1->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item4 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item5 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_HEADER_THIS, _("This &viewer:"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item4->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, ecID_VIEWER_DIALOG_BROWSE_HEADER, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 0 );

    wxTextCtrl *item7 = new wxTextCtrl( parent, ecID_VIEWER_DIALOG_HEADER_TEXT, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item1->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticBox *item9 = new wxStaticBox( parent, -1, _("View documentation using") );
    wxSizer *item8 = new wxStaticBoxSizer( item9, wxVERTICAL );

    wxRadioButton *item10 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_BUILTIN, _("&Built-in viewer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item8->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item11 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item12 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_ASSOCIATED, _("Associated browser"), wxDefaultPosition, wxDefaultSize, 0 );
    item11->Add( item12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item11->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item13 = new wxButton( parent, ecID_VIEWER_DIALOG_ASSOC_INFO, _("&About..."), wxDefaultPosition, wxDefaultSize, 0 );
    item11->Add( item13, 0, wxALIGN_CENTRE|wxALL, 5 );

    item8->Add( item11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxSizer *item14 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item15 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_THIS, _("This &browser:"), wxDefaultPosition, wxDefaultSize, 0 );
    item14->Add( item15, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item14->Add( 20, 20, 1, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    wxButton *item16 = new wxButton( parent, ecID_VIEWER_DIALOG_BROWSE_DOC, _("Br&owse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item14->Add( item16, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item8->Add( item14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxTextCtrl *item17 = new wxTextCtrl( parent, ecID_VIEWER_DIALOG_DOC_TEXT, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item8->Add( item17, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#if 0
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("View header files using") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxVERTICAL );

    wxRadioButton *item3 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_HEADER_ASSOCIATED, _("Associated viewer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item1->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxSizer *item4 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item5 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_HEADER_THIS, _("This &viewer:"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item4->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, ecID_VIEWER_DIALOG_BROWSE_HEADER, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 0 );

    wxTextCtrl *item7 = new wxTextCtrl( parent, ecID_VIEWER_DIALOG_HEADER_TEXT, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item1->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

//    item0->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticBox *item9 = new wxStaticBox( parent, -1, _("View documentation using") );
    wxSizer *item8 = new wxStaticBoxSizer( item9, wxVERTICAL );

    wxRadioButton *item10 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_BUILTIN, _("&Built-in viewer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item8->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxRadioButton *item11 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_ASSOCIATED, _("Associated browser"), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item12 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item13 = new wxRadioButton( parent, ecID_VIEWER_DIALOG_DOC_THIS, _("This &browser:"), wxDefaultPosition, wxDefaultSize, 0 );
    item12->Add( item13, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item12->Add( 20, 20, 1, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    wxButton *item14 = new wxButton( parent, ecID_VIEWER_DIALOG_BROWSE_DOC, _("Br&owse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item12->Add( item14, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    item8->Add( item12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxTextCtrl *item15 = new wxTextCtrl( parent, ecID_VIEWER_DIALOG_DOC_TEXT, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item8->Add( item15, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
#endif

    // Disable this option because we don't yet have a built-in browser
#if 0 // !ecUSE_EXPERIMENTAL_CODE
    FindWindow(ecID_VIEWER_DIALOG_DOC_BUILTIN)->Enable(FALSE);
#endif

    // Add validators
    FindWindow(ecID_VIEWER_DIALOG_HEADER_TEXT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_strViewer));
    FindWindow(ecID_VIEWER_DIALOG_DOC_TEXT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_strBrowser));

    // Add context-sensitive help
    item2->SetHelpText(_("Allows you to select a viewer to display header files."));
    FindWindow(ecID_VIEWER_DIALOG_HEADER_ASSOCIATED)->SetHelpText(_("Select the default viewer to display header files."));
    FindWindow(ecID_VIEWER_DIALOG_HEADER_THIS)->SetHelpText(_("Select a viewer of your choice to display header files."));
    FindWindow(ecID_VIEWER_DIALOG_BROWSE_HEADER)->SetHelpText(_("Browses for a viewer to be used to display header files."));
    FindWindow(ecID_VIEWER_DIALOG_HEADER_TEXT)->SetHelpText(_("Specify the viewer executable to be used to display header files."));
    item9->SetHelpText(_("Allows you to select a viewer to display documentation."));
    FindWindow(ecID_VIEWER_DIALOG_DOC_BUILTIN)->SetHelpText(_("Select the internal HTML help mechanism to display HTML-based help."));
    FindWindow(ecID_VIEWER_DIALOG_DOC_ASSOCIATED)->SetHelpText(_("Select the default browser to display HTML-based help."));
    FindWindow(ecID_VIEWER_DIALOG_DOC_THIS)->SetHelpText(_("Select a browser of your choice to display HTML-based help."));
    FindWindow(ecID_VIEWER_DIALOG_BROWSE_DOC)->SetHelpText(_("Browses for a browser to be used to display HTML-based help."));
    FindWindow(ecID_VIEWER_DIALOG_ASSOC_INFO)->SetHelpText(_("Shows information about the associated viewer."));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
}

void ecViewerOptionsDialog::OnUpdateViewerText(wxUpdateUIEvent& event)
{
    event.Enable( FindWindow(ecID_VIEWER_DIALOG_HEADER_THIS) &&
                    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_HEADER_THIS))->GetValue() );
}

void ecViewerOptionsDialog::OnUpdateBrowserText(wxUpdateUIEvent& event)
{
    event.Enable( FindWindow(ecID_VIEWER_DIALOG_DOC_THIS) &&
                    ((wxRadioButton*) FindWindow(ecID_VIEWER_DIALOG_DOC_THIS))->GetValue() );
}

void ecViewerOptionsDialog::OnBrowseForViewer(wxCommandEvent& event)
{
    wxString currentViewer = ((wxTextCtrl*) FindWindow(ecID_VIEWER_DIALOG_HEADER_TEXT))->GetValue();

    wxFileDialog dialog(this, _("Choose a viewer executable"), wxPathOnly(currentViewer),
        wxFileNameFromPath(currentViewer), wxT("*.exe"));

    if (dialog.ShowModal() == wxID_OK)
    {
        ((wxTextCtrl*) FindWindow(ecID_VIEWER_DIALOG_HEADER_TEXT))->SetValue(dialog.GetPath());
    }
}

void ecViewerOptionsDialog::OnBrowseForBrowser(wxCommandEvent& event)
{
    wxString currentViewer = ((wxTextCtrl*) FindWindow(ecID_VIEWER_DIALOG_DOC_TEXT))->GetValue();

    wxFileDialog dialog(this, _("Choose a browser executable"), wxPathOnly(currentViewer),
        wxFileNameFromPath(currentViewer), wxT("*.exe"));

    if (dialog.ShowModal() == wxID_OK)
    {
        ((wxTextCtrl*) FindWindow(ecID_VIEWER_DIALOG_DOC_TEXT))->SetValue(dialog.GetPath());
    }
}

void ecViewerOptionsDialog::OnShowAssociatedViewerInfo(wxCommandEvent& event)
{
    wxString msg;
    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(wxT("html"));
    if ( !ft )
    {
        msg += wxT("It is not possible to determine the associated browser for HTML documents.\n\n");
    }
    else
    {
        msg += wxT("The associated MIME type for HTML is:\n");
        wxString mimeType(wxT("Unknown"));
        ft->GetMimeType(& mimeType);
        msg += mimeType;
        msg += wxT("\n");

        wxString descr;
        if (ft->GetDescription(& descr))
        {
            msg += descr;
            msg += wxT("\n");
        }
        msg += wxT("\n");

        wxString cmd;
        wxString url(wxT("http://example-url.html"));
        bool ok = ft->GetOpenCommand(&cmd,
            wxFileType::MessageParameters(url, _T("")));

        if (ok)
        {
            msg += wxT("The associated command is:\n");
            msg += cmd;
            msg += wxT("\n");
        }
        msg += wxT("\n");
    }

    msg += wxT("If this MIME type is not defined or looks wrong, please consult your ");
    msg += wxT("Configuration Tool documentation for how to set up an association.\n");
#if defined(__WXGTK__)
    msg += wxT("On Unix, this can be done by adding an entry to your ~/.mailcap file.");
#endif

    delete ft;
   
    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_INFORMATION|wxOK);
}

/* Path options dialog
 */

IMPLEMENT_CLASS(ecPathOptionsDialog, wxPanel)

ecPathOptionsDialog::ecPathOptionsDialog(wxWindow* parent):
    wxPanel(parent, ecID_SETTINGS_PATH)
{
    CreateControls(this);    

    SetHelpText(_("The path options dialog allows you to change tool paths."));
}

bool ecPathOptionsDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();
    return TRUE;
}

bool ecPathOptionsDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();
    return TRUE;
}

void ecPathOptionsDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("Build Tools") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxVERTICAL );

    wxStaticText *item3 = new wxStaticText( parent, ecID_PATHS_BUILD_MSG, 
        _("Enter the location of the arm-elf build tools "
          "folder, which should contain arm-elf-gcc. You can "
          "type in a path or use the Browse button to "
          "navigate to a folder."),
        wxDefaultPosition, wxSize(-1,70), 0 );
    item1->Add( item3, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item4 = new wxBoxSizer( wxHORIZONTAL );

    wxString *strs5 = (wxString*) NULL;
    wxComboBox *item5 = new wxComboBox( parent, ecID_PATHS_BUILD_COMBO, _(""), wxDefaultPosition, wxSize(100,-1), 0, strs5, wxCB_DROPDOWN );
    item4->Add( item5, 20, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item4, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxButton *item6 = new wxButton( parent, ecID_PATHS_BUILD_BROWSE, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxStaticBox *item8 = new wxStaticBox( parent, -1, _("User Tools") );
    wxSizer *item7 = new wxStaticBoxSizer( item8, wxVERTICAL );

    wxStaticText *item9 = new wxStaticText( parent, ecID_PATHS_USER_MSG, 
        _("Enter the location of the user tools folder, "
          "which should contain cat and ls. You can type in "
          "a path or use the Browse button to navigate to a "
          "folder."),
        wxDefaultPosition, wxSize(-1,60), 0 );
    item7->Add( item9, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item10 = new wxBoxSizer( wxHORIZONTAL );

    wxString *strs11 = (wxString*) NULL;
    wxComboBox *item11 = new wxComboBox( parent, ecID_PATHS_USER_COMBO, _(""), wxDefaultPosition, wxSize(100,-1), 0, strs11, wxCB_DROPDOWN );
    item10->Add( item11, 20, wxALIGN_CENTRE|wxALL, 5 );

    item7->Add( item10, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxButton *item12 = new wxButton( parent, ecID_PATHS_USER_BROWSE, _("&Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
    item7->Add( item12, 0, wxALIGN_CENTRE|wxALL, 5 );
    item0->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );


    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );

    // Add context-sensitive help
    FindWindow( ecID_PATHS_BUILD_COMBO )->SetHelpText(_("Select the build tools folder."));
    FindWindow( ecID_PATHS_BUILD_BROWSE )->SetHelpText(_("Browse for the build tools folder."));
    FindWindow( ecID_PATHS_USER_COMBO )->SetHelpText(_("Select the user tools folder."));
    FindWindow( ecID_PATHS_USER_BROWSE )->SetHelpText(_("Browse for the user tools folder."));
}

/* Conflict resolution options dialog
 */

IMPLEMENT_CLASS(ecConflictResolutionOptionsDialog, wxPanel)

ecConflictResolutionOptionsDialog::ecConflictResolutionOptionsDialog(wxWindow* parent):
    wxPanel(parent, ecID_SETTINGS_CONFLICT_RESOLUTION)
{
    CreateControls(this);    

    m_suggestFixes = ((wxGetApp().GetSettings().m_nRuleChecking & ecSettings::SuggestFixes) != 0);
    m_immediate = ((wxGetApp().GetSettings().m_nRuleChecking & ecSettings::Immediate) != 0);
    m_deferred = ((wxGetApp().GetSettings().m_nRuleChecking & ecSettings::Deferred) != 0);

    SetHelpText(_("The conflict resolution options dialog allows you to change options related to conflict resolution."));
}

bool ecConflictResolutionOptionsDialog::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();
    return TRUE;
}

bool ecConflictResolutionOptionsDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    int& ruleChecking = wxGetApp().GetSettings().m_nRuleChecking;

    ruleChecking = 0;
    if (m_suggestFixes)
        ruleChecking |= ecSettings::SuggestFixes ;
    if (m_immediate)
        ruleChecking |= ecSettings::Immediate ;
    if (m_deferred)
        ruleChecking |= ecSettings::Deferred ;

    return TRUE;
}

void ecConflictResolutionOptionsDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("Check for conflicts:") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxVERTICAL );

    wxCheckBox *item3 = new wxCheckBox( parent, ecID_CONFLICT_OPTIONS_AFTER_ITEM_CHANGED, _("After any item changed"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxCheckBox *item4 = new wxCheckBox( parent, ecID_CONFLICT_OPTIONS_BEFORE_SAVING, _("Before &saving configuration"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxCheckBox *item5 = new wxCheckBox( parent, ecID_CONFLICT_OPTIONS_AUTOSUGGEST, _("&Automatically suggest fixes"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );

    // Add context-sensitive help
    item2->SetHelpText(_("Options related to conflict resolution."));
    item3->SetHelpText(_("Check for configuration option conflicts after any configuration option value is changed."));
    item4->SetHelpText(_("Check for configuration option conflicts before saving the configuration."));
    item5->SetHelpText(_("When configuration conflicts are found, provide possible solutions."));

    // Add validators
    FindWindow(ecID_CONFLICT_OPTIONS_AFTER_ITEM_CHANGED)->SetValidator(wxGenericValidator(& m_immediate));
    FindWindow(ecID_CONFLICT_OPTIONS_BEFORE_SAVING)->SetValidator(wxGenericValidator(& m_deferred));
    FindWindow(ecID_CONFLICT_OPTIONS_AUTOSUGGEST)->SetValidator(wxGenericValidator(& m_suggestFixes));
}

/* Run options dialog
 */

IMPLEMENT_CLASS(ecRunOptionsDialog, wxPanel)

BEGIN_EVENT_TABLE(ecRunOptionsDialog, wxPanel)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_DOWNLOAD_TIMEOUT, ecRunOptionsDialog::OnUpdateDownloadTimeout)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_RUNTIME_TIMEOUT, ecRunOptionsDialog::OnUpdateRuntimeTimeout)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_SERIAL_PORT_ADDR, ecRunOptionsDialog::OnUpdateSerial)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_SERIAL_PORT_SPEED, ecRunOptionsDialog::OnUpdateSerial)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_TCPIP_HOST, ecRunOptionsDialog::OnUpdateTCPIP)
    EVT_UPDATE_UI(ecID_RUN_PROPERTIES_TCPIP_PORT, ecRunOptionsDialog::OnUpdateTCPIP)
END_EVENT_TABLE()

ecRunOptionsDialog::ecRunOptionsDialog(wxWindow* parent):
    wxPanel(parent, ecID_SETTINGS_RUN),
    m_serialOn(TRUE), m_TCPIPOn(FALSE)
{
    CreateControls(this);    

    SetHelpText(_("The run properties dialog allows you to change options related to running tests."));
}

bool ecRunOptionsDialog::TransferDataToWindow()
{
#if 0
    // m_strTarget now set in ecConfigToolDoc::RunTests()
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    wxString hardware;
    if (doc)
    {
        hardware = doc->GetCdlConfig ()->get_hardware ().c_str();
    }
    else
    {
         hardware = _("Unknown");
    }
    wxGetApp().GetSettings().GetRunTestsSettings().m_strTarget = hardware;
#endif

    // Serial/TCPIP
    m_serialOn = wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial;
    m_TCPIPOn = !wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial;

    switch (wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType)
    {
    case TIMEOUT_NONE:
        m_downloadTimeoutString = _("None");
        break;
    case TIMEOUT_SPECIFIED:
        m_downloadTimeoutString = _("Specified");
        break;
    default:
    case TIMEOUT_AUTOMATIC:
        m_downloadTimeoutString = _("Calculated from file size");
        break;
    }

    switch (wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType)
    {
    case TIMEOUT_NONE:
        m_runtimeTimeoutString = _("None");
        break;
    case TIMEOUT_SPECIFIED:
        m_runtimeTimeoutString = _("Specified");
        break;
    default:
    case TIMEOUT_AUTOMATIC:
        m_runtimeTimeoutString = _("Default");
        break;
    }

    m_baudString.Printf("%d", wxGetApp().GetSettings().GetRunTestsSettings().m_nBaud);

    wxPanel::TransferDataToWindow();

    return TRUE;
}

bool ecRunOptionsDialog::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    if (m_downloadTimeoutString == _("None"))
        wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType = TIMEOUT_NONE;
    else if (m_downloadTimeoutString == _("Specified"))
        wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType = TIMEOUT_SPECIFIED;
    else
        wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeoutType = TIMEOUT_AUTOMATIC;

    if (m_runtimeTimeoutString == _("None"))
        wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType = TIMEOUT_NONE;
    else if (m_runtimeTimeoutString == _("Specified"))
        wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType = TIMEOUT_SPECIFIED;
    else
        wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeoutType = TIMEOUT_AUTOMATIC;

    wxGetApp().GetSettings().GetRunTestsSettings().m_nBaud = (int) wxAtol(m_baudString);

    // Serial/TCPIP
    wxGetApp().GetSettings().GetRunTestsSettings().m_bSerial = m_serialOn;

    return TRUE;
}

void ecRunOptionsDialog::CreateControls( wxPanel *parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("Platform:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticText *item3 = new wxStaticText( parent, ecID_RUN_PROPERTIES_PLATFORM, _("xxxx"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxTOP, 5 );

    wxStaticBox *item5 = new wxStaticBox( parent, -1, _("Timeouts") );
    wxSizer *item4 = new wxStaticBoxSizer( item5, wxVERTICAL );

    wxSizer *item6 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item7 = new wxStaticText( parent, wxID_STATIC, _("Download:"), wxDefaultPosition, wxSize(60,-1), 0 );
    item6->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs8[] = 
    {
        _("None"), 
        _("Specified"), 
        _("Calculated from file size")
    };
    wxChoice *item8 = new wxChoice( parent, ecID_RUN_PROPERTIES_DOWNLOAD_CHOICE, wxDefaultPosition, wxSize(100,-1), 3, strs8, 0 );
    item6->Add( item8, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxSpinCtrl *item9 = new wxSpinCtrl( parent, ecID_RUN_PROPERTIES_DOWNLOAD_TIMEOUT, "0", wxDefaultPosition, wxSize(50,-1), 0, 0, 10000, 0 );
    item6->Add( item9, 0, wxALIGN_CENTRE|wxALL, 5 );

    item4->Add( item6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item10 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item11 = new wxStaticText( parent, wxID_STATIC, _("Runtime:"), wxDefaultPosition, wxSize(60,-1), 0 );
    item10->Add( item11, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs12[] = 
    {
        _("None"), 
        _("Specified"), 
        _("Default")
    };
    wxChoice *item12 = new wxChoice( parent, ecID_RUN_PROPERTIES_RUNTIME_CHOICE, wxDefaultPosition, wxSize(100,-1), 3, strs12, 0 );
    item10->Add( item12, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxSpinCtrl *item13 = new wxSpinCtrl( parent, ecID_RUN_PROPERTIES_RUNTIME_TIMEOUT, "0", wxDefaultPosition, wxSize(50,-1), 0, 0, 10000, 0 );
    item10->Add( item13, 0, wxALIGN_CENTRE|wxALL, 5 );

    item4->Add( item10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxStaticBox *item15 = new wxStaticBox( parent, -1, _("Connection") );
    wxSizer *item14 = new wxStaticBoxSizer( item15, wxVERTICAL );

    wxSizer *item16 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item17 = new wxRadioButton( parent, ecID_RUN_PROPERTIES_SERIAL, _("&Serial"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    item16->Add( item17, 0, wxALIGN_CENTRE|wxALL, 5 );

    item16->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticText *item18 = new wxStaticText( parent, wxID_STATIC, _("&Port:"), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( item18, 0, wxALIGN_CENTRE|wxLEFT|wxTOP|wxBOTTOM, 5 );

    wxString strs19[] = 
    {
        _("COM1"), 
        _("COM2"), 
        _("COM3"), 
        _("COM4"), 
        _("COM5"), 
        _("COM6"), 
        _("COM7"), 
        _("COM8")
    };
    wxChoice *item19 = new wxChoice( parent, ecID_RUN_PROPERTIES_SERIAL_PORT_ADDR, wxDefaultPosition, wxSize(70,-1), 8, strs19, 0 );
    item16->Add( item19, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticText *item20 = new wxStaticText( parent, wxID_STATIC, _("&Baud:"), wxDefaultPosition, wxDefaultSize, 0 );
    item16->Add( item20, 0, wxALIGN_CENTRE|wxLEFT|wxTOP|wxBOTTOM, 5 );

    wxString strs21[] = 
    {
        _("110"), 
        _("300"), 
        _("600"), 
        _("1200"), 
        _("2400"), 
        _("4800"), 
        _("9600"), 
        _("14400"), 
        _("19200"), 
        _("38400"), 
        _("56000"), 
        _("57600"), 
        _("115200"), 
        _("128000"), 
        _("256000")
    };
    wxChoice *item21 = new wxChoice( parent, ecID_RUN_PROPERTIES_SERIAL_PORT_SPEED, wxDefaultPosition, wxSize(70,-1), 15, strs21, 0 );
    item16->Add( item21, 0, wxALIGN_CENTRE|wxALL, 5 );

    item14->Add( item16, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item22 = new wxBoxSizer( wxHORIZONTAL );

    wxRadioButton *item23 = new wxRadioButton( parent, ecID_RUN_PROPERTIES_TCPIP, _("&TCP/IP"), wxDefaultPosition, wxDefaultSize, 0 );
    item22->Add( item23, 0, wxALIGN_CENTRE|wxALL, 5 );

    item22->Add( 2, 2, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticText *item24 = new wxStaticText( parent, wxID_STATIC, _("&Address:"), wxDefaultPosition, wxDefaultSize, 0 );
    item22->Add( item24, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxTextCtrl *item25 = new wxTextCtrl( parent, ecID_RUN_PROPERTIES_TCPIP_HOST, _(""), wxDefaultPosition, wxSize(100,-1), 0 );
    item22->Add( item25, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxStaticText *item26 = new wxStaticText( parent, wxID_STATIC, _(":"), wxDefaultPosition, wxDefaultSize, 0 );
    item22->Add( item26, 0, wxALIGN_CENTRE, 5 );

    wxTextCtrl *item27 = new wxTextCtrl( parent, ecID_RUN_PROPERTIES_TCPIP_PORT, _(""), wxDefaultPosition, wxSize(40,-1), 0 );
    item22->Add( item27, 0, wxALIGN_CENTRE|wxALL, 5 );

    item14->Add( item22, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );

    // Add context-sensitive help
    FindWindow(ecID_RUN_PROPERTIES_PLATFORM)->SetHelpText(_("Shows the hardware currently selected."));
    FindWindow(ecID_RUN_PROPERTIES_DOWNLOAD_CHOICE)->SetHelpText(_("Specifies the kind of timeout to be applied to the download phase of test execution."));
    FindWindow(ecID_RUN_PROPERTIES_DOWNLOAD_TIMEOUT)->SetHelpText(_("Specifies a fixed-period timeout (in seconds) to be applied to the download phase of test execution."));
    FindWindow(ecID_RUN_PROPERTIES_RUNTIME_CHOICE)->SetHelpText(_("Specifies the kind of timeout to be applied to the run phase of test execution."));
    FindWindow(ecID_RUN_PROPERTIES_RUNTIME_TIMEOUT)->SetHelpText(_("Specifies a fixed-period timeout (in seconds) to be applied to the run phase of test execution."));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL)->SetHelpText(_("Specifies that download is local, using a serial communications port."));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL_PORT_ADDR)->SetHelpText(_("Specifies the communication ports to be used for local download."));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL_PORT_SPEED)->SetHelpText(_("Specifies the baud rate at which the communications port is to operate."));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP)->SetHelpText(_("Specifies that download is remote, using GDB remote protocol via a TCP/IP port."));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP_HOST)->SetHelpText(_("Specifies the TCP/IP host to be used for remote download."));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP_PORT)->SetHelpText(_("Specifies the TCP/IP port number to be used for remote download."));

    // Add validators
    FindWindow(ecID_RUN_PROPERTIES_PLATFORM)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_strTarget));
    FindWindow(ecID_RUN_PROPERTIES_DOWNLOAD_CHOICE)->SetValidator(wxGenericValidator(& m_downloadTimeoutString));
    FindWindow(ecID_RUN_PROPERTIES_DOWNLOAD_TIMEOUT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_nDownloadTimeout));
    FindWindow(ecID_RUN_PROPERTIES_RUNTIME_CHOICE)->SetValidator(wxGenericValidator(& m_runtimeTimeoutString));
    FindWindow(ecID_RUN_PROPERTIES_RUNTIME_TIMEOUT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_nTimeout));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL)->SetValidator(wxGenericValidator(& m_serialOn));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP)->SetValidator(wxGenericValidator(& m_TCPIPOn));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL_PORT_ADDR)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_strPort));
    FindWindow(ecID_RUN_PROPERTIES_SERIAL_PORT_SPEED)->SetValidator(wxGenericValidator(& m_baudString));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP_HOST)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_strLocalTCPIPHost));
    FindWindow(ecID_RUN_PROPERTIES_TCPIP_PORT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().GetRunTestsSettings().m_nLocalTCPIPPort));

}

void ecRunOptionsDialog::OnUpdateDownloadTimeout(wxUpdateUIEvent& event)
{
    wxChoice* downloadTimeoutChoice = (wxChoice*) FindWindow(ecID_RUN_PROPERTIES_DOWNLOAD_CHOICE);
    wxString sel = downloadTimeoutChoice->GetStringSelection();
    event.Enable( sel == _("Specified") );
}

void ecRunOptionsDialog::OnUpdateRuntimeTimeout(wxUpdateUIEvent& event)
{
    wxChoice* runTimeoutChoice = (wxChoice*) FindWindow(ecID_RUN_PROPERTIES_RUNTIME_CHOICE);
    wxString sel = runTimeoutChoice->GetStringSelection();
    event.Enable( sel == _("Specified") );
}

void ecRunOptionsDialog::OnUpdateSerial(wxUpdateUIEvent& event)
{
    wxRadioButton* radio = (wxRadioButton*) FindWindow(ecID_RUN_PROPERTIES_SERIAL);
    event.Enable( radio->GetValue() );
}

void ecRunOptionsDialog::OnUpdateTCPIP(wxUpdateUIEvent& event)
{
    wxRadioButton* radio = (wxRadioButton*) FindWindow(ecID_RUN_PROPERTIES_SERIAL);
    event.Enable( !radio->GetValue() );
}
