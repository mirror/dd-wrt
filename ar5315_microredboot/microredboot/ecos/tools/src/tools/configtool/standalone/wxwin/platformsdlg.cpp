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
// Version:     $Id: platformsdlg.cpp,v 1.2 2001/03/23 13:38:04 julians Exp $
// Purpose:
// Description: Implementation file for the ecPlatformsDialog
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
#pragma implementation "platformsdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"

#include "configtool.h"
#include "platformsdlg.h"
#include "platformeditordlg.h"

#ifdef __WXMSW__
#include "wx/msw/winundef.h"
#endif

BEGIN_EVENT_TABLE(ecPlatformsDialog, ecDialog)
    EVT_BUTTON(ecID_PLATFORMS_MODIFY, ecPlatformsDialog::OnModify)
    EVT_BUTTON(ecID_PLATFORMS_ADD, ecPlatformsDialog::OnAdd)
    EVT_BUTTON(ecID_PLATFORMS_DELETE, ecPlatformsDialog::OnDelete)
    EVT_BUTTON(wxID_OK, ecPlatformsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecPlatformsDialog::OnCancel)
    EVT_UPDATE_UI(ecID_PLATFORMS_MODIFY, ecPlatformsDialog::OnUpdateAny)
    EVT_UPDATE_UI(ecID_PLATFORMS_DELETE, ecPlatformsDialog::OnUpdateAny)
END_EVENT_TABLE()

const wxChar* ecPlatformsDialog::sm_arpszTypes[]={
  wxT("Hardware with breakpoint support"),
  wxT("Simulator"),
  wxT("Synthetic target"),
  wxT("Hardware without breakpoint support"),
  wxT("Remote simulator")
};


