//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
// appsettings.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians
// Date:        2000/08/29
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/appsettings.h#3 $
// Purpose:
// Description: Header file for the ConfigTool application settings
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_APPSETTINGS_H_
#define _ECOS_APPSETTINGS_H_

#include "wx/config.h"
#include "filename.h"
#include "ecutils.h"
#include "eCosTest.h"

#ifdef __WXMSW__
#include "wx/msw/winundef.h"
#endif

// Frame status
#define ecSHOW_STATUS_NORMAL        0x01
#define ecSHOW_STATUS_MINIMIZED     0x02
#define ecSHOW_STATUS_MAXIMIZED     0x03

// What kind of browser (wxHtmlHelpController, default browser, custom browser)
enum ecBrowserType { ecInternal, ecAssociatedExternal, ecCustomExternal };

/*
 * ecRunTestSettings
 * Settings relating to running tests
 */
enum ResetType   {RESET_NONE,  RESET_X10, RESET_MANUAL};
enum TimeoutType {TIMEOUT_NONE,TIMEOUT_SPECIFIED, TIMEOUT_AUTOMATIC};

class ecRunTestsSettings: public wxObject
{
DECLARE_DYNAMIC_CLASS(ecRunTestsSettings)
public:
    ecRunTestsSettings();
    ecRunTestsSettings(const ecRunTestsSettings& settings);

//// Operations
    void Copy(const ecRunTestsSettings& settings);

    bool SaveConfig(wxConfig& config);
    bool LoadConfig(wxConfig& config);

//// Data members
    CeCosTest::ExecutionParameters  m_ep;
    int         m_nTimeout;
    int         m_nDownloadTimeout;
    int         m_nTimeoutType;
    int         m_nDownloadTimeoutType;
    bool        m_bRemote;
    bool        m_bSerial;
    int         m_nBaud;
    int         m_nLocalTCPIPPort;
    int         m_nReset;
    int         m_nResourcePort;
    int         m_nRemotePort;
    bool        m_bFarmed;
    wxString    m_strPort; // Serial port
    wxString    m_strTarget;

    wxString    m_strRemoteHost;
    wxString    m_strResourceHost;
    wxString    m_strLocalTCPIPHost;
    wxString    m_strReset;
};

/*
 * ecSettings
 * Various application settings
 */

class ecSettings: public wxObject
{
DECLARE_DYNAMIC_CLASS(ecSettings)
public:
    ecSettings();
    ecSettings(const ecSettings& settings);
    ~ecSettings();

//// Operations
    // Copy from settings to 'this'
    void Copy(const ecSettings& settings);

    // Load config info
    bool LoadConfig();

    // Save config info
    bool SaveConfig();

    // Load and save font descriptions
    bool LoadFont(wxConfig& config, const wxString& windowName, wxFont& font);
    bool SaveFont(wxConfig& config, const wxString& windowName, const wxFont& font);
    bool LoadFonts(wxConfig& config);
    bool SaveFonts(wxConfig& config);
    bool ApplyFontsToWindows();

    // Do some initialisation within ecApp::OnInit
    bool Init();

    // Show settings dialog
    void ShowSettingsDialog(const wxString& page = wxEmptyString);

    // Create new filename
    wxString GenerateFilename(const wxString& rootName);

    // Go looking for potential candidates in SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
    int GetRepositoryRegistryClues(wxArrayString& arstr, const wxString& pszPrefix);

    // Finds the path of the latest installed eCos (Unix only)
    wxString FindLatestVersion();

    // Find the subkey of the latest installed eCos, e.g. "1.4.9"
    wxString GetInstallVersionKey ();
    
#ifdef __WXMSW__
    // Find the location of the Cygwin installation
    wxString GetCygwinInstallPath ();
#endif

//// Accessors

    wxString GetAppName() const { return m_appName; }

    // Get a name suitable for the configuration file on all platforms:
    // e.g. eCos Configuration Tool on Windows, .eCosConfigTool on Unix
    wxString GetConfigAppName() const ;

    wxString& GetLastFilename() { return m_lastFilename; }

    const ecFileName& DefaultExternalBrowser() ;

    const wxString& GetUserToolsDir() const { return m_userToolsDir; }
    const wxString& GetBuildToolsDir() const { return m_buildToolsDir; }  /* Only used if no other clues */

    wxStringToStringMap& GetBinDirs() { return m_arstrBinDirs; }

    ecRunTestsSettings& GetRunTestsSettings() { return m_runTestsSettings; }

    wxWindowSettings& GetWindowSettings() { return m_windowSettings; }

public:
    bool                    m_showToolBar;
    wxRect                  m_frameSize;
    wxString                m_appName;     // The current name of the app...
    bool                    m_showSplashScreen; // Show the splash screen
    wxString                m_userName;
    int                     m_serialNumber;
    wxString                m_lastFilename; // So we can auto-generate sensible filenames
    int                     m_frameStatus;

	// Sash window sizes
	wxSize					m_treeSashSize;
	wxSize					m_conflictsSashSize;
	wxSize					m_propertiesSashSize;
	wxSize					m_shortDescrSashSize;
	wxSize					m_memorySashSize;
	wxSize					m_outputSashSize;
    int                     m_configPaneWidth; // The sash to the right of the tree
    bool                    m_showConflictsWindow;
    bool                    m_showPropertiesWindow;
    bool                    m_showShortDescrWindow;
    bool                    m_showMemoryWindow;
    bool                    m_showOutputWindow;

    // Are we showing macro names, or ordinary names?
    bool                    m_showMacroNames ;

    // Are we merely editing the .ecc file and not saving build trees?
    // This option is specified on the command line and not saved.
    bool                    m_editSaveFileOnly;

    // Viewers & browsers
    bool                    m_bUseCustomViewer;
    bool                    m_bUseExternalBrowser;
    wxString                m_strBrowser;
    ecFileName              m_strViewer;
    ecBrowserType           m_eUseCustomBrowser;
    static ecFileName       m_strDefaultExternalBrowser;
    bool                    m_bHex;

    ecFileName              m_userToolsDir;
    ecFileName              m_buildToolsDir; /* Only used if no other clues */

    wxString                m_strMakeOptions;

    wxStringToStringMap     m_arstrBinDirs;  // Not saved
    wxArrayString           m_userToolPaths; // Not saved

    ecFileName              m_strRepository; // This is saved/loaded via ecSettings, and
                                             // copied to/from ecConfigToolDoc on doc creation/deletion.
                                             // This is because the doc doesn't have its own profile loading/saving.

    enum {Never=0, Immediate=1, Deferred=2, SuggestFixes=4};
    int                     m_nRuleChecking; // OR of above values

    // Find dialog settings
    wxString                m_findText;
    bool                    m_findMatchWholeWord;
    bool                    m_findMatchCase;
    bool                    m_findDirection; // Down is TRUE, Up is FALSE
    wxString                m_findSearchWhat; // Macro names, item names etc.
    wxPoint                 m_findDialogPos; // Position of dialog

    // Run tests settings
    ecRunTestsSettings      m_runTestsSettings;

    // Font settings
    wxWindowSettings        m_windowSettings;

    // Packages dialog settings
    bool                    m_omitHardwarePackages;
    bool                    m_matchPackageNamesExactly;
};

#endif
        // _ECOS_APPSETTINGS_H_
