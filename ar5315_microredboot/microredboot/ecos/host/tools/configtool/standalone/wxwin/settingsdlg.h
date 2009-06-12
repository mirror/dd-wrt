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
// settingsdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/11
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/settingsdlg.h#3 $
// Purpose:
// Description: Header file for ecSettingsDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_SETTINGSDLG_H_
#define _ECOS_SETTINGSDLG_H_

#ifdef __GNUG__
    #pragma interface "settingsdlg.cpp"
#endif

class ecDisplayOptionsDialog; // Configuration Pane/Font
class ecViewerOptionsDialog;
class ecPathOptionsDialog;
class ecConflictResolutionOptionsDialog;
class ecRunOptionsDialog;

/*
 * ecSettingsDialog
 *
 * This the application settings dialog, containing
 * several further tabbed panels.
 *
 */

class ecSettingsDialog: public wxDialog
{
DECLARE_CLASS(ecSettingsDialog)
public:
    ecSettingsDialog(wxWindow* parent);

    void OnOK(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);
    void OnPageChange(wxNotebookEvent& event);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    inline wxNotebook* GetNotebook() const { return m_notebook; }

    void SetSelection(int sel);

protected:

    ecDisplayOptionsDialog*             m_displayOptions;
    ecViewerOptionsDialog*              m_viewerOptions;
    ecPathOptionsDialog*                m_pathOptions;
    ecConflictResolutionOptionsDialog*  m_conflictResolutionOptions;
    ecRunOptionsDialog*                 m_runOptions;
    wxNotebook*                         m_notebook;

DECLARE_EVENT_TABLE()
};

/* Display options dialog
 */

class ecDisplayOptionsDialog: public wxPanel
{
DECLARE_CLASS(ecDisplayOptionsDialog)
public:
    ecDisplayOptionsDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    void OnChangeFont(wxCommandEvent& event);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

DECLARE_EVENT_TABLE()
};

#define ecID_DISPLAY_OPTIONS_LABELS        10100
#define ecID_DISPLAY_OPTIONS_INTEGER_ITEMS 10101
#define ecID_DISPLAY_OPTIONS_FONT_CHOICE   10102
#define ecID_DISPLAY_OPTIONS_CHANGE_FONT   10103
#define ecID_DISPLAY_OPTIONS_SHOW_SPLASH   10104

/* Viewer options dialog
 */

class ecViewerOptionsDialog: public wxPanel
{
DECLARE_CLASS(ecViewerOptionsDialog)
public:
    ecViewerOptionsDialog(wxWindow* parent);

//// Operations
    void CreateControls( wxPanel *parent);

//// Overrides
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

//// Event handlers
    void OnUpdateViewerText(wxUpdateUIEvent& event);
    void OnUpdateBrowserText(wxUpdateUIEvent& event);
    void OnBrowseForViewer(wxCommandEvent& event);
    void OnBrowseForBrowser(wxCommandEvent& event);
    void OnShowAssociatedViewerInfo(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

#define ecID_VIEWER_DIALOG_HEADER_ASSOCIATED 10008
#define ecID_VIEWER_DIALOG_HEADER_THIS 10009
#define ecID_VIEWER_DIALOG_BROWSE_HEADER 10010
#define ecID_VIEWER_DIALOG_HEADER_TEXT 10011
#define ecID_VIEWER_DIALOG_DOC_BUILTIN 10012
#define ecID_VIEWER_DIALOG_DOC_ASSOCIATED 10013
#define ecID_VIEWER_DIALOG_DOC_THIS 10014
#define ecID_VIEWER_DIALOG_BROWSE_DOC 10015
#define ecID_VIEWER_DIALOG_DOC_TEXT 10016
#define ecID_VIEWER_DIALOG_ASSOC_INFO 10017

/* Path options dialog
 */

class ecPathOptionsDialog: public wxPanel
{
DECLARE_CLASS(ecPathOptionsDialog)
public:
    ecPathOptionsDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
};

#define ecID_PATHS_BUILD_MSG 10019
#define ecID_PATHS_BUILD_COMBO 10020
#define ecID_PATHS_BUILD_BROWSE 10021
#define ecID_PATHS_USER_MSG 10022
#define ecID_PATHS_USER_COMBO 10023
#define ecID_PATHS_USER_BROWSE 10024

/* Conflict resolution options dialog
 */

class ecConflictResolutionOptionsDialog: public wxPanel
{
DECLARE_CLASS(ecConflictResolutionOptionsDialog)
public:
    ecConflictResolutionOptionsDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    bool    m_suggestFixes;
    bool    m_immediate;
    bool    m_deferred;
};

#define ecID_CONFLICT_OPTIONS_AFTER_ITEM_CHANGED 10025
#define ecID_CONFLICT_OPTIONS_BEFORE_SAVING 10026
#define ecID_CONFLICT_OPTIONS_AUTOSUGGEST 10027

/* Run options dialog
 */

class ecRunOptionsDialog: public wxPanel
{
DECLARE_CLASS(ecRunOptionsDialog)
DECLARE_EVENT_TABLE()
public:
    ecRunOptionsDialog(wxWindow* parent);

    void CreateControls( wxPanel *parent);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void OnUpdateDownloadTimeout(wxUpdateUIEvent& event);
    void OnUpdateRuntimeTimeout(wxUpdateUIEvent& event);
    void OnUpdateSerial(wxUpdateUIEvent& event);
    void OnUpdateTCPIP(wxUpdateUIEvent& event);

    wxString    m_downloadTimeoutString;
    wxString    m_runtimeTimeoutString;
    wxString    m_baudString;
    bool        m_serialOn;
    bool        m_TCPIPOn;
};

#define ecID_RUN_PROPERTIES_PLATFORM 10077
#define ecID_RUN_PROPERTIES_DOWNLOAD_CHOICE 10078
#define ecID_RUN_PROPERTIES_DOWNLOAD_TIMEOUT 10079
#define ecID_RUN_PROPERTIES_RUNTIME_CHOICE 10080
#define ecID_RUN_PROPERTIES_RUNTIME_TIMEOUT 10081
#define ecID_RUN_PROPERTIES_SERIAL 10082
#define ecID_RUN_PROPERTIES_SERIAL_PORT_ADDR 10083
#define ecID_RUN_PROPERTIES_SERIAL_PORT_SPEED 10084
#define ecID_RUN_PROPERTIES_TCPIP 10085
#define ecID_RUN_PROPERTIES_TCPIP_HOST 10086
#define ecID_RUN_PROPERTIES_TCPIP_PORT 10087

#endif
    // _ECOS_SETTINGSDLG_H_
