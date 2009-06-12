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
// Version:     $Id: reposdlg.cpp,v 1.5 2002/01/02 14:28:55 julians Exp $
// Purpose:
// Description: Implementation file for ecRepositoryInfoDialog
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
    #pragma implementation "reposdlg.cpp"
#endif

#include "ecpch.h"

#include "wx/wxhtml.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "reposdlg.h"
#include "configtool.h"
#include "configtooldoc.h"

//----------------------------------------------------------------------------
// ecRepositoryInfoDialog
//----------------------------------------------------------------------------

// WDR: event table for ecRepositoryInfoDialog

BEGIN_EVENT_TABLE(ecRepositoryInfoDialog,wxDialog)
END_EVENT_TABLE()

ecRepositoryInfoDialog::ecRepositoryInfoDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    AddControls(this);

    Centre(wxBOTH);
}

bool ecRepositoryInfoDialog::AddControls(wxWindow* parent)
{
#if 0
    wxColour backgroundColour = * wxBLACK; // wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DFACE);
    wxColour buttonBackgroundColour = * wxWHITE; // backgroundColour;
    wxColour buttonForegroundColour = * wxBLACK; // wxSystemSettings::GetSystemColour(wxSYS_COLOUR_BTNTEXT);

    if (!wxGetApp().GetHiColour())
    {
        backgroundColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DFACE);
        buttonBackgroundColour = backgroundColour;
        buttonForegroundColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_BTNTEXT);
    }
    
    this->SetBackgroundColour(backgroundColour);
#endif

    wxSize htmlSize(440, 380);

    // Note: in later versions of wxWin this will be fixed so wxRAISED_BORDER
    // does the right thing. Meanwhile, this is a workaround.
#ifdef __WXMSW__
    long borderStyle = wxDOUBLE_BORDER;
#else
    long borderStyle = wxRAISED_BORDER;
#endif

    wxHtmlWindow* html = new wxHtmlWindow(this, ecID_REPOS_DIALOG_HTML_WINDOW, wxDefaultPosition, htmlSize,
        borderStyle);
    html -> SetBorders(5);

    wxString info;
    if (CreateHtmlInfo(info))
        html -> SetPage(info);
    else
    {
        wxString msg;
        msg.Printf(wxT("<html><head><title>Warning</title></head><body><P>Sorry, could not obtain repository information.<P></body></html>"));
        html->SetPage(msg);
    }
        
    //// Start of sizer-based control creation

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxWindow *item1 = parent->FindWindow( ecID_REPOS_DIALOG_HTML_WINDOW );
    wxASSERT( item1 );
    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item2 = new wxButton( parent, wxID_CANCEL, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
#if 0
    item2->SetBackgroundColour(buttonBackgroundColour);
    item2->SetForegroundColour(buttonForegroundColour);
#endif
    item2->SetDefault();

    //item0->Add( item2, 0, wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    //item0->Add( item2, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    item0->Add( item2, 0, wxALIGN_RIGHT|wxALL, 10 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );
    return TRUE;
}

bool ecRepositoryInfoDialog::CreateHtmlInfo(wxString& info)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

    info += wxT("<html><head><title>Repository Information</title>\n");
    info += wxT("<body bgcolor=\"#FFFFE1\">\n");

    info += wxT("<table width=\"100%\">");
    info += wxT("<tr><td>\n");
    wxString s;
    s.Printf(wxT("<img src=\"memory:ecoslogosmall.png\" align=right vspace=8 border=0><p>"));
    info += s;
    info += wxT("</td><td>\n");
    info += wxT("<font size=+2><b>Repository Information</b></font>\n");

    info += wxT("</td></tr></table>\n");

    info += wxT("<hr>\n");

    ///////////////////////////////////////////////////
    info += wxT("<b>Repository path:</b><P>\n");

    info += wxT("<ul>");

    if (doc)
    {
        info += doc->GetRepository();
    }
    else
    {
        info += wxT("Not loaded.");
    }

    info += wxT("</ul><P>");

    ///////////////////////////////////////////////////
    info += wxT("<b>Save file:</b><P>\n");

    info += wxT("<ul>");

    if (doc)
    {
        info += doc->GetFilename();
    }
    else
    {
        info += wxT("Not loaded.");
    }

    info += wxT("</ul><p>");

    ///////////////////////////////////////////////////
    info += wxT("<b>Hardware template:</b><P>\n");

    info += wxT("<ul>");

    if (doc)
    {
        wxString hardware = doc->GetCdlConfig ()->get_hardware ().c_str();

        info += hardware;
        info += wxT(" ");

        info += doc->GetTemplateVersion () ;
    }
    else
    {
        info += wxT("Unknown.");
    }

    info += wxT("</ul><P>");

    ///////////////////////////////////////////////////
    info += wxT("<b>Default package:</b><P>\n");

    info += wxT("<ul>");

    if (doc)
    {
        wxString package = doc->GetCdlConfig ()->get_template ().c_str();

        info += package;
    }
    else
    {
        info += wxT("Unknown.");
    }

    info += wxT("</ul><P>");

    info += wxT("</body></html>");

    return TRUE;
}
