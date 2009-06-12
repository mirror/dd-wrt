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
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf, jld
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is a collection of utility functions.
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

#ifdef __GNUG__
#pragma implementation "ecutils.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ecutils.h"
#include "wx/listctrl.h"
#include "wx/stream.h"

#include <float.h> // for DBL_DIG macro
#include <sys/types.h>
#include <sys/stat.h>

#ifndef __WXMSW__
#include <errno.h>
#endif

#ifdef __WXMSW__
#include <tlhelp32.h>
#endif

#ifdef __CYGWIN__
#include <sys/cygwin.h> /* for cygwin_conv_to_*_path() */
#endif

#if 0

#define INCLUDEFILE <string>
#include "IncludeSTL.h"

#endif

// Chop str into pieces, using cSep as separator.
// " and \ have usual semantics
// Return value is array of pieces found.

int ecUtils::Chop(const wxString& psz, wxArrayString &ar, wxChar cSep,
                  bool bObserveStrings/*=false*/,bool bBackslashQuotes/*=false*/)

{
    if(wxT(' ') == cSep)
    {
        return Chop(psz, ar, wxT("\t\n\v\f\r "), bObserveStrings, bBackslashQuotes);
    } else {
        wxASSERT(wxT('\0')!=cSep);
        wxChar c[2]={cSep,wxT('\0')};
        return Chop(psz,ar,wxString(c),bObserveStrings,bBackslashQuotes);
    }
}

int ecUtils::Chop(const wxString& str, wxArrayString &ar, const wxString& sep,
                  bool bObserveStrings/*=false*/,bool bBackslashQuotes/*=false*/)
{
    ar.Clear();
    
    const wxChar* pszSep = (const wxChar*) sep;
    const wxChar* psz = (const wxChar*) str;
    
    int i=0;
    for(;;){
        // Skip multiple separators
        while(*psz && wxStrchr(pszSep,*psz)){
            psz++;
        }
        if(!*psz){
            return i;
        }
        wxString strTok;
        if(bObserveStrings){
            bool bInString=FALSE;
            do{
                if(*psz == wxT('\\') && bBackslashQuotes && psz[1]){
                    strTok += psz[1];
                    psz++;
                } else if(*psz == wxT('"')){
                    bInString ^= 1;
                } else if (!bInString && *psz && NULL != wxStrchr(pszSep,*psz)) {
                    break;
                } else {
                    strTok+=*psz;
                }
            } while (*++psz);
        } else {
            const wxChar* pszStart=psz;
            do {
                psz++;
            } while (*psz && ! wxStrchr(pszSep,*psz));
            strTok=wxString(pszStart,psz-pszStart);
        }
        ar.Add(strTok);
        i++;
    }
    return ar.GetCount();
}

#if 0

// vararg-style message box formatter
int ecUtils::MessageBoxF (LPCTSTR pszFormat, ...)
{
  int rc;
  va_list args;
  va_start(args, pszFormat);
  rc=ecUtils::vMessageBox(MB_OK, pszFormat,args);
  va_end(args);
  return rc;
}

// As above, but with type as first parameter.
int ecUtils::MessageBoxFT (UINT nType, LPCTSTR pszFormat, ...)
{
  int rc;
  va_list args;
  va_start(args, pszFormat);
  rc=ecUtils::vMessageBox(nType, pszFormat,args);
  va_end(args);
  return rc;
}

