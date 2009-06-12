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
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/aboutdlg.h#3 $
// Purpose:
// Description: Header file for ecAboutDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_ABOUTDLG_H_
#define _ECOS_ABOUTDLG_H_

#ifdef __GNUG__
    #pragma interface "aboutdlg.cpp"
#endif

#include "wx/splash.h"

//----------------------------------------------------------------------------
// ecAboutDialog
//----------------------------------------------------------------------------

class ecAboutDialog: public wxDialog
{
public:
    // constructors and destructors
    ecAboutDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    
    bool AddControls(wxWindow* parent);

private:
    DECLARE_EVENT_TABLE()
};

/*
 * ecSplashScreen. We have to derive from wxSplashScreen so we can tell
 * when the window has been destroyed (so we don't risk destroying a window
 * that's already been destroyed)
 */

class WXDLLEXPORT ecSplashScreen: public wxSplashScreen
{
public:
    ecSplashScreen(const wxBitmap& bitmap, long splashStyle, int milliseconds, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxSIMPLE_BORDER|wxFRAME_FLOAT_ON_PARENT);
    ~ecSplashScreen();

protected:
};


#endif
    // _ECOS_ABOUTDLG_H_
