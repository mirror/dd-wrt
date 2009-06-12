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
// configpropdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/02
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/configpropdlg.h#3 $
// Purpose:
// Description: Header file for ecConfigPropertiesDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGPROPDLG_H_
#define _ECOS_CONFIGPROPDLG_H_

#ifdef __GNUG__
#pragma interface "configpropdlg.cpp"
#endif

#include "wx/wx.h"
#include "wx/listctrl.h"

// Forward declarations
class ecPropertyListCtrl;
class ecConfigItem;

class ecConfigPropertiesDialog : public wxDialog
{
public:
// Ctor(s)
    ecConfigPropertiesDialog(wxWindow* parent, ecConfigItem* item);

//// Event handlers

    void OnClose(wxCommandEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);

protected:

    ecPropertyListCtrl* m_listCtrl;
    ecConfigItem*       m_item;

private:
    DECLARE_EVENT_TABLE()
};

#define ecID_CONFIG_PROPERTIES_LIST 2064


#endif
        // _ECOS_CONFIGPROPDLG_H_
