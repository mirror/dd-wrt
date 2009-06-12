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
// appsettings.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians
// Date:        2000/08/29
// Version:     $Id: appsettings.cpp,v 1.27 2001/12/14 17:34:03 julians Exp $
// Purpose:
// Description: Implementation file for the ecSettings
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
#pragma implementation "appsettings.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/config.h"
#include "wx/file.h"

#include "appsettings.h"
#include "configtree.h"
#include "outputwin.h"
#include "shortdescrwin.h"
#include "mainwin.h"

// For SystemInfo
#ifdef __WXMSW__
#include <windows.h>
#include "wx/msw/winundef.h"
#ifdef GetTempPath
#undef GetTempPath
#endif
#endif

/*
* User-changeable and other settings to be saved between sessions
*/

IMPLEMENT_DYNAMIC_CLASS(ecSettings, wxObject)

ecSettings::ecSettings()
{
    m_showToolBar = TRUE;
    m_frameSize = wxRect(0, 0, 600, 500);
    m_appName = _("eCos Configuration Tool");
    m_showSplashScreen = TRUE;
    m_userName = wxEmptyString;
    m_serialNumber = 0;
    m_frameStatus = ecSHOW_STATUS_NORMAL;
    
    m_treeSashSize = wxSize(400, 1000);
    m_conflictsSashSize = wxSize(1000, 80);
    m_propertiesSashSize = wxSize(1000, 80);
    m_shortDescrSashSize = wxSize(1000, 80);
    m_memorySashSize = wxSize(1000, 50);
    m_outputSashSize = wxSize(1000, 50);
    m_configPaneWidth = 200;
    
    m_showConflictsWindow = FALSE;
    m_showPropertiesWindow = TRUE;
    m_showShortDescrWindow = TRUE;
    m_showMemoryWindow = FALSE;
    m_showOutputWindow = TRUE;

    m_showMacroNames = FALSE;
    
    m_bUseCustomViewer = FALSE;
    m_bUseExternalBrowser = FALSE;
    m_eUseCustomBrowser = ecAssociatedExternal;
    m_bHex = FALSE;
    m_nRuleChecking = Immediate|Deferred|SuggestFixes ;

#ifdef __WXMSW__
    m_strViewer = wxT("notepad");
#endif

    // Find dialog settings
    m_findText = wxEmptyString;
    m_findMatchWholeWord = FALSE;
    m_findMatchCase = FALSE;
    m_findDirection = TRUE; // Down is TRUE, Up is FALSE
    m_findSearchWhat = _("Macro names"); // Macro names, item names etc.
    m_findDialogPos = wxPoint(-1, -1);

    m_editSaveFileOnly = FALSE;

    // Packages dialog settings
    m_omitHardwarePackages = FALSE;
    m_matchPackageNamesExactly = FALSE;
}

// Copy constructor
ecSettings::ecSettings(const ecSettings& settings)
{
    Copy(settings);
}

void ecSettings::Copy(const ecSettings& settings)
{
    m_showToolBar = settings.m_showToolBar;
    m_frameSize = settings.m_frameSize;
    m_showSplashScreen = settings.m_showSplashScreen;
    m_userName = settings.m_userName;
    m_serialNumber = settings.m_serialNumber;
    
    m_treeSashSize = settings.m_treeSashSize;
    m_conflictsSashSize = settings.m_conflictsSashSize;
    m_propertiesSashSize = settings.m_propertiesSashSize;
    m_memorySashSize = settings.m_memorySashSize;
    m_outputSashSize = settings.m_outputSashSize;
    m_configPaneWidth = settings.m_configPaneWidth;    
    m_showMacroNames = settings.m_showMacroNames;
    
    m_bUseCustomViewer = settings.m_bUseCustomViewer;
    m_bUseExternalBrowser = settings.m_bUseExternalBrowser;
    m_eUseCustomBrowser = settings.m_eUseCustomBrowser;
    m_bHex = settings.m_bHex;
    m_nRuleChecking = settings.m_nRuleChecking;

    // Find dialog settings
    m_findText = settings.m_findText;
    m_findMatchWholeWord = settings.m_findMatchWholeWord;
    m_findMatchCase = settings.m_findMatchCase;
    m_findDirection = settings.m_findDirection; // Down is TRUE, Up is FALSE
    m_findSearchWhat = settings.m_findSearchWhat;
    m_findDialogPos = settings.m_findDialogPos;

    // Run tests settings
    m_runTestsSettings = settings.m_runTestsSettings;

    m_userToolsDir = settings.m_userToolsDir;
    m_buildToolsDir = settings.m_buildToolsDir;

    m_editSaveFileOnly = settings.m_editSaveFileOnly;

    // Packages dialog settings
    m_omitHardwarePackages = settings.m_omitHardwarePackages;
    m_matchPackageNamesExactly = settings.m_matchPackageNamesExactly;
}

