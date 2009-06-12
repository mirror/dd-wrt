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
// configtoolview.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/05
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/configtoolview.h#3 $
// Purpose:
// Description: Header file for ecConfigToolView
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGTOOLVIEW_H_
#define _ECOS_CONFIGTOOLVIEW_H_

#ifdef __GNUG__
#pragma interface "configtoolview.h"
#endif

#include "wx/docview.h"
#include "wx/treectrl.h"

/*
 * ecConfigToolView
 */

class ecConfigItem;
class ecConfigToolView: public wxView
{
    DECLARE_DYNAMIC_CLASS(ecConfigToolView)
public:
    ecConfigToolView();
    ~ecConfigToolView() {};
    
//// Overrides
    bool OnCreate(wxDocument *doc, long flags);
    void OnDraw(wxDC *dc);
    void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
    bool OnClose(bool deleteWindow = TRUE);
    void OnChangeFilename();

//// Operations
    void Refresh(const wxString& macroName);
    void Refresh (wxTreeItemId h);
    ecConfigItem *DoFind(const wxString& what, wxWindow* parent) ;

//// Helpers
    bool IsWordChar(wxChar c);

//// Event handlers

    // General disabler
    void OnUpdateDisable(wxUpdateUIEvent& event);

DECLARE_EVENT_TABLE()

protected:
    wxTreeItemId m_expandedForFind;
};

/*
 * ecConfigToolHint
 *
 * Hint to pass to UpdateAllViews
 *
 */

// Update hint symbols
#define ecNoHint                0
#define ecAllSaved              1
#define ecNameFormatChanged     2
#define ecIntFormatChanged      3
#define ecClear                 4
#define ecValueChanged          5
#define ecExternallyChanged     6
#define ecSelChanged            7
#define ecFilenameChanged       8
#define ecMemLayoutChanged      9

class ecConfigItem;
class ecConfigToolHint: public wxObject
{
public:
    ecConfigToolHint(ecConfigItem* item, int op) { m_item = item; m_op = op; }

    ecConfigItem*   m_item;
    int             m_op;
};


#endif
        // _ECOS_CONFIGTOOLVIEW_H_
