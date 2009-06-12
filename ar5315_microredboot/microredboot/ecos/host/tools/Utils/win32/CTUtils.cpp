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
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is a collection of utility functions.
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#include "stdafx.h"
#include "CTUtils.h"

#include <float.h> // for DBL_DIG macro
#include <sys/types.h>
#include <sys/stat.h>

#define INCLUDEFILE <string>
#include "IncludeSTL.h"

// Chop str into pieces, using cSep as separator.
// " and \ have usual semantics
// Return value is array of pieces found.

int CUtils::Chop(LPCTSTR psz,CStringArray &ar,TCHAR cSep,bool bObserveStrings/*=false*/,bool bBackslashQuotes/*=false*/)
{
  if(_TCHAR(' ')==cSep){
    return Chop(psz,ar,_T("\t\n\v\f\r "),bObserveStrings,bBackslashQuotes);
  } else {
    ASSERT(_TCHAR('\0')!=cSep);
    TCHAR c[2]={cSep,_TCHAR('\0')};
    return Chop(psz,ar,c,bObserveStrings,bBackslashQuotes);
  }
}

int CUtils::Chop(LPCTSTR psz,CStringArray &ar,LPCTSTR pszSep,bool bObserveStrings/*=false*/,bool bBackslashQuotes/*=false*/)
{
	ar.RemoveAll();
	int i=0;
	for(;;){
		// Skip multiple separators
    while(*psz&&_tcschr(pszSep,*psz)){
      psz++;
    }
		if(!*psz){
			return i;
		}
		CString strTok;
		if(bObserveStrings){
			BOOL bInString=FALSE;
			do{
				if(*psz==_TCHAR('\\') && bBackslashQuotes && psz[1]){
					strTok+=psz[1];
					psz++;
				} else if(*psz==_TCHAR('"')){
					bInString ^= 1;
				} else if (!bInString && *psz && NULL!=_tcschr(pszSep,*psz)) {
					break;
				} else {
					strTok+=*psz;
				}
			} while (*++psz);
		} else {
      LPCTSTR pszStart=psz;
			do {
        psz++;
			} while (*psz && !_tcschr(pszSep,*psz));
      strTok=CString(pszStart,psz-pszStart);
		}
		ar.SetAtGrow(i++,strTok);
	}
    return ar.GetSize();
}


// vararg-style message box formatter
int CUtils::MessageBoxF (LPCTSTR pszFormat, ...)
{
  int rc;
  va_list args;
  va_start(args, pszFormat);
  rc=CUtils::vMessageBox(MB_OK, pszFormat,args);
  va_end(args);
  return rc;
}

// As above, but with type as first parameter.
int CUtils::MessageBoxFT (UINT nType, LPCTSTR pszFormat, ...)
{
  int rc;
  va_list args;
  va_start(args, pszFormat);
  rc=CUtils::vMessageBox(nType, pszFormat,args);
  va_end(args);
  return rc;
}

int CUtils::vMessageBox(UINT nType, LPCTSTR  pszFormat, va_list marker)
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


BOOL CUtils::StrToItemIntegerType(const CString & str,__int64 &d)
{
	extern int errno;
	PTCHAR pEnd;
	BOOL rc;
	errno=0;
	BOOL bHex=(str.GetLength()>2 && str[0]==_TCHAR('0') && (str[1]==_TCHAR('x')||str[1]==_TCHAR('X')));
	d=_tcstol(str,&pEnd,bHex?16:10);
	rc=(0==errno && (*pEnd==_TCHAR('\0')));
	return rc;
}

const CString CUtils::IntToStr(__int64 d,bool bHex)
{
  CString s;
  s.Format(bHex?_T("0x%08x"):_T("%d"),d);
  return s;
}

const CString CUtils::DoubleToStr (double dValue)
{
  CString s;
  s.Format (_T("%.*e"), DBL_DIG, dValue);
  return s;
}

BOOL CUtils::StrToDouble (const CString & strValue, double &dValue)
{
	extern int errno;
	PTCHAR pEnd;
	errno = 0;
	dValue = _tcstod (strValue, &pEnd);
	return (0 == errno) && (*pEnd == _TCHAR('\0'));
}