int ecUtils::vMessageBox(UINT nType, LPCTSTR  pszFormat, va_list marker)
{
  int rc=0;
  for(int nLength=100;nLength;) {
    TCHAR *buf=new TCHAR[1+nLength];
    int n=_vsntprintf(buf, nLength, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      rc=AfxMessageBox(buf,nType);
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
  return rc;
}

#endif

bool ecUtils::StrToItemIntegerType(const wxString & str, long &d)
{
	wxChar* pEnd;
	bool rc;
	errno=0;
	bool bHex=(str.Len() > 2 && str[0]==wxT('0') && (str[1]==wxT('x')||str[1]==wxT('X')));
	//d=_tcstol(str,&pEnd,bHex?16:10);
	d=wxStrtol(str,&pEnd,bHex?16:10);
	rc=(0==errno && (*pEnd==wxT('\0')));
	return rc;
}

const wxString ecUtils::IntToStr(long d,bool bHex)
{
  wxString s;
  s.Printf(bHex?wxT("0x%08x"):wxT("%d"),d);
  return s;
}

const wxString ecUtils::DoubleToStr (double dValue)
{
  wxString s;
  s.Printf(wxT("%.*e"), DBL_DIG, dValue);
  return s;
}

bool ecUtils::StrToDouble (const wxString & strValue, double &dValue)
{
	wxChar* pEnd;
	errno = 0;
	//dValue = _tcstod (strValue, &pEnd);
	dValue = wxStrtod(strValue, &pEnd);
	return (0 == errno) && (*pEnd == wxT('\0'));
}

#if 0
const wxString ecUtils::Explanation(CFileException & exc)
{
	wxString strMsg;
	switch(exc.m_cause){
		case CFileException::none: strMsg=wxT("No error occurred.");break;
		case CFileException::generic: strMsg=wxT("   An unspecified error occurred.");break;
		case CFileException::fileNotFound: strMsg=wxT("   The file could not be located.");break;
		case CFileException::badPath: strMsg=wxT("   All or part of the path is invalid.");break;
		case CFileException::tooManyOpenFiles: strMsg=wxT("   The permitted number of open files was exceeded.");break;
		case CFileException::accessDenied: strMsg=wxT("   The file could not be accessed.");break;
		case CFileException::invalidFile: strMsg=wxT("   There was an attempt to use an invalid file handle.");break;
		case CFileException::removeCurrentDir: strMsg=wxT("   The current working directory cannot be removed.");break;
		case CFileException::directoryFull: strMsg=wxT("   There are no more directory entries.");break;
		case CFileException::badSeek: strMsg=wxT("   There was an error trying to set the file pointer.");break;
		case CFileException::hardIO: strMsg=wxT("   There was a hardware error.");break;
		case CFileException::sharingViolation: strMsg=wxT("   SHARE.EXE was not loaded, or a shared region was locked.");break;
		case CFileException::lockViolation: strMsg=wxT("   There was an attempt to lock a region that was already locked.");break;
		case CFileException::diskFull: strMsg=wxT("   The disk is full.");break;
		case CFileException::endOfFile: strMsg=wxT("   The end of file was reached. ");break;
		default:
			strMsg=wxT(" Unknown cause");
			break;
	}
	return strMsg;
}


const wxString ecUtils::LoadString(UINT id)
{
	wxString str;
	str.LoadString(id);
	return str;
}
#endif

const wxString ecUtils::NativeToPosixPath(const wxString & native)
{
#ifdef __CYGWIN__
    if (native.IsEmpty())
        return native;
    else
    {
        wxString posix;
        cygwin_conv_to_posix_path(native.c_str(), posix.GetWriteBuf(MAXPATHLEN + 1));
        posix.UngetWriteBuf();
        return posix;
    }
#else
    return native;
#endif
}

const wxString ecUtils::PosixToNativePath(const wxString & posix)
{
#ifdef __CYGWIN__
    if (posix.IsEmpty())
        return posix;
    else
    {
        wxString native;
        cygwin_conv_to_win32_path(posix.c_str(), native.GetWriteBuf(MAXPATHLEN + 1));
        native.UngetWriteBuf();
        return native;
    }
#else
    return posix;
#endif
}

bool ecUtils::AddToPath(const ecFileName & strFolder, bool bAtFront)
{
    wxString strPath,strOldPath;
    
    if (wxGetEnv(wxT("PATH"), & strOldPath))
    {
        // Place the user tools folders at the head or tail of the new path
        if(bAtFront)
        {
            strPath.Printf(wxT("%s;%s"), (const wxChar*) strFolder.ShortName(), (const wxChar*) strOldPath);
        } else
        {
            strPath.Printf(wxT("%s;%s"), (const wxChar*) strOldPath, (const wxChar*) strFolder.ShortName());
        }
    } else
    {
        // unlikely, but ...
        strPath = strFolder;
    }
    return (TRUE == wxSetEnv(wxT("PATH"),strPath));
}

#if 0		
wxString ecUtils::GetLastErrorMessageString()
{
	wxString str;
	PTCHAR pszMsg;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&pszMsg,
		0,
		NULL 
	);

	// Display the string.
	str=pszMsg;
  str.TrimRight();
	// Free the buffer.
	LocalFree(pszMsg);
	return str;

}

bool ecUtils::Launch(const ecFileName &strFileName, const ecFileName &strViewer)
{
	bool rc=false;

	if(!strViewer.IsEmpty())//use custom editor
	{
		wxString strCmdline(strViewer);
		
		PTCHAR pszCmdLine=strCmdline.GetBuffer(strCmdline.GetLength());
		GetShortPathName(pszCmdLine,pszCmdLine,strCmdline.GetLength());
		strCmdline.ReleaseBuffer();

		strCmdline+=_TCHAR(' ');
		strCmdline+=strFileName;
		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		si.cb = sizeof(STARTUPINFO); 
		si.lpReserved = NULL; 
		si.lpReserved2 = NULL; 
		si.cbReserved2 = 0; 
		si.lpDesktop = NULL; 
		si.dwFlags = 0; 
		si.lpTitle=NULL;

		if(CreateProcess(
			NULL, // app name
			//strCmdline.GetBuffer(strCmdline.GetLength()),    // command line
			strCmdline.GetBuffer(strCmdline.GetLength()),    // command line
			NULL, // process security
			NULL, // thread security
			TRUE, // inherit handles
			0,
			NULL, // environment
			NULL, // current dir
			&si, // startup info
			&pi)){
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
			rc=true;
		} else {
			MessageBoxF(wxT("Failed to invoke %s.\n"),strCmdline);
		}
		strCmdline.ReleaseBuffer();
	} else {// Use association
		TCHAR szExe[MAX_PATH];
		HINSTANCE h=FindExecutable(strFileName,wxT("."),szExe);
		if(int(h)<=32){
			wxString str;
			switch(int(h)){
				case 0:  str=wxT("The system is out of memory or resources.");break;
				case 31: str=wxT("There is no association for the specified file type.");break;
				case ERROR_FILE_NOT_FOUND: str=wxT("The specified file was not found.");break;
				case ERROR_PATH_NOT_FOUND: str=wxT("The specified path was not found.");break;
				case ERROR_BAD_FORMAT:     str=wxT("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).");break;
				default: break;
			}
			MessageBoxF(wxT("Failed to open document %s.\r\n%s"),strFileName,str);
		} else {

			SHELLEXECUTEINFO sei = {sizeof(sei), 0, AfxGetMainWnd()->GetSafeHwnd(), wxT("open"),
					strFileName, NULL, NULL, SW_SHOWNORMAL, AfxGetInstanceHandle( )};

			sei.hInstApp=0;
			HINSTANCE hInst=ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),wxT("open"), strFileName, NULL, wxT("."), 0)/*ShellExecuteEx(&sei)*/;
			if(int(hInst)<=32/*sei.hInstApp==0*/)
			{
				wxString str;
				switch(int(hInst))
				{
					case 0 : str=wxT("The operating system is out of memory or resources. ");break;
					case ERROR_FILE_NOT_FOUND : str=wxT("The specified file was not found. ");break;
					case ERROR_PATH_NOT_FOUND : str=wxT("The specified path was not found. ");break;
					case ERROR_BAD_FORMAT : str=wxT("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image). ");break;
					case SE_ERR_ACCESSDENIED : str=wxT("The operating system denied access to the specified file. ");break;
					case SE_ERR_ASSOCINCOMPLETE : str=wxT("The filename association is incomplete or invalid. ");break;
					case SE_ERR_DDEBUSY : str=wxT("The DDE transaction could not be completed because other DDE transactions were being processed. ");break;
					case SE_ERR_DDEFAIL : str=wxT("The DDE transaction failed. ");break;
					case SE_ERR_DDETIMEOUT : str=wxT("The DDE transaction could not be completed because the request timed out. ");break;
					case SE_ERR_DLLNOTFOUND : str=wxT("The specified dynamic-link library was not found. ");break;
					//case SE_ERR_FNF : str=wxT("The specified file was not found. ");break;
					case SE_ERR_NOASSOC : str=wxT("There is no application associated with the given filename extension. ");break;
					case SE_ERR_OOM : str=wxT("There was not enough memory to complete the operation. ");break;
					//case SE_ERR_PNF : str=wxT("The specified path was not found. ");break;
					case SE_ERR_SHARE : str=wxT("A sharing violation occurred. ");break;
					default: str=wxT("An unexpected error occurred");break;
				}
				MessageBoxF(wxT("Failed to open document %s using %s.\r\n%s"),strFileName,szExe,str);
			} else {
				rc=true;
			}
		}
	}
	return rc;
}

