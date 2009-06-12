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
// buildoptionsdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: buildoptionsdlg.cpp,v 1.2 2001/03/23 13:38:04 julians Exp $
// Purpose:
// Description: Implementation file for ecBuildOptionsDialog
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
#pragma implementation "buildoptionsdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef __WXGTK__
#include "bitmaps/package_open.xpm"
#include "bitmaps/package_version.xpm"
#endif

#include "wx/cshelp.h"

#include "configtool.h"
#include "configtooldoc.h"
#include "buildoptionsdlg.h"
#include "flags.hxx"

BEGIN_EVENT_TABLE(ecBuildOptionsDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecBuildOptionsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecBuildOptionsDialog::OnCancel)
    EVT_CHOICE(ecID_BUILD_OPTIONS_CATEGORY, ecBuildOptionsDialog::OnSelCategory)
    EVT_TREE_SEL_CHANGED(ecID_BUILD_OPTIONS_PACKAGES_TREE, ecBuildOptionsDialog::OnSelTree)
    EVT_INIT_DIALOG(ecBuildOptionsDialog::OnInitDialog)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecBuildOptionsDialog::ecBuildOptionsDialog(wxWindow* parent):
    m_imageList(16, 16, 1),
    m_arEntries(wxGetApp().GetConfigToolDoc()->GetBuildInfo().entries)

