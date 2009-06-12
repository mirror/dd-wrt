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
// Version:     $Id: finddlg.cpp,v 1.5 2002/02/28 18:30:35 julians Exp $
// Purpose:
// Description: Implementation file for ecFindDialog
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
    #pragma implementation "finddlg.cpp"
#endif

#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/valgen.h"

#include "finddlg.h"
#include "configtool.h"
#include "configtooldoc.h"
#include "configtoolview.h"
#include "configtree.h"
#include "mainwin.h"
#include "appsettings.h"

//----------------------------------------------------------------------------
// ecFindDialog
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ecFindDialog,wxDialog)
    EVT_BUTTON(ecID_FIND_DIALOG_NEXT, ecFindDialog::OnFindNext)
    EVT_CLOSE(ecFindDialog::OnCloseWindow)
    EVT_BUTTON(wxID_CANCEL, ecFindDialog::OnCancel)
END_EVENT_TABLE()

ecFindDialog::ecFindDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( )
{
    m_directionSelection = 1;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    wxDialog::Create( parent, id, title, position, size, style );

    AddControls(this);

    if (wxGetApp().GetSettings().m_findDialogPos.x == -1 && wxGetApp().GetSettings().m_findDialogPos.y == -1)
        Centre(wxBOTH);
    else
        Move(wxGetApp().GetSettings().m_findDialogPos);
}