#endif

void ecUtils::UnicodeToCStr(const wxChar* str,char *&psz)
{
    int nLength=1 + wxStrlen(str);
    psz=new char[nLength];
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP, 0, str, -1, psz, nLength, NULL, NULL);
#else
    strcpy(psz,str);
#endif
}

std::string ecUtils::UnicodeToStdStr(const wxChar* str)
{
    std::string stdstr;
    char *psz;
    UnicodeToCStr(str,psz);
    stdstr=std::string(psz);
    delete psz;
    return stdstr;
}

// ecUtils::StripExtraWhitespace() returns a modified version of
// a string in which each sequence of whitespace characters is
// replaced by a single space

static bool ecIsSpace(wxChar ch)
{
	return (ch == wxT(' ') || ch == wxT('\r') || ch == wxT('\n') || ch == wxT('\t'));
}

wxString ecUtils::StripExtraWhitespace (const wxString & strInput)
{
    wxString strOutput;
    wxChar* o=strOutput.GetWriteBuf(1+strInput.Len());
    for(const wxChar* c=strInput.GetData();*c;c++){
        if(ecIsSpace(*c)){
            *o++=wxT(' ');
            if (ecIsSpace(c[1])){
                for(c=c+2; ecIsSpace(*c);c++);
                c--;
            }
        } else {
            *o++=*c;
        }
    }
    *o=0;
    strOutput.UngetWriteBuf();
    strOutput.Trim(TRUE);
    strOutput.Trim(FALSE);
    return strOutput;
#if 0    
    wxString strOutput;
    LPTSTR o=strOutput.GetBuffer(1+strInput.GetLength());
    for(LPCTSTR c=strInput;*c;c++){
        if(_istspace(*c)){
            *o++=_TCHAR(' ');
            if (_istspace(c[1])){
                for(c=c+2;_istspace(*c);c++);
                c--;
            }
        } else {
            *o++=*c;
        }
    }
    *o=0;
    strOutput.ReleaseBuffer();
    strOutput.TrimLeft();
    strOutput.TrimRight();
    return strOutput;
#endif
}

#if 0
ecFileName ecUtils::WPath(const std::string &str)
{
  // Convert a path as read from cdl into host format
  // Change / to \ throughout
  ecFileName
    strPath(str.c_str());
  strPath.Replace (_TCHAR('/'), _TCHAR('\\'));
  return strPath;
}

// Copy file helper function.
// This makes sure the destination file is only touched as necessary.
// It is written using Posix calls lest it should be more broadly applicable.

bool ecUtils::CopyFile(LPCTSTR pszSource,LPCTSTR pszDest)
{
  // Compare the files.  First set rc to the result of the comparison (true if the same)
  bool rc=false;

  struct _stat s1,s2;
  if(-1!=_tstat(pszSource,&s1) && -1!=_tstat(pszDest,&s2) && s1.st_size==s2.st_size){
    // Files both exist and are of equal size
    FILE *f1=_tfopen(pszSource,wxT("rb"));
    if(f1){
      FILE *f2=_tfopen(pszDest,wxT("rb"));
      if(f2){
        int nSize1,nSize2;
        rc=true;
        do{
          char buf1[4096],buf2[4096];
          nSize1=fread(buf1,1,sizeof buf1,f1);
          nSize2=fread(buf2,1,sizeof buf2,f2);
          if(nSize1!=nSize2 || 0!=memcmp(buf1,buf2,nSize1)){
            rc=false;
            break;
          }
        } while (nSize1>0);
        fclose(f2);
      }
      fclose(f1);
    }
  }

  if(rc){
    // Files are identical
  } else {
    rc=TRUE==::CopyFile(pszSource,pszDest,FALSE);
    if(rc){
    } else {
      MessageBoxF(wxT("Failed to copy '%s' to '%s' - %s"),pszSource,pszDest,GetLastErrorMessageString());
    }
  }

  return rc;
}

#endif

/*
 * wxStringToStringMap
 *
 * Stores string values keyed by strings
 */

void wxStringToStringMap::Set(const wxString& key, const wxString& value)
{
    wxString oldValue;
    if (Find(key, oldValue))
        Remove(key);
    m_hashTable.Put(key, (wxObject*) new wxString(value));
}

bool wxStringToStringMap::Remove(const wxString& key)
{
    wxString* str = (wxString*) m_hashTable.Delete(key);
    if (str)
    {
        delete str;
        return TRUE;
    }
    else
        return FALSE;
}

bool wxStringToStringMap::Find(const wxString& key, wxString& value)
{
    wxString* str = (wxString*) m_hashTable.Get(key);
    if (str)
    {
        value = * str;
        return TRUE;
    }
    else
        return FALSE;
}

void wxStringToStringMap::Clear()
{
    m_hashTable.BeginFind();
    wxNode* node;
    while ((node = m_hashTable.Next()))
    {
        wxString* str = (wxString*) node->Data();
        delete str;
    }
}

void wxStringToStringMap::BeginFind()
{
    m_hashTable.BeginFind();
}

