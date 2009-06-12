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
// mainwin.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians, jld
// Date:        2000/08/24
// Version:     $Id: mainwin.cpp,v 1.57 2002/02/28 18:30:35 julians Exp $
// Purpose:
// Description: Implementation file for the ConfigTool main window
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
#pragma implementation "mainwin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/wxhtml.h"
#include "wx/filedlg.h"
#include "wx/wfstream.h"

#include "mainwin.h"
#include "configtool.h"
#include "configtree.h"
#include "propertywin.h"
#include "conflictwin.h"
#include "mltwin.h"
#include "outputwin.h"
#include "shortdescrwin.h"
#include "conflictsdlg.h"
#include "aboutdlg.h"
#include "finddlg.h"
#include "settingsdlg.h"
#include "platformsdlg.h"
#include "buildoptionsdlg.h"
#include "templatesdlg.h"
#include "admindlg.h"
#include "packagesdlg.h"
#include "configtooldoc.h"
#include "configtoolview.h"
#include "folderdlg.h"
#include "reposdlg.h"
#include "docsystem.h"
#include "symbols.h"

#ifdef __WXMSW__
#include "wx/msw/winundef.h"
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------
// the application icon
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "bitmaps/configtool.xpm"
#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/paste.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/delete.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/cshelp.xpm"
#include "bitmaps/search.xpm"
#include "bitmaps/stopbuild.xpm"
#include "bitmaps/buildlibrary.xpm"
#include "bitmaps/newregion.xpm"
#include "bitmaps/newsection.xpm"
#include "bitmaps/properties.xpm"
#endif

// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ecMainFrame, wxDocParentFrame)
    EVT_MENU(wxID_EXIT,  ecMainFrame::OnQuit)
    EVT_MENU(wxID_ABOUT, ecMainFrame::OnAbout)
    EVT_MENU(ecID_REPOSITORY_INFO, ecMainFrame::OnRepositoryInfo)
    EVT_MENU(wxID_FIND, ecMainFrame::OnFind)
    EVT_MENU(ecID_FIND_NEXT, ecMainFrame::OnFindNext)
    EVT_SASH_DRAGGED_RANGE(ecID_CONFIG_SASH_WINDOW, ecID_OUTPUT_SASH_WINDOW, ecMainFrame::OnSashDrag)
    EVT_SIZE(ecMainFrame::OnSize)
    EVT_IDLE(ecMainFrame::OnIdle)
    EVT_CLOSE(ecMainFrame::OnCloseWindow)
    EVT_MENU(ecID_TOGGLE_CONFLICTS, ecMainFrame::OnToggleWindow)
    EVT_MENU(ecID_TOGGLE_PROPERTIES, ecMainFrame::OnToggleWindow)
    EVT_MENU(ecID_TOGGLE_MEMORY, ecMainFrame::OnToggleWindow)
    EVT_MENU(ecID_TOGGLE_SHORT_DESCR, ecMainFrame::OnToggleWindow)
    EVT_MENU(ecID_TOGGLE_OUTPUT, ecMainFrame::OnToggleWindow)
    EVT_MENU(ecID_TOOLBARS, ecMainFrame::OnToggleToolbar)
    EVT_MENU(ecID_CONFIGTOOL_HELP, ecMainFrame::OnHelpConfigtool)
    EVT_MENU(ecID_ECOS_HELP, ecMainFrame::OnHelpEcos)
    EVT_MENU(ecID_CONTEXT_HELP, ecMainFrame::OnHelpContext)
    EVT_MENU(ecID_RESOLVE_CONFLICTS, ecMainFrame::OnResolveConflicts)
    EVT_UPDATE_UI(ecID_RESOLVE_CONFLICTS, ecMainFrame::OnUpdateResolveConflicts)
    EVT_MENU(ecID_SETTINGS, ecMainFrame::OnSettings)
    EVT_MENU(ecID_PLATFORMS, ecMainFrame::OnPlatforms)
    EVT_MENU(ecID_BUILD_OPTIONS, ecMainFrame::OnBuildOptions)
    EVT_MENU(ecID_PATHS_BUILD_TOOLS, ecMainFrame::OnBuildToolsPath)
    EVT_MENU(ecID_PATHS_USER_TOOLS, ecMainFrame::OnUserToolsPath)
    EVT_MENU(ecID_BUILD_TEMPLATES, ecMainFrame::OnTemplates)
    EVT_MENU(ecID_ADMINISTRATION, ecMainFrame::OnAdmin)
    EVT_MENU(ecID_BUILD_PACKAGES, ecMainFrame::OnPackages)
    EVT_MENU(ecID_RUN_TESTS, ecMainFrame::OnRunTests)
    EVT_MENU(ecID_BUILD_REPOSITORY, ecMainFrame::OnChooseRepository)
    EVT_MENU(ecID_WHATS_THIS, ecMainFrame::OnWhatsThis)
    EVT_MENU(ecID_SAVE_OUTPUT, ecMainFrame::OnSaveOutput)
    EVT_MENU(ecID_IMPORT, ecMainFrame::OnImport)
    EVT_MENU(ecID_EXPORT, ecMainFrame::OnExport)
    EVT_MENU(ecID_REDHAT_WEB_HOME, ecMainFrame::OnWebRedHatHome)
    EVT_MENU(ecID_REDHAT_WEB_ECOS, ecMainFrame::OnWebEcos)
    EVT_MENU(ecID_REDHAT_WEB_NET_RELEASE, ecMainFrame::OnWebNetRelease)
    EVT_MENU(ecID_REDHAT_WEB_UITRON, ecMainFrame::OnWebUitron)
    EVT_MENU(ecID_STOP_BUILD, ecMainFrame::OnStopBuild)
    EVT_MENU(ecID_BUILD_LIBRARY, ecMainFrame::OnBuildLibrary)
    EVT_MENU(ecID_BUILD_TESTS, ecMainFrame::OnBuildTests)
    EVT_MENU(ecID_CLEAN, ecMainFrame::OnClean)
    EVT_MENU(ecID_SHELL, ecMainFrame::OnShell)
    EVT_MENU(ecID_INDEX_DOCS, ecMainFrame::OnIndexDocs)
    EVT_MENU(ecID_GENERATE_BUILD_TREE, ecMainFrame::OnGenerateBuildTree)

    EVT_UPDATE_UI(ecID_PLATFORMS, ecMainFrame::OnUpdatePlatforms)
    EVT_UPDATE_UI(ecID_BUILD_OPTIONS, ecMainFrame::OnUpdateBuildOptions)
    EVT_UPDATE_UI(ecID_PATHS_BUILD_TOOLS, ecMainFrame::OnUpdateBuildToolsPath)
    EVT_UPDATE_UI(ecID_PATHS_USER_TOOLS, ecMainFrame::OnUpdateUserToolsPath)
    EVT_UPDATE_UI(ecID_BUILD_TEMPLATES, ecMainFrame::OnUpdateTemplates)
    EVT_UPDATE_UI(ecID_ADMINISTRATION, ecMainFrame::OnUpdateAdmin)
    EVT_UPDATE_UI(ecID_BUILD_PACKAGES, ecMainFrame::OnUpdatePackages)
    EVT_UPDATE_UI(ecID_RUN_TESTS, ecMainFrame::OnUpdateRunTests)
    EVT_UPDATE_UI(ecID_BUILD_REPOSITORY, ecMainFrame::OnUpdateChooseRepository)
    EVT_UPDATE_UI(ecID_CLEAN, ecMainFrame::OnUpdateClean)
    EVT_UPDATE_UI(ecID_REPOSITORY_INFO, ecMainFrame::OnUpdateRepositoryInfo)  
    EVT_UPDATE_UI(ecID_INDEX_DOCS, ecMainFrame::OnUpdateIndexDocs)

    EVT_UPDATE_UI(ecID_TOGGLE_CONFLICTS, ecMainFrame::OnUpdateToggleWindow)
    EVT_UPDATE_UI(ecID_TOGGLE_PROPERTIES, ecMainFrame::OnUpdateToggleWindow)
    EVT_UPDATE_UI(ecID_TOGGLE_MEMORY, ecMainFrame::OnUpdateToggleWindow)
    EVT_UPDATE_UI(ecID_TOGGLE_SHORT_DESCR, ecMainFrame::OnUpdateToggleWindow)
    EVT_UPDATE_UI(ecID_TOGGLE_OUTPUT, ecMainFrame::OnUpdateToggleWindow)
    EVT_UPDATE_UI(ecID_TOOLBARS, ecMainFrame::OnUpdateToggleToolbar)

    // Disable commands that don't make sense immediately or are not yet implemented.
    // Also, for text controls, disable if they are not being enabled by currently-focussed controls.
    EVT_UPDATE_UI(wxID_COPY, ecMainFrame::OnUpdateDisable)
    EVT_UPDATE_UI(wxID_CUT, ecMainFrame::OnUpdateDisable)
    EVT_UPDATE_UI(wxID_PASTE, ecMainFrame::OnUpdateDisable)
    EVT_UPDATE_UI(wxID_SELECTALL, ecMainFrame::OnUpdateSelectAll)
    EVT_UPDATE_UI(wxID_CLEAR, ecMainFrame::OnUpdateClear)

    EVT_UPDATE_UI(wxID_FIND, ecMainFrame::OnUpdateFind)
    EVT_UPDATE_UI(ecID_FIND_NEXT, ecMainFrame::OnUpdateFindNext)

    EVT_MENU(ecID_NEW_REGION, ecMainFrame::OnNewRegion)
    EVT_MENU(ecID_NEW_SECTION, ecMainFrame::OnNewSection)
    EVT_MENU(ecID_DELETE, ecMainFrame::OnDeleteRegionOrSection)
    EVT_MENU(ecID_PROPERTIES, ecMainFrame::OnRegionOrSectionProperties)

    EVT_UPDATE_UI(ecID_STOP_BUILD, ecMainFrame::OnUpdateStopBuild)
    EVT_UPDATE_UI(ecID_BUILD_LIBRARY, ecMainFrame::OnUpdateBuildLibrary)
    EVT_UPDATE_UI(ecID_BUILD_TESTS, ecMainFrame::OnUpdateBuildTests)
    EVT_UPDATE_UI(ecID_GENERATE_BUILD_TREE, ecMainFrame::OnUpdateGenerateBuildTree)

    EVT_UPDATE_UI(ecID_SHELL, ecMainFrame::OnUpdateShell)

    EVT_UPDATE_UI(ecID_NEW_REGION, ecMainFrame::OnUpdateNewRegion)
    EVT_UPDATE_UI(ecID_NEW_SECTION, ecMainFrame::OnUpdateNewSection)
    EVT_UPDATE_UI(ecID_DELETE, ecMainFrame::OnUpdateDeleteRegionOrSection)
    EVT_UPDATE_UI(ecID_PROPERTIES, ecMainFrame::OnUpdateRegionOrSectionProperties)

    EVT_UPDATE_UI(ecID_IMPORT, ecMainFrame::OnUpdateImport)
    EVT_UPDATE_UI(ecID_EXPORT, ecMainFrame::OnUpdateExport)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecMainFrame::ecMainFrame(wxDocManager *manager, const wxString& title, const wxPoint& pos, const wxSize& size):
    wxDocParentFrame(manager, (wxFrame *)NULL, ecID_MAIN_FRAME, title, pos, size, wxDEFAULT_FRAME_STYLE|wxCLIP_CHILDREN|wxNO_FULL_REPAINT_ON_RESIZE )
{
    m_splitter = NULL;
    m_scrolledWindow = NULL;
    m_tree = NULL;
    m_configSashWindow = NULL;
    m_valueWindow = NULL;
    m_outputWindow = NULL;
    m_shortDescrWindow = NULL;
    m_conflictsSashWindow = NULL;
    m_propertiesSashWindow = NULL;
    m_memorySashWindow = NULL;
    m_shortDescrSashWindow = NULL;
    m_outputSashWindow = NULL;
    m_propertyListWindow = NULL;
    m_findDialog = NULL;
    m_mltWindow = NULL;
    
#ifdef __WXMAC__
    // we need this in order to allow the about menu relocation, since ABOUT is
    // not the default id of the about menu
    wxApp::s_macAboutMenuItemId = wxID_EXIT;
#endif
    
    CreateWindows();

    wxIcon mainIcon(wxICON(configtool));
    wxGetApp().SetMainIcon(mainIcon);
    
    // set the frame icon
    SetIcon(mainIcon);
    
    // create a menu bar

    // File menu
    wxMenu *fileMenu = new wxMenu(wxT(""), wxMENU_TEAROFF);
    
    fileMenu->Append(wxID_NEW, _("&New\tCtrl+N"), _("Creates a new document"));
#if 0
    wxMenuItem* menuItem = new wxMenuItem(fileMenu, wxID_NEW, _("&New\tCtrl+N"), _("Creates a new document"));
    menuItem->SetBitmaps(wxBITMAP(new));
    fileMenu->Append(menuItem);
#endif
    
    fileMenu->Append(wxID_OPEN, _("&Open\tCtrl+O"), _("Opens an existing document"));
    fileMenu->Append(wxID_SAVE, _("&Save\tCtrl+S"), _("Saves the active document"));
    fileMenu->Append(wxID_SAVEAS, _("Save &As..."), _("Saves the active document with a new name"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ecID_IMPORT, _("&Import..."), _("Imports a minimal configuration exported from another configuration"));
    fileMenu->Append(ecID_EXPORT, _("&Export..."), _("Exports a minimal configuration for importing into another configuration"));
    fileMenu->AppendSeparator();   
    fileMenu->Append(wxID_EXIT, _("E&xit\tAlt+X"), _("Quits the application"));

    // A history of files visited. Use this menu.
    wxGetApp().GetDocManager()->FileHistoryUseMenu(fileMenu);
    // Load file history
    {
        wxConfig config(wxGetApp().GetSettings().GetConfigAppName());
	config.SetPath(wxT("FileHistory/"));
        wxGetApp().GetDocManager()->FileHistoryLoad(config);
    }
    
    // Edit menu
    wxMenu* editMenu = new wxMenu(wxT(""), wxMENU_TEAROFF);
    
    editMenu->Append(wxID_CUT, _("Cu&t\tCtrl+X"), _("Cuts the output pane selection and moves it to the Clipboard"));
    editMenu->Append(wxID_COPY, _("&Copy\tCtrl+C"), _("Copies the output pane selection to the clipboard"));
    editMenu->Append(wxID_PASTE, _("&Paste\tCtrl+V"), _("Inserts Clipboard contents"));
    editMenu->Append(wxID_CLEAR, _("&Clear"), _("Erases everything in the output pane"));
    editMenu->AppendSeparator();
    editMenu->Append(wxID_SELECTALL, _("&Select All\tCtrl+A"), _("Selects the entire output pane"));
    editMenu->AppendSeparator();
    editMenu->Append(wxID_FIND, _("&Find...\tCtrl+F"), _("Finds the specified text"));
    editMenu->Append(ecID_FIND_NEXT, _("Find &Next\tF3"), _("Finds the next item matching the Find text"));
    editMenu->AppendSeparator();
    editMenu->Append(ecID_SAVE_OUTPUT, _("Sa&ve Output..."), _("Saves the contents of the output pane"));
    
    // View menu
    wxMenu* viewMenu = new wxMenu(wxT(""), wxMENU_TEAROFF);
    
    viewMenu->Append(ecID_SETTINGS, _("&Settings...\tCtrl+T"), _("Shows the application settings dialog"));
    viewMenu->AppendSeparator();
    viewMenu->Append(ecID_TOOLBARS, _("&Toolbar"), _("Shows or hides the toolbar"), TRUE);
    viewMenu->Append(ecID_TOGGLE_PROPERTIES, _("&Properties\tAlt+1"), _("Shows or hides the properties window"), TRUE);
    viewMenu->Append(ecID_TOGGLE_OUTPUT, _("&Output\tAlt+2"), _("Shows the output window"), TRUE);
    viewMenu->Append(ecID_TOGGLE_SHORT_DESCR, _("&Short Description\tAlt+3"), _("Shows or hides the short description window"), TRUE);

    viewMenu->Append(ecID_TOGGLE_CONFLICTS, _("&Conflicts\tAlt+4"), _("Shows or hides the conflicts window"), TRUE);
#if ecUSE_MLT
    viewMenu->Append(ecID_TOGGLE_MEMORY, _("&Memory Layout\tAlt+5"), _("Shows or hides the memory layout window"), TRUE);
    //viewMenu->Enable(ecID_TOGGLE_MEMORY, FALSE);
#endif

    // Not clear what these do, so let's not have them.
    //viewMenu->Append(ecID_VIEW_NEXT, _("&Next\tAlt+F6"), _("Selects the next visible pane"));
    //viewMenu->Append(ecID_VIEW_PREVIOUS, _("&Previous\tShift+Alt+F6"), _("Selects the previous visible pane"));
    
    // Build menu
    wxMenu* buildMenu = new wxMenu(wxT(""), wxMENU_TEAROFF);
    
    buildMenu->Append(ecID_BUILD_LIBRARY, _("&Library\tF7"), _("Builds the library"));
    buildMenu->Append(ecID_BUILD_TESTS, _("&Tests\tShift+F7"), _("Builds the tests"));
    buildMenu->Append(ecID_CLEAN, _("&Clean"), _("Deletes intermediate and output files"));
    buildMenu->Append(ecID_STOP_BUILD, _("&Stop"), _("Stops the build"));
    buildMenu->AppendSeparator();
    buildMenu->Append(ecID_GENERATE_BUILD_TREE, _("&Generate Build Tree"), _("Explicitly recreates the build tree"));
    buildMenu->AppendSeparator();
    buildMenu->Append(ecID_BUILD_OPTIONS, _("&Options..."), _("Changes build options"));
    buildMenu->Append(ecID_BUILD_REPOSITORY, _("&Repository..."), _("Selects repository"));
    buildMenu->Append(ecID_BUILD_TEMPLATES, _("&Templates..."), _("Selects the package templates"));
    buildMenu->Append(ecID_BUILD_PACKAGES, _("&Packages..."), _("Selects individual packages"));
    
    // Tools menu
    wxMenu* toolsMenu = new wxMenu(wxT(""), wxMENU_TEAROFF);
    
    wxMenu* pathMenu = new wxMenu;
    pathMenu->Append(ecID_PATHS_BUILD_TOOLS, _("&Build Tools..."), _("Specifies the folder containing the build tools"));
    pathMenu->Append(ecID_PATHS_USER_TOOLS, _("&User Tools..."), _("Specifies the folder containing the user tools"));
    toolsMenu->Append(ecID_PATHS, _("&Paths"), pathMenu);

    toolsMenu->Append(ecID_SHELL, _("&Shell..."), _("Invokes a command shell"));
    toolsMenu->Append(ecID_RUN_TESTS, _("&Run Tests...\tCtrl+F5"), _("Runs the configuration tests"));
    toolsMenu->Append(ecID_PLATFORMS, _("&Platforms..."), _("Edits the platforms list"));
    toolsMenu->Append(ecID_RESOLVE_CONFLICTS, _("Resolve &Conflicts..."), _("Resolves conflicts"));
    toolsMenu->Append(ecID_ADMINISTRATION, _("&Administration..."), _("Performs repository administration tasks"));
#if 0
    toolsMenu->AppendSeparator();
    toolsMenu->Append(ecID_TOOLS_OPTIONS, _("&Options..."), _("Changes configuration options"));
#endif
    toolsMenu->Append(ecID_INDEX_DOCS, _("Regenerate Help &Index"), _("Regenerates the online help contents"));
    
    // Help menu
    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(ecID_CONFIGTOOL_HELP, _("&Configuration Tool Help\tShift+F1"), _("Displays help"));
    helpMenu->Append(ecID_ECOS_HELP, _("&eCos Documentation"), _("Displays the documentation home page"));
    helpMenu->Append(ecID_CONTEXT_HELP, _("&Help On..."), _("Displays help for clicked-on windows"));
    helpMenu->AppendSeparator();
    
    wxMenu* webMenu = new wxMenu;
    webMenu->Append(ecID_REDHAT_WEB_HOME, _("&Red Hat Home Page"), _("Opens the Red Hat home page"));
    webMenu->Append(ecID_REDHAT_WEB_ECOS, _("&eCos Product Page"), _("Opens the eCos product page"));
    webMenu->Append(ecID_REDHAT_WEB_NET_RELEASE, _("eCos &Net Release Page"), _("Opens the eCos net release page"));
    webMenu->AppendSeparator();
    webMenu->Append(ecID_REDHAT_WEB_UITRON, _("&ITRON"), _("Opens the ITRON specification page"));
    
//    helpMenu->Append(ecID_REDHAT_WEB, _("&Red Hat on the Web"), webMenu);
//    helpMenu->AppendSeparator();
    helpMenu->Append(ecID_REPOSITORY_INFO, _("Repository &Information...\tCtrl+I"), _("Displays information about the current repository"));
    helpMenu->Append(wxID_ABOUT, _("&About the Configuration Tool..."), _("Displays program information, version and copyright"));
    
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _("&File"));
    menuBar->Append(editMenu, _("&Edit"));
    menuBar->Append(viewMenu, _("&View"));
    menuBar->Append(buildMenu, _("&Build"));
    menuBar->Append(toolsMenu, _("&Tools"));
    menuBar->Append(helpMenu, _("&Help"));
    
    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
    
    // Create the toolbar
    RecreateToolbar();
    
    if (!wxGetApp().GetSettings().m_showToolBar)
        GetToolBar()->Show( FALSE );
    
    // Create the status bar
    CreateStatusBar(4, wxST_SIZEGRIP);
    
    int* widths = new int[4];
    widths[0] = -1; widths[1] = 100; widths[2] = 40; widths[3] = 80;
    SetStatusWidths(4, widths);
    delete[] widths;
    
    SetStatusText(_("No conflicts"), ecFailRulePane);
    SetStatusText(_("Ready"), ecStatusPane);
}

