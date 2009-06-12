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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf, jld
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Interface to various global utility functions.  Everything in this
//				class is static (there are no instances).
//              Modified by julians for wxWindows (originally CTUtils.h)
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//  
//===========================================================================

#ifndef _ECUTILS_H_
#define _ECUTILS_H_

#ifdef __GNUG__
#pragma interface "ecutils.cpp"
#endif

#include "wx/hash.h"
#include "wx/confbase.h"
#include "wx/dynarray.h"

#include "filename.h"
#include "cdl.hxx"

//#define INCLUDEFILE <string>
//#include "IncludeSTL.h"

//#include <stdarg.h>     // vsnprintf

/*
 * ecUtils
 * This class implements a miscellany of utility functions.
 * They are being implemented as and when they are required during Configtool
 * development.
 */

class ecUtils {
public:
#if 0
    static bool Launch (const ecFileName &strFileName,const ecFileName &strViewer);
    static wxString GetLastErrorMessageString ();
#endif    

    static const wxString PosixToNativePath(const wxString & posix);
    static const wxString NativeToPosixPath(const wxString & native);
    static bool AddToPath (const ecFileName &strFolder, bool bAtFront=true);

#if 0
    static const wxString LoadString (UINT id);
    
    // Messagebox functions
    
    // Vararg message box compositor
    static int MessageBoxF (LPCTSTR pszFormat, ...);
    // Same, with type
    static int MessageBoxFT (UINT nType, LPCTSTR pszFormat, ...);
    // As above but with resource
    static int MessageBoxFR (UINT nID, UINT nType, LPCTSTR pszFormat, ...);
    // Same, with type
    static int MessageBoxFR (UINT nID, LPCTSTR pszFormat, ...);
    // vararg form
    static int vMessageBox(UINT nType, LPCTSTR  pszFormat, va_list marker);
    
#endif
    // String functions
    
    // Chop the string into pieces using separator cSep.
    // The Boolean controls whether " and \ make a difference
    static int Chop(const wxString& psz,wxArrayString &ar, wxChar cSep=wxT(' '),
        bool bObserveStrings = FALSE, bool bBackslashQuotes = FALSE);

    static int Chop(const wxString& psz,wxArrayString &ar, const wxString& pszSep,
        bool bObserveStrings=FALSE, bool bBackslashQuotes = FALSE);

    // String -> Integer, observing the current hex/decimal setting
    //static BOOL StrToItemIntegerType(const wxString &str, __int64 &d);
    static bool StrToItemIntegerType(const wxString &str, long &d);
    static bool StrToDouble (const wxString &strValue, double &dValue);

    // Integer -> String, observing the current hex/decimal setting
    static const wxString IntToStr(long d,bool bHex=false);
    static const wxString DoubleToStr (double dValue);
    static wxString StripExtraWhitespace (const wxString & strInput);

    static void UnicodeToCStr(const wxChar* str,char *&psz);
    static std::string UnicodeToStdStr(const wxChar* str);

#if 0    
    // Provide a failure explanation for what just went wrong
    static const wxString Explanation (CFileException &exc);
    static ecFileName WPath(const std::string &str);
    static bool CopyFile (LPCTSTR pszSource,LPCTSTR pszDest);
#endif
};

/*
 * wxStringToStringMap
 *
 * Stores string values keyed by strings.
 * Note that only one value is allowed per key.
 */

class wxStringToStringMap: public wxObject
{
public:
    wxStringToStringMap(): m_hashTable(wxKEY_STRING) {}
    ~wxStringToStringMap() { Clear(); }

//// Operations

    // Remove the key/value pair
    bool Remove(const wxString& key);

    // Clear the map
    void Clear();

    // Begin a search through the whole map: use Next to find key/value pairs
    void BeginFind();

    // Retrieve the next key/value pair, FALSE indicates end of data
    bool Next(wxString& key, wxString& value);

//// Accessors

    // Replaces the value (if any) associated with the key
    void Set(const wxString& key, const wxString& value);

    // Finds a value associated with the key, returns FALSE if none
    bool Find(const wxString& key, wxString& value);

protected:
    wxHashTable m_hashTable;
};

// Is str a member of arr?
bool wxArrayStringIsMember(const wxArrayString& arr, const wxString& str);

// Eliminate .. and .
wxString wxGetRealPath(const wxString& path);

// A version of the above but prepending the cwd (current path) first
// if 'path' is relative
wxString wxGetRealPath(const wxString& cwd, const wxString& path);

// Find the absolute path where this application has been run from.
// argv0 is wxTheApp->argv[0]
// cwd is the current working directory (at startup)
// appVariableName is the name of a variable containing the directory for this app, e.g.
//   MYAPPDIR. This is used as a last resort. Or should it be a first resort?
wxString wxFindAppPath(const wxString& argv0, const wxString& cwd, const wxString& appVariableName = wxEmptyString);

// Find the text of the list control item at the given column
class WXDLLEXPORT wxListCtrl;
wxString wxListCtrlGetItemTextColumn(wxListCtrl& listCtrl, long item, int col);

// Select the given item
void wxListCtrlSelectItem(wxListCtrl& listCtrl, long sel, bool deselectOther = TRUE);

// Find the selection
long wxListCtrlGetSelection(wxListCtrl& listCtrl);

// Find which column the cursor is on
int wxListCtrlFindColumn(wxListCtrl& listCtrl, int noCols, int x);