bool wxStringToStringMap::Next(wxString& key, wxString& value)
{
    wxNode* node = m_hashTable.Next();
    if (node)
    {
        value = * (wxString*) node->Data();
        return TRUE;
    }
    else
        return FALSE;
}

// Is str a member of arr?
bool wxArrayStringIsMember(const wxArrayString& arr, const wxString& str)
{
    size_t i;
    for (i = (size_t) 0; i < arr.GetCount(); i++)
        if (arr[i] == str)
            return TRUE;

    return FALSE;
}

// Eliminate .. and .
wxString wxGetRealPath(const wxString& path)
{
    wxChar* p = new wxChar[path.Len() + 1];
    wxStrcpy(p, (const wxChar*) path);
    wxRealPath(p);

    wxString str(p);
    delete[] p;
    return str;
}

// A version of the above but prepending 'cwd' (current path) first
// if 'path' is relative
wxString wxGetRealPath(const wxString& cwd, const wxString& path)
{
    wxString path1(path);

    if (!wxIsAbsolutePath(path))
    {
        path1 = cwd;
        if (path1.Last() != wxFILE_SEP_PATH)
            path1 += wxFILE_SEP_PATH;
        path1 += path;
    }

    return wxGetRealPath(path1);
}

// Find the absolute path where this application has been run from.
// argv0 is wxTheApp->argv[0]
// cwd is the current working directory (at startup)
// appVariableName is the name of a variable containing the directory for this app, e.g.
// MYAPPDIR. This is checked first.

wxString wxFindAppPath(const wxString& argv0, const wxString& cwd, const wxString& appVariableName)
{
    wxString str;

    // Try appVariableName
    if (!appVariableName.IsEmpty())
    {
        str = wxGetenv(appVariableName);
        if (!str.IsEmpty())
            return str;
    }

    if (wxIsAbsolutePath(argv0))
        return wxPathOnly(argv0);
    else
    {
        // Is it a relative path?
        wxString currentDir(cwd);
        if (currentDir.Last() != wxFILE_SEP_PATH)
            currentDir += wxFILE_SEP_PATH;

        str = currentDir + argv0;
        if (wxFileExists(str))
            return wxPathOnly(str);
    }

    // OK, it's neither an absolute path nor a relative path.
    // Search PATH.

    wxPathList pathList;
    pathList.AddEnvList(wxT("PATH"));
    str = pathList.FindAbsoluteValidPath(argv0);
    if (!str.IsEmpty())
        return wxPathOnly(str);

    // Failed
    return wxEmptyString;
}

// Make a path name with no separators, out of a full pathname,
// e.g. opt_ecos_ecos-1.4.5 out of /opt/ecos/ecos-1.4.5
wxString ecMakeNameFromPath(const wxString& path)
{
    wxString p(path);
    p.Replace(wxT("/"), wxT("_"));
    p.Replace(wxT("\\"), wxT("_"));
    p.Replace(wxT(":"), wxT("_"));
    return p;
}


// Find the text of the list control item at the given column
wxString wxListCtrlGetItemTextColumn(wxListCtrl& listCtrl, long item, int col)
{
    wxListItem listItem;
    listItem.m_mask = wxLIST_MASK_TEXT;
    listItem.m_itemId = item;
    listItem.m_col = col;

    if (listCtrl.GetItem(listItem))
        return listItem.m_text;
    else
        return wxEmptyString;
}