const CString CUtils::Explanation(CFileException & exc)
{
	CString strMsg;
	switch(exc.m_cause){
		case CFileException::none: strMsg=_T("No error occurred.");break;
		case CFileException::generic: strMsg=_T("   An unspecified error occurred.");break;
		case CFileException::fileNotFound: strMsg=_T("   The file could not be located.");break;
		case CFileException::badPath: strMsg=_T("   All or part of the path is invalid.");break;
		case CFileException::tooManyOpenFiles: strMsg=_T("   The permitted number of open files was exceeded.");break;
		case CFileException::accessDenied: strMsg=_T("   The file could not be accessed.");break;
		case CFileException::invalidFile: strMsg=_T("   There was an attempt to use an invalid file handle.");break;
		case CFileException::removeCurrentDir: strMsg=_T("   The current working directory cannot be removed.");break;
		case CFileException::directoryFull: strMsg=_T("   There are no more directory entries.");break;
		case CFileException::badSeek: strMsg=_T("   There was an error trying to set the file pointer.");break;
		case CFileException::hardIO: strMsg=_T("   There was a hardware error.");break;
		case CFileException::sharingViolation: strMsg=_T("   SHARE.EXE was not loaded, or a shared region was locked.");break;
		case CFileException::lockViolation: strMsg=_T("   There was an attempt to lock a region that was already locked.");break;
		case CFileException::diskFull: strMsg=_T("   The disk is full.");break;
		case CFileException::endOfFile: strMsg=_T("   The end of file was reached. ");break;
		default:
			strMsg=_T(" Unknown cause");
			break;
	}
	return strMsg;
}


const CString CUtils::LoadString(UINT id)
{
	CString str;
	str.LoadString(id);
	return str;
}

bool CUtils::AddToPath(const CFileName & strFolder, bool bAtFront)
{
	CString strPath,strOldPath;
	int nSize=GetEnvironmentVariable(_T("PATH"), NULL, 0);
	if(nSize>0){
    GetEnvironmentVariable(_T("PATH"),strOldPath.GetBuffer(1+nSize),nSize);
    strOldPath.ReleaseBuffer();
		// Place the user tools folders at the head or tail of the new path
		if(bAtFront){
			strPath.Format(_T("%s;%s"),strFolder.ShortName(),strOldPath);
		} else {
			strPath.Format(_T("%s;%s"),strOldPath,strFolder.ShortName());
		}
	} else {
		// unlikely, but ...
		strPath=strFolder;
	}
  return TRUE==::SetEnvironmentVariable(_T("PATH"),strPath);
}

		