bool ecFindDialog::AddControls(wxWindow* parent)
{
    //// Start of sizer-based control creation

    wxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item1 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item2 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item3 = new wxStaticText( parent, wxID_STATIC, _(" Fi&nd what:"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->Add( item3, 0, wxALIGN_CENTRE|wxLEFT|wxTOP|wxBOTTOM, 5 );

    wxTextCtrl *item4 = new wxTextCtrl( parent, ecID_FIND_DIALOG_WHAT, _(""), wxDefaultPosition, wxSize(200,-1), 0 );
    item2->Add( item4, 1, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item2, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item6 = new wxBoxSizer( wxVERTICAL );

    wxCheckBox *item7 = new wxCheckBox( parent, ecID_FIND_DIALOG_MATCH_WHOLE, _("Match &whole word only"), wxDefaultPosition, wxDefaultSize, 0 );
    item6->Add( item7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxCheckBox *item8 = new wxCheckBox( parent, ecID_FIND_DIALOG_MATCH_CASE, _("Match &case"), wxDefaultPosition, wxDefaultSize, 0 );
    item6->Add( item8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item5->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs9[] = 
    {
        _("&Up"), 
        _("&Down")
    };
    wxRadioBox *item9 = new wxRadioBox( parent, ecID_FIND_DIALOG_DIRECTION, _("Direction"), wxDefaultPosition, wxDefaultSize, 2, strs9, 1, wxRA_SPECIFY_ROWS );
    item5->Add( item9, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item5, 0, wxALIGN_CENTRE|wxRIGHT, 5 );

    wxSizer *item10 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item11 = new wxStaticText( parent, wxID_STATIC, _("&Search in:"), wxDefaultPosition, wxDefaultSize, 0 );
    item10->Add( item11, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxString strs12[] = 
    {
        _("Macro names"), 
        _("Item names"), 
        _("Short descriptions"), 
        _("Current Values"), 
        _("Default Values")
    };
    wxChoice *item12 = new wxChoice( parent, ecID_FIND_DIALOG_SEARCH_IN, wxDefaultPosition, wxSize(130,-1), 5, strs12, 0 );
    item10->Add( item12, 0, wxALIGN_CENTRE|wxALL, 5 );

    item1->Add( item10, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

    item0->Add( item1, 0, wxALIGN_CENTRE|wxTOP|wxBOTTOM, 5 );

    wxSizer *item13 = new wxBoxSizer( wxVERTICAL );

    wxButton *item14 = new wxButton( parent, ecID_FIND_DIALOG_NEXT, _("&Find Next"), wxDefaultPosition, wxDefaultSize, 0 );
    item14->SetDefault();
    item13->Add( item14, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item15 = new wxButton( parent, wxID_CANCEL, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    item13->Add( item15, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item13->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item13, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5 );

    FindWindow(ecID_FIND_DIALOG_WHAT)->SetFocus();

    // Add validators
    FindWindow(ecID_FIND_DIALOG_WHAT)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_findText));
    FindWindow(ecID_FIND_DIALOG_MATCH_WHOLE)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_findMatchWholeWord));
    FindWindow(ecID_FIND_DIALOG_MATCH_CASE)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_findMatchCase));
    FindWindow(ecID_FIND_DIALOG_SEARCH_IN)->SetValidator(wxGenericValidator(& wxGetApp().GetSettings().m_findSearchWhat));
    FindWindow(ecID_FIND_DIALOG_DIRECTION)->SetValidator(wxGenericValidator(& m_directionSelection));

    // Add help text
    FindWindow(ecID_FIND_DIALOG_WHAT)->SetHelpText(_("Enter your search text here."));
    FindWindow(ecID_FIND_DIALOG_MATCH_WHOLE)->SetHelpText(_("Check to match whole words, clear to match parts of words."));
    FindWindow(ecID_FIND_DIALOG_MATCH_CASE)->SetHelpText(_("Check if you want upper and lower case to be significant, clear otherwise."));
    FindWindow(ecID_FIND_DIALOG_SEARCH_IN)->SetHelpText(_("Choose the category for your search."));
    FindWindow(ecID_FIND_DIALOG_DIRECTION)->SetHelpText(_("Choose the direction of your search."));
    FindWindow(ecID_FIND_DIALOG_NEXT)->SetHelpText(_("Click Find Next to search for the next match."));
    FindWindow(wxID_CANCEL)->SetHelpText(_("Click Close to close the dialog."));

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );
    return TRUE;
}

bool ecFindDialog::TransferDataToWindow()
{
    // Convert to radiobox selection index from bool
    m_directionSelection = wxGetApp().GetSettings().m_findDirection ? 1 : 0 ;
    
    wxWindow::TransferDataToWindow();

    return TRUE;
}

bool ecFindDialog::TransferDataFromWindow()
{
    wxWindow::TransferDataFromWindow();

    // Convert from radiobox selection index to bool
    wxGetApp().GetSettings().m_findDirection = (m_directionSelection == 1);
    
    return TRUE;
}

void ecFindDialog::OnFindNext(wxCommandEvent& event)
{
    if (!TransferDataFromWindow())
        return;
    if (!wxGetApp().GetConfigToolDoc())
        return;

    ecConfigToolView *pControlView = (ecConfigToolView*) wxGetApp().GetConfigToolDoc()->GetFirstView();

    ecConfigItem* item = pControlView->DoFind(wxGetApp().GetSettings().m_findText, this);

    if (item)
    {
        // Is the find window on top of the item?
        wxRect rect1, rect2;

        if (wxGetApp().GetTreeCtrl()->GetBoundingRect(item->GetTreeItem(), rect1))
        {
            wxPoint topLeft(rect1.x, rect1.y);
            wxPoint bottomRight(rect1.GetRight(), rect1.GetBottom());

            topLeft = wxGetApp().GetTreeCtrl()->ClientToScreen(topLeft);
            bottomRight = wxGetApp().GetTreeCtrl()->ClientToScreen(bottomRight);

            rect2 = GetRect(); // screen coords

            if (rect2.Inside(topLeft) || rect2.Inside(bottomRight))
            {
                Move(wxPoint(topLeft.x + rect1.width, rect2.y));
            }
        }
    }
}

void ecFindDialog::OnCloseWindow(wxCloseEvent& event)
{
    wxGetApp().GetSettings().m_findDialogPos = GetPosition();
    wxGetApp().GetMainFrame()->m_findDialog = NULL;

    event.Skip();
}

void ecFindDialog::OnCancel(wxCommandEvent& event)
{
    wxGetApp().GetSettings().m_findDialogPos = GetPosition();
    wxGetApp().GetMainFrame()->m_findDialog = NULL;

    event.Skip();
}