ecSettings::~ecSettings()
{
}

// Do some initialisation within ecApp::OnInit
bool ecSettings::Init()
{
    return TRUE;
}

// Get a name suitable for the configuration file on all platforms:
// e.g. eCos Configuration Tool on Windows, .eCosConfigTool on Unix
wxString ecSettings::GetConfigAppName() const
{
#ifdef __WXGTK__
    return wxString(wxT("eCosConfigTool"));
#else
    return GetAppName();
#endif
}


// Create new filename
wxString ecSettings::GenerateFilename(const wxString& rootName)
{
    wxString path;
    if (!m_lastFilename.IsEmpty())
        path = wxPathOnly(m_lastFilename);
    else
	//        path = wxGetApp().GetAppDir();
        path = wxGetCwd();
    
    wxString filename(path);
    if (filename.Last() != wxFILE_SEP_PATH )
        filename += wxFILE_SEP_PATH;
    filename += rootName;
    
    wxString fullFilename = filename + wxT(".ecc");
    int i = 0;
    wxString postfixStr;
    while (wxFileExists(fullFilename))
    {
        i ++;
        postfixStr.Printf("%d", i);
        fullFilename = filename + postfixStr + wxT(".ecc");
    }
    
    m_lastFilename = fullFilename;
    return fullFilename;
}