// Refresh children of this window (e.g. on Windows, laying out a window containing
// controls can cause screen corruption)
void wxRefreshControls(wxWindow* win);

// Make a path name with no separators, out of a full pathname,
// e.g. opt_ecos_ecos-1.4.5 out of /opt/ecos/ecos-1.4.5
wxString ecMakeNameFromPath(const wxString& path);

// Kill a process
int ecKill(long pid, wxSignal sig);

class WXDLLEXPORT wxOutputStream;

wxOutputStream& operator <<(wxOutputStream&, const wxString& s);
wxOutputStream& operator <<(wxOutputStream&, const char c);
wxOutputStream& operator <<(wxOutputStream&, long l);

/*
 * Class for recursively killing a process and its children.
 * This has been taken from CSubprocess, but we can't use CSubprocess
 * directly for killing a process because it assumes it's been used to create
 * the process.
 */

class wxProcessKiller
{

public:
    const wxString ErrorString() const;

    wxProcessKiller(int pid);
    ~wxProcessKiller();
    
    void SetVerbose   (bool b)         { m_bVerbose=b; }
      
    int Pid() const { return m_idProcess; } // returns process id (even when process is terminated)
    
    // Get the process exit code.  This can be:
    //   exit code of process (if terminated)
    //   0xffffffff (if process not yet run)
    //   GetLastError result (if process could not be run)
    int GetExitCode() { return m_nExitCode; }
    
    // Kill the process:
    bool Kill(bool bRecurse = true);
    
    // Is the process running?
    bool ProcessAlive();
    
protected:
    static const wxString Name (int pid); // for debugging - only works under NT
    
    struct wxPInfo;
    struct wxPInfo {
        wxPInfo *pParent;
#ifdef _WIN32
        wxLongLong_t tCreation;
#endif
        long tCpu;
        int PID;
        int PPID;
        bool IsChildOf(int pid) const;
    };
    
    typedef std::vector<wxPInfo> wxPInfoArray;
    
    static bool PSExtract(wxPInfoArray &arPinfo);
    static void SetParents(wxPInfoArray &arPinfo);
    
#ifdef _WIN32
    static long GetPlatform();
    WXHANDLE m_hrPipe;
    WXHANDLE m_hwPipe;
    WXHANDLE m_hProcess;     // This handle is "owned" by the ThreadFunc
    static WXHINSTANCE hInstLib1, hInstLib2;
    int m_nErr;
#else
    int m_tty;
    wxString m_strCmd;
#endif
    
    bool m_bVerbose;
    int m_nExitCode;
    int m_idProcess;
    
    static const unsigned int PROCESS_KILL_EXIT_CODE;
};

/*
 * ecDialog
 * Supports features we want to have for all dialogs in the application.
 * So far, this just allows dialogs to be resizeable under MSW by
 * refreshing the controls in OnSize (otherwise there's a mess)
 */

class ecDialog: public wxDialog
{
public:
    DECLARE_CLASS(ecDialog)

    ecDialog() {};

    ecDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition,
        const wxSize& sz = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE)
    {
        Create(parent, id, title, pos, sz, style);
    }
/*
    bool Create(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition,
        const wxSize& sz = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
*/

    void OnSize(wxSizeEvent& event);

DECLARE_EVENT_TABLE()
};

/*
 * Implements saving/loading of window settings - fonts only for now
 */

class wxWindowSettingsObject: public wxObject
{
public:
    wxWindowSettingsObject(const wxString& name, wxWindow* win) { m_windowName = name; if (win) m_arrWindow.Add(win); }
    wxString    m_windowName;
    wxFont      m_font;
    wxArrayPtrVoid m_arrWindow;
};

class wxWindowSettings: public wxObject
{
public:
    wxWindowSettings() { m_useDefaults = TRUE; };
    ~wxWindowSettings() { Clear(); }

//// Operations
    wxWindowSettingsObject* FindSettings(const wxString& windowName) const;
    void Add(const wxString& name, wxWindow* win = NULL) { m_settings.Append(new wxWindowSettingsObject(name, win)); }

    bool LoadConfig(wxConfigBase& config);
    bool SaveConfig(wxConfigBase& config);

    bool LoadFont(wxConfigBase& config, const wxString& windowName, wxFont& font);
    bool SaveFont(wxConfigBase& config, const wxString& windowName, const wxFont& font);

    void Clear() { m_settings.DeleteContents(TRUE); m_settings.Clear(); m_settings.DeleteContents(FALSE); }

    bool ApplyFontsToWindows();

//// Accessors
    wxFont GetFont(const wxString& name) const;
    void SetFont(const wxString& name, const wxFont& font);

    wxWindow* GetWindow(const wxString& name) const;
    void SetWindow(const wxString& name, wxWindow* win);

    wxArrayPtrVoid* GetWindows(const wxString& name) const;
    void SetWindows(const wxString& name, wxArrayPtrVoid& arr);

    int GetCount() const { return m_settings.Number(); }
    wxWindowSettingsObject* GetNth(int i) const { return (wxWindowSettingsObject*) m_settings.Nth(i)->Data(); }
    wxString GetName(int i) const { return GetNth(i)->m_windowName; }

    void SetUseDefaults(bool useDefaults) { m_useDefaults = useDefaults; }
    bool GetUseDefaults() const { return m_useDefaults; }

public:
    wxList  m_settings;
    bool    m_useDefaults;
};

#endif
// _ECUTILS_H_