// Select the given item
void wxListCtrlSelectItem(wxListCtrl& listCtrl, long sel, bool deselectOthers)
{
    long n = listCtrl.GetItemCount();
    long i;
    if (deselectOthers)
    {
        for (i = 0; i < n; i++)
        {
            if (listCtrl.GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
            {
                listCtrl.SetItemState(i, wxLIST_STATE_SELECTED, 0);
            }
        }
    }
    listCtrl.SetItemState(sel, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

// Find the selection
long wxListCtrlGetSelection(wxListCtrl& listCtrl)
{
    long n = listCtrl.GetItemCount();
    long i;
    for (i = 0; i < n; i++)
    {
        if (listCtrl.GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
        {
            return i;
        }
    }
    return -1;
}

// Find which column the cursor is on
int wxListCtrlFindColumn(wxListCtrl& listCtrl, int noCols, int x)
{
    int col = 0;
    
    // Find which column we're on
    int width = 0;
    int i;
    for (i = 0; i < noCols; i++)
    {
        width += listCtrl.GetColumnWidth(i);
        if (x <= width)
        {
            col = i;
            break;
        }
    }
    return col;
}

// Utility function
void wxRefreshControls(wxWindow* win)
{
    wxNode *node = win->GetChildren().First();
    while (node)
    {
        wxWindow* win = (wxWindow*) node->Data();
        win->Refresh();
        node = node->Next();
    }
}

wxOutputStream& operator <<(wxOutputStream& stream, const wxString& s)
{
    stream.Write(s, s.Length());
    return stream;
}

wxOutputStream& operator <<(wxOutputStream& stream, long l)
{
    wxString str;
    str.Printf("%ld", l);
    return stream << str;
}

wxOutputStream& operator <<(wxOutputStream& stream, const char c)
{
    wxString str;
    str.Printf("%c", c);
    return stream << str;
}

/*
 * ecDialog
 * Supports features we want to have for all dialogs in the application.
 * So far, this just allows dialogs to be resizeable under MSW by
 * refreshing the controls in OnSize (otherwise there's a mess)
 */

IMPLEMENT_CLASS(ecDialog, wxDialog)

BEGIN_EVENT_TABLE(ecDialog, wxDialog)
#ifdef __WXMSW__
    EVT_SIZE(ecDialog::OnSize)
#endif
END_EVENT_TABLE()

void ecDialog::OnSize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);

    wxRefreshControls(this);
}

/*
 * Implements saving/loading of window settings - fonts only for now
 */

wxWindowSettingsObject* wxWindowSettings::FindSettings(const wxString& windowName) const
{
    wxNode* node = m_settings.First();
    while (node)
    {
        wxWindowSettingsObject* obj = (wxWindowSettingsObject*) node->Data();
        if (obj->m_windowName.CmpNoCase(windowName) == 0)
            return obj;
        node = node->Next();
    }
    return NULL;
}

bool wxWindowSettings::LoadConfig(wxConfigBase& config)
{
    unsigned int i = 0;
    for (i = 0; i < GetCount(); i++)
    {
        wxWindowSettingsObject* obj = GetNth(i);

        wxString name(obj->m_windowName);
        name.Replace(wxT(" "), wxT(""));

        LoadFont(config, name, obj->m_font);
    }

    return TRUE;
}

bool wxWindowSettings::SaveConfig(wxConfigBase& config)
{
    unsigned int i = 0;
    for (i = 0; i < GetCount(); i++)
    {
        wxWindowSettingsObject* obj = GetNth(i);

        wxString name(obj->m_windowName);
        name.Replace(wxT(" "), wxT(""));

        SaveFont(config, name, obj->m_font);
    }

    return TRUE;
}

// Load and save font descriptions
bool wxWindowSettings::LoadFont(wxConfigBase& config, const wxString& windowName, wxFont& font)
{
    wxString pathBase(wxT("/Fonts/"));
    pathBase += windowName;
    pathBase += wxT("/");

    int pointSize, family, style, weight;
    bool underlined = FALSE;
    wxString faceName;

    if (!config.Read(pathBase + wxT("PointSize"), & pointSize))
        return FALSE;

    if (!config.Read(pathBase + wxT("Family"), & family))
        return FALSE;

    if (!config.Read(pathBase + wxT("Style"), & style))
        return FALSE;

    if (!config.Read(pathBase + wxT("Weight"), & weight))
        return FALSE;

    config.Read(pathBase + wxT("Underlined"), (bool*) & underlined);
    config.Read(pathBase + wxT("FaceName"), & faceName);

    wxFont font1(pointSize, family, style, weight, underlined, faceName);
    font = font1;

    return TRUE;    
}

bool wxWindowSettings::SaveFont(wxConfigBase& config, const wxString& windowName, const wxFont& font)
{
    if (!font.Ok())
        return FALSE;

    wxString pathBase(wxT("/Fonts/"));
    pathBase += windowName;
    pathBase += wxT("/");

    config.Write(pathBase + wxT("PointSize"), (long) font.GetPointSize());
    config.Write(pathBase + wxT("Family"), (long) font.GetFamily());
    config.Write(pathBase + wxT("Style"), (long) font.GetStyle());
    config.Write(pathBase + wxT("Weight"), (long) font.GetWeight());
    config.Write(pathBase + wxT("Underlined"), (long) font.GetUnderlined());
    config.Write(pathBase + wxT("FaceName"), font.GetFaceName());

    return TRUE;    
}

wxFont wxWindowSettings::GetFont(const wxString& name) const
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
        return wxFont();
    else
        return obj->m_font;
}

void wxWindowSettings::SetFont(const wxString& name, const wxFont& font)
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
    {
        obj = new wxWindowSettingsObject(name, NULL) ;
        obj->m_font = font;
        m_settings.Append(obj);
    }
    obj->m_font = font;
}

wxWindow* wxWindowSettings::GetWindow(const wxString& name) const
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
        return NULL;
    if (obj->m_arrWindow.GetCount() > 0)
        return (wxWindow*) obj->m_arrWindow[0];
    else
        return NULL;
}

void wxWindowSettings::SetWindow(const wxString& name, wxWindow* win)
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
    {
        obj = new wxWindowSettingsObject(name, win) ;
        m_settings.Append(obj);
    }
    obj->m_arrWindow.Clear();

    if (win)
        obj->m_arrWindow.Add(win);
}

wxArrayPtrVoid* wxWindowSettings::GetWindows(const wxString& name) const
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
        return NULL;
    return & obj->m_arrWindow ;
}

void wxWindowSettings::SetWindows(const wxString& name, wxArrayPtrVoid& arr)
{
    wxWindowSettingsObject* obj = FindSettings(name);
    if (!obj)
    {
        obj = new wxWindowSettingsObject(name, NULL) ;
        m_settings.Append(obj);
    }
    obj->m_arrWindow.Clear() ;
    obj->m_arrWindow = arr;
}

bool wxWindowSettings::ApplyFontsToWindows()
{
    if (m_useDefaults)
        return FALSE;

    unsigned int i = 0;
    for (i = 0; i < GetCount(); i++)
    {
        wxWindowSettingsObject* obj = GetNth(i);

        unsigned int j = 0;
        for (j = 0; j < obj->m_arrWindow.GetCount(); j++)
        {
            wxWindow* win = (wxWindow*) obj->m_arrWindow[j];
            win->SetFont(obj->m_font);
            win->Refresh();
        }
    }
    return TRUE;
}

#ifdef __WIN32__
// This will be obsolete when we switch to using the version included
// in wxWindows (from wxWin 2.3.1 onwards)
enum ecKillError
{
    ecKILL_OK,              // no error
    ecKILL_BAD_SIGNAL,      // no such signal
    ecKILL_ACCESS_DENIED,   // permission denied
    ecKILL_NO_PROCESS,      // no such process
    ecKILL_ERROR            // another, unspecified error
};
#endif

// ----------------------------------------------------------------------------
// process management
// ----------------------------------------------------------------------------

#ifdef __WIN32__

// structure used to pass parameters from wxKill() to wxEnumFindByPidProc()
struct wxNewFindByPidParams
{
    wxNewFindByPidParams() { hwnd = 0; pid = 0; }

    // the HWND used to return the result
    HWND hwnd;

    // the PID we're looking from
    DWORD pid;
};

// wxKill helper: EnumWindows() callback which is used to find the first (top
// level) window belonging to the given process
static BOOL CALLBACK wxEnumFindByPidProc(HWND hwnd, LPARAM lParam)
{
    DWORD pid;
    (void)::GetWindowThreadProcessId(hwnd, &pid);

    wxNewFindByPidParams *params = (wxNewFindByPidParams *)lParam;
    if ( pid == params->pid )
    {
        // remember the window we found
        params->hwnd = hwnd;

        // return FALSE to stop the enumeration
        return FALSE;
    }

    // continue enumeration
    return TRUE;
}

