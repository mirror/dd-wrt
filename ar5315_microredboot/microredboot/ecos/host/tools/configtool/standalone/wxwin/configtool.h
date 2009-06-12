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
// configtool.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/08/24
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/configtool.h#3 $
// Purpose:
// Description: main header file for the ConfigTool application
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGTOOL_H_
#define _ECOS_CONFIGTOOL_H_

#ifdef __GNUG__
#pragma interface "configtool.h"
#endif

#include "wx/wx.h"
#include "wx/help.h"
#include "wx/docview.h"
#include "wx/process.h"
#include "wx/timer.h"
#include "splittree.h"

#include "appsettings.h"

class ecValueWindow;
class ecMainFrame;
class ecConfigTreeCtrl;
class ecMemoryLayoutWindow;
class ecConfigToolDoc;
class ecSplashScreen;
class ecPipedProcess;

/*
#ifdef __WXMSW__
#include "wx/msw/helpchm.h"
#undef wxHelpController
#define wxHelpController wxCHMHelpController
#endif
*/

#if defined(__WXGTK__) || defined(__WXMSW__)
#include "wx/html/helpctrl.h"
#undef wxHelpController
#define wxHelpController wxHtmlHelpController
#endif

#ifdef __VISUALC__
#pragma warning(disable:4786)
#endif

#if defined(__WXMSW__) && defined(__WXDEBUG__)
// MLT code does not build yet so ecUSE_MLT is always 0
#define ecUSE_MLT   0
#else
#define ecUSE_MLT   0
#endif

// If 1, can optionally use non-standard wxHTML extension to set the base doc path
// If 0, NEVER uses relative paths.
#define ecDOCSYSTEM_USE_RELATIVE_URLS 1

#if defined(__WXMSW__) && defined(__WXDEBUG__)
#define ecUSE_EXPERIMENTAL_CODE 1
#endif

class WXDLLEXPORT wxZipFSHandler;
class WXDLLEXPORT wxFileSystem;

// Define a new application type, each program should derive a class from wxApp
class ecApp : public wxApp
{
    friend class ecMainFrame;
public:
//// Ctor & dtor
    ecApp();
    ~ecApp();

//// Operations
    virtual bool OnInit();
    virtual int OnExit();

    // Log to output window
    void Log(const wxString& msg);

    // Launch strFileName with the given viewer, or find a viewer if strViewer s empty
    bool Launch(const wxString & strFileName,const wxString &strViewer);

    // Under Unix, (*cmdLine) is set to the command string appropriate to setting up the variables
    // Under Windows, cmdLine isn't used
    bool PrepareEnvironment(bool bWithBuildTools = TRUE, wxString* cmdLine = NULL);

    // Fire off a subprocess to build the library or tests
    void Build(const wxString& strWhat = wxEmptyString) ;

    // Mount drive e.g. /c
    static void CygMount(wxChar c);
    // Mount in text mode e.g. /ecos-c
    static void CygMountText(wxChar c);

    bool InitializeHelpController();

    void SetStatusText(const wxString& text, bool clearFailingRulesPane = TRUE);

//// Helpers
    // Check if there is a (unique) .ecc file in dir
    bool FindSaveFileInDir(const wxString& dir, wxString& saveFile);

    // Initialize window settings object
    bool InitializeWindowSettings(bool beforeWindowConstruction);

    // Load resources from disk
    bool LoadResources();

    // Load a bitmap resource from resource.bin
    bool LoadBitmapResource(wxBitmap& bitmap, const wxString& filename, int bitmapType, bool addToMemoryFS);

    // Load a text resource from resource.bin
    bool LoadTextResource(wxString& text, const wxString& filename, bool addToMemoryFS);

    // Get a text resource from the memory filesystem
    bool GetMemoryTextResource(const wxString& filename, wxString& text);

    // Version-stamp the splash screen
    bool VersionStampSplashScreen();

//// Accessors

    // Get settings
    ecSettings& GetSettings() { return m_settings; }

    // Get help controller
    wxHelpController& GetHelpController() { return * m_helpController; }
    bool HasHelpController() const { return (m_helpController != NULL); }
    void DestroyHelpController() { if (m_helpController) delete m_helpController; m_helpController = NULL; }

    // Get app dir
    wxString GetAppDir() const { return m_appDir; }

    // Prepend the current app program directory to the name
    wxString GetFullAppPath(const wxString& filename) const;

    // Are we running in 32K colours or more?
    bool GetHiColour() const;

    // Get main icon
    const wxIcon& GetMainIcon() const { return m_mainIcon; }
    void SetMainIcon(const wxIcon& icon) { m_mainIcon = icon; }

    // What's This? menu with single item
    wxMenu* GetWhatsThisMenu() const { return m_whatsThisMenu; }

    // Main frame
    ecMainFrame* GetMainFrame() const { return m_mainFrame; }