// Create the windows
void ecMainFrame::CreateWindows()
{
    ecSettings& stg = wxGetApp().GetSettings();
    
    // Create the sash layout windows first
    
    // Sash window for the output window
    m_outputSashWindow = new wxSashLayoutWindow(this, ecID_OUTPUT_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_SIBLINGS);
    m_outputSashWindow->SetDefaultSize(stg.m_outputSashSize);
    m_outputSashWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_outputSashWindow->SetAlignment(wxLAYOUT_BOTTOM);
    m_outputSashWindow->SetSashVisible(wxSASH_TOP, TRUE);
    m_outputWindow = new ecOutputWindow(m_outputSashWindow, ecID_OUTPUT_WINDOW, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxCLIP_CHILDREN|wxTE_READONLY|wxTE_RICH);
    m_outputWindow->SetHelpText(_("The output window displays various warning and informational messages."));
    
    // Sash window for the memory window
    m_memorySashWindow = new wxSashLayoutWindow(this, ecID_MEMORY_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxSW_3D|wxCLIP_SIBLINGS);
    m_memorySashWindow->SetDefaultSize(stg.m_memorySashSize);
    m_memorySashWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_memorySashWindow->SetAlignment(wxLAYOUT_BOTTOM);
    m_memorySashWindow->SetSashVisible(wxSASH_TOP, TRUE);
    //wxTextCtrl* memoryWindow = new wxTextCtrl(m_memorySashWindow, ecID_MEMORY_WINDOW, wxT("This will be the memory layout window."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxCLIP_CHILDREN|wxTE_NO_VSCROLL|wxTE_READONLY);
    m_mltWindow = new ecMemoryLayoutWindow(m_memorySashWindow, ecID_MEMORY_WINDOW, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN|wxSUNKEN_BORDER);
    m_mltWindow->SetHelpText(_("The memory layout window presents a graphical view of the memory layout of the currently selected\ncombination of target architecture, platform and start-up type."));
    
    // Sash window for the config tree
    m_configSashWindow = new wxSashLayoutWindow(this, ecID_CONFIG_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_CHILDREN/*|wxCLIP_SIBLINGS*/);
    m_configSashWindow->SetDefaultSize(stg.m_treeSashSize);
    m_configSashWindow->SetOrientation(wxLAYOUT_VERTICAL);
    m_configSashWindow->SetAlignment(wxLAYOUT_LEFT);
    m_configSashWindow->SetSashVisible(wxSASH_RIGHT, TRUE);
    
    // Sash window for the conflicts window
    m_conflictsSashWindow = new wxSashLayoutWindow(this, ecID_CONFLICTS_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_SIBLINGS);
    m_conflictsSashWindow->SetDefaultSize(stg.m_conflictsSashSize);
    m_conflictsSashWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_conflictsSashWindow->SetAlignment(wxLAYOUT_TOP);
    m_conflictsSashWindow->SetSashVisible(wxSASH_BOTTOM, TRUE);
    m_conflictsWindow = new ecConflictListCtrl(m_conflictsSashWindow, ecID_CONFLICTS_WINDOW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxCLIP_CHILDREN|wxSUNKEN_BORDER);
    m_conflictsWindow->SetHelpText(_("The conflicts window lists any outstanding conflicts in the configuration."));
    
    // Sash window for the properties window
    m_propertiesSashWindow = new wxSashLayoutWindow(this, ecID_PROPERTIES_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_SIBLINGS);
    m_propertiesSashWindow->SetDefaultSize(stg.m_propertiesSashSize);
    m_propertiesSashWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_propertiesSashWindow->SetAlignment(wxLAYOUT_TOP);
    m_propertiesSashWindow->SetSashVisible(wxSASH_BOTTOM, TRUE);
//    wxTextCtrl* propertiesWindow = new wxTextCtrl(m_propertiesSashWindow, ecID_PROPERTIES_WINDOW, wxT("This will be the properties window."), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxCLIP_CHILDREN|wxTE_NO_VSCROLL|wxTE_READONLY);
    m_propertyListWindow = new ecPropertyListCtrl(m_propertiesSashWindow, ecID_PROPERTIES_WINDOW, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxCLIP_CHILDREN|wxLC_VRULES|wxLC_HRULES|wxSUNKEN_BORDER);
    m_propertyListWindow->SetHelpText(_("The properties window shows the properties of the selected configuration item."));
    
    // Sash window for the short description window
    m_shortDescrSashWindow = new wxSashLayoutWindow(this, ecID_SHORT_DESCR_SASH_WINDOW, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxCLIP_SIBLINGS);
    m_shortDescrSashWindow->SetDefaultSize(stg.m_shortDescrSashSize);
    m_shortDescrSashWindow->SetOrientation(wxLAYOUT_HORIZONTAL);
    m_shortDescrSashWindow->SetAlignment(wxLAYOUT_TOP);
    //m_shortDescrSashWindow->SetSashVisible(wxSASH_TOP, TRUE);
    m_shortDescrWindow = new ecShortDescriptionWindow(m_shortDescrSashWindow, ecID_SHORT_DESCR_WINDOW, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxCLIP_CHILDREN/*|wxTE_NO_VSCROLL*/|wxTE_READONLY);
    m_shortDescrWindow->SetBackgroundColour(wxColour(255, 255, 225));
    m_shortDescrWindow->SetHelpText(_("The short description window displays brief help on a selected configuration item."));
    
    // Create a composite widget to represent the scrolling config window
    m_scrolledWindow = new ecSplitterScrolledWindow(m_configSashWindow, ecID_SCROLLED_WINDOW, wxDefaultPosition,
        wxSize(400, 100), wxNO_BORDER | wxCLIP_CHILDREN | wxVSCROLL);
    m_splitter = new wxThinSplitterWindow(m_scrolledWindow, ecID_SPLITTER_WINDOW, wxDefaultPosition,
        wxSize(400, 100), wxSP_3DBORDER | wxCLIP_CHILDREN /* | wxSP_LIVE_UPDATE */);
    m_splitter->SetSashSize(2);
    m_tree = new ecConfigTreeCtrl(m_splitter, ecID_TREE_CTRL, wxDefaultPosition,
        wxSize(200, 100), wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxNO_BORDER );
    m_valueWindow = new ecValueWindow(m_splitter, ecID_VALUE_WINDOW, wxDefaultPosition,
        wxSize(200, 100), wxNO_BORDER);
    m_splitter->SplitVertically(m_tree, m_valueWindow);
    m_splitter->SetMinimumPaneSize(100);
    //m_splitter->AdjustScrollbars();
    m_splitter->SetSashPosition(wxGetApp().GetSettings().m_configPaneWidth);
    m_scrolledWindow->SetTargetWindow(m_tree);  
    m_scrolledWindow->EnableScrolling(FALSE, FALSE);
    m_tree->SetHelpText(_("The configuration window is the principal window used to configure eCos.\nIt takes the form of a tree-based representation of the configuration items within the currently loaded eCos packages."));
    m_valueWindow->SetHelpText(m_tree->GetHelpText());
    
    // Let the two controls know about each other
    m_valueWindow->SetTreeCtrl(m_tree);
    m_tree->SetCompanionWindow(m_valueWindow);

    // Set visibility according to config settings
    if (!wxGetApp().GetSettings().m_showConflictsWindow)
        m_conflictsSashWindow->Show(FALSE);
    if (!wxGetApp().GetSettings().m_showPropertiesWindow)
        m_propertiesSashWindow->Show(FALSE);
    if (!wxGetApp().GetSettings().m_showShortDescrWindow)
        m_shortDescrSashWindow->Show(FALSE);
    if (!wxGetApp().GetSettings().m_showOutputWindow)
        m_outputSashWindow->Show(FALSE);
    if (!wxGetApp().GetSettings().m_showMemoryWindow)
        m_memorySashWindow->Show(FALSE);
}
// event handlers

void ecMainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void ecMainFrame::OnFind(wxCommandEvent& event)
{
    if (m_findDialog)
    {
        m_findDialog->Raise();
        return;
    }

    m_findDialog = new ecFindDialog (this, ecID_FIND_DIALOG, _("Find in configuration"));
    m_findDialog->Show(TRUE);

    // For some reason, under Windows, the text control doesn't get the focus if we set the focus
    // and then call Show. We have to set the focus afterwards instead.
    m_findDialog->FindWindow(ecID_FIND_DIALOG_WHAT)->SetFocus();
}

void ecMainFrame::OnFindNext(wxCommandEvent& event)
{
    if (wxGetApp().GetConfigToolDoc())
    {
        ecConfigToolView* view = (ecConfigToolView*) wxGetApp().GetConfigToolDoc()->GetFirstView() ;

        view->DoFind(wxGetApp().GetSettings().m_findText, this);
    }
}

void ecMainFrame::OnUpdateFind(wxUpdateUIEvent& event)
{
    event.Enable( wxGetApp().GetConfigToolDoc() != NULL );
}

void ecMainFrame::OnUpdateFindNext(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && !wxGetApp().GetSettings().m_findText.IsEmpty() );
}

void ecMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
//    ecAboutDialog dialog(this, ecID_ABOUT_DIALOG, _("About eCos Configuration Tool"));
//    dialog.ShowModal();
	wxString msg;
	msg.Printf("eCos Configuration Tool %s (%s %s)\n\nCopyright (c) Red Hat, Inc. 1998-2002\nCopyright (c) John Dallaway 2003", ecCONFIGURATION_TOOL_VERSION, __DATE__, __TIME__);
    wxMessageBox(msg, _("About eCos Configuration Tool"), wxICON_INFORMATION | wxOK);
}

void ecMainFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    // First, we need to resize the sash windows proportionately,
    // or we may end up with negative sizes, etc.
    wxRect rect = GetRect();
    if (rect != wxGetApp().GetSettings().m_frameSize)
    {
        double factorX = ((double) rect.GetWidth() / (double) wxGetApp().GetSettings().m_frameSize.GetWidth());
        double factorY = ((double) rect.GetHeight() / (double) wxGetApp().GetSettings().m_frameSize.GetHeight());
        
        wxNode* node = GetChildren().First();
        while (node)
        {
            wxWindow* win = (wxWindow*) node->Data();
            if (win->IsKindOf(CLASSINFO(wxSashLayoutWindow)))
            {
                wxSashLayoutWindow* sashWin = (wxSashLayoutWindow*) win;
                wxSize sz = sashWin->GetSize();
                sashWin->SetDefaultSize(wxSize((int) ((double) sz.x * factorX), (int) ((double) sz.y * factorY)));
            }
            node = node->Next();
        }
    }
    
    wxLayoutAlgorithm layout;
    layout.LayoutFrame(this);
    
    wxGetApp().GetSettings().m_frameSize = rect;
}

