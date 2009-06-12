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
// sectiondlg.h.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/sectiondlg.h#3 $
// Purpose:
// Description: Header file for ecSectionDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_SECTIONDLG_H_
#define _ECOS_SECTIONDLG_H_

#ifdef __GNUG__
    #pragma interface "sectiondlg.cpp"
#endif

#include "wx/notebook.h"

//----------------------------------------------------------------------------
// ecSectionDialog
//----------------------------------------------------------------------------

class ecSectionGeneralDialog;
class ecSectionRelocationDialog;
class ecSectionNoteDialog;

class ecSectionDialog: public wxDialog
{
DECLARE_CLASS(ecSectionDialog)
public:
    ecSectionDialog(wxWindow* parent);

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnApply(wxCommandEvent& event);
    void OnPageChange(wxNotebookEvent& event);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    inline wxNotebook* GetNotebook() const { return m_notebook; }

protected:

    ecSectionGeneralDialog*             m_general;
    ecSectionRelocationDialog*          m_relocation;
    ecSectionNoteDialog*                m_note;
    wxNotebook*                         m_notebook;

DECLARE_EVENT_TABLE()
};

/* General page
 */

class ecSectionGeneralDialog: public wxPanel
{
DECLARE_CLASS(ecSectionGeneralDialog)
public:
    ecSectionGeneralDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
};

#define ecID_SECTION_GENERAL_LINKER 10037
#define ecID_SECTION_GENERAL_USER 10038
#define ecID_SECTION_GENERAL_LINKER_TEXT 10039
#define ecID_SECTION_GENERAL_USER_TEXT 10040
#define ecID_SECTION_GENERAL_KNOWN_SIZE 10041
#define ecID_SECTION_GENERAL_KNOWN_SIZE_CHOICE 10042
#define ecID_SECTION_GENERAL_ABSOLUTE 10043
#define ecID_SECTION_GENERAL_FOLLOWING 10044
#define ecID_SECTION_GENERAL_ABSOLUTE_TEXT 10045
#define ecID_SECTION_GENERAL_FOLLOWING_TEXT 10046
#define ecID_SECTION_GENERAL_ALIGNMENT 10047

/* Relocation page
 */

class ecSectionRelocationDialog: public wxPanel
{
DECLARE_CLASS(ecSectionRelocationDialog)
public:
    ecSectionRelocationDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
};

#define ecID_SECTION_RELOCATION_RELOCATE 10048
#define ecID_SECTION_RELOCATION_ABSOLUTE 10049
#define ecID_SECTION_RELOCATION_ABSOLUTE_TEXT 10050
#define ecID_SECTION_RELOCATION_FOLLOWING 10051
#define ecID_SECTION_RELOCATION_FOLLOWING_CHOICE 10052

/* Note page
 */

class ecSectionNoteDialog: public wxPanel
{
DECLARE_CLASS(ecSectionNoteDialog)
public:
    ecSectionNoteDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
};

#define ecID_SECTION_NOTE_TEXT 10053

#endif
    // _ECOS_SECTIONDLG_H_