    // Config tree control
    ecConfigTreeCtrl* GetTreeCtrl() const ;

    // MLT window
    ecMemoryLayoutWindow* GetMLTWindow() const ;

    // Document manager
    wxDocManager* GetDocManager() const { return m_docManager; }

    // Get active document
    ecConfigToolDoc* GetConfigToolDoc() const;

    wxString GetHelpFile() const { return m_helpFile; }
    void SetHelpFile(const wxString& file) { m_helpFile = file; }

    wxFileSystem* GetFileSystem() const { return m_fileSystem; }

    // Lock out value changes while conflicts are being resolved, for example
    bool GetValuesLocked() { return (m_valuesLocked > 0); }
    void LockValues() { m_valuesLocked ++; }
    void UnlockValues() { m_valuesLocked --; wxASSERT( m_valuesLocked >= 0); }

//// Events
    void OnWhatsThis(wxCommandEvent& event);
    void OnProcessTerminated(wxProcess* process);


//// Data members
    ecSettings          m_settings;
    wxString            m_appDir;
    wxHelpController*   m_helpController;
    wxIcon              m_mainIcon; // Reuse in dialogs
    wxMenu*             m_whatsThisMenu;
    wxDocManager*       m_docManager;
    ecMainFrame*        m_mainFrame;
    static bool         sm_arMounted[26];
    int                 m_valuesLocked; // Lock values from being changed

public:

    ecSplashScreen*     m_splashScreen;
    wxString            m_strOriginalPath;
    ecConfigToolDoc*    m_currentDoc;
    wxString            m_helpFile;
    ecPipedProcess*     m_pipedProcess;
    wxBitmap            m_splashScreenBitmap;
    wxZipFSHandler*     m_zipHandler;
    wxFileSystem*       m_fileSystem;

DECLARE_EVENT_TABLE()
};

DECLARE_APP(ecApp);

/*
 * ecPingTimer
 * Just to force idle processing now and again while
 * the library or tests are building
 */

class ecPingTimer: public wxTimer
{
public:
    ecPingTimer() {}

    virtual void Notify() ;
};

/*
 * ecPipedProcess
 * For running builds and capturing the output
 */

class ecPipedProcess : public wxProcess
{
public:
    ecPipedProcess()
    {
        Redirect();
        m_pingTimer.Start(100);
        m_pid = 0;
    }
    ~ecPipedProcess()
    {
        m_pingTimer.Stop();
    }

//// Overrides
    virtual void OnTerminate(int pid, int status);

//// Operations
    virtual bool HasInput();

//// Accessors
    void SetPid(long pid) { m_pid = pid; }
    long GetPid() const { return m_pid; }

protected:
    ecPingTimer m_pingTimer;
    long        m_pid;
};

// IDs for the controls and the menu commands

//// Menus & tools
#define ecID_NEW_REGION                 2102
#define ecID_NEW_SECTION                2103
#define ecID_DELETE                     2104
#define ecID_PROPERTIES                 2105

// File menu
#define ecID_IMPORT                     2150
#define ecID_EXPORT                     2151

// Edit menu
#define ecID_SAVE_OUTPUT                2152
#define ecID_FIND_NEXT                  2153

// View menu
#define ecID_SETTINGS                   2162
#define ecID_TOOLBARS                   2163
#define ecID_TOOLBARS_STANDARD          2164
#define ecID_TOOLBARS_MEMORY            2165
#define ecID_VIEW_NEXT                  2166
#define ecID_VIEW_PREVIOUS              2167
#define ecID_TOGGLE_CONFLICTS           2168
#define ecID_TOGGLE_PROPERTIES          2169
#define ecID_TOGGLE_MEMORY              2170
#define ecID_TOGGLE_SHORT_DESCR         2171
#define ecID_TOGGLE_OUTPUT              2172

// Build menu
#define ecID_STOP_BUILD                 2180
#define ecID_BUILD_LIBRARY              2181
#define ecID_BUILD_TESTS                2182
#define ecID_CLEAN                      2183
#define ecID_BUILD_OPTIONS              2184
#define ecID_BUILD_REPOSITORY           2185
#define ecID_BUILD_TEMPLATES            2186
#define ecID_BUILD_PACKAGES             2187
#define ecID_GENERATE_BUILD_TREE        2188

// Tools menu
#define ecID_PATHS                      2200
#define ecID_PATHS_BUILD_TOOLS          2201
#define ecID_PATHS_USER_TOOLS           2202
#define ecID_SHELL                      2203
#define ecID_RUN_TESTS                  2204
#define ecID_PLATFORMS                  2205
#define ecID_RESOLVE_CONFLICTS          2206
#define ecID_ADMINISTRATION             2207
#define ecID_TOOLS_OPTIONS              2208
#define ecID_INDEX_DOCS                 2209