// Load config info
bool ecSettings::LoadConfig()
{
    wxConfig config(wxGetApp().GetSettings().GetConfigAppName());
    
    config.Read(_("/Window Status/FrameStatus"), & m_frameStatus);
    config.Read(_("/Window Status/ShowToolBar"), (bool*) & m_showToolBar);
    config.Read(_("/Window Status/ShowSplashScreen"), (bool*) & m_showSplashScreen);
    config.Read(_("/Window Status/ShowConflictsWindow"), (bool*) & m_showConflictsWindow);
    config.Read(_("/Window Status/ShowPropertiesWindow"), (bool*) & m_showPropertiesWindow);
    config.Read(_("/Window Status/ShowShortDescrWindow"), (bool*) & m_showShortDescrWindow);
    config.Read(_("/Window Status/ShowMemoryWindow"), (bool*) & m_showMemoryWindow);
    config.Read(_("/Window Status/ShowOutputWindow"), (bool*) & m_showOutputWindow);
    
    config.Read(_("/Files/LastFile"), & m_lastFilename);
    
    config.Read(_("/Window Size/WindowX"), & m_frameSize.x);
    config.Read(_("/Window Size/WindowY"), & m_frameSize.y);
    config.Read(_("/Window Size/WindowWidth"), & m_frameSize.width);
    config.Read(_("/Window Size/WindowHeight"), & m_frameSize.height);

    config.Read(_("/Window Size/TreeSashWidth"), & m_treeSashSize.x);
    config.Read(_("/Window Size/TreeSashHeight"), & m_treeSashSize.y);
    config.Read(_("/Window Size/ConfigPaneWidth"), & m_configPaneWidth);
    config.Read(_("/Window Size/ConflictsWidth"), & m_conflictsSashSize.x);
    config.Read(_("/Window Size/ConflictsHeight"), & m_conflictsSashSize.y);
    config.Read(_("/Window Size/PropertiesWidth"), & m_propertiesSashSize.x);
    config.Read(_("/Window Size/PropertiesHeight"), & m_propertiesSashSize.y);
    config.Read(_("/Window Size/ShortDescrWidth"), & m_shortDescrSashSize.x);
    config.Read(_("/Window Size/ShortDescrHeight"), & m_shortDescrSashSize.y);
    config.Read(_("/Window Size/OutputWidth"), & m_outputSashSize.x);
    config.Read(_("/Window Size/OutputHeight"), & m_outputSashSize.y);
    config.Read(_("/Window Size/MemoryWidth"), & m_memorySashSize.x);
    config.Read(_("/Window Size/MemoryHeight"), & m_memorySashSize.y);

    config.Read(_("/Options/ShowMacroNames"), (bool*) & m_showMacroNames);
    config.Read(_("/Options/UseCustomViewer"), (bool*) & m_bUseCustomViewer);
    config.Read(_("/Options/UseExternalBrowser"), (bool*) & m_bUseExternalBrowser);
    
    int tmp = (int) m_eUseCustomBrowser;
    config.Read(_("/Options/UseCustomBrowser"), & tmp);
    m_eUseCustomBrowser = (ecBrowserType) tmp;
    
    config.Read(_("/Options/Browser"), & m_strBrowser);
    config.Read(_("/Options/Viewer"), & m_strViewer);
    config.Read(_("/Options/HexDisplay"), (bool*) & m_bHex);
    config.Read(_("/Options/UseDefaultFonts"), (bool*) & m_windowSettings.m_useDefaults);
    config.Read(_("/Rule/Checking"), & m_nRuleChecking);

    // Find dialog settings
    config.Read(_("/Find/Text"), & m_findText);
    config.Read(_("/Find/MatchWholeWord"), (bool*) & m_findMatchWholeWord);
    config.Read(_("/Find/MatchCase"), & m_findMatchCase);
    config.Read(_("/Find/Direction"), (bool*) & m_findDirection);
    config.Read(_("/Find/SearchWhat"), & m_findSearchWhat);
    config.Read(_("/Find/DialogX"), & m_findDialogPos.x);
    config.Read(_("/Find/DialogY"), & m_findDialogPos.y);

    // Package dialog settings
    config.Read(_("/Packages/OmitHardwarePackages"), & m_omitHardwarePackages);
    config.Read(_("/Packages/MatchPackageNamesExactly"), & m_matchPackageNamesExactly);

    // Run tests settings
    m_runTestsSettings.LoadConfig(config);

    // Fonts
    m_windowSettings.LoadConfig(config);   
    
    if (!config.Read(_("/Paths/UserToolsDir"), & m_userToolsDir))
    {
        // Use the default provided by the installer
        config.Read(_("Default User Tools Path"), & m_userToolsDir);
    }

    // Only to be used if we fail to find the information installed
    // with the Configuration Tool.
    config.Read(_("/Paths/BuildToolsDir"), & m_buildToolsDir);
    if (m_buildToolsDir.IsEmpty()) // first invocation by this user
    {
        // we have no clues as to the location of the build tools so
        // test for ../../../gnutools relative to the configtool location
        wxFileName gnutools = wxFileName (wxGetApp().GetAppDir(), wxEmptyString);
        gnutools.Normalize(); // remove trailing "./" if present
		if (2 < gnutools.GetDirCount())
        {
            gnutools.RemoveDir (gnutools.GetDirCount()-1);
            gnutools.RemoveDir (gnutools.GetDirCount()-1);
            gnutools.RemoveDir (gnutools.GetDirCount()-1);
            gnutools.AppendDir (wxT("gnutools"));
            if (gnutools.DirExists()) // we've found the gnutools
                m_buildToolsDir = gnutools.GetFullPath();
        }
    }

    // look for *objcopy in and under the build tools directory
    if (! m_buildToolsDir.IsEmpty())
    {
        wxArrayString objcopyFiles;
        wxString objcopyFileSpec(wxT("objcopy"));
#ifdef __WXMSW__
        objcopyFileSpec += wxT(".exe");
#endif
        size_t objcopyCount = wxDir::GetAllFiles(m_buildToolsDir, &objcopyFiles, wxT("*") + objcopyFileSpec, wxDIR_FILES | wxDIR_DIRS);
        for (int count=0; count < objcopyCount; count++)
        {
            wxFileName file (objcopyFiles [count]);
            wxString new_prefix (file.GetFullName().Left (file.GetFullName().Find(objcopyFileSpec)));
            if ((! new_prefix.IsEmpty()) && ('-' == new_prefix.Last()))
                new_prefix = new_prefix.Left (new_prefix.Len() - 1); // strip off trailing hyphen
            m_arstrBinDirs.Set(new_prefix, file.GetPath(wxPATH_GET_VOLUME));
        }
    }

    if (!config.Read(_("/Build/Make Options"), & m_strMakeOptions))
    {
#ifdef __WXMSW__
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        m_strMakeOptions.Printf(_T("-j%d"),SystemInfo.dwNumberOfProcessors);
#endif
    }
    
    // Set default build tools binary directories as specified by the installer
    ecFileName strDefaultBuildToolsPath;

#ifdef __WXMSW__
    {
        // This should look in HKEY_LOCAL_MACHINE

        wxConfig config2(wxT("eCos"), wxEmptyString, wxEmptyString, wxEmptyString, wxCONFIG_USE_GLOBAL_FILE|wxCONFIG_USE_LOCAL_FILE);

        wxString versionKey = GetInstallVersionKey();
        wxConfigPathChanger path(& config2, wxString(wxT("/")) + versionKey + wxT("/"));

        if (!versionKey.IsEmpty() && config2.Read(wxT("Default Build Tools Path"), & strDefaultBuildToolsPath))
        {
#ifdef __WXMSW__
            wxString gccExe(wxT("*-gcc.exe"));
#else
            wxString gccExe(wxT("*-gcc"));
#endif
            
            // Note that this is not a recursive search. Compilers for
            // different targets may be in the same directory. This finds all targets.
            
            // look for *-gcc[.exe] in the default build tools directory
            wxLogNull log;
            wxDir finder(strDefaultBuildToolsPath);
            wxString filename;
            
            if (finder.IsOpened())
            {
                bool bMore = finder.GetFirst(& filename, gccExe);
                while (bMore)
                {
                    wxString targetName = filename.Left(filename.Find(wxT("-gcc")));
                    m_arstrBinDirs.Set(targetName, strDefaultBuildToolsPath);
                    
                    bMore = finder.GetNext(& filename);
                }
            }
        }
    }
#endif

#ifndef __WXMSW__
    // Look in the PATH for build tools, under Unix
    {
        wxString strPath;
        if (wxGetEnv(wxT("PATH"), & strPath))
        {
	    wxString gccExe(wxT("*-gcc"));

	    wxArrayString arstrPath;
            ecUtils::Chop(strPath, arstrPath, wxT(':'));

            for (int i = arstrPath.GetCount()-1;i >= 0; --i)
            { // Reverse order is important to treat path correctly
                if (wxT(".") != arstrPath[i] && !arstrPath[i].IsEmpty())
                {
                    wxLogNull log;
                    wxDir finder(arstrPath[i]);
                    wxString filename;

                    if (finder.IsOpened())
                    {
                        bool bMore = finder.GetFirst(& filename, gccExe);
                        while (bMore)
                        {
                            wxString targetName = filename.Left(filename.Find(wxT("-gcc")));
                            m_arstrBinDirs.Set(targetName, arstrPath[i]);

                            bMore = finder.GetNext(& filename);
                        }
                    }
                }
            }
        }
    }
#endif
    
    // Read build tools directories (current user)
    
    {
        wxConfigPathChanger path(& config, wxT("/Build Tools/"));
        //config.SetPath(wxT("/Build Tools"));
        wxString key(wxT(""));
        long index;
        bool bMore = config.GetFirstEntry(key, index);
        while (bMore)
        {
            wxString value;
            if (config.Read(key, & value))
            {
                m_arstrBinDirs.Set(key, value);
            }
            bMore = config.GetNextEntry(key, index);
        }
    }
    
    // Read toolchain paths (local machine again)
#ifdef __WXMSW__    
    wxArrayString arstrToolChainPaths;

    // Use eCos just as a test.
    //GetRepositoryRegistryClues(arstrToolChainPaths,_T("eCos"));
    GetRepositoryRegistryClues(arstrToolChainPaths,_T("GNUPro eCos"));
    
    size_t i;
    for (i = (size_t) 0; i < arstrToolChainPaths.GetCount(); i++)
    {
        ecFileName strDir(arstrToolChainPaths[i]);
        strDir += wxT("H-i686-cygwin32\\bin");
        
        if (strDir.IsDir())
        {
            // This is a potential toolchain location. Look for *-gcc.exe
            wxLogNull log;
            wxDir finder(strDefaultBuildToolsPath);
            wxString filename;
            
            if (finder.IsOpened())
            {
                bool bMore = finder.GetFirst(& filename, wxT("*-gcc.exe"));
                while (bMore)
                {
                    // TODO: if there is more than one path, we will have to
                    // check the existance of this target name in m_arstrBinDirs and
                    // append to the end, or something.
                    wxString targetName = filename.Left(filename.Find(wxT("-gcc")));
                    m_arstrBinDirs.Set(targetName, strDefaultBuildToolsPath);
                    
                    bMore = finder.GetNext(& filename);
                }
            }
        }
    }

    // The official user tools are now Cygwin 00r1. If you can't find these,
    // try GNUPro unsupported.
    GetRepositoryRegistryClues(m_userToolPaths, wxT("GNUPro 00r1"));
    if (m_userToolPaths.GetCount() == 0)
    {
        GetRepositoryRegistryClues(m_userToolPaths, wxT("Cygwin 00r1"));
    }

    if (m_userToolPaths.GetCount() > 0)
    {
        for ( i = (size_t) 0 ; i < m_userToolPaths.GetCount(); i++)
        {
            ecFileName str(m_userToolPaths[i]);
            str += "H-i686-cygwin32\\bin";
            if(str.IsDir())
            {
                m_userToolPaths[i] = str;
            } else
            {
                m_userToolPaths.Remove(i);
                i--;
            }
        }
    }
    else
    {
        GetRepositoryRegistryClues(m_userToolPaths, wxT("GNUPro unsupported"));
        
        for ( i = (size_t) 0 ; i < m_userToolPaths.GetCount(); i++)
        {
            ecFileName str(m_userToolPaths[i]);
            str += "H-i686-cygwin32\\bin";
            if(str.IsDir())
            {
                m_userToolPaths[i] = str;
            } else
            {
                m_userToolPaths.Remove(i);
                i--;
            }
        }
    }
#endif
    
    // Include the path in the set of potential user paths
    {
        wxString strPath;
        if (wxGetEnv(wxT("PATH"), & strPath))
        {
            wxArrayString arstrPath;
            ecUtils::Chop(strPath, arstrPath, wxT(';'));
            
            for (int i = arstrPath.GetCount()-1;i >= 0; --i)
            { // Reverse order is important to treat path correctly

                const ecFileName &strFolder = arstrPath[i];
                if (wxT(".") != strFolder && !strFolder.IsEmpty())
                {
                    ecFileName strFile(strFolder);
                    strFile += wxT("ls.exe");
                    if ( strFile.Exists() )
                    {
                        if (!wxArrayStringIsMember(m_userToolPaths, strFolder))
                        {
                            m_userToolPaths.Add(strFolder);
                        }

                        if ( m_userToolsDir.IsEmpty() )
                        {
                            m_userToolsDir = strFolder;
                        }
                    }
                }
            }
        }
    }
    
    // Load current repository from eCos Configuration Tool/Paths/RepositoryDir
    {
        wxConfig eCosConfig(wxGetApp().GetSettings().GetConfigAppName(), wxEmptyString, wxEmptyString, wxEmptyString, wxCONFIG_USE_GLOBAL_FILE|wxCONFIG_USE_LOCAL_FILE);
        wxConfigPathChanger path(& config, wxT("/Repository/"));

        //if (!eCosConfig.Read(wxT("Folder"), & m_strRepository))
        if (!eCosConfig.Read(wxT("/Paths/RepositoryDir"), & m_strRepository))
        {
#ifdef __WXMSW__
            // If we can't find the current folder, look for clues in the registry.
            wxArrayString arstr;
            switch (GetRepositoryRegistryClues(arstr, wxT("eCos")))
            {
            case 0:
                break;
            case 1:
            default:
                m_strRepository = arstr[0];
                break;
            }
#elif defined(__WXGTK__)
            // If we can't find the current folder, look for the latest version
            // in /opt/ecos
            m_strRepository = FindLatestVersion();
#else
            // Unsupported platform
            m_strRepositor = wxEmptyString;
#endif
        }

        // If we have set ECOS_REPOSITORY, this overrides whatever we have
        // read or found.
        wxString envVarValue = wxGetenv(wxT("ECOS_REPOSITORY"));
        if (!envVarValue.IsEmpty())
        {
            // Note that ECOS_REPOSITORY has the packages (or ecc) folder in the name.
            // In order to be in the form that is compatible with configtool operation,
            // it needs to have that stripped off.
            envVarValue = ecUtils::PosixToNativePath(envVarValue); // accommodate posix-style ECOS_REPOSITORY value under Cygwin
            wxString packagesName = wxFileNameFromPath(envVarValue);
            if (packagesName == wxT("ecc") || packagesName == wxT("packages"))
                envVarValue = wxPathOnly(envVarValue);

            m_strRepository = envVarValue;
        }
    }

#ifdef __WXMSW__
    if (m_userToolsDir.IsEmpty())
        m_userToolsDir = GetCygwinInstallPath() + wxT("\\bin");
#else
    if (m_userToolsDir.IsEmpty())
        m_userToolsDir = wxT("/bin");
#endif
    
    return TRUE;
}