// This will be obsolete when we switch to using the version included
// in wxWindows (from wxWin 2.3.1 onwards)
int wxNewKill(long pid, wxSignal sig, ecKillError *krc = NULL)
{
#ifdef __WIN32__
    // get the process handle to operate on
    HANDLE hProcess = ::OpenProcess(SYNCHRONIZE |
                                    PROCESS_TERMINATE |
                                    PROCESS_QUERY_INFORMATION,
                                    FALSE, // not inheritable
                                    (DWORD)pid);
    if ( hProcess == NULL )
    {
        if ( krc )
        {
            if ( ::GetLastError() == ERROR_ACCESS_DENIED )
            {
                *krc = ecKILL_ACCESS_DENIED;
            }
            else
            {
                *krc = ecKILL_NO_PROCESS;
            }
        }

        return -1;
    }

    bool ok = TRUE;
    switch ( sig )
    {
        case wxSIGKILL:
            // kill the process forcefully returning -1 as error code
            if ( !::TerminateProcess(hProcess, (UINT)-1) )
            {
                wxLogSysError(_("Failed to kill process %d"), pid);

                if ( krc )
                {
                    // this is not supposed to happen if we could open the
                    // process
                    *krc = ecKILL_ERROR;
                }

                ok = FALSE;
            }
            break;

        case wxSIGNONE:
            // do nothing, we just want to test for process existence
            break;

        default:
            // any other signal means "terminate"
            {
                wxNewFindByPidParams params;
                params.pid = (DWORD)pid;

                // EnumWindows() has nice semantics: it returns 0 if it found
                // something or if an error occured and non zero if it
                // enumerated all the window
                if ( !::EnumWindows(wxEnumFindByPidProc, (LPARAM)&params) )
                {
                    // did we find any window?
                    if ( params.hwnd )
                    {
                        // tell the app to close
                        //
                        // NB: this is the harshest way, the app won't have
                        //     opportunity to save any files, for example, but
                        //     this is probably what we want here. If not we
                        //     can also use SendMesageTimeout(WM_CLOSE)
                        if ( !::PostMessage(params.hwnd, WM_QUIT, 0, 0) )
                        {
                            wxLogLastError(_T("PostMessage(WM_QUIT)"));
                        }
                    }
                    else // it was an error then
                    {
                        wxLogLastError(_T("EnumWindows"));

                        ok = FALSE;
                    }
                }
                else // no windows for this PID
                {
                    if ( krc )
                    {
                        *krc = ecKILL_ERROR;
                    }

                    ok = FALSE;
                }
            }
    }

    // the return code
    DWORD rc;

    if ( ok )
    {
        // as we wait for a short time, we can use just WaitForSingleObject()
        // and not MsgWaitForMultipleObjects()
        switch ( ::WaitForSingleObject(hProcess, 500 /* msec */) )
        {
            case WAIT_OBJECT_0:
                // process terminated
                if ( !::GetExitCodeProcess(hProcess, &rc) )
                {
                    wxLogLastError(_T("GetExitCodeProcess"));
                }
                break;

            default:
                wxFAIL_MSG( _T("unexpected WaitForSingleObject() return") );
                // fall through

            case WAIT_FAILED:
                wxLogLastError(_T("WaitForSingleObject"));
                // fall through

            case WAIT_TIMEOUT:
                if ( krc )
                {
                    *krc = ecKILL_ERROR;
                }

                rc = STILL_ACTIVE;
                break;
        }
    }
    else // !ok
    {
        // just to suppress the warnings about uninitialized variable
        rc = 0;
    }

    ::CloseHandle(hProcess);

    // the return code is the same as from Unix kill(): 0 if killed
    // successfully or -1 on error
    if ( sig == wxSIGNONE )
    {
        if ( ok && rc == STILL_ACTIVE )
        {
            // there is such process => success
            return 0;
        }
    }
    else // not SIGNONE
    {
        if ( ok && rc != STILL_ACTIVE )
        {
            // killed => success
            return 0;
        }
    }
#else // Win15
    wxFAIL_MSG( _T("not implemented") );
#endif // Win32/Win16

    // error
    return -1;
}
#endif

int ecKill(long pid, wxSignal sig)
{
#if defined(__UNIX__) && !defined(__CYGWIN__)
    return wxKill(pid, sig);
#elif defined(__WXMSW__)
    return wxNewKill(pid, sig);
#else
    return -1;
#endif
}

#ifdef _WIN32
  #include <tlhelp32.h>

  WXHINSTANCE wxProcessKiller::hInstLib1 = VER_PLATFORM_WIN32_NT==wxProcessKiller::GetPlatform()? (WXHINSTANCE) LoadLibrary(_T("PSAPI.DLL")) : (WXHINSTANCE) LoadLibrary(_T("Kernel32.DLL")) ;
  WXHINSTANCE wxProcessKiller::hInstLib2 = VER_PLATFORM_WIN32_NT==wxProcessKiller::GetPlatform()? (WXHINSTANCE) LoadLibrary(_T("NTDLL.DLL")): (WXHINSTANCE) NULL;

#endif

const unsigned int wxProcessKiller::PROCESS_KILL_EXIT_CODE=0xCCFFCCFF;

wxProcessKiller::wxProcessKiller(int pid):
  m_bVerbose(false),
  m_nExitCode(-1),
  m_idProcess(pid)
{
#ifdef _WIN32
      m_hProcess = (WXHANDLE) ::OpenProcess(SYNCHRONIZE |
          PROCESS_TERMINATE |
          PROCESS_QUERY_INFORMATION,
          FALSE, // not inheritable
          (DWORD) m_idProcess);
#endif
}

wxProcessKiller::~wxProcessKiller()
{
#ifdef _WIN32
    if (m_hProcess)
    {
        ::CloseHandle((HANDLE) m_hProcess);
    }
#endif
}

