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
// packages :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians
// Date:        2000/09/28
// Version:     $Id: packagesdlg.cpp,v 1.9 2001/12/14 17:34:03 julians Exp $
// Purpose:
// Description: Implementation file for ecPackagesDialog
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
#pragma implementation "packagesdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/valgen.h"
#include "wx/tokenzr.h"

#include "configtool.h"
#include "packagesdlg.h"
#include "configtooldoc.h"
#include "ecutils.h"

BEGIN_EVENT_TABLE(ecPackagesDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecPackagesDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecPackagesDialog::OnCancel)
    EVT_BUTTON(ecID_PACKAGES_DIALOG_ADD, ecPackagesDialog::OnAdd)
    EVT_BUTTON(ecID_PACKAGES_DIALOG_REMOVE, ecPackagesDialog::OnRemove)
    EVT_LISTBOX_DCLICK(ecID_PACKAGES_DIALOG_AVAILABLE_LIST, ecPackagesDialog::OnDblClickListBox1)
    EVT_LISTBOX_DCLICK(ecID_PACKAGES_DIALOG_USE_LIST, ecPackagesDialog::OnDblClickListBox2)
    EVT_LISTBOX(ecID_PACKAGES_DIALOG_AVAILABLE_LIST, ecPackagesDialog::OnClickListBox1)
    EVT_LISTBOX(ecID_PACKAGES_DIALOG_USE_LIST, ecPackagesDialog::OnClickListBox2)
    EVT_LISTBOX(ecID_PACKAGES_DIALOG_VERSION, ecPackagesDialog::OnSelectVersion)
    EVT_INIT_DIALOG(ecPackagesDialog::OnInitDialog)
    EVT_BUTTON(ecID_PACKAGES_DIALOG_CLEAR, ecPackagesDialog::OnClearKeywords)
    EVT_CHECKBOX(ecID_PACKAGES_DIALOG_OMIT_HARDWARE, ecPackagesDialog::OnClickOmitHardwarePackages)
    EVT_CHECKBOX(ecID_PACKAGES_DIALOG_EXACT_MATCH, ecPackagesDialog::OnClickExactMatch)
    EVT_TEXT(ecID_PACKAGES_DIALOG_KEYWORDS, ecPackagesDialog::OnUpdateKeywordText)
    EVT_IDLE(ecPackagesDialog::OnIdle)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecPackagesDialog::ecPackagesDialog(wxWindow* parent):