CString CUtils::GetLastErrorMessageString()
{
	CString str;
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

bool CUtils::Launch(const CFileName &strFileName, const CFileName &strViewer)
{
	bool rc=false;

	if(!strViewer.IsEmpty())//use custom editor
	{
		CString strCmdline(strViewer);
		
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
			MessageBoxF(_T("Failed to invoke %s.\n"),strCmdline);
		}
		strCmdline.ReleaseBuffer();
	} else {// Use association
		TCHAR szExe[MAX_PATH];
		HINSTANCE h=FindExecutable(strFileName,_T("."),szExe);
		if(int(h)<=32){
			CString str;
			switch(int(h)){
				case 0:  str=_T("The system is out of memory or resources.");break;
				case 31: str=_T("There is no association for the specified file type.");break;
				case ERROR_FILE_NOT_FOUND: str=_T("The specified file was not found.");break;
				case ERROR_PATH_NOT_FOUND: str=_T("The specified path was not found.");break;
				case ERROR_BAD_FORMAT:     str=_T("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).");break;
				default: break;
			}
			MessageBoxF(_T("Failed to open document %s.\r\n%s"),strFileName,str);
		} else {

			SHELLEXECUTEINFO sei = {sizeof(sei), 0, AfxGetMainWnd()->GetSafeHwnd(), _T("open"),
					strFileName, NULL, NULL, SW_SHOWNORMAL, AfxGetInstanceHandle( )};

			sei.hInstApp=0;
			HINSTANCE hInst=ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),_T("open"), strFileName, NULL, _T("."), 0)/*ShellExecuteEx(&sei)*/;
			if(int(hInst)<=32/*sei.hInstApp==0*/)
			{
				CString str;
				switch(int(hInst))
				{
					case 0 : str=_T("The operating system is out of memory or resources. ");break;
					case ERROR_FILE_NOT_FOUND : str=_T("The specified file was not found. ");break;
					case ERROR_PATH_NOT_FOUND : str=_T("The specified path was not found. ");break;
					case ERROR_BAD_FORMAT : str=_T("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image). ");break;
					case SE_ERR_ACCESSDENIED : str=_T("The operating system denied access to the specified file. ");break;
					case SE_ERR_ASSOCINCOMPLETE : str=_T("The filename association is incomplete or invalid. ");break;
					case SE_ERR_DDEBUSY : str=_T("The DDE transaction could not be completed because other DDE transactions were being processed. ");break;
					case SE_ERR_DDEFAIL : str=_T("The DDE transaction failed. ");break;
					case SE_ERR_DDETIMEOUT : str=_T("The DDE transaction could not be completed because the request timed out. ");break;
					case SE_ERR_DLLNOTFOUND : str=_T("The specified dynamic-link library was not found. ");break;
					//case SE_ERR_FNF : str=_T("The specified file was not found. ");break;
					case SE_ERR_NOASSOC : str=_T("There is no application associated with the given filename extension. ");break;
					case SE_ERR_OOM : str=_T("There was not enough memory to complete the operation. ");break;
					//case SE_ERR_PNF : str=_T("The specified path was not found. ");break;
					case SE_ERR_SHARE : str=_T("A sharing violation occurred. ");break;
					default: str=_T("An unexpected error occurred");break;
				}
				MessageBoxF(_T("Failed to open document %s using %s.\r\n%s"),strFileName,szExe,str);
			} else {
				rc=true;
			}
		}
	}
	return rc;
}

void CUtils::UnicodeToCStr(LPCTSTR str,char *&psz)
{
    int nLength=1+_tcslen(str);
    psz=new char[nLength];
    #ifdef _UNICODE
    WideCharToMultiByte(CP_ACP, 0, str, -1, psz, nLength, NULL, NULL);
    #else
    strcpy(psz,str);
    #endif
}

std::string CUtils::UnicodeToStdStr(LPCTSTR str)
{
    std::string stdstr;
    char *psz;
    UnicodeToCStr(str,psz);
    stdstr=std::string(psz);
    delete psz;
    return stdstr;
}

// CUtils::StripExtraWhitespace() returns a modified version of
// a string in which each sequence of whitespace characters is
// replaced by a single space

CString CUtils::StripExtraWhitespace (const CString & strInput)
{
  CString strOutput;
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
}

CFileName CUtils::WPath(const std::string &str)
{
  // Convert a path as read from cdl into host format
  // Change / to \ throughout
  CFileName
    strPath(str.c_str());
  strPath.Replace (_TCHAR('/'), _TCHAR('\\'));
  return strPath;
}

// Copy file helper function.
// This makes sure the destination file is only touched as necessary.
// It is written using Posix calls lest it should be more broadly applicable.

bool CUtils::CopyFile(LPCTSTR pszSource,LPCTSTR pszDest)
{
  // Compare the files.  First set rc to the result of the comparison (true if the same)
  bool rc=false;

  struct _stat s1,s2;
  if(-1!=_tstat(pszSource,&s1) && -1!=_tstat(pszDest,&s2) && s1.st_size==s2.st_size){
    // Files both exist and are of equal size
    FILE *f1=_tfopen(pszSource,_T("rb"));
    if(f1){
      FILE *f2=_tfopen(pszDest,_T("rb"));
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
    TRACE(_T("Copy not necessary: '%s' to '%s'\n"),pszSource,pszDest);
  } else {
    rc=TRUE==::CopyFile(pszSource,pszDest,FALSE);
    if(rc){
      TRACE(_T("Copied '%s' to '%s'\n"),pszSource,pszDest);
    } else {
      MessageBoxF(_T("Failed to copy '%s' to '%s' - %s"),pszSource,pszDest,GetLastErrorMessageString());
    }
  }

  return rc;
}