#ifdef __WXMSW__
wxString ecSettings::GetCygwinInstallPath()
{
    HKEY hKey = 0;
    DWORD type;
    BYTE value[256];
    DWORD sz = sizeof(value);
    wxString strCygwinInstallPath;

    // look for the "/" mount point in the system registry settings
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Cygnus Solutions\\Cygwin\\mounts v2\\/", 0, KEY_READ, &hKey)) {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "native", NULL, & type, value, & sz)) {
            strCygwinInstallPath = (const char*) value;
        }
        RegCloseKey(hKey);
    }

    // if not yet found, look for the "/" mount point in the user's registry settings
    hKey = 0;
    sz = sizeof(value);
    if (strCygwinInstallPath.IsEmpty() && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Cygnus Solutions\\Cygwin\\mounts v2\\/", 0, KEY_READ, &hKey))) {
        if (ERROR_SUCCESS == RegQueryValueEx(hKey, "native", NULL, & type, value, & sz)) {
            strCygwinInstallPath = (const char*) value;
        }
        RegCloseKey(hKey);
    }

    return strCygwinInstallPath;
}
#endif

// Save config info
bool ecSettings::SaveConfig()
{
    wxConfig config(wxGetApp().GetSettings().GetConfigAppName());
    
    config.Write(_("/Files/LastFile"), m_lastFilename);
    
    config.Write(_("/Window Status/FrameStatus"), (long) m_frameStatus);
    config.Write(_("/Window Status/ShowToolBar"), m_showToolBar);
    config.Write(_("/Window Status/ShowSplashScreen"), m_showSplashScreen);
    config.Write(_("/Window Status/ShowConflictsWindow"), m_showConflictsWindow);
    config.Write(_("/Window Status/ShowPropertiesWindow"), m_showPropertiesWindow);
    config.Write(_("/Window Status/ShowShortDescrWindow"), m_showShortDescrWindow);
    config.Write(_("/Window Status/ShowMemoryWindow"), m_showMemoryWindow);
    config.Write(_("/Window Status/ShowOutputWindow"), m_showOutputWindow);
    
    config.Write(_("/Window Size/WindowX"), (long) m_frameSize.x);
    config.Write(_("/Window Size/WindowY"), (long) m_frameSize.y);
    config.Write(_("/Window Size/WindowWidth"), (long) m_frameSize.width);
    config.Write(_("/Window Size/WindowHeight"), (long) m_frameSize.height);
    config.Write(_("/Window Size/TreeSashWidth"), (long) m_treeSashSize.x);
    config.Write(_("/Window Size/TreeSashHeight"), (long) m_treeSashSize.y);
    config.Write(_("/Window Size/ConfigPaneWidth"), (long) m_configPaneWidth);
    config.Write(_("/Window Size/ConflictsWidth"), (long) m_conflictsSashSize.x);
    config.Write(_("/Window Size/ConflictsHeight"), (long) m_conflictsSashSize.y);
    config.Write(_("/Window Size/PropertiesWidth"), (long) m_propertiesSashSize.x);
    config.Write(_("/Window Size/PropertiesHeight"), (long) m_propertiesSashSize.y);
    config.Write(_("/Window Size/ShortDescrWidth"), (long) m_shortDescrSashSize.x);
    config.Write(_("/Window Size/ShortDescrHeight"), (long) m_shortDescrSashSize.y);
    config.Write(_("/Window Size/OutputWidth"), (long) m_outputSashSize.x);
    config.Write(_("/Window Size/OutputHeight"), (long) m_outputSashSize.y);
    config.Write(_("/Window Size/MemoryWidth"), (long) m_memorySashSize.x);
    config.Write(_("/Window Size/MemoryHeight"), (long) m_memorySashSize.y);
    
    config.Write(_("/Options/ShowMacroNames"), m_showMacroNames);
    config.Write(_("/Options/UseCustomViewer"), m_bUseCustomViewer);
    config.Write(_("/Options/UseExternalBrowser"), m_bUseExternalBrowser);
    config.Write(_("/Options/UseCustomBrowser"), (long) m_eUseCustomBrowser);
    config.Write(_("/Options/Browser"), m_strBrowser);
    config.Write(_("/Options/Viewer"), m_strViewer);
    config.Write(_("/Options/HexDisplay"), m_bHex);
    config.Write(_("/Options/UseDefaultFonts"), m_windowSettings.m_useDefaults);

    config.Write(_("/Rule/Checking"), (long) m_nRuleChecking);
    
    config.Write(_("/Paths/UserToolsDir"), m_userToolsDir);
    config.Write(_("/Paths/BuildToolsDir"), m_buildToolsDir);
    
    config.Write(_("/Build/Make Options"), m_strMakeOptions);

    // Find dialog settings
    config.Write(_("/Find/Text"), m_findText);
    config.Write(_("/Find/MatchWholeWord"), m_findMatchWholeWord);
    config.Write(_("/Find/MatchCase"), m_findMatchCase);
    config.Write(_("/Find/Direction"), m_findDirection);
    config.Write(_("/Find/SearchWhat"), m_findSearchWhat);
    config.Write(_("/Find/DialogX"), (long) m_findDialogPos.x);
    config.Write(_("/Find/DialogY"), (long) m_findDialogPos.y);
    
    // Package dialog settings
    config.Write(_("/Packages/OmitHardwarePackages"), m_omitHardwarePackages);
    config.Write(_("/Packages/MatchPackageNamesExactly"), m_matchPackageNamesExactly);

    // Save current repository to eCos Configuration Tool/Paths/RepositoryDir
    // UNLESS it was overridden by ECOS_REPOSITORY
    {
        wxString envVarValue = wxGetenv(wxT("ECOS_REPOSITORY"));
        if (m_strRepository == envVarValue)
        {
            // Don't override the value in the local settings
        }
        else
            config.Write(wxT("/Paths/RepositoryDir"), m_strRepository);
    }
    
    // Run tests settings
    m_runTestsSettings.SaveConfig(config);

    // Fonts
    m_windowSettings.SaveConfig(config);
    
    return TRUE;
}