// Help menu
#define ecID_CONFIGTOOL_HELP            2220
#define ecID_ECOS_HELP                  2221
#define ecID_REDHAT_WEB                 2222
#define ecID_REDHAT_WEB_HOME            2223
#define ecID_REDHAT_WEB_ECOS            2224
#define ecID_REDHAT_WEB_NET_RELEASE     2225
#define ecID_REDHAT_WEB_UITRON          2226
#define ecID_CONTEXT_HELP               2227
#define ecID_REPOSITORY_INFO            2228

// Tree right-click menu
#define ecID_WHATS_THIS            2250
#define ecID_TREE_PROPERTIES            2251
#define ecID_TREE_RESTORE_DEFAULTS      2252
#define ecID_TREE_VISIT_DOC             2253
#define ecID_TREE_VIEW_HEADER           2254
#define ecID_TREE_UNLOAD_PACKAGE        2255

// Conflict window right-click menu
#define ecID_LOCATE_ITEM                2256
#define ecID_RESOLVE_ITEM               2257

//// Controls & windows

#define ecID_TREE_CTRL                  2000
#define ecID_SPLITTER_WINDOW            2001
#define ecID_VALUE_WINDOW               2002
#define ecID_MAIN_FRAME                 2003
#define ecID_SCROLLED_WINDOW            2004
#define ecID_TOOLBAR                    2005

#define ecID_CONFLICTS_WINDOW           2006
#define ecID_PROPERTIES_WINDOW          2007
#define ecID_MEMORY_WINDOW              2008
#define ecID_SHORT_DESCR_WINDOW         2009
#define ecID_OUTPUT_WINDOW              2010

#define ecID_CONFIG_SASH_WINDOW         2020
#define ecID_CONFLICTS_SASH_WINDOW      2021
#define ecID_PROPERTIES_SASH_WINDOW     2022
#define ecID_MEMORY_SASH_WINDOW         2023
#define ecID_SHORT_DESCR_SASH_WINDOW    2024
#define ecID_OUTPUT_SASH_WINDOW         2025
#define ecID_ABOUT_DIALOG               2026
#define ecID_ABOUT_DIALOG_HTML_WINDOW   2027
#define ecID_FIND_DIALOG                2028

// The control used to edit a config item value
#define ecID_ITEM_EDIT_WINDOW           2030

#define ecID_SETTINGS_DIALOG            2031
#define ecID_SETTINGS_NOTEBOOK          2032
#define ecID_SETTINGS_DISPLAY           2033
#define ecID_SETTINGS_VIEWER            2034
#define ecID_SETTINGS_PATH              2035
#define ecID_SETTINGS_CONFLICT_RESOLUTION 2036
#define ecID_SETTINGS_RUN               2037

#define ecID_SECTION_DIALOG             2040
#define ecID_SECTION_NOTEBOOK           2041
#define ecID_SECTION_GENERAL            2042
#define ecID_SECTION_RELOCATION         2043
#define ecID_SECTION_NOTE               2044

#define ecID_PLATFORM_EDITOR_DIALOG     2050
#define ecID_BUILD_OPTIONS_DIALOG       2051
#define ecID_PLATFORMS_DIALOG           2052
#define ecID_TEMPLATES_DIALOG           2053
#define ecID_ADMIN_DIALOG               2054
#define ecID_PACKAGES_DIALOG            2055

#define ecID_RUN_TESTS_DIALOG           2056
#define ecID_RUN_TESTS_NOTEBOOK         2057
#define ecID_RUN_TESTS_EXECUTABLES      2058
#define ecID_RUN_TESTS_OUTPUT           2059
#define ecID_RUN_TESTS_SUMMARY          2060

#define ecID_RUN_TESTS_RUN              2061
#define ecID_RUN_TESTS_PROPERTIES       2062

//#define ecID_RUN_TESTS_TEST_LIST        2062
//#define ecID_RUN_TESTS_SUMMARY_LIST     2063
//#define ecID_CONFIG_PROPERTIES_LIST     2064

#define ecID_CHOOSE_REPOSITORY_DIALOG   2070
#define ecID_CONFIG_PROPERTIES_DIALOG   2071

#define ecID_LICENSE_DIALOG             2080
#define ecID_LICENSE_TEXT               2081

#define ecID_FOLDER_DIALOG              2090
#define ecID_FOLDER_DIALOG_MSG          2091
#define ecID_FOLDER_DIALOG_BROWSE       2092
#define ecID_FOLDER_DIALOG_PATHS        2093

#define ecID_REPOS_DIALOG_HTML_WINDOW   2094
#define ecID_REPOS_DIALOG               2095

#define ecID_BUILD_TOOLS_DIALOG         2300
#define ecID_USER_TOOLS_DIALOG          2301
#define ecID_EDIT_STRING_DIALOG         2302
#define ecID_STRING_EDIT_TEXTCTRL       2303

#endif
        // _ECOS_CONFIGTOOL_H_