// Frame constructor
ecPlatformsDialog::ecPlatformsDialog(wxWindow* parent)
{
    m_listCtrl = NULL;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_PLATFORMS_DIALOG, _("Platforms"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    Centre(wxBOTH);

    unsigned int i;
    for(i=0; i < CeCosTestPlatform::Count(); i++)
    {
        Add(*CeCosTestPlatform::Get(i));
    }
}

ecPlatformsDialog::~ecPlatformsDialog()
{
    Clear();
}

void ecPlatformsDialog::Clear()
{
    wxNode* node = m_arTargetInfo.First();
    while (node)
    {
        CeCosTestPlatform* platform = (CeCosTestPlatform*) node->Data();
        delete platform;
        node = node->Next();
    }
    m_arTargetInfo.Clear();
}

void ecPlatformsDialog::CreateControls(wxWindow* parent)
{
    // Create custom windows first
    m_listCtrl = new ecPlatformsListCtrl(parent, ecID_PLATFORMS_LIST, wxDefaultPosition, wxSize(450, 300), wxLC_REPORT|wxCLIP_CHILDREN|wxSUNKEN_BORDER);
    m_listCtrl->InsertColumn(0, _("Target"), wxLIST_FORMAT_LEFT, 70);
    m_listCtrl->InsertColumn(1, _("Prefix"), wxLIST_FORMAT_LEFT, 70);
    m_listCtrl->InsertColumn(2, _("Commands"), wxLIST_FORMAT_LEFT, 70);
    m_listCtrl->InsertColumn(3, _("Inferior"), wxLIST_FORMAT_LEFT, 70);
    m_listCtrl->InsertColumn(4, _("Prompt"), wxLIST_FORMAT_LEFT, 70);
    m_listCtrl->InsertColumn(5, _("ServerSideGdb"), wxLIST_FORMAT_LEFT, 80);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item2 = new wxButton( parent, ecID_PLATFORMS_MODIFY, _("&Modify..."), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item3 = new wxButton( parent, ecID_PLATFORMS_ADD, _("&Add..."), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, ecID_PLATFORMS_DELETE, _("&Delete..."), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item5 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxWindow *item7 = parent->FindWindow( ecID_PLATFORMS_LIST );
    wxASSERT( item7 );
    item0->Add( item7, 2, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#if 0
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item2 = new wxButton( parent, ecID_PLATFORMS_MODIFY, "&Modify...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item3 = new wxButton( parent, ecID_PLATFORMS_ADD, "&Add...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item4 = new wxButton( parent, ecID_PLATFORMS_DELETE, "&Delete...", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item4, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item5 = new wxButton( parent, wxID_OK, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, wxID_CANCEL, "&Cancel", wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 0 );

    wxWindow *item7 = parent->FindWindow( ecID_PLATFORMS_LIST );
    wxASSERT( item7 );
    item0->Add( item7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
#endif

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    ((wxButton*) FindWindow(wxID_OK))->SetDefault();

    // Add context-sensitive help text
    parent->FindWindow( ecID_PLATFORMS_LIST )->SetHelpText(_("Displays the list of platforms."));
    parent->FindWindow( ecID_PLATFORMS_MODIFY )->SetHelpText(_("Changes the characteristics of the currently selected platform."));
    parent->FindWindow( ecID_PLATFORMS_ADD )->SetHelpText(_("Adds a new platform."));
    parent->FindWindow( ecID_PLATFORMS_DELETE )->SetHelpText(_("Removes the currently selected platform."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without saving any changes you have made."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif
}

void ecPlatformsDialog::OnModify(wxCommandEvent& event)
{
    long n = m_listCtrl->GetItemCount();
    long i;
    for (i = 0; i < n; i++)
    {
        if (m_listCtrl->GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
        {
            CeCosTestPlatform *pti=Platform(i);

            ecPlatformEditorDialog dlg(this);

            dlg.m_strPlatform = pti->Name();
            dlg.m_strPrefix = pti->Prefix();
            dlg.m_strGDB = pti->GdbCmds();
            dlg.m_strCaption = wxT("Modify Platform");
            dlg.m_strPrompt = pti->Prompt();
            dlg.m_bServerSideGdb = pti->ServerSideGdb();
            dlg.m_strInferior = pti->Inferior();

            if ( wxID_CANCEL != dlg.ShowModal() )
            {
                *pti = CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior);
                m_listCtrl->SetItem(i, 1, pti->Prefix());
                m_listCtrl->SetItem(i, 2, pti->GdbCmds());
                m_listCtrl->SetItem(i, 3, pti->Inferior());
                m_listCtrl->SetItem(i, 4, pti->Prompt());
                m_listCtrl->SetItem(i, 5, pti->ServerSideGdb() ? wxT("Y") : wxT("N"));
            }            
        }
    }
}

static long ecFindListCtrlSelection(long& whereFrom, wxListCtrl* listCtrl)
{
    long n = listCtrl->GetItemCount();
    long i;
    for (i = whereFrom; i < n; i++)
    {
        if (listCtrl->GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
        {
            whereFrom = i+1;
            return i;
        }
    }
    return -1;
}

void ecPlatformsDialog::OnDelete(wxCommandEvent& event)
{
    long sel = -1;
    long whereFrom = 0;
    do
    {
        sel = ecFindListCtrlSelection(whereFrom, m_listCtrl);
        if (sel > -1)
        {
            if (wxYES == wxMessageBox(wxT("Are you sure you wish to delete this platform?"), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxYES_NO, this))
            {
                delete Platform(sel);
                m_listCtrl->DeleteItem(sel);
                delete m_arTargetInfo.Nth(sel);
                whereFrom = 0;
            }
        }
    } while (sel > -1) ;
}

void ecPlatformsDialog::OnAdd(wxCommandEvent& event)
{
    ecPlatformEditorDialog dlg(this);
    dlg.m_strCaption = wxT("New Platform");
    if (wxID_OK == dlg.ShowModal())
    {
        if( -1 == m_listCtrl->FindItem(-1, dlg.m_strPlatform) )
        {
            Add(CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior));  
        } else {
            wxMessageBox(wxT("That platform name is already in use."), wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK, this);
        }
    }
}

void ecPlatformsDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecPlatformsDialog::OnOK(wxCommandEvent& event)
{
    event.Skip();
}

void ecPlatformsDialog::Add(const CeCosTestPlatform &ti)
{
    wxListCtrl* listCtrl = (wxListCtrl*) FindWindow( ecID_PLATFORMS_LIST );
    int i = listCtrl->GetItemCount();

    listCtrl->InsertItem(i,ti.Name());
    listCtrl->SetItem(i,1,ti.Prefix());
    listCtrl->SetItem(i,2,ti.GdbCmds());
    listCtrl->SetItem(i,3,ti.Inferior());
    listCtrl->SetItem(i,4,ti.Prompt());
    listCtrl->SetItem(i,5,ti.ServerSideGdb() ? wxT("Y"):wxT("N"));

    m_arTargetInfo.Append((wxObject*) new CeCosTestPlatform(ti));
}

void ecPlatformsDialog::OnUpdateAny(wxUpdateUIEvent& event)
{
    event.Enable( m_listCtrl->GetSelectedItemCount() > 0 );
}

void ecPlatformsDialog::OnDoubleLClick() 
{
    wxCommandEvent event;
    OnModify(event);
}

void ecPlatformsDialog::OnDeleteKey()
{
    wxCommandEvent event;
    OnDelete(event);
}

/*
 * ecPlatformsListCtrl
 */

IMPLEMENT_CLASS(ecPlatformsListCtrl, wxListCtrl)

BEGIN_EVENT_TABLE(ecPlatformsListCtrl, wxListCtrl)
    EVT_LEFT_DCLICK(ecPlatformsListCtrl::OnDoubleLClick)
    EVT_CHAR(ecPlatformsListCtrl::OnChar)
END_EVENT_TABLE()

ecPlatformsListCtrl::ecPlatformsListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxListCtrl(parent, id, pt, sz, style)
{
}

void ecPlatformsListCtrl::OnDoubleLClick(wxMouseEvent& event)
{
    ecPlatformsDialog* parent = (ecPlatformsDialog*) GetParent();
    parent->OnDoubleLClick();
}

void ecPlatformsListCtrl::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        ecPlatformsDialog* parent = (ecPlatformsDialog*) GetParent();
        parent->OnDeleteKey();
    }
    else
        event.Skip();
}