void ecMainFrame::RecreateToolbar()
{
    // delete and recreate the toolbar
    wxToolBarBase *toolBar = GetToolBar();
    if (toolBar)
    {
        delete toolBar;
        SetToolBar(NULL);
    }
    
    long style = wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL | wxTB_DOCKABLE;
    
    toolBar = CreateToolBar(style, ecID_TOOLBAR);
    
    toolBar->SetMargins( 4, 4 );
    
    // Set up toolbar
    wxBitmap toolBarBitmaps[20];
    
    toolBarBitmaps[0] = wxBITMAP(new);
    toolBarBitmaps[1] = wxBITMAP(open);
    toolBarBitmaps[2] = wxBITMAP(save);
    toolBarBitmaps[3] = wxBITMAP(copy);
    toolBarBitmaps[4] = wxBITMAP(cut);
    toolBarBitmaps[5] = wxBITMAP(paste);
    toolBarBitmaps[6] = wxBITMAP(search);
    toolBarBitmaps[7] = wxBITMAP(stopbuild);
    toolBarBitmaps[8] = wxBITMAP(buildlibrary);
    toolBarBitmaps[9] = wxBITMAP(help);
    toolBarBitmaps[10] = wxBITMAP(newregion);
    toolBarBitmaps[11] = wxBITMAP(newsection);
    toolBarBitmaps[12] = wxBITMAP(delete);
    toolBarBitmaps[13] = wxBITMAP(properties);
    toolBarBitmaps[14] = wxBITMAP(cshelp);
    
    toolBar->AddTool(wxID_NEW, toolBarBitmaps[0], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, wxT("New file"));
    toolBar->AddTool(wxID_OPEN, toolBarBitmaps[1], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, wxT("Open file"));
    toolBar->AddTool(wxID_SAVE, toolBarBitmaps[2], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, wxT("Save file"));
    
    toolBar->AddSeparator();
    
    toolBar->AddTool(wxID_CUT, toolBarBitmaps[4], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Cut");
    toolBar->AddTool(wxID_COPY, toolBarBitmaps[3], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Copy");
    toolBar->AddTool(wxID_PASTE, toolBarBitmaps[5], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Paste");
    toolBar->AddTool(wxID_FIND, toolBarBitmaps[6], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Search");
    
    toolBar->AddSeparator();
    
    toolBar->AddTool(ecID_STOP_BUILD, toolBarBitmaps[7], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Stop build");
    toolBar->AddTool(ecID_BUILD_LIBRARY, toolBarBitmaps[8], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Build library");
#if ecUSE_MLT
    toolBar->AddSeparator();
    toolBar->AddTool(ecID_NEW_REGION, toolBarBitmaps[10], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "New region");
    toolBar->AddTool(ecID_NEW_SECTION, toolBarBitmaps[11], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "New section");
    toolBar->AddTool(ecID_DELETE, toolBarBitmaps[12], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Delete");
    toolBar->AddTool(ecID_PROPERTIES, toolBarBitmaps[13], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Properties");
#endif
    toolBar->AddSeparator();
    toolBar->AddTool(ecID_CONTEXT_HELP, toolBarBitmaps[14], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Show help for clicked-on windows");
    toolBar->AddTool(ecID_ECOS_HELP, toolBarBitmaps[9], wxNullBitmap, FALSE, -1, -1, (wxObject *) NULL, "Show help");
    
    // after adding the buttons to the toolbar, must call Realize() to reflect
    // the changes
    toolBar->Realize();

    toolBar->SetHelpText(_("The toolbar allows quick access to commonly-used commands."));
}

// Toggle one of the windows
void ecMainFrame::OnToggleWindow(wxCommandEvent& event)
{
    ToggleWindow(event.GetId());
}

// Toggle the given window on or off
void ecMainFrame::ToggleWindow(int windowId)
{
    wxWindow* win = NULL;
    bool *configSetting = NULL;
    switch (windowId)
    {
    case ecID_TOGGLE_CONFLICTS:
        win = m_conflictsSashWindow;
        configSetting = & wxGetApp().GetSettings().m_showConflictsWindow;
        break;
    case ecID_TOGGLE_PROPERTIES:
        win = m_propertiesSashWindow;
        configSetting = & wxGetApp().GetSettings().m_showPropertiesWindow;
        break;
    case ecID_TOGGLE_MEMORY:
        win = m_memorySashWindow;
        configSetting = & wxGetApp().GetSettings().m_showMemoryWindow;
        break;
    case ecID_TOGGLE_SHORT_DESCR:
        win = m_shortDescrSashWindow;
        configSetting = & wxGetApp().GetSettings().m_showShortDescrWindow;
        break;
    case ecID_TOGGLE_OUTPUT:
        win = m_outputSashWindow;
        configSetting = & wxGetApp().GetSettings().m_showOutputWindow;
        break;
    }
    if (win)
    {
        bool showing = !win->IsShown();

        win->Show(showing);
        * configSetting = showing;

        // Make sure we don't have ridiculous sizes
        if (showing && (windowId == ecID_TOGGLE_CONFLICTS || windowId == ecID_TOGGLE_PROPERTIES || windowId == ecID_TOGGLE_SHORT_DESCR))
        {
            m_conflictsSashWindow->SetDefaultSize(wxSize(2000, 50));
            m_propertiesSashWindow->SetDefaultSize(wxSize(2000, 50));
            m_shortDescrSashWindow->SetDefaultSize(wxSize(2000, 50));

            wxSize frameSize = GetClientSize();
            wxSize configSize = m_configSashWindow->GetSize();

            if ((frameSize.x - configSize.x) < 5)
            {
                // We must resize the config window
                m_configSashWindow->SetDefaultSize(wxSize(frameSize.x/2, configSize.y));
            }
        }
        
        wxLayoutAlgorithm layout;
        layout.LayoutFrame(this);
    }
}

void ecMainFrame::OnUpdateToggleWindow(wxUpdateUIEvent& event)
{
    wxWindow* win = NULL;
    switch (event.GetId())
    {
    case ecID_TOGGLE_CONFLICTS:
        win = m_conflictsSashWindow;
        break;
    case ecID_TOGGLE_PROPERTIES:
        win = m_propertiesSashWindow;
        break;
    case ecID_TOGGLE_MEMORY:
        win = m_memorySashWindow;
        break;
    case ecID_TOGGLE_SHORT_DESCR:
        win = m_shortDescrSashWindow;
        break;
    case ecID_TOGGLE_OUTPUT:
        win = m_outputSashWindow;
        break;
    }
    if (win)
    {
        event.Enable( TRUE );
        event.Check( win->IsShown() );

        // Not implemented
#if !ecUSE_MLT
        if (event.GetId() == ecID_TOGGLE_MEMORY)
            event.Enable( FALSE );
#endif
    }
}

void ecMainFrame::OnUpdateDisable(wxUpdateUIEvent& event)
{
    event.Enable( FALSE );
}

void ecMainFrame::OnToggleToolbar(wxCommandEvent& event)
{
    GetToolBar()->Show( ! GetToolBar()->IsShown() );
    
    wxSizeEvent sizeEvent(GetSize(), GetId());
    GetEventHandler()->ProcessEvent(sizeEvent);
#ifdef __WXGTK__
    GtkOnSize( GetPosition().x, GetPosition().y, GetSize().x, GetSize().y);
#endif
}

void ecMainFrame::OnUpdateToggleToolbar(wxUpdateUIEvent& event)
{
    event.Check( GetToolBar()->IsShown() );
}

// Respond to a sash drag operation, by setting the new size
// for this window and then recalculating the layout.
void ecMainFrame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;
    
    switch (event.GetId())
    {
    case ecID_CONFIG_SASH_WINDOW:
        {
            m_configSashWindow->SetDefaultSize(wxSize(event.GetDragRect().width, 2000));
            break;
        }
    case ecID_CONFLICTS_SASH_WINDOW:
        {
            // Change the height of the properties window so we don't affect the
            // short description window
            int deltaY = event.GetDragRect().height - m_conflictsSashWindow->GetSize().y;
            int propertiesHeight = 0;
            if (m_propertiesSashWindow->IsShown())
            {
                propertiesHeight = m_propertiesSashWindow->GetSize().y - deltaY ;
                if (propertiesHeight <= 0)
                    return;
                else
                    m_propertiesSashWindow->SetDefaultSize(wxSize(2000, propertiesHeight));
            }
            m_conflictsSashWindow->SetDefaultSize(wxSize(2000, event.GetDragRect().height));
            break;
        }
    case ecID_PROPERTIES_SASH_WINDOW:
        {
            m_propertiesSashWindow->SetDefaultSize(wxSize(2000, event.GetDragRect().height));
            break;
        }
    case ecID_SHORT_DESCR_SASH_WINDOW:
        {
            m_shortDescrSashWindow->SetDefaultSize(wxSize(2000, event.GetDragRect().height));
            break;
        }
    case ecID_MEMORY_SASH_WINDOW:
        {
            m_memorySashWindow->SetDefaultSize(wxSize(2000, event.GetDragRect().height));
            break;
        }
    case ecID_OUTPUT_SASH_WINDOW:
        {
            m_outputSashWindow->SetDefaultSize(wxSize(2000, event.GetDragRect().height));
            break;
        }
    default:
        {
            wxFAIL_MSG( _("Shouldn't get here.") );
            break;
        }
    }

    if (event.GetId() == ecID_MEMORY_SASH_WINDOW || event.GetId() == ecID_OUTPUT_SASH_WINDOW)
    {
        // Special processing so we don't spoil the layout of the
        // conflicts/properties/short description windows
        wxList minorWindows;
        GetMinorWindows(minorWindows);

        int memoryLayoutHeight = m_memorySashWindow->IsShown() ? m_memorySashWindow->GetSize().y : 0;
        int outputHeight = m_memorySashWindow->IsShown() ? m_outputSashWindow->GetSize().y : 0;
        int cx, cy;
        GetClientSize(& cx, & cy);

        // Calculate how much space will be left after this drag operation.
        int heightLeft;
        if (event.GetId() == ecID_MEMORY_SASH_WINDOW)
            heightLeft = cy - outputHeight - event.GetDragRect().height;
        else
            heightLeft = cy - memoryLayoutHeight - event.GetDragRect().height;

        DivideSpaceEvenly(minorWindows, wxSize(0, heightLeft), wxVERTICAL);
        RestoreDefaultWindowSizes(minorWindows);
    }

    wxLayoutAlgorithm layout;
    if (!layout.LayoutFrame(this))
    {
        // If layout failed, restored default sizes.
        wxNode* node = GetChildren().First();
        while (node)
        {
            wxWindow* win = (wxWindow*) node->Data();
            if (win->IsKindOf(CLASSINFO(wxSashLayoutWindow)))
            {
                wxSashLayoutWindow* sashWin = (wxSashLayoutWindow*) win;
                wxSize sz = sashWin->GetSize();
                sashWin->SetDefaultSize(sz);
            }
            node = node->Next();
        }
    }
    
}

void ecMainFrame::OnIdle(wxIdleEvent& event)
{
    // Normal idle processing
    wxFrame::OnIdle(event);

    wxString text;
    if (GetStatusBar())
        text = GetStatusBar()->GetStatusText(0);

    // Set the title if we have no document
    if (!wxGetApp().GetConfigToolDoc() && GetTitle() != wxGetApp().GetSettings().GetAppName())
        SetTitle(wxGetApp().GetSettings().GetAppName());
    
    if ( wxGetApp().m_pipedProcess && wxGetApp().m_pipedProcess->HasInput() )
    {
        event.RequestMore();
    }

    if ( wxGetApp().m_pipedProcess )
    {
        if (text != _("Building..."))
            SetStatusText(_("Building..."), 0);
    }
    else if (text != _("Ready"))
        SetStatusText(_("Ready"), 0);
}

void ecMainFrame::OnCloseWindow(wxCloseEvent& event)
{
    wxBusyCursor busy;

    if (!wxGetApp().GetDocManager()->Clear(FALSE) && event.CanVeto())
    {
        event.Veto();
        return;
    }
    if (wxGetApp().m_pipedProcess)
        wxGetApp().m_pipedProcess->Detach();

    if (m_findDialog)
        m_findDialog->Close(TRUE);

    wxGetApp().DestroyHelpController();
    
    if (IsMaximized())
        wxGetApp().GetSettings().m_frameStatus = ecSHOW_STATUS_MAXIMIZED ;
    else if (IsIconized())
        wxGetApp().GetSettings().m_frameStatus = ecSHOW_STATUS_MINIMIZED ;
    else
        wxGetApp().GetSettings().m_frameStatus = ecSHOW_STATUS_NORMAL ;
    
    if (!IsMaximized() && !IsIconized())
        wxGetApp().GetSettings().m_frameSize = GetRect();
    
    wxGetApp().GetSettings().m_showToolBar = GetToolBar()->IsShown();
    
    wxGetApp().GetSettings().m_treeSashSize = m_configSashWindow->GetSize();
    wxGetApp().GetSettings().m_propertiesSashSize = m_propertiesSashWindow->GetSize();
    wxGetApp().GetSettings().m_conflictsSashSize = m_conflictsSashWindow->GetSize();
    wxGetApp().GetSettings().m_shortDescrSashSize = m_shortDescrSashWindow->GetSize();
    wxGetApp().GetSettings().m_memorySashSize = m_memorySashWindow->GetSize();
    wxGetApp().GetSettings().m_outputSashSize = m_outputSashWindow->GetSize();
    wxGetApp().GetSettings().m_configPaneWidth = m_splitter->GetSashPosition();
    
    event.Skip();
}

// Enumerate the visible 'minor' sash windows,
// i.e. those in the top-right segment of the frame
void ecMainFrame::GetMinorWindows(wxList& list)
{
    if (m_conflictsSashWindow->IsShown())
        list.Append(m_conflictsSashWindow);
    if (m_propertiesSashWindow->IsShown())
        list.Append(m_propertiesSashWindow);
    if (m_shortDescrSashWindow->IsShown())
        list.Append(m_shortDescrSashWindow);
}

// Get all visible sash windows
void ecMainFrame::GetSashWindows(wxList& list)
{
    wxNode* node = GetChildren().First();
    while (node)
    {
        wxWindow* win = (wxWindow*) node->Data();
        if (win->IsKindOf(CLASSINFO(wxSashLayoutWindow)) && win->IsShown())
        {
            list.Append(win);
        }
        node = node->Next();
    }
}

// Divide the given space evenly amongst some windows
void ecMainFrame::DivideSpaceEvenly(wxList& list, const wxSize& space, int orient)
{
    if (list.Number() == 0)
        return;

    // Find total size first
    int totalSize = 0;
    double proportion = 0.0;
    wxNode* node = list.First();
    while (node)
    {
        wxWindow* win = (wxWindow*) node->Data();
        wxSize sz = win->GetSize();
        if (orient == wxHORIZONTAL)
            totalSize += sz.x;
        else
            totalSize += sz.y;
        node = node->Next();
    }
    if (orient == wxHORIZONTAL)
    {
        if (totalSize == 0)
            return;

        proportion = ((double) space.x / (double) totalSize);
    }
    else
    {
        if (totalSize == 0)
            return;

        proportion = ((double) space.y / (double) totalSize);
    }
    node = list.First();
    while (node)
    {
        wxWindow* win = (wxWindow*) node->Data();
        wxSize sz = win->GetSize();
        if (orient == wxHORIZONTAL)
            sz.x = (int) (sz.x * proportion);
        else
            sz.y = (int) (sz.y * proportion);
        win->SetSize(sz);
        node = node->Next();
    }
}

// Restore the sash window default size from the actual window size
void ecMainFrame::RestoreDefaultWindowSizes(wxList& list)
{
    wxNode* node = list.First();
    while (node)
    {
        wxSashLayoutWindow* sashWin = (wxSashLayoutWindow*) node->Data();
        wxSize sz = sashWin->GetSize();
        sashWin->SetDefaultSize(sz);
        node = node->Next();
    }
}

void ecMainFrame::OnHelpEcos(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
    {
        wxString strURL(wxT("index.html"));
        doc->QualifyDocURL(strURL, FALSE);
        switch (wxGetApp().GetSettings().m_eUseCustomBrowser)
        {
        case ecInternal:
            {
                if (wxGetApp().HasHelpController())
                    wxGetApp().GetHelpController().DisplayContents();
                break;
            }
        default:
            {
                doc->ShowURL(strURL);
            }
        }
    }
}

void ecMainFrame::OnHelpConfigtool(wxCommandEvent& event)
{
    //wxString strURL(wxT("redirect/the-ecos-configuration-tool.html"));
    wxString strURL(wxGetApp().GetFullAppPath(wxT("manual/user-guides.2.html")));
    if (!wxFileExists(strURL))
	strURL = wxT("user-guide/the-ecos-configuration-tool.html");

    if (wxGetApp().GetConfigToolDoc())
    {
        wxGetApp().GetConfigToolDoc()->ShowURL(strURL);
    }
}

void ecMainFrame::OnHelpContext(wxCommandEvent& event)
{
    wxContextHelp contextHelp;
}

void ecMainFrame::OnResolveConflicts(wxCommandEvent& event)
{
/*
    ecResolveConflictsDialog dialog(this);
    dialog.ShowModal();
*/
    if ( ecConfigToolDoc::NotDone == wxGetApp().GetConfigToolDoc()->ResolveGlobalConflicts() )
    {
        // Global inference handler was never invoked.  Say something
        wxString msg;
        msg.Printf(_("No solutions can be automatically determined for the current set of conflicts."));
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    }    
}

void ecMainFrame::OnUpdateResolveConflicts(wxUpdateUIEvent& event)
{
    if (!wxGetApp().GetConfigToolDoc() || !wxGetApp().GetConfigToolDoc()->GetCdlInterpreter())
    {
        event.Enable(FALSE);
        return;
    }

    event.Enable(wxGetApp().GetConfigToolDoc()->GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size()>0);
}

void ecMainFrame::OnSettings(wxCommandEvent& event)
{
    ecSettingsDialog dialog(this);
    dialog.ShowModal();
}

void ecMainFrame::OnPlatforms(wxCommandEvent& event)
{
    ecPlatformsDialog dialog(this);
    if (dialog.ShowModal() == wxID_OK)
    {
        CeCosTestPlatform::RemoveAllPlatforms();
        unsigned int i ;
        for(i=0; i < dialog.PlatformCount();i++){
            CeCosTestPlatform::Add(*dialog.Platform(i));
        }
        CeCosTestPlatform::Save();
    }
}

void ecMainFrame::OnBuildOptions(wxCommandEvent& event)
{
    ecBuildOptionsDialog dialog(this);
    dialog.ShowModal();
}

void ecMainFrame::OnTemplates(wxCommandEvent& event)
{
    ecTemplatesDialog dialog(this);
    if (dialog.ShowModal() == wxID_OK)
	{
#ifdef __WXMSW__
        // Ensure display gets updated
        ::UpdateWindow((HWND) GetHWND());
        //wxYield();
#endif
        ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

		doc->SelectHardware(dialog.GetSelectedHardware());
		doc->SelectTemplate(dialog.GetSelectedTemplate(), dialog.GetSelectedTemplateVersion());
	}
}

void ecMainFrame::OnAdmin(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

    wxASSERT (doc) ;

    if (wxYES == wxMessageBox(_("This command will close the current document.\n\nDo you wish to continue?"),
        wxGetApp().GetAppName(), wxYES_NO, this))
    {
        wxString shellCommands;
        // ensure that the user tools are on the path for use by ecosadmin.tcl
        // TODO: need to something else for Linux (since it returns settings in shellCommands)
        if (wxGetApp().PrepareEnvironment(FALSE, & shellCommands))
        {
            // make sure we use doc data before the doc is destroyed

            ecAdminDialog dlg(this, doc->GetPackagesDir(), wxGetApp().GetSettings().GetUserToolsDir());

            if (dlg.ShowModal() == wxID_OK)
            {
                // Create new document
                wxGetApp().GetDocManager()->CreateDocument(wxEmptyString, wxDOC_NEW);
            }
        }
    }
}

void ecMainFrame::OnPackages(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
        doc->SelectPackages();
}

void ecMainFrame::OnRunTests(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
        doc->RunTests();
}

void ecMainFrame::OnChooseRepository(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc && !doc->OnSaveModified())
        return;

    if (!doc)
    {
        doc = (ecConfigToolDoc*) wxGetApp().GetDocManager()->CreateDocument(wxString(""), wxDOC_NEW|ecDOC_PROMPT_FOR_REPOSITORY);
        return;
    }

    if (doc)
        doc->m_bRepositoryOpen = FALSE;

    if (wxGetApp().GetConfigToolDoc()->OpenRepository(wxEmptyString, TRUE))
    {
        // TODO
#if 0
        // reset the document title as shown in the frame window
        GetDocTemplate ()->SetDefaultTitle (this);
        
        // load the memory layout for the default target-platform-startup from the new repository
        NewMemoryLayout (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf")));
#endif

        doc->UpdateAllViews(NULL);
        doc->UpdateFailingRuleCount();
    }
    else
    {
        if (doc)
            doc->m_bRepositoryOpen = TRUE;
    }
}

void ecMainFrame::OnBuildToolsPath(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (!doc)
        return;

    // add the current build tools dir to the drop-down list box
    wxArrayString arstrPaths;
    if (!wxGetApp().GetSettings().m_buildToolsDir.IsEmpty())
        arstrPaths.Add(wxGetApp().GetSettings().m_buildToolsDir);

    // also add the sub-directory containing tools for the current command prefix
    wxString value;
    wxStringToStringMap& map = wxGetApp().GetSettings().GetBinDirs();
    const wxString strPrefix(doc->GetCurrentTargetPrefix());
    if (map.Find(strPrefix, value) && (wxNOT_FOUND == arstrPaths.Index(value)))
        arstrPaths.Add(value);

    wxString msg;
    msg.Printf(_("Enter the location of the %s build tools\n"
          "folder. You can type in a path or use the\n"
          "Browse button to navigate to a folder."),
          (const wxChar*) (strPrefix.IsEmpty() ? wxString(wxT("native")) : strPrefix));
    wxString caption(_("Build Tools Path"));

    ecFolderDialog dialog(wxGetApp().GetSettings().m_buildToolsDir, arstrPaths, msg, this, ecID_BUILD_TOOLS_DIALOG, caption);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path (dialog.GetPath());

        // look for *objcopy under the user-specified build tools directory
        wxArrayString objcopyFiles;
        wxString objcopyFileSpec(wxT("objcopy"));
#ifdef __WXMSW__
        objcopyFileSpec += wxT(".exe");
#endif
        size_t objcopyCount = wxDir::GetAllFiles(path, &objcopyFiles, wxT("*") + objcopyFileSpec, wxDIR_FILES | wxDIR_DIRS);
        bool bPrefixFound = false;
        for (int count=0; count < objcopyCount; count++)
        {
            wxFileName file (objcopyFiles [count]);
            wxString new_prefix (file.GetFullName().Left (file.GetFullName().Find(objcopyFileSpec)));
            if ((! new_prefix.IsEmpty()) && ('-' == new_prefix.Last()))
                new_prefix = new_prefix.Left (new_prefix.Len() - 1); // strip off trailing hyphen 
            if (new_prefix == strPrefix)
                bPrefixFound = true;
        }

        wxString msg;
        msg.Printf(wxT("%s does not appear to contain the build tools - use this folder anyway?"), (const wxChar*) path);

        if(bPrefixFound ||
            (wxYES == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO)))
        {
            for (int count=0; count < objcopyCount; count++)
            {
                wxFileName file (objcopyFiles [count]);
                wxString new_prefix (file.GetFullName().Left (file.GetFullName().Find(objcopyFileSpec)));
                if ((! new_prefix.IsEmpty()) && ('-' == new_prefix.Last()))
                    new_prefix = new_prefix.Left (new_prefix.Len() - 1); // strip off trailing hyphen
                map.Set(new_prefix, file.GetPath(wxPATH_GET_VOLUME));
            }
            wxGetApp().GetSettings().m_buildToolsDir = path;
        }
    }
}

void ecMainFrame::OnUserToolsPath(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (!doc)
        return;

    wxArrayString paths = wxGetApp().GetSettings().m_userToolPaths;
    if (!wxGetApp().GetSettings().m_userToolsDir.IsEmpty())
        paths.Add(wxGetApp().GetSettings().m_userToolsDir);

    wxString msg(_("Enter the location of the user tools folder,\n"
          "which should contain cat and ls. You can type in\n"
          "a path or use the Browse button to navigate to a\n"
          "folder."));

    wxString caption(_("User Tools Path"));
    wxString defaultPath(wxGetApp().GetSettings().m_userToolsDir);

    ecFolderDialog dialog(defaultPath, paths, msg, this, ecID_USER_TOOLS_DIALOG, caption);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString path(dialog.GetPath());
        ecFileName strFile(path);

#ifdef __WXMSW__
        wxString exeSuffix(wxT(".exe"));
#else
        wxString exeSuffix(wxEmptyString);
#endif
        wxString prog(wxString(wxT("ls")) + exeSuffix);

        strFile += (const wxChar*) prog;

        wxString msg;
        msg.Printf(wxT("%s does not appear to contain the user tools - use this folder anyway?"), (const wxChar*) path);

        if(strFile.Exists() ||
            (wxYES == wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO)))
        {
            wxGetApp().GetSettings().m_userToolsDir = path;
        }
    }
}

// Process events for the window with the focus first.
bool ecMainFrame::ProcessEvent(wxEvent& event)
{
    static wxEvent* s_lastEvent = NULL;

    if (& event == s_lastEvent)
        return FALSE;

    if (event.IsCommandEvent() && !event.IsKindOf(CLASSINFO(wxChildFocusEvent)))
    {
        s_lastEvent = & event;
        
        wxWindow* focusWin = wxFindFocusDescendant(this);
        bool success = FALSE;
        if (focusWin)
        {
            //long windowId = focusWin->GetId();
            //wxLogDebug("Found focus window %d", windowId);
            success = focusWin->GetEventHandler()->ProcessEvent(event);
        }
        if (!success)
            success = wxDocParentFrame::ProcessEvent(event);
        
        s_lastEvent = NULL;
        return success;
    }
    else
    {
        return wxDocParentFrame::ProcessEvent(event);
    }
}

void ecMainFrame::SetFailRulePane(int nCount)
{
    wxString strCount;
    switch (nCount)
    {
    case 0:
        strCount = wxT("No conflicts");
        break;
    case 1:
        strCount = wxT("1 conflict");
        break;
    default:
        strCount.Printf (_("%d conflicts"), nCount);
        break;
    }
    if (GetStatusBar())
    {
        GetStatusBar()->SetStatusText(strCount, ecFailRulePane);
    }
}

// Update the title, either via the document's view or explicitly if no doc
void ecMainFrame::UpdateFrameTitle()
{
    if (wxGetApp().GetConfigToolDoc())
        wxGetApp().GetConfigToolDoc()->GetFirstView()->OnChangeFilename();
    else
        SetTitle(wxGetApp().GetSettings().GetAppName());
}

void ecMainFrame::OnUpdatePlatforms(wxUpdateUIEvent& event)
{
    event.Enable(TRUE);
}

void ecMainFrame::OnUpdateBuildOptions(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdateBuildToolsPath(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdateUserToolsPath(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdateTemplates(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdateAdmin(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdatePackages(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL);
}

void ecMainFrame::OnUpdateRunTests(wxUpdateUIEvent& event)
{
    event.Enable(wxGetApp().GetConfigToolDoc() != NULL && !wxGetApp().GetConfigToolDoc()->GetInstallTree().IsEmpty() && (wxGetApp().m_pipedProcess == NULL));
}

void ecMainFrame::OnUpdateChooseRepository(wxUpdateUIEvent& event)
{
    event.Enable(TRUE);
}

void ecMainFrame::OnWhatsThis(wxCommandEvent& event)
{
    wxGetApp().OnWhatsThis(event);
}

void ecMainFrame::OnSaveOutput(wxCommandEvent& event)
{
    ecOutputWindow* win = GetOutputWindow();
    if (!win)
        return;

    wxFileDialog dialog(this, _("Choose a file for saving the output window contents"),
        wxT(""), wxT("output.txt"), wxT("*.txt"), wxSAVE|wxOVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK)
    {
        if (!win->SaveFile(dialog.GetPath()))
        {
            wxMessageBox(_("Sorry, there was a problem saving the file."), wxGetApp().GetSettings().GetAppName(),
                wxICON_EXCLAMATION|wxID_OK);
        }
    }
}

void ecMainFrame::OnUpdateSelectAll(wxUpdateUIEvent& event)
{
    wxWindow* win = wxWindow::FindFocus();
    event.Enable (win && win->IsKindOf(CLASSINFO(wxTextCtrl)) );
}

void ecMainFrame::OnUpdateClear(wxUpdateUIEvent& event)
{
    wxWindow* win = wxWindow::FindFocus();
    event.Enable (win && win->IsKindOf(CLASSINFO(wxTextCtrl)) );
}

void ecMainFrame::OnImport(wxCommandEvent& event)
{
    if (wxGetApp().GetConfigToolDoc())
    {
        wxGetApp().GetConfigToolDoc()->ImportFile();
    }
}

void ecMainFrame::OnExport(wxCommandEvent& event)
{
    if (wxGetApp().GetConfigToolDoc())
    {
        wxGetApp().GetConfigToolDoc()->ExportFile();
    }
}

void ecMainFrame::OnUpdateImport(wxUpdateUIEvent& event)
{
    event.Enable ( wxGetApp().GetConfigToolDoc() != NULL );
}

void ecMainFrame::OnUpdateExport(wxUpdateUIEvent& event)
{
    event.Enable ( wxGetApp().GetConfigToolDoc() != NULL );
}

void ecMainFrame::OnWebRedHatHome(wxCommandEvent& event)
{
    wxString strURL(wxT("http://www.redhat.com"));
    if (wxGetApp().GetConfigToolDoc())
        wxGetApp().GetConfigToolDoc()->ShowURL(strURL);
}

void ecMainFrame::OnWebEcos(wxCommandEvent& event)
{
    wxString strURL(wxT("http://www.redhat.com/products/ecos"));
    if (wxGetApp().GetConfigToolDoc())
        wxGetApp().GetConfigToolDoc()->ShowURL(strURL);
}

void ecMainFrame::OnWebNetRelease(wxCommandEvent& event)
{
    wxString strURL(wxT("http://sources.redhat.com/ecos"));
    if (wxGetApp().GetConfigToolDoc())
        wxGetApp().GetConfigToolDoc()->ShowURL(strURL);
}

void ecMainFrame::OnWebUitron(wxCommandEvent& event)
{
    wxString strURL(wxT("http://www.itron.gr.jp/"));
    if (wxGetApp().GetConfigToolDoc())
        wxGetApp().GetConfigToolDoc()->ShowURL(strURL);
}

void ecMainFrame::OnBuildLibrary(wxCommandEvent& event)
{
    // TODO: possibly add wxT("clean build") to ensure library is
    // cleanly built. No, can't do that because it would clean
    // out any user code too :-(

    bool regenerateBuildTree = FALSE;

    if (wxGetApp().GetSettings().m_editSaveFileOnly)
    {
        int ans = wxMessageBox(wxT("Running in --edit-only mode so there may not be an up-to-date build tree.\nBuild the tree now?"), wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO|wxCANCEL);

        if (ans == wxYES)
        {
            regenerateBuildTree = TRUE;
        }
        else if (ans == wxCANCEL)
            return;
    }

    if (regenerateBuildTree)
    {
        ecConfigToolDoc* pDoc = wxGetApp().GetConfigToolDoc();
        if (!pDoc)
            return;

        if (!pDoc->GenerateBuildTree())
            return ;
    }

    wxGetApp().Build();
}

void ecMainFrame::OnBuildTests(wxCommandEvent& event)
{
    bool regenerateBuildTree = FALSE;

    if (wxGetApp().GetSettings().m_editSaveFileOnly)
    {
        int ans = wxMessageBox(wxT("Running in --edit-only mode so there may not be an up-to-date build tree.\nBuild the tree now?"), wxGetApp().GetSettings().GetAppName(), wxICON_QUESTION|wxYES_NO|wxCANCEL);

        if (ans == wxYES)
        {
            regenerateBuildTree = TRUE;
        }
        else if (ans == wxCANCEL)
            return;
    }

    if (regenerateBuildTree)
    {
        ecConfigToolDoc* pDoc = wxGetApp().GetConfigToolDoc();
        if (!pDoc)
            return;

        if (!pDoc->GenerateBuildTree())
            return ;
    }

    wxGetApp().Build(wxT("tests"));
}

void ecMainFrame::OnStopBuild(wxCommandEvent& event)
{
    if (wxGetApp().m_pipedProcess)
    {
        long pid = wxGetApp().m_pipedProcess->GetPid();
        wxGetApp().m_pipedProcess->Detach();

        wxProcessKiller pKiller(pid);
        pKiller.Kill(TRUE);
    }
}

void ecMainFrame::OnClean(wxCommandEvent& event)
{
    wxGetApp().Build(wxT("clean"));
}

void ecMainFrame::OnShell(wxCommandEvent& event)
{
    ecConfigToolDoc *pDoc=wxGetApp().GetConfigToolDoc();
    if (!pDoc)
        return;

    wxString variableSettings;
    if (wxGetApp().PrepareEnvironment(TRUE, & variableSettings))
    {
#ifdef __WXMSW__
        wxString currentDir = wxGetCwd();
        wxSetWorkingDirectory(pDoc->GetBuildTree());

        wxExecute("bash.exe");

        if (!currentDir.IsEmpty()) // if the current directory was changed
        { 
            wxSetWorkingDirectory(currentDir); // restore the previous current directory
        }
#else
        wxString cmdLine = wxString(wxT("xterm"));

        // TODO: query an appropriate variable, and/or have a setting for this
        wxExecute(cmdLine);
#endif
    }
}

void ecMainFrame::OnUpdateBuildLibrary(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && (wxGetApp().m_pipedProcess == NULL ));
}

void ecMainFrame::OnUpdateBuildTests(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && (wxGetApp().m_pipedProcess == NULL ));
}

void ecMainFrame::OnUpdateStopBuild(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && (wxGetApp().m_pipedProcess != NULL ));
}

void ecMainFrame::OnUpdateClean(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && (wxGetApp().m_pipedProcess == NULL ));
}

void ecMainFrame::OnRepositoryInfo(wxCommandEvent& event)
{
    ecRepositoryInfoDialog dialog(this, ecID_REPOSITORY_INFO, _("Repository Information"));
    dialog.ShowModal();
}

void ecMainFrame::OnUpdateShell(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) );
}

void ecMainFrame::OnUpdateRepositoryInfo(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) );
}

