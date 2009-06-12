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
// Version:     $Id: aboutdlg.cpp,v 1.12 2001/09/05 14:35:53 julians Exp $
// Purpose:
// Description: Implementation file for ecAboutDialog
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
    #pragma implementation "aboutdlg.cpp"
#endif

#include "ecpch.h"

#include "wx/wxhtml.h"
#include "wx/datetime.h"
#include "wx/file.h"
#include "wx/fs_mem.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "aboutdlg.h"
#include "configtool.h"
#include "symbols.h"

//----------------------------------------------------------------------------
// ecAboutDialog
//----------------------------------------------------------------------------

// WDR: event table for ecAboutDialog

BEGIN_EVENT_TABLE(ecAboutDialog,wxDialog)
END_EVENT_TABLE()

ecAboutDialog::ecAboutDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
{
    AddControls(this);

    Centre(wxBOTH);
}

bool ecAboutDialog::AddControls(wxWindow* parent)
{
    wxString htmlText;

    if (!wxGetApp().GetMemoryTextResource(wxT("about.htm"), htmlText))
    {
        wxSetWorkingDirectory(wxGetApp().GetAppDir());
        wxString htmlFile(wxGetApp().GetFullAppPath(wxT("about.htm")));
        
        if (wxFileExists(htmlFile))
        {
            wxFile file;
            file.Open(htmlFile, wxFile::read);
            long len = file.Length();
            char* buf = htmlText.GetWriteBuf(len + 1);
            file.Read(buf, len);
            buf[len] = 0;
            htmlText.UngetWriteBuf();
        }
    }

    if (htmlText.IsEmpty())
    {
        htmlText.Printf(wxT("<html><head><title>Warning</title></head><body><P>Sorry, could not find resource for About dialog<P></body></html>"));
    }

    // Customize the HTML
    htmlText.Replace(wxT("$VERSION$"), ecCONFIGURATION_TOOL_VERSION);
    htmlText.Replace(wxT("$DATE$"), __DATE__);
    
    wxSize htmlSize(420, 390);

    // Note: in later versions of wxWin this will be fixed so wxRAISED_BORDER
    // does the right thing. Meanwhile, this is a workaround.
#ifdef __WXMSW__
    long borderStyle = wxDOUBLE_BORDER;
#else
    long borderStyle = wxRAISED_BORDER;
#endif

    wxHtmlWindow* html = new wxHtmlWindow(this, ecID_ABOUT_DIALOG_HTML_WINDOW, wxDefaultPosition, htmlSize, borderStyle);
    html -> SetBorders(0);
    html -> SetPage(htmlText);
        
    //// Start of sizer-based control creation

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxWindow *item1 = parent->FindWindow( ecID_ABOUT_DIALOG_HTML_WINDOW );
    wxASSERT( item1 );
    item0->Add( item1, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item2 = new wxButton( parent, wxID_CANCEL, "&OK", wxDefaultPosition, wxDefaultSize, 0 );
    item2->SetDefault();

    item0->Add( item2, 0, wxALIGN_RIGHT|wxALL, 10 );

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );
    return TRUE;
}

/*
 * ecSplashScreen.
 */


ecSplashScreen::ecSplashScreen(const wxBitmap& bitmap, long splashStyle, int milliseconds, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
    wxSplashScreen(bitmap, splashStyle, milliseconds, parent, id, pos, size, style)
{
}

ecSplashScreen::~ecSplashScreen()
{
    wxGetApp().m_splashScreen = NULL;
}