bool wxProcessKiller::Kill(bool bRecurse)
{
    wxPInfoArray arPinfo;
    bool rc=false;
    if(m_idProcess && -1!=m_idProcess){
        // Start of with the easy one:
        if(bRecurse) {
            // Need to gather this information before we orphan our grandchildren:
            PSExtract(arPinfo);
        }
        
#ifdef _WIN32
        
        if(m_hProcess){
            rc=(TRUE==::TerminateProcess((HANDLE) m_hProcess,PROCESS_KILL_EXIT_CODE));
            // dtor's (or subsequent Run's) responsibility to close the handle
        }
        
#else
        rc=(0==kill(m_idProcess,SIGTERM));
        int status;
        waitpid(m_idProcess,&status,WNOHANG);
#endif
        
        if(bRecurse) {
            // kill process *and* its children
            // FIXME: needs to be top-down
            for(int i=0;i<(signed)arPinfo.size();i++){
                if(arPinfo[i].IsChildOf(m_idProcess)){
                    
#ifdef _WIN32
                    // begin hack
                    // WHY NECESSARY??
                    const wxString strName(Name(arPinfo[i].PID));
                    if(_tcsstr(strName,_T("eCosTest")) || _tcsstr(strName,_T("cmd.EXE")) || _tcsstr(strName,_T("CMD.EXE")) || arPinfo[i].PID==(signed)GetCurrentProcessId()){
                        continue;
                    }
                    // end hack
                    HANDLE hProcess=::OpenProcess(PROCESS_TERMINATE,false,arPinfo[i].PID);
                    if(hProcess){
                        rc&=(TRUE==::TerminateProcess(hProcess,PROCESS_KILL_EXIT_CODE));
                        CloseHandle(hProcess);
                    } else {
                        rc=false;
                    }
#else
                    rc&=(0==kill(arPinfo[i].PID,SIGTERM));
                    int status;
                    waitpid(arPinfo[i].PID,&status,WNOHANG);
#endif
                }
            }
        }
    }
    return rc;
}

#ifdef _WIN32
bool wxProcessKiller::PSExtract(wxProcessKiller::wxPInfoArray &arPinfo)
{
    bool rc=false;
    arPinfo.clear();
    // If Windows NT:
    switch(GetPlatform()) {
    case VER_PLATFORM_WIN32_NT:
        if(hInstLib1) {
            
            // Get procedure addresses.
            static BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * ) = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))GetProcAddress( (HINSTANCE) hInstLib1, "EnumProcesses" ) ;
            if( lpfEnumProcesses) {
                
                if(hInstLib2) {
                    
                    static DWORD (WINAPI *lpfNtQueryInformationProcess)( HANDLE, int, void *, DWORD, LPDWORD ) =
                        (DWORD(WINAPI *)(HANDLE, int, void *, DWORD, LPDWORD)) GetProcAddress( (HINSTANCE) hInstLib2,"NtQueryInformationProcess" ) ;
                    
                    if(lpfNtQueryInformationProcess){
                        DWORD dwMaxPids=256;
                        DWORD dwPidSize;
                        DWORD *arPids = NULL ;
                        do {
                            delete [] arPids;
                            arPids=new DWORD[dwMaxPids];
                        } while(lpfEnumProcesses(arPids, dwMaxPids, &dwPidSize) && dwPidSize/sizeof(DWORD)==dwMaxPids) ;
                        
                        if(dwPidSize/sizeof(DWORD)<dwMaxPids){
                            rc=true;
                            for( DWORD dwIndex = 0 ; (signed)dwIndex < dwPidSize/sizeof(DWORD); dwIndex++ ) {
                                // Regardless of OpenProcess success or failure, we
                                // still call the enum func with the ProcID.
                                DWORD pid=arPids[dwIndex];
                                HANDLE hProcess=::OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, pid ); 
                                if (hProcess ) {
                                    struct {
                                        DWORD ExitStatus; // receives process termination status
                                        DWORD PebBaseAddress; // receives process environment block address
                                        DWORD AffinityMask; // receives process affinity mask
                                        DWORD BasePriority; // receives process priority class
                                        ULONG UniqueProcessId; // receives process identifier
                                        ULONG InheritedFromUniqueProcessId; // receives parent process identifier
                                    } pbi;
                                    memset( &pbi, 0, sizeof(pbi)); 
                                    DWORD retLen; 
                                    __int64 ftCreation,ftExit,ftKernel,ftUser;
                                    if(lpfNtQueryInformationProcess(hProcess, 0 /*ProcessBasicInformation*/, &pbi, sizeof(pbi), &retLen)>=0 &&
                                        TRUE==::GetProcessTimes (hProcess,(FILETIME *)&ftCreation,(FILETIME *)&ftExit,(FILETIME *)&ftKernel,(FILETIME *)&ftUser)){
                                        // The second test is important.  It excludes orphaned processes who appear to have been adopted by virtue of a new
                                        // process having been created with the same ID as their original parent.
                                        wxPInfo p;
                                        p.PID=pid;
                                        p.PPID=pbi.InheritedFromUniqueProcessId;
                                        p.tCreation=ftCreation;
                                        p.tCpu=Time((ftKernel+ftUser)/10000);
                                        arPinfo.push_back(p);
                                    }
                                    
                                    CloseHandle(hProcess); 
                                    
                                }
                            }
                        }
                        delete [] arPids;
                    }          
                }
            }      
        }
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        
        if( hInstLib1) {
            
            static HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD)=
                (HANDLE(WINAPI *)(DWORD,DWORD))GetProcAddress( (HINSTANCE) hInstLib1,"CreateToolhelp32Snapshot" ) ;
            static BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32)=
                (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))GetProcAddress( (HINSTANCE) hInstLib1, "Process32First" ) ;
            static BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32)=
                (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))GetProcAddress( (HINSTANCE) hInstLib1, "Process32Next" ) ;
            if( lpfProcess32Next && lpfProcess32First && lpfCreateToolhelp32Snapshot) {
                
                // Get a handle to a Toolhelp snapshot of the systems
                // processes.
                HANDLE hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
                if(INVALID_HANDLE_VALUE != hSnapShot) {
                    // Get the first process' information.
                    PROCESSENTRY32 procentry;
                    procentry.dwSize = sizeof(PROCESSENTRY32) ;
                    if(lpfProcess32First( hSnapShot, &procentry )){
                        rc=true;
                        do {
                            wxPInfo p;
                            p.PID=procentry.th32ProcessID;
                            p.PPID=procentry.th32ParentProcessID;
                            arPinfo.push_back(p);
                        } while(lpfProcess32Next( hSnapShot, &procentry ));
                    }
                    CloseHandle(hSnapShot);
                }
            }
        }
        break;
    default:
        break;
    }    
    
    SetParents(arPinfo);
    
    if(!rc){
        wxLogError(_T("Couldn't get process information!\n"));
    }
    return rc;
}