void ecSettings::ShowSettingsDialog(const wxString& page)
{
#if 0
    ecSettingsDialog* dialog = new ecSettingsDialog(wxGetApp().GetTopWindow());
    if (!page.IsEmpty())
        dialog->GetNotebook()->SetSelection(ecFindNotebookPage(dialog->GetNotebook(), page));
    
    int ret = dialog->ShowModal();
    dialog->Destroy();
#endif
}

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

// TODO for non-Windows platforms
const ecFileName& ecSettings::DefaultExternalBrowser()
{
#ifdef __WXMSW__
    static bool bFirstTime=TRUE;
    if(bFirstTime){
        const ecFileName strFile(ecFileName::GetTempPath()+wxT("index.html"));
        wxFile f;
        if(f.Create(strFile, TRUE))
        {
            f.Close();
            bool rc=((int)  ::FindExecutable(strFile,wxT("."),m_strDefaultExternalBrowser.GetWriteBuf(MAX_PATH))>32);
            m_strDefaultExternalBrowser.UngetWriteBuf();
            if(!rc){
                m_strDefaultExternalBrowser=wxT("");
            }
            wxRemoveFile(strFile);
        }
        bFirstTime=FALSE;
    }
#endif
    return m_strDefaultExternalBrowser;
}

ecFileName ecSettings::m_strDefaultExternalBrowser;