{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_BUILD_OPTIONS_DIALOG, _("Build Options"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    m_imageList.Add(wxICON(package_open));
    m_imageList.Add(wxICON(package_version));
    m_treeCtrl->SetImageList(& m_imageList);

#if 0
    // Add some dummy items
    wxTreeItemId rootId = m_treeCtrl->AddRoot(_("Configuration"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("eCos HAL"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("I/O sub-system"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("Serial device drivers"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("Infrastructure"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("eCos kernel"), 0, -1);
    m_treeCtrl->AppendItem(rootId, _("C library"), 0, -1);

    m_treeCtrl->Expand(rootId);
#endif

    Centre(wxBOTH);
}

ecBuildOptionsDialog::~ecBuildOptionsDialog()
{
    m_treeCtrl->SetImageList(NULL);
}

void ecBuildOptionsDialog::OnInitDialog(wxInitDialogEvent& event)
{
    m_treeCtrl->DeleteAllItems();
    //wxTreeItemId rootId = m_treeCtrl->AddRoot(_("Configuration"), 0, -1);
    CreateItems(wxGetApp().GetConfigToolDoc()->GetFirstItem(), wxTreeItemId());

    m_treeCtrl->Expand(m_treeCtrl->GetRootItem());
    ((wxChoice*) FindWindow(ecID_BUILD_OPTIONS_CATEGORY))->SetSelection(0);
}

void ecBuildOptionsDialog::CreateControls(wxWindow* parent)
{
    m_treeCtrl = new wxTreeCtrl(parent, ecID_BUILD_OPTIONS_PACKAGES_TREE,
        wxDefaultPosition, wxSize(280, 220), wxTR_HAS_BUTTONS | wxSUNKEN_BORDER);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("&Category:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs3[] = 
    {
        _("CFLAGS"), 
        _("LDFLAGS")
    };
    wxChoice *item3 = new wxChoice( parent, ecID_BUILD_OPTIONS_CATEGORY, wxDefaultPosition, wxSize(90,-1), 2, strs3, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 20, 20, 20, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->SetDefault();
    item1->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item6 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item7 = new wxStaticText( parent, wxID_STATIC, _("&Packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item6->Add( item7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item8 = parent->FindWindow( ecID_BUILD_OPTIONS_PACKAGES_TREE );
    wxASSERT( item8 );
    item6->Add( item8, 10, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item5->Add( item6, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

    wxSizer *item9 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item10 = new wxStaticText( parent, wxID_STATIC, _("&Flags:"), wxDefaultPosition, wxDefaultSize, 0 );
    item9->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString *strs11 = (wxString*) NULL;
    wxListBox *item11 = new wxListBox( parent, ecID_BUILD_OPTIONS_FLAGS, wxDefaultPosition, wxSize(170,240), 0, strs11, 0 );
    item9->Add( item11, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item5->Add( item9, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

    item0->Add( item5, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#if 0
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("&Category:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs3[] = 
    {
        _("CFLAGS"), 
        _("LDFLAGS")
    };
    wxChoice *item3 = new wxChoice( parent, ecID_BUILD_OPTIONS_CATEGORY, wxDefaultPosition, wxSize(90,-1), 2, strs3, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( 20, 20, 20, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->SetDefault();
    item1->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item6 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item7 = new wxStaticText( parent, wxID_STATIC, _("&Packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item6->Add( item7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item8 = parent->FindWindow( ecID_BUILD_OPTIONS_PACKAGES_TREE );
    wxASSERT( item8 );
    item6->Add( item8, 10, wxALIGN_CENTRE|wxALL, 5 );

    item5->Add( item6, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

    wxSizer *item9 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item10 = new wxStaticText( parent, wxID_STATIC, _("&Flags:"), wxDefaultPosition, wxDefaultSize, 0 );
    item9->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString *strs11 = (wxString*) NULL;
    wxListBox *item11 = new wxListBox( parent, ecID_BUILD_OPTIONS_FLAGS, wxDefaultPosition, wxSize(170,240), 0, strs11, 0 );
    item9->Add( item11, 0, wxALIGN_CENTRE|wxALL, 5 );

    item5->Add( item9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

    item0->Add( item5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
#endif

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    // Add context-sensitive help text
    parent->FindWindow( ecID_BUILD_OPTIONS_CATEGORY )->SetHelpText(_("Selects the categorty of build flags for which options are to be displayed."));
    parent->FindWindow( ecID_BUILD_OPTIONS_PACKAGES_TREE )->SetHelpText(_("Displays a view of the packages currently included in the configuration."));
    parent->FindWindow( ecID_BUILD_OPTIONS_FLAGS )->SetHelpText(_("Lists the flags defined for the packages selected in the adjacent Packages window."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without saving any changes you have made."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

}

void ecBuildOptionsDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecBuildOptionsDialog::OnOK(wxCommandEvent& event)
{
    event.Skip();
}

void ecBuildOptionsDialog::CreateItems(ecConfigItem *pti, wxTreeItemId hParent)
{
    if (pti->IsPackage() || !hParent.IsOk())
    {
        wxTreeItemId h;

        if (!hParent.IsOk()) // pti->GetItemNameOrMacro() == _("Configuration"))
            h = m_treeCtrl->AddRoot(pti->GetItemNameOrMacro(), 0, -1);
        else
            h = m_treeCtrl->AppendItem(hParent, pti->GetItemNameOrMacro(), 0, -1);

        m_treeCtrl->SetItemData(h, new ecBuildOptionsData(pti));
        // m_Tree.SetItemImage(h,18,18);
        for(ecConfigItem *pChild=pti->FirstChild();pChild;pChild=pChild->NextSibling()){
            CreateItems(pChild,h);
        }
    }
}

void ecBuildOptionsDialog::Redisplay (wxTreeItemId item)
{
    ecConfigItem *pti= ((ecBuildOptionsData*) m_treeCtrl->GetItemData(item))->GetConfigItem();

    const CdlValuable valuable = pti->GetCdlValuable();
    std::string name;
    const CdlBuildInfo_Loadable *pe=NULL;
    if(valuable){
        const char *pszname=valuable->get_name().c_str();
        for(EntriesArray::size_type j=0;j<m_arEntries.size();j++){
            if(0==strcmp(m_arEntries[j].name.c_str(),pszname)){
                pe=&m_arEntries[j];
                break;
            }
        }
    }

    wxString strCat = ((wxChoice*) FindWindow(ecID_BUILD_OPTIONS_CATEGORY))->GetStringSelection();

    const wxString strFlags=get_flags(wxGetApp().GetConfigToolDoc()->GetCdlConfig(), pe, ecUtils::UnicodeToStdStr(strCat)).c_str();

    wxArrayString ar;
    ecUtils::Chop(strFlags,ar, wxT(' '), FALSE, FALSE);

    wxListBox* listBox = (wxListBox*) FindWindow( ecID_BUILD_OPTIONS_FLAGS );
    
    wxString strEdit;
    bool bRedraw=(listBox->GetCount() != ar.GetCount());
    if(!bRedraw)
    {
        for(int i=0;i<ar.GetCount();i++){
            wxString strOld = listBox->GetString(i);

            if(strOld!=ar[i])
            {
                bRedraw = TRUE;
                break;
            }
        }
    }
    if(bRedraw){
        listBox->Clear();
        for(int i=0;i<ar.GetCount();i++){
            listBox->Append(ar[i]);
        }
    }
}

void ecBuildOptionsDialog::OnSelCategory(wxCommandEvent& event)
{
    wxTreeItemId item = m_treeCtrl->GetSelection();
    if (item.IsOk())
    {
        Redisplay(item);
    }
}

void ecBuildOptionsDialog::OnSelTree(wxTreeEvent& event)
{
    wxTreeItemId item = m_treeCtrl->GetSelection();
    if (item.IsOk())
    {
        Redisplay(item);
    }
}