void ecMainFrame::OnNewRegion(wxCommandEvent& event)
{
#if ecUSE_MLT
#endif
}

void ecMainFrame::OnNewSection(wxCommandEvent& event)
{
#if ecUSE_MLT
#endif
}

void ecMainFrame::OnDeleteRegionOrSection(wxCommandEvent& event)
{
#if ecUSE_MLT
#endif
}

void ecMainFrame::OnRegionOrSectionProperties(wxCommandEvent& event)
{
#if ecUSE_MLT
#endif
}

void ecMainFrame::OnUpdateNewRegion(wxUpdateUIEvent& event)
{
#if ecUSE_MLT
#else
    event.Enable( FALSE );
#endif
}

void ecMainFrame::OnUpdateNewSection(wxUpdateUIEvent& event)
{
#if ecUSE_MLT
#else
    event.Enable( FALSE );
#endif
}

void ecMainFrame::OnUpdateDeleteRegionOrSection(wxUpdateUIEvent& event)
{
#if ecUSE_MLT
#else
    event.Enable( FALSE );
#endif
}

void ecMainFrame::OnUpdateRegionOrSectionProperties(wxUpdateUIEvent& event)
{
#if ecUSE_MLT
#else
    event.Enable( FALSE );
#endif
}

void ecMainFrame::OnIndexDocs(wxCommandEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
    {
        doc->RebuildHelpIndex(TRUE) ;
    }
}

void ecMainFrame::OnUpdateIndexDocs(wxUpdateUIEvent& event)
{
    event.Enable( wxGetApp().GetConfigToolDoc() != NULL );
}

void ecMainFrame::OnGenerateBuildTree(wxCommandEvent& event)
{
    if (wxGetApp().GetConfigToolDoc() && wxGetApp().GetConfigToolDoc()->CanGenerateBuildTree())
    {
        if (!wxGetApp().GetConfigToolDoc()->GenerateBuildTree())
        {
            // Error probably already reported
        }
    }
}

void ecMainFrame::OnUpdateGenerateBuildTree(wxUpdateUIEvent& event)
{
    event.Enable( (wxGetApp().GetConfigToolDoc() != NULL) && wxGetApp().GetConfigToolDoc()->CanGenerateBuildTree());
}