// Go looking for potential candidates in SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
int ecSettings::GetRepositoryRegistryClues(wxArrayString& arstr, const wxString& pszPrefix)
{
    arstr.Clear();

#ifdef __WXMSW__
    wxConfig config(wxT("Windows"), wxT("Microsoft"), wxEmptyString, wxEmptyString, wxCONFIG_USE_GLOBAL_FILE);
    config.SetPath(wxT("/CurrentVersion/App Paths"));

    wxString key(wxT(""));
    long index;
    bool bMore = config.GetFirstGroup(key, index);
    while (bMore)
    {
        if (key.Find(pszPrefix) == 0)
        {
            wxString value;
            //wxString key2(wxString(wxT("/")) + key + wxString(wxT("/Path")));
            wxString key2(key + wxString(wxT("/Path")));
            if (config.Read(key2, & value))
            {
                arstr.Add(value);
            }
        }
        bMore = config.GetNextGroup(key, index);
    }
    return arstr.GetCount();
#else
    return 0;
#endif
}

// Finds the path of the latest installed eCos
wxString ecSettings::FindLatestVersion()
{
#ifdef __WXGTK__
    wxString ecosRoot(wxT("/opt/ecos"));
    wxLogNull log;
    wxDir dir(ecosRoot);
    
    wxString latestDir;
    wxString latestVersion;
    
    if (dir.IsOpened())
    {
        wxString filename;
        bool cont = dir.GetFirst(& filename, wxT("ecos-*"), wxDIR_DIRS);
        while (cont)
        {
            wxString ver(filename.Mid(5));
            if (latestVersion.IsEmpty() || latestVersion.CompareTo(ver) < 0)
            {
                latestVersion = ver;
                latestDir = ecosRoot + wxT("/") + filename;
            }
            
            cont = dir.GetNext(& filename);
        }
    }
//    if (latestDir.IsEmpty())
//        latestDir = wxGetCwd();
    return latestDir;
#else
    wxMessageBox(wxT("FindLatestVersion() is only implemented for Unix."));
    return wxEmptyString;
#endif
}