m_timer(this)
{
    m_bHardwarePackageSelected = FALSE;
    m_keywords = wxEmptyString;
    m_updateLists = FALSE;
    m_updateInterval = 600; // Milliseconds
    wxStartTimer();
    
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
    
    ecDialog::Create(parent, ecID_PACKAGES_DIALOG, _("Packages"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);
    
    CreateControls(this);
    
    m_timer.Start(200);
    
    Centre(wxBOTH);
}

ecPackagesDialog::~ecPackagesDialog()
{
    m_timer.Stop();
}

// TODO: implement wxLB_SORT style in wxGTK.
void ecPackagesDialog::CreateControls(wxWindow* parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );
    
    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );
    
    wxSizer *item2 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item3 = new wxStaticText( parent, wxID_STATIC, _("Available &packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxString *strs4 = (wxString*) NULL;
    wxListBox *item4 = new wxListBox( parent, ecID_PACKAGES_DIALOG_AVAILABLE_LIST, wxDefaultPosition, wxSize(230,190), 0, strs4, wxLB_SORT|wxLB_HSCROLL );
    item2->Add( item4, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
    
    item1->Add( item2, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
    
    wxSizer *item5 = new wxBoxSizer( wxVERTICAL );
    
    wxButton *item6 = new wxButton( parent, ecID_PACKAGES_DIALOG_ADD, _("&Add >>"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item7 = new wxButton( parent, ecID_PACKAGES_DIALOG_REMOVE, _("<< &Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item1->Add( item5, 0, wxALIGN_CENTRE|wxALL, 0 );
    
    wxSizer *item8 = new wxBoxSizer( wxVERTICAL );
    
    wxStaticText *item9 = new wxStaticText( parent, wxID_STATIC, _("&Use these packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxString *strs10 = (wxString*) NULL;
    wxListBox *item10 = new wxListBox( parent, ecID_PACKAGES_DIALOG_USE_LIST, wxDefaultPosition, wxSize(230,190), 0, strs10, wxLB_SORT|wxLB_HSCROLL );
    item8->Add( item10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
    
    item1->Add( item8, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
    
    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxStaticText *item11 = new wxStaticText( parent, wxID_STATIC, _("&Version:"), wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item11, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 10 );
    
    wxString *strs12 = (wxString*) NULL;
    wxChoice *item12 = new wxChoice( parent, ecID_PACKAGES_DIALOG_VERSION, wxDefaultPosition, wxSize(100,-1), 0, strs12, 0 );
    item0->Add( item12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 10 );
    
    wxTextCtrl *item13 = new wxTextCtrl( parent, ecID_PACKAGES_DIALOG_DESCRIPTION, _(""), wxDefaultPosition, wxSize(80,110), wxTE_MULTILINE );
    item0->Add( item13, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 10 );
    
    wxStaticText *item14 = new wxStaticText( parent, wxID_STATIC, _("&Keywords:"), wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item14, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 10 );
    
    wxSizer *item15 = new wxBoxSizer( wxHORIZONTAL );
    
    wxTextCtrl *item16 = new wxTextCtrl( parent, ecID_PACKAGES_DIALOG_KEYWORDS, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item15->Add( item16, 1, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item17 = new wxButton( parent, ecID_PACKAGES_DIALOG_CLEAR, _("C&lear"), wxDefaultPosition, wxDefaultSize, 0 );
    item15->Add( item17, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item0->Add( item15, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
    
    wxSizer *item18 = new wxBoxSizer( wxHORIZONTAL );
    
    wxCheckBox *item19 = new wxCheckBox( parent, ecID_PACKAGES_DIALOG_OMIT_HARDWARE, _("&Omit hardware packages"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->Add( item19, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    wxCheckBox *item20 = new wxCheckBox( parent, ecID_PACKAGES_DIALOG_EXACT_MATCH, _("&Match exactly"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->Add( item20, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item18->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item21 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item21->SetDefault();
    item18->Add( item21, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item22 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->Add( item22, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item0->Add( item18, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
    
#if 0
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );
    
    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );
    
    wxSizer *item2 = new wxBoxSizer( wxVERTICAL );
    
    wxStaticText *item3 = new wxStaticText( parent, wxID_STATIC, _("Available &packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxString *strs4 = (wxString*) NULL;
    wxListBox *item4 = new wxListBox( parent, ecID_PACKAGES_DIALOG_AVAILABLE_LIST, wxDefaultPosition, wxSize(230,190), 0, strs4, wxLB_SORT|wxLB_HSCROLL );
    item2->Add( item4, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
    
    item1->Add( item2, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
    
    wxSizer *item5 = new wxBoxSizer( wxVERTICAL );
    
    wxButton *item6 = new wxButton( parent, ecID_PACKAGES_DIALOG_ADD, _("&Add >>"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item7 = new wxButton( parent, ecID_PACKAGES_DIALOG_REMOVE, _("<< &Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item5, 0, wxALIGN_CENTRE|wxALL, 0 );
    
    wxSizer *item8 = new wxBoxSizer( wxVERTICAL );
    
    wxStaticText *item9 = new wxStaticText( parent, wxID_STATIC, _("&Use these packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item8->Add( item9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxString *strs10 = (wxString*) NULL;
    wxListBox *item10 = new wxListBox( parent, ecID_PACKAGES_DIALOG_USE_LIST, wxDefaultPosition, wxSize(230,190), 0, strs10, wxLB_SORT|wxLB_HSCROLL );
    item8->Add( item10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
    
    item1->Add( item8, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
    
    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    
    wxStaticText *item11 = new wxStaticText( parent, wxID_STATIC, _("&Version:"), wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item11, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 10 );
    
    wxString *strs12 = (wxString*) NULL;
    wxChoice *item12 = new wxChoice( parent, ecID_PACKAGES_DIALOG_VERSION, wxDefaultPosition, wxSize(100,-1), 0, strs12, 0 );
    item0->Add( item12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 10 );
    
    wxTextCtrl *item13 = new wxTextCtrl( parent, ecID_PACKAGES_DIALOG_DESCRIPTION, _(""), wxDefaultPosition, wxSize(80,110), wxTE_MULTILINE );
    item0->Add( item13, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 10 );
    
    wxStaticText *item14 = new wxStaticText( parent, wxID_STATIC, _("&Keywords:"), wxDefaultPosition, wxDefaultSize, 0 );
    item0->Add( item14, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 10 );
    
    wxSizer *item15 = new wxBoxSizer( wxHORIZONTAL );
    
    wxTextCtrl *item16 = new wxTextCtrl( parent, ecID_PACKAGES_DIALOG_KEYWORDS, _(""), wxDefaultPosition, wxSize(80,-1), 0 );
    item15->Add( item16, 1, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item17 = new wxButton( parent, ecID_PACKAGES_DIALOG_CLEAR, _("C&lear"), wxDefaultPosition, wxDefaultSize, 0 );
    item15->Add( item17, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item0->Add( item15, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
    
    wxSizer *item18 = new wxBoxSizer( wxHORIZONTAL );
    
    wxCheckBox *item19 = new wxCheckBox( parent, ecID_PACKAGES_DIALOG_OMIT_HARDWARE, _("Omit hardware packages"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->Add( item19, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item18->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );
    
    wxButton *item20 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item20->SetDefault();
    item18->Add( item20, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item21 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item18->Add( item21, 0, wxALIGN_CENTRE|wxALL, 5 );
    
    item0->Add( item18, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
#endif
    
#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item18->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif
    
    parent->FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS )->SetFocus();
    
    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    //item0->SetSizeHints( parent );
    
    // Add context-sensitive help text
    parent->FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST )->SetHelpText(_("Displays the list of packages available, but not currently loaded."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_USE_LIST )->SetHelpText(_("Displays the list of packages currently loaded."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_ADD )->SetHelpText(_("Add one or more packages to the list to be loaded."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_REMOVE )->SetHelpText(_("Removes one or more packages from the list to be loaded."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_VERSION )->SetHelpText(_("Displays the version of the selected packages."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_DESCRIPTION )->SetHelpText(_("Displays a description of the selected package (blank if more than one package is selected)."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS )->SetHelpText(_("Enter keywords here to restrict displayed packages."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_CLEAR )->SetHelpText(_("Clears the keyword field."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_OMIT_HARDWARE)->SetHelpText(_("Check this to omit hardware packages, uncheck to show all packages."));
    parent->FindWindow( ecID_PACKAGES_DIALOG_EXACT_MATCH )->SetHelpText(_("Check this to display exact matches between keyword and aliases (case insensitive)."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without saving any changes you have made."));
    
#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif
    
    // Add validators
    parent->FindWindow( ecID_PACKAGES_DIALOG_DESCRIPTION )->SetValidator(wxGenericValidator(& m_packageDescription));
    parent->FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS )->SetValidator(wxGenericValidator(& m_keywords));
    parent->FindWindow( ecID_PACKAGES_DIALOG_OMIT_HARDWARE )->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_omitHardwarePackages));
    parent->FindWindow( ecID_PACKAGES_DIALOG_EXACT_MATCH )->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_matchPackageNamesExactly));
}

void ecPackagesDialog::OnInitDialog(wxInitDialogEvent& event)
{
    // Note: InitControls must be here, because data will be added
    // between construction of the dialog, and OnInitDialog.
    InitControls();
    TransferDataToWindow();
}

void ecPackagesDialog::InitControls()
{
    Fill();
}

void ecPackagesDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecPackagesDialog::OnOK(wxCommandEvent& event)
{
    TransferDataFromWindow();
    event.Skip();
}

// For each word in keywords, is it contained in 'str'?
bool ecPackagesDialog::MatchesKeyword(wxArrayString& keywords, const wxString& str)
{
    // _Every_ keyword must match
    size_t i;
    for (i = 0; i < keywords.GetCount(); i++)
    {
        if (str.Find(keywords[i]) == -1)
            return FALSE;
    }
    return TRUE;
}

void ecPackagesDialog::Fill()
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    ecConfigToolDoc * pDoc = wxGetApp().GetConfigToolDoc ();

    // wxGTK doesn't deselect items properly when clearing, I think
    int i;
    for (i = 0; i < availableList->GetCount(); i++)
        if (availableList->Selected(i))
            availableList->Deselect(i);
    for (i = 0; i < useList->GetCount(); i++)
        if (useList->Selected(i))
            useList->Deselect(i);
    
    availableList->Clear();
    useList->Clear();
    ClearDescription();
    
    wxString s2(m_keywords);
    s2.MakeLower();
    
    // Tokenize
    wxArrayString keywords;
    wxStringTokenizer tok(s2);
    while (tok.HasMoreTokens())
    {
        keywords.Add(tok.GetNextToken());
    }
    
    // Initialize the controls
    for (i = 0; i < GetCount(); i++)
    {
        const wxString& str = m_items[i];
        wxListBox* lb = m_arnItems[i] ? useList : availableList;
        
        wxString macroName(pDoc->GetPackageName (str));
        
        // check if the package is a hardware package
        
        if ((!wxGetApp().GetSettings().m_omitHardwarePackages) || !pDoc->GetCdlPkgData ()->is_hardware_package (ecUtils::UnicodeToStdStr (macroName)))
        {
            bool matches = TRUE;
            
            if (!m_keywords.IsEmpty())
            {
                // Descriptive name
                wxString s1(str);
                s1.MakeLower();

                // macro name
                wxString s3(macroName);
                s3.MakeLower();

                // Match all aliases
                const std::vector<std::string> & aliases = pDoc->GetCdlPkgData ()->get_package_aliases (ecUtils::UnicodeToStdStr (macroName));
                    
                if (wxGetApp().GetSettings().m_matchPackageNamesExactly)
                {
                    int noMatches = 0;

                    if (s2 == s1 || s2 == s3)
                        noMatches ++;

                    size_t j;
                    for (j = 0; j < aliases.size(); j ++)
                    {
                        wxString alias(aliases[j].c_str());
                        alias.MakeLower();
                        if (s2 == alias)
                            noMatches ++;
                    }

                    matches = (noMatches > 0);                   

                }
                else
                {
                    // Concatenate all possible text together, and match against that
                    wxString toMatch;
                    
                    toMatch += s1;
                    
                    toMatch += s3;
                    
                    size_t j;
                    for (j = 0; j < aliases.size(); j ++)
                    {
                        wxString alias(aliases[j].c_str());
                        alias.MakeLower();
                        toMatch += alias;
                    }
                    
                    matches = MatchesKeyword(keywords, toMatch);
                }
            }
            
            if (matches)
                lb->Append(str, (void*) i);
        }
    }
    
    UpdateAddRemoveButtons();
    
    if (availableList->GetCount() == 1)
    {
        availableList->SetSelection(0);
        UpdatePackageDescription();
        UpdateVersionList ();
        UpdateHardwareSelectionFlag ();
    }
    else if (useList->GetCount() == 1)
    {
        useList->SetSelection(0);
        UpdatePackageDescription();
        UpdateVersionList ();
        UpdateHardwareSelectionFlag ();
    }
    wxTextCtrl* textCtrl = (wxTextCtrl*) FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS );
    // Necessary or TransferDataToWindow will cause insertion position to change
    textCtrl->SetInsertionPointEnd();
}

void ecPackagesDialog::Add(wxListBox* from, wxListBox* to)
{
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    wxArrayInt selections;
    wxStringList selectionsStrings;
    int n = from -> GetSelections( selections );
    
    if (n > 0)
    {
        int i;
        for (i = 0; i < selections.GetCount(); i++)
        {
            wxString str = from -> GetString(selections[i]);
            selectionsStrings.Add(str);
        }
        
        // Now delete from one list and remove from t'other
        for (i = 0; i < selectionsStrings.Number(); i++)
        {
            wxString str = selectionsStrings[i];
            
            // Remove
            int toDelete =  from -> FindString(str);
            int itemIndex = -1;
            if (toDelete > -1)
            {
                itemIndex = (int) from -> GetClientData(toDelete);
                from -> Delete(toDelete);
            }
            
            wxASSERT (itemIndex > -1);
            
            // Add
            to -> Append(str, (void*) itemIndex);
            
            if (to == useList)
            {
                m_added.Add(str);
            }
            else
            {
                m_added.Remove(str);
            }
            
            // Select it
            int addedIndex = to->FindString(str);
            to->Select(addedIndex);
            
        }
        //ClearDescription();
        ClearSelections(* from);
        UpdateHardwareSelectionFlag();
        UpdatePackageDescription();
        to->SetFocus();
    }
}

void ecPackagesDialog::OnAdd(wxCommandEvent& event)
{
    if (m_bHardwarePackageSelected)
    {
        HardwarePackageMessageBox ();
        return;
    }
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    Add(availableList, useList);
    
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnRemove(wxCommandEvent& event)
{
    if (m_bHardwarePackageSelected)
    {
        HardwarePackageMessageBox ();
        return;
    }
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    Add(useList, availableList);
    
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnDblClickListBox1(wxCommandEvent& event)
{
    if (m_bHardwarePackageSelected)
    {
        HardwarePackageMessageBox ();
        return;
    }
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    Add(availableList, useList);
    
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnDblClickListBox2(wxCommandEvent& event)
{
    if (m_bHardwarePackageSelected)
    {
        HardwarePackageMessageBox ();
        return;
    }
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    Add(useList, availableList);
    
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnClickListBox1(wxCommandEvent& event)
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
#if 0    
    int sel = event.GetSelection();
    if (sel > -1)
    {
        // TODO: check that this works for multiple-selection listboxes
        DisplayDescription(availableList->GetString(sel));
    }
#endif
    
    ClearSelections(*useList);
    UpdatePackageDescription ();
    UpdateVersionList ();
    UpdateHardwareSelectionFlag ();
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnClickListBox2(wxCommandEvent& event)
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
#if 0    
    int sel = event.GetSelection();
    if (sel > -1)
    {
        // TODO: check that this works for multiple-selection listboxes
        DisplayDescription(useList->GetString(sel));
    }
#endif
    
    ClearSelections(*availableList);
    UpdatePackageDescription ();
    UpdateVersionList ();
    UpdateHardwareSelectionFlag ();
    UpdateAddRemoveButtons();
}

void ecPackagesDialog::OnSelectVersion(wxCommandEvent& event)
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    wxChoice* versionChoice = (wxChoice*) FindWindow( ecID_PACKAGES_DIALOG_VERSION );
    
    if (-1 == versionChoice->GetSelection ()) // if there is no version selection
        return; // do nothing
    
    wxListBox * pListBox = NULL;
    
    wxArrayInt selected1, selected2;
    availableList->GetSelections(selected1);
    useList->GetSelections(selected2);
    
    int nListSelCount = selected1.GetCount ();
    if (nListSelCount > 0)
    {
        pListBox = availableList;
    }
    else
    {
        nListSelCount = selected2.GetCount ();
        if (nListSelCount)
            pListBox = useList;
    }
    
    wxASSERT (pListBox);
    
    if (!pListBox)
        return;
    
    // retrieve the list box indices of the selected packages
    
    wxArrayInt* selected = (pListBox == availableList ? & selected1 : & selected2);
    
    int nIndex;
    for (nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
    {
        // set the package version to that specified in the version combo box
        wxString str = versionChoice->GetString(nIndex);
        
        // itemIndex is the index into the list of item names. It gets stored with all the listbox items.
        int itemIndex = (int) pListBox->GetClientData((*selected)[nIndex]);
        m_currentVersions[(size_t)itemIndex] = str;
    }
}

void ecPackagesDialog::OnClearKeywords(wxCommandEvent& event)
{
    wxTextCtrl* textCtrl = (wxTextCtrl*) FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS );
    textCtrl->SetValue(wxT(""));
    TransferDataFromWindow();
    Fill();
    m_updateLists = FALSE;
    wxStartTimer();
    FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS )->SetFocus();
}

void ecPackagesDialog::OnUpdateKeywordText(wxCommandEvent& event)
{
    // Work around a bug in GTK+ that sends a text update command when
    // clicking on one of the listboxes.
    wxTextCtrl* textCtrl = (wxTextCtrl*) FindWindow( ecID_PACKAGES_DIALOG_KEYWORDS );
    wxString value = textCtrl->GetValue();
    if (value == m_keywords)
        return;
    
    TransferDataFromWindow();
    m_updateLists = TRUE;
    wxStartTimer();
}

void ecPackagesDialog::OnClickOmitHardwarePackages(wxCommandEvent& event)
{
    TransferDataFromWindow();
    Fill();
}

void ecPackagesDialog::OnClickExactMatch(wxCommandEvent& event)
{
    TransferDataFromWindow();
    Fill();
}

void ecPackagesDialog::OnIdle(wxIdleEvent& event)
{
    long elapsed = wxGetElapsedTime(FALSE);
    if (m_updateLists && (elapsed > m_updateInterval))
    {
        m_updateLists = FALSE;
        Fill();
        wxStartTimer();
    }
}

void ecPackagesDialog::Insert(const wxString& str, bool added, const wxString& descr, const wxString& version)
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    m_items.Add(str);
    m_descriptions.Add(str);
    m_currentVersions.Add(version);
    m_arnItems.Add(added);
    
    if (added)
        m_added.Add(str);
    
    //(added ? useList : availableList) -> Append(str);
}

bool ecPackagesDialog::IsAdded(const wxString& str)
{
    return (m_added.Index(str) != wxNOT_FOUND);
    
    //	return (((wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST ))->FindString(str) > -1) ;
}

static int ecPositionInStringList(const wxStringList& list, const wxString& item)
{
    int i;
    for (i = 0 ; i < list.GetCount(); i++)
        if (list[i] == item)
            return i;
        else
            i ++;
        return -1;
}

void ecPackagesDialog::DisplayDescription(const wxString& item)
{
    //wxTextCtrl* descrCtrl = (wxTextCtrl*) FindWindow( ecID_PACKAGES_DIALOG_DESCRIPTION ) ;
    ecConfigToolDoc * pDoc = wxGetApp().GetConfigToolDoc ();
    
    //    int pos = ecPositionInStringList(m_items, item);
    //    if (pos > -1)
    {
        wxString text;
        //        wxString descr = m_descriptions[pos];
        //        text += descr;
        //        text += wxT("\n");
        
        wxString macroName(pDoc->GetPackageName (item));
        
        // Match all aliases
        const std::vector<std::string> & aliases = pDoc->GetCdlPkgData ()->get_package_aliases (ecUtils::UnicodeToStdStr (macroName));
        
        size_t j;
        for (j = 0; j < aliases.size(); j ++)
        {
            if (j == 1)
                text += wxT(".\nAliases: ");
            else if (j > 1)
                text += wxT(", ");
            
            wxString alias(aliases[j].c_str());
            text += alias;
        }
        text += wxT("\nMacro: ");
        text += macroName;
        text += wxT("\n\n");
        
        wxString descr = pDoc->GetCdlPkgData ()->get_package_description (ecUtils::UnicodeToStdStr (macroName)).c_str ();
        
        text += ecUtils::StripExtraWhitespace (descr);
        
        m_packageDescription = text;
        
        //descrCtrl->SetValue(text);
    }
}

void ecPackagesDialog::ClearDescription()
{
    wxTextCtrl* descrCtrl = (wxTextCtrl*) FindWindow( ecID_PACKAGES_DIALOG_DESCRIPTION ) ;
    descrCtrl->SetValue(wxEmptyString);
}

wxString ecPackagesDialog::GetVersion (const wxString& item)
{
    int nCount;
    for (nCount = GetCount() - 1; nCount >= 0; --nCount)
    {
        if (m_items [nCount] == item)
        {
            return m_currentVersions [nCount];
        }
    }
    wxASSERT (FALSE);
    return wxEmptyString;
}

void ecPackagesDialog::UpdateHardwareSelectionFlag()
{
    m_bHardwarePackageSelected = FALSE;
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    wxListBox * pListBox = NULL;
    
    wxArrayInt selections1, selections2;
    wxArrayInt* selections = NULL;
    availableList->GetSelections(selections1);
    useList->GetSelections(selections2);
    
    int nListSelCount = selections1.GetCount ();
    if (nListSelCount)
    {
        pListBox = availableList;
        selections = & selections1;
    }
    else
    {
        nListSelCount = selections2.GetCount ();
        if (nListSelCount)
        {
            pListBox = useList;
            selections = & selections2;
        }
    }
    
    if (pListBox) // if there are packages selected
    {
        ecConfigToolDoc * pDoc = wxGetApp().GetConfigToolDoc ();
        
        // retrieve the list box indices of the selected packages
        
        //int * arnIndices = new int [nListSelCount];
        //pListBox->GetSelItems (nListSelCount, arnIndices);
        
        int nIndex;
        for (nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
        {
            wxString strPackageAlias = pListBox->GetString((*selections)[nIndex]);
            
            // check if the package is a hardware package
            
            //TRACE (_T("Checking '%s' for hardware status\n"), strPackageAlias);
            if (pDoc->GetCdlPkgData ()->is_hardware_package (ecUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias))))
            {
                m_bHardwarePackageSelected = TRUE;
                break;
            }
        }
    }
}

void ecPackagesDialog::HardwarePackageMessageBox()
{
    // TODO: could give the user the choice of going to the template dialog.
    wxMessageBox (wxT("Add and remove hardware packages by selecting a new hardware template."),
        wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
}

void ecPackagesDialog::UpdatePackageDescription ()
{
    ecConfigToolDoc * pDoc = wxGetApp().GetConfigToolDoc ();
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    wxListBox * pListBox = NULL;
    
    wxArrayInt selections1, selections2;
    wxArrayInt* selections = NULL;
    availableList->GetSelections(selections1);
    useList->GetSelections(selections2);
    
    if (1 == selections1.GetCount ())
    {
        pListBox = availableList;
        selections = & selections1;
    }
    else if (1 == selections2.GetCount ())
    {
        pListBox = useList;
        selections = & selections2;
    }
    
    if (pListBox && selections)
    {
        int nIndex = (*selections)[0];
        wxString strPackageAlias = pListBox->GetString(nIndex);
        
        DisplayDescription(strPackageAlias);
        //m_packageDescription = pDoc->GetCdlPkgData ()->get_package_description (ecUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias))).c_str ();
        //m_packageDescription = ecUtils::StripExtraWhitespace (m_packageDescription);
    }
    else
    {
        m_packageDescription = wxEmptyString;
    }
    TransferDataToWindow ();
}

void ecPackagesDialog::UpdateVersionList ()
{
    ecConfigToolDoc * pDoc = wxGetApp().GetConfigToolDoc ();
    
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    wxChoice* versionChoice = (wxChoice*) FindWindow( ecID_PACKAGES_DIALOG_VERSION );
    
    versionChoice->Clear(); // clear the version combo box
    
    wxArrayInt selections1, selections2;
    wxArrayInt* selections = NULL;
    availableList->GetSelections(selections1);
    useList->GetSelections(selections2);
    wxListBox* pListBox = NULL;
    
    if (selections1.GetCount () > 0)
    {
        pListBox = availableList;
        selections = & selections1;
    }
    else if (selections2.GetCount () > 0)
    {
        pListBox = useList;
        selections = & selections2;
    }
    
    if (pListBox) // if there are packages selected
    {
        std::list<std::string> common_versions;
        bool bCommonSelectedVersion = true;
        int nCommonVersionIndex=-1;
        int nListSelCount = selections->GetCount();
        
        // retrieve the list box indices of the selected packages
        
        //int * arnIndices = new int [nListSelCount];
        //pListBox->GetSelItems (nListSelCount, arnIndices);
        int nIndex;
        for (nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
        {
            // retrieve the first package alias
            wxString strPackageAlias = pListBox->GetString ((*selections)[nIndex]);
            
            // retrieve the dialog item array index for use in
            // comparing current version strings
            const int nVersionIndex = (int) pListBox->GetClientData ((*selections)[nIndex]);
            
            // retrieve the installed version array
            
            //TRACE (_T("Retrieving versions for '%s'\n"), strPackageAlias);
            const std::vector<std::string>& versions = pDoc->GetCdlPkgData ()->get_package_versions (ecUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias)));
            
            if (0 == nIndex) // if this is the first selected package
            {
                // use the version array to initialise a linked list of version
                // strings held in common between the selected packages
                unsigned int uCount;
                for (uCount = 0; uCount < versions.size (); uCount++)
                {
                    //TRACE (_T("Adding common version '%s'\n"), wxString (versions [uCount].c_str ()));
                    common_versions.push_back (versions [uCount]);
                }
                nCommonVersionIndex = nVersionIndex; // save the item array index
            }
            else // this is not the first selected package
            {
                std::list<std::string>::iterator i_common_versions = common_versions.begin ();
                while (i_common_versions != common_versions.end ()) // iterate through the common versions
                {
                    if (versions.end () == std::find (versions.begin (), versions.end (), * i_common_versions)) // if the common version is not in the versions list
                    {
                        //TRACE (_T("Removing common version '%s'\n"), CString (i_common_versions->c_str ()));
                        common_versions.erase (i_common_versions++); // remove the version from the common versions list
                    }
                    else
                    {
                        i_common_versions++;
                    }
                }
                if (bCommonSelectedVersion) // if the selected versions of all preceding packages are identical
                {
                    // check if the selected version of this package matches that of the preceding ones
                    bCommonSelectedVersion = (m_currentVersions [nVersionIndex] == m_currentVersions [nCommonVersionIndex]);
                }
            }
        }
        
        // add the common versions to the version combo box
        
        std::list<std::string>::iterator i_common_versions;
        for (i_common_versions = common_versions.begin (); i_common_versions != common_versions.end (); i_common_versions++)
        {
            //TRACE (_T("Adding version '%s'\n"), CString (i_common_versions->c_str ()));
            versionChoice->Append(wxString (i_common_versions->c_str ()));
        }
        
        // select the common current version (if any) in the version combo box
        
        if (bCommonSelectedVersion)
        {
            //TRACE (_T("Selecting version '%s'\n"), m_arstrVersions [nCommonVersionIndex]);
            versionChoice->SetStringSelection (m_currentVersions [nCommonVersionIndex]);
        }
        
        // enable the version combo box only if there are multiple common versions
        
        versionChoice->Enable (common_versions.size () > 1);
    }
    else // there are no packages selected
    {
        versionChoice->Enable (FALSE); // disable the version combo box
    }
}

void ecPackagesDialog::UpdateAddRemoveButtons()
{
    wxListBox* availableList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_AVAILABLE_LIST );
    wxListBox* useList = (wxListBox*) FindWindow( ecID_PACKAGES_DIALOG_USE_LIST );
    
    wxArrayInt selections1, selections2;
    availableList->GetSelections(selections1);
    useList->GetSelections(selections2);
    
    FindWindow( ecID_PACKAGES_DIALOG_ADD )->Enable( selections1.GetCount() > 0 );
    FindWindow( ecID_PACKAGES_DIALOG_REMOVE )->Enable( selections2.GetCount() > 0 );
}

void ecPackagesDialog::ClearSelections(wxListBox& lbox)
{
    int i;
    for (i = 0; i < lbox.GetCount(); i++)
    {
        lbox.Deselect(i);
    }
}

void ecPackagesTimer::Notify()
{
    static bool s_inNotify = FALSE;
    
    if (s_inNotify)
        return;
    
    s_inNotify = TRUE;
    
    // On Windows, simply having the timer going will ping the message queue
    // and cause idle processing to happen.
    // On Unix, this doesn't happen so we have to do the processing explicitly.
#ifdef __WXMSW__
    // Nothing to do
#else
    if ( m_dialog )
    {
        wxIdleEvent event;
        m_dialog->OnIdle(event);
    }
#endif
    
    s_inNotify = FALSE;
}
