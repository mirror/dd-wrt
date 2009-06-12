/////////////////////////////////////////////////////////////////////////////
// Name:        msgdlgex.cpp
// Purpose:     wxMessageDialogEx
// Author:      Julian Smart
// Modified by:
// Created:     12/12/2000
// RCS-ID:      $Id: msgdlgex.cpp,v 1.2 2001/06/11 14:22:31 julians Exp $
// Copyright:   (c) Julian Smart
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
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "msgdlgex.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "msgdlgex.h"

// Include wxWindow's headers

#include "wx/sizer.h"
#include "wx/statline.h"
#include "wx/statbox.h"
#include "wx/stattext.h"
#include "wx/statbmp.h"
#include "wx/bmpbuttn.h"

//----------------------------------------------------------------------------
// wxMessageDialogEx
//----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxMessageDialogEx,wxDialog)

BEGIN_EVENT_TABLE(wxMessageDialogEx,wxDialog)
    EVT_BUTTON( -1, wxMessageDialogEx::OnCommand )
END_EVENT_TABLE()

wxMessageDialogEx::wxMessageDialogEx( wxWindow *parent, const wxString& message, const wxString &caption,
    long style, const wxPoint& position) :
    wxDialog( parent, -1, caption, position, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
{
    m_dialogStyle = style;

    wxBeginBusyCursor();

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );

    // 1) icon
    if (style & wxICON_MASK)
    {
         wxStaticBitmap *icon = new wxStaticBitmap(
            this, -1, wxTheApp->GetStdIcon((int)(style & wxICON_MASK)));
         icon_text->Add( icon, 0, wxCENTER );
    }

    // 2) text
    icon_text->Add( CreateTextSizer( message ), 0, wxCENTER | wxLEFT, 10 );

    topsizer->Add( icon_text, 0, wxCENTER | wxLEFT|wxRIGHT|wxTOP, 10 );

#if wxUSE_STATLINE
    // 3) static line
    topsizer->Add( new wxStaticLine( this, -1 ), 0, wxEXPAND | wxLEFT|wxRIGHT|wxTOP, 10 );
#endif

    // 4) buttons
    topsizer->Add( CreateButtonSizer( style ), 0, wxCENTRE | wxALL, 10 );

    SetAutoLayout( TRUE );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );
    wxSize size( GetSize() );
    if (size.x < size.y*3/2)
    {
        size.x = size.y*3/2;
        SetSize( size );
    }

    Centre( wxBOTH | wxCENTER_FRAME);

    wxEndBusyCursor();
}

wxMessageDialogEx::wxMessageDialogEx()
{
    m_dialogStyle = 0;
}

wxMessageDialogEx::~wxMessageDialogEx()
{
}

void wxMessageDialogEx::OnCommand(wxCommandEvent &event)
{
    EndModal(event.GetId());
}

wxSizer *wxMessageDialogEx::CreateButtonSizer( long flags )
{
    wxBoxSizer *box = new wxBoxSizer( wxHORIZONTAL );

#if defined(__WXMSW__) || defined(__WXMAC__)
    static const int margin = 6;
#else
    static const int margin = 10;
#endif

    wxButton *ok = (wxButton *) NULL;
    wxButton *cancel = (wxButton *) NULL;
    wxButton *yes = (wxButton *) NULL;
    wxButton *no = (wxButton *) NULL;
    wxButton *yestoall = (wxButton *) NULL;
    wxButton *notoall = (wxButton *) NULL;
    wxButton *abort = (wxButton *) NULL;
    wxButton *retry = (wxButton *) NULL;
    wxButton *ignore = (wxButton *) NULL;

    // always show an OK button, unless only YES_NO is given
    // NO, not in this dialog.
    //if ((flags & wxYES_NO) == 0) flags = flags | wxOK;

    if (flags & wxMD_YES_NO)
    {
        yes = new wxButton( this, wxID_YES, _("Yes") );
        box->Add( yes, 0, wxLEFT|wxRIGHT, margin );
        no = new wxButton( this, wxID_NO, _("No") );
        box->Add( no, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_YES)
    {
        if (!yes)
        {
            yes = new wxButton( this, wxID_YES, _("Yes") );
            box->Add( yes, 0, wxLEFT|wxRIGHT, margin );
        }
    }

    if (flags & wxMD_YESTOALL)
    {
        yestoall = new wxButton( this, wxID_YESTOALL, _("Yes to All") );
        box->Add( yestoall, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_NO)
    {
        if (!no)
        {
            no = new wxButton( this, wxID_NO, _("No") );
            box->Add( no, 0, wxLEFT|wxRIGHT, margin );
        }
    }

    if (flags & wxMD_NOTOALL)
    {
        notoall = new wxButton( this, wxID_NOTOALL, _("No to All") );
        box->Add( notoall, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_ABORT)
    {
        abort = new wxButton( this, wxID_ABORT, _("Abort") );
        box->Add( abort, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_RETRY)
    {
        retry = new wxButton( this, wxID_RETRY, _("Retry") );
        box->Add( retry, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_IGNORE)
    {
        ignore = new wxButton( this, wxID_IGNORE, _("Ignore") );
        box->Add( ignore, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_OK)
    {
        ok = new wxButton( this, wxID_OK, _("OK") );
        box->Add( ok, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_CANCEL)
    {
        cancel = new wxButton( this, wxID_CANCEL, _("Cancel") );
        box->Add( cancel, 0, wxLEFT|wxRIGHT, margin );
    }

    if (flags & wxMD_HELP)
        box->Add( new wxButton( this, wxID_HELP, _("Help")  ), 0, wxLEFT|wxRIGHT, margin );

    bool setDefault = FALSE;

    if (flags & wxMD_NO_DEFAULT)
    {
        if (no)
        {
            no->SetDefault();
            no->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_YES_DEFAULT)
    {
        if (yes)
        {
            yes->SetDefault();
            yes->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_YESTOALL_DEFAULT)
    {
        if (yestoall)
        {
            yestoall->SetDefault();
            yestoall->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_NOTOALL_DEFAULT)
    {
        if (notoall)
        {
            notoall->SetDefault();
            notoall->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_ABORT_DEFAULT)
    {
        if (abort)
        {
            abort->SetDefault();
            abort->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_RETRY_DEFAULT)
    {
        if (retry)
        {
            retry->SetDefault();
            retry->SetFocus();
            setDefault = TRUE;
        }
    }
    else if (flags & wxMD_IGNORE_DEFAULT)
    {
        if (ignore)
        {
            ignore->SetDefault();
            ignore->SetFocus();
            setDefault = TRUE;
        }
    }

    if (!setDefault)
    {
        if (ok)
        {
            ok->SetDefault();
            ok->SetFocus();
        }
        else if (yes)
        {
            yes->SetDefault();
            yes->SetFocus();
        }
    }

    return box;
}