wxString ecSettings::GetInstallVersionKey ()
{
#ifdef __WXMSW__

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

    wxString strKey = wxT("SOFTWARE\\eCos");
    wxString strVersionKey = wxT("");
    wxString rc = wxT("");
    wxChar pszBuffer [MAX_PATH + 1];
    HKEY hKey;
    
    // get the greatest eCos version subkey
    if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, strKey, 0L, KEY_READ, &hKey)) {
        DWORD dwIndex = 0;
        while (ERROR_SUCCESS == RegEnumKey (hKey, dwIndex++, (LPTSTR) pszBuffer, sizeof (pszBuffer))) {
            if (strVersionKey.CompareTo (pszBuffer) < 0) {
                strVersionKey = pszBuffer;
            }
        }
        RegCloseKey (hKey);
    }
    return strVersionKey;
#else
    wxMessageBox(wxT("GetInstallVersionKey() is only implemented for Windows."));
    return wxEmptyString;
#endif
}

/*
 * ecRunTestSettings
 * Settings relating to running tests
 */

IMPLEMENT_DYNAMIC_CLASS(ecRunTestsSettings, wxObject)

ecRunTestsSettings::ecRunTestsSettings():
    m_ep(CeCosTest::ExecutionParameters::RUN),
    m_nTimeout(900),
    m_nDownloadTimeout(120),
    m_nTimeoutType(TIMEOUT_AUTOMATIC),
    m_nDownloadTimeoutType(TIMEOUT_SPECIFIED),
    m_bRemote(FALSE),
    m_bSerial(TRUE),
    m_strPort(wxT("COM1")),
    m_nBaud(38400),
    m_nLocalTCPIPPort(1),
    m_nReset(RESET_MANUAL),
    m_nResourcePort(1),
    m_nRemotePort(1),
    m_bFarmed(TRUE),
    m_strRemoteHost(wxT("")),
    m_strResourceHost(wxT("")),
    m_strLocalTCPIPHost(wxT("")),
    m_strReset(wxT(""))
{
}