#else // UNIX

bool wxProcessKiller::PSExtract(wxProcessKiller::wxPInfoArray &arPinfo)
{
    arPinfo.clear();
    int i;
    FILE *f=popen("ps -l",_T("r") MODE_TEXT);
    if(f){
        char buf[100];
        while(fgets(buf,sizeof(buf)-1,f)){
            TCHAR discard[100];
            wxPInfo p;
            // Output is in the form
            //  F S   UID   PID  PPID  C PRI  NI ADDR    SZ WCHAN  TTY          TIME CMD
            //100 S   490   877   876  0  70   0    -   368 wait4  pts/0    00:00:00 bash
            int F,UID,C,PRI,NI,SZ,HH,MM,SS; 
            bool rc=(15==_stscanf(buf,_T("%d %s %d %d %d %d %d %d %s %d %s %s %d:%d:%d"),&F,discard,&UID,&p.PID,&p.PPID,&C,&PRI,&NI,discard,&SZ,discard,discard,&HH,&MM,&SS));
            if(rc){
                p.tCpu=1000*(SS+60*(60*HH+MM));
                arPinfo.push_back(p);
            }
        }
        pclose(f);
        for(i=0;i<(signed)arPinfo.size();i++){
            int pid=arPinfo[i].PPID;
            arPinfo[i].pParent=0;
            for(int j=0;j<(signed)arPinfo.size();j++){
                if(i!=j && arPinfo[j].PID==pid){
                    arPinfo[i].pParent=&arPinfo[j];
                    break;
                }
            }
        }
    } else {
        wxLogError(_T("Failed to run ps -l\n"));
    }
    return true; //FIXME
}

#endif

void wxProcessKiller::SetParents(wxProcessKiller::wxPInfoArray &arPinfo)
{
    int i;
    for(i=0;i<(signed)arPinfo.size();i++){
        wxPInfo &p=arPinfo[i];
        p.pParent=0;
        for(int j=0;j<(signed)arPinfo.size();j++){
            if(arPinfo[j].PID==p.PPID 
#ifdef _WIN32
                && arPinfo[j].tCreation<p.tCreation
#endif
                )
            {
                arPinfo[i].pParent=&arPinfo[j];
                break;
            }
        }
    }
    
    // Check for circularity
    bool bCircularity=false;
    for(i=0;i<(signed)arPinfo.size();i++){
        wxPInfo *p=&arPinfo[i];
        for(int j=0;j<(signed)arPinfo.size() && p;j++){
            p=p->pParent;
        }
        // If all is well, p should be NULL here.  Otherwise we have a loop.
        if(p){
            // Make sure it can't foul things up:
            arPinfo[i].pParent=0;
            bCircularity=true;
        }
    }
    
    if(bCircularity){
        wxLogError(_T("!!! Circularly linked process list at index %d\n"),i);
        for(int k=0;k<(signed)arPinfo.size();k++){
            const wxPInfo &p=arPinfo[k];
            wxLogError(_T("%d: %s ppid=%4d\n"),k,(LPCTSTR)Name(p.PID),p.PPID);
        }
    }
}

bool wxProcessKiller::wxPInfo::IsChildOf(int pid) const
{
    for(wxPInfo *p=pParent;p && p!=this;p=p->pParent) { // guard against circular linkage
        if(p->PID==pid){
            return true;
        }
    }
    return false;
}

const wxString wxProcessKiller::Name(int pid)
{
    wxString str;
    str.Printf(_T("id=%d"),pid);
#ifdef _DEBUG
#ifdef _WIN32
    if(VER_PLATFORM_WIN32_NT==GetPlatform() && hInstLib1){
        static BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *, DWORD, LPDWORD ) =
            (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD)) GetProcAddress( (HINSTANCE) hInstLib1,"EnumProcessModules" ) ;
        static DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD )=
            (DWORD (WINAPI *)(HANDLE, HMODULE,LPTSTR, DWORD )) GetProcAddress( (HINSTANCE) hInstLib1,"GetModuleFileNameExA" ) ;
        if( lpfEnumProcessModules &&  lpfGetModuleFileNameEx ) {
            HANDLE hProcess=::OpenProcess(PROCESS_ALL_ACCESS,false,pid);
            if(hProcess) {
                HMODULE hMod;
                DWORD dwSize;
                if(lpfEnumProcessModules( hProcess, &hMod, sizeof(HMODULE), &dwSize ) ){
                    // Get Full pathname:
                    TCHAR buf[1+MAX_PATH];
                    lpfGetModuleFileNameEx( hProcess, hMod, buf, MAX_PATH);
                    str+=_TCHAR(' ');
                    str+=buf;
                }
                CloseHandle(hProcess);
            }
        }
    }
#endif
#endif
    return str;
}

#ifdef _WIN32
long wxProcessKiller::GetPlatform()
{
    OSVERSIONINFO  osver;
    osver.dwOSVersionInfoSize = sizeof( osver ) ;
    return GetVersionEx( &osver ) ? (long) osver.dwPlatformId : (long)-1;
}
#endif

const wxString wxProcessKiller::ErrorString() const
{
#ifdef _WIN32
    TCHAR *pszMsg;
    FormatMessage(  
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        m_nErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&pszMsg,
        0,
        NULL 
        );
    return pszMsg;
#else 
    return strerror(errno);
#endif
}

