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
// propertywin.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/04
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/propertywin.h#3 $
// Purpose:
// Description: Header file for ecPropertyListCtrl
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_PROPERTYWIN_H_
#define _ECOS_PROPERTYWIN_H_

#ifdef __GNUG__
#pragma interface "propertywin.h"
#endif

#include "wx/listctrl.h"

class ecConfigItem;

class ecPropertyListCtrl : public wxListCtrl
{
public:
    enum ecFieldType {ecType, ecValue, ecDefaultValue, ecMacro, ecFile, ecURL, ecEnabled, ecMAXFIELDTYPE};

// Ctor(s)
    ecPropertyListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);

//// Event handlers

    void OnRightClick(wxMouseEvent& event);
    void OnDoubleClick(wxMouseEvent& event);

//// Accessors
    int SetItem (ecFieldType f, const wxString& value);
    int SetItem (const wxString& item, const wxString& value,int nInsertAs,int nRepeat=1);
    int SetProperty (const wxString& value, CdlProperty prop);
	static bool PropertyInConflictsList (CdlProperty property, const std::list<CdlConflict> & conflicts);

//// Operations

    void Fill(ecConfigItem *pti);
    void AddColumns();
    void RefreshValue();

protected:
private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecPropertyListCtrl)

    double m_fWidth;
    enum {NCOLS=2};
    double m_f[NCOLS]; // relative proportions of columns
    int m_nLastCol;
    int m_nOnSizeRecursionCount;
    int m_nMaxValueWidth;
    int m_nFirstProperty;
    ecConfigItem *m_pti;

    static const std::string ecPropertyListCtrl::visible_properties [];
	static const wxChar* sm_fieldTypeImage[ecMAXFIELDTYPE];
};


#endif
        // _ECOS_PROPERTYWIN_H_