ecRunTestsSettings::ecRunTestsSettings(const ecRunTestsSettings& settings)
{
    Copy(settings);
}

void ecRunTestsSettings::Copy(const ecRunTestsSettings& settings)
{
    m_nTimeoutType = settings.m_nTimeoutType;
    m_nDownloadTimeoutType = settings.m_nDownloadTimeoutType;
    m_bRemote = settings.m_bRemote;
    m_bSerial = settings.m_bSerial;
    m_strPort = settings.m_strPort;
    m_nBaud = settings.m_nBaud;
    m_nLocalTCPIPPort = settings.m_nLocalTCPIPPort;
    m_nReset = settings.m_nReset;
    m_nResourcePort = settings.m_nResourcePort;
    m_nRemotePort = settings.m_nRemotePort;
    m_bFarmed = settings.m_bFarmed;
    m_strTarget = settings.m_strTarget;
    m_strRemoteHost = settings.m_strRemoteHost;
    m_strResourceHost = settings.m_strResourceHost;
    m_strLocalTCPIPHost = settings.m_strLocalTCPIPHost;
    m_strReset = settings.m_strReset;
}

bool ecRunTestsSettings::LoadConfig(wxConfig& config)
{
    config.Read(_("/Run Tests/TimeoutType"), & m_nTimeoutType);
    config.Read(_("/Run Tests/DownloadTimeoutType"), & m_nDownloadTimeoutType);
    config.Read(_("/Run Tests/Remote"), (bool*) & m_bRemote);
    config.Read(_("/Run Tests/Serial"), (bool*) & m_bSerial);
    config.Read(_("/Run Tests/Port"), & m_strPort);
    config.Read(_("/Run Tests/Baud"), & m_nBaud);
    config.Read(_("/Run Tests/LocalTCPIPHost"), & m_strLocalTCPIPHost);
    config.Read(_("/Run Tests/LocalTCPIPPort"), & m_nLocalTCPIPPort);
//  Reset type is determined at run-time for standalone configtool
//    config.Read(_("/Run Tests/ResetType"), & m_nReset);
    config.Read(_("/Run Tests/ResetString"), & m_strReset);
    config.Read(_("/Run Tests/ResourceHost"), & m_strResourceHost);
    config.Read(_("/Run Tests/ResourcePort"), & m_nResourcePort);
    config.Read(_("/Run Tests/RemoteHost"), & m_strRemoteHost);
    config.Read(_("/Run Tests/RemotePort"), & m_nRemotePort);
    config.Read(_("/Run Tests/Farmed"), (bool*) & m_bFarmed);

    return TRUE;
}

bool ecRunTestsSettings::SaveConfig(wxConfig& config)
{
    config.Write(_("/Run Tests/TimeoutType"), (long) m_nTimeoutType);
    config.Write(_("/Run Tests/DownloadTimeoutType"), (long) m_nDownloadTimeoutType);
    config.Write(_("/Run Tests/Remote"), m_bRemote);
    config.Write(_("/Run Tests/Serial"), m_bSerial);
    config.Write(_("/Run Tests/Port"), m_strPort);
    config.Write(_("/Run Tests/Baud"), (long) m_nBaud);
    config.Write(_("/Run Tests/LocalTCPIPHost"), m_strLocalTCPIPHost);
    config.Write(_("/Run Tests/LocalTCPIPPort"), (long) m_nLocalTCPIPPort);
    config.Write(_("/Run Tests/ResetType"), (long) m_nReset);
    config.Write(_("/Run Tests/ResetString"), m_strReset);
    config.Write(_("/Run Tests/ResourceHost"), m_strResourceHost);
    config.Write(_("/Run Tests/ResourcePort"), (long) m_nResourcePort);
    config.Write(_("/Run Tests/RemoteHost"), m_strRemoteHost);
    config.Write(_("/Run Tests/RemotePort"), (long) m_nRemotePort);
    config.Write(_("/Run Tests/Farmed"), m_bFarmed);

    return TRUE;
}

