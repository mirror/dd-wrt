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
// packagesdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/28
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/packagesdlg.h#3 $
// Purpose:
// Description: Header file for ecPackagesDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_PACKAGESDLG_H_
#define _ECOS_PACKAGESDLG_H_

#ifdef __GNUG__
#pragma interface "packagesdlg.cpp"
#endif

#include "ecutils.h"
#include "wx/timer.h"

/*
 * ecPackagesTimer
 * Just to force idle processing now and again
 * so that the keyword search works under OSes that
 * have unreliable OnIdle processing
 */

class ecPackagesDialog;
class ecPackagesTimer: public wxTimer
{
public:
    ecPackagesTimer(ecPackagesDialog* dialog) { m_dialog = dialog; }

    virtual void Notify() ;
 protected:
    ecPackagesDialog* m_dialog;
};

/*
 * Add/remove packages
 */

class ecPackagesDialog : public ecDialog
{
public:
// Ctor(s)
    ecPackagesDialog(wxWindow* parent);
    ~ecPackagesDialog();

//// Event handlers

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    void OnClickListBox1(wxCommandEvent& event);
    void OnClickListBox2(wxCommandEvent& event);
    void OnDblClickListBox1(wxCommandEvent& event);
    void OnDblClickListBox2(wxCommandEvent& event);
    void OnSelectVersion(wxCommandEvent& event);
    void OnInitDialog(wxInitDialogEvent& event);

//// Operations
    void CreateControls(wxWindow* parent);
    void InitControls();
    void Fill();
    void Insert(const wxString& str, bool added, const wxString& descr = wxEmptyString, const wxString& version = wxEmptyString);
    void Add(wxListBox* from, wxListBox* to);
    void DisplayDescription(const wxString& item);
    void ClearDescription();
    void HardwarePackageMessageBox();
    void UpdateHardwareSelectionFlag();
    void UpdatePackageDescription();
    void UpdateVersionList();
    void UpdateAddRemoveButtons();
    void ClearSelections(wxListBox& lbox);

    void OnClearKeywords(wxCommandEvent& event);
    void OnUpdateKeywordText(wxCommandEvent& event);
    void OnClickOmitHardwarePackages(wxCommandEvent& event);
    void OnClickExactMatch(wxCommandEvent& event);

    void OnIdle(wxIdleEvent& event);

//// Helpers
    // For each word in keywords, is it contained in 'str'?
    bool MatchesKeyword(wxArrayString& keywords, const wxString& str);

//// Accessors
    bool IsAdded(const wxString& str) ;
    int GetCount() const { return m_items.Number(); }
    wxString GetVersion (const wxString& item);

protected:

private:
    DECLARE_EVENT_TABLE()

    wxStringList    m_items;
    wxStringList    m_descriptions;
    wxArrayString   m_currentVersions;
    wxArrayString   m_added; // All those packages currently (or to-be) added
    wxArrayInt      m_arnItems;   // Whether in 'use list' (1) or not (0)

    bool            m_bHardwarePackageSelected;
    wxString        m_packageDescription;
    wxString        m_keywords;
    bool            m_updateLists; // If true, the keyword changed and we need to update the lists
    long            m_updateInterval; // Interval before display is updated
    ecPackagesTimer m_timer;
};

#define ecID_PACKAGES_DIALOG_AVAILABLE_LIST     10066
#define ecID_PACKAGES_DIALOG_ADD                10067
#define ecID_PACKAGES_DIALOG_REMOVE             10068
#define ecID_PACKAGES_DIALOG_USE_LIST           10069
#define ecID_PACKAGES_DIALOG_VERSION            10070
#define ecID_PACKAGES_DIALOG_DESCRIPTION        10071
#define ecID_PACKAGES_DIALOG_KEYWORDS           10072
#define ecID_PACKAGES_DIALOG_CLEAR              10073
#define ecID_PACKAGES_DIALOG_OMIT_HARDWARE      10074
#define ecID_PACKAGES_DIALOG_EXACT_MATCH        10075

#endif
        // _ECOS_PACKAGESDLG_H_
