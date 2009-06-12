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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/09/11
// Version:		0.01
// Purpose:		Class to encapsulate filename operations (i.e. broadly on a file which probably do not involve opening
//              it).  Importantly, the + and += operators performs filename segment addition, making sure that only one '\\'
//				comes between each piece.
// Description:	Interface of a the filename class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _FILENAME_H
#define _FILENAME_H
#include <afxtempl.h>
class CFileName;
typedef CArray<CFileName,CFileName&> CFileNameArray;
class CFileName : public CString {
public:
	void ReplaceExtension (LPCTSTR pszNewExt);
  
  // previous directory is returned:
  static CFileName SetCurrentDirectory (LPCTSTR pszDir);
  const CString Root() const;
  const CString Extension() const;
  static CFileName GetTempPath();
  bool IsAbsolute() const;
  
  static const TCHAR cSep;        // The path separator ('\' on windows)
  
  // Standard ctors
  CFileName():CString(){}
  CFileName(const CFileName& stringSrc):CString(stringSrc){Normalize();}
  CFileName(const CString& stringSrc):CString(stringSrc){Normalize();}
  CFileName(TCHAR ch, int nRepeat = 1):CString(ch,nRepeat){Normalize();}
  CFileName(LPCSTR lpsz):CString(lpsz){Normalize();}
  CFileName(LPCWSTR lpsz):CString(lpsz){Normalize();}
  CFileName(LPCSTR lpch, int nLength):CString(lpch,nLength){Normalize();}
  CFileName(LPCWSTR lpch, int nLength):CString(lpch,nLength){Normalize();}
  CFileName(const unsigned char* psz):CString(psz){Normalize();}
  
  // Construct from path fragments
  CFileName(LPCTSTR,LPCTSTR);
  CFileName(LPCTSTR,LPCTSTR,LPCTSTR);
  CFileName(LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR);
  CFileName(LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR);
  
  // catenation operators: exactly one separator is placed between L and R
  const CFileName& operator+=(const CFileName& string);
  const CFileName& operator+=(TCHAR ch);
#ifdef _UNICODE
  const CFileName& operator+=(char ch);
#endif
  const CFileName& operator+=(LPCTSTR lpsz);
  friend CFileName AFXAPI operator+(const CFileName& string1,const CFileName& string2);
  friend CFileName AFXAPI operator+(const CFileName& string, TCHAR ch);
  friend CFileName AFXAPI operator+(TCHAR ch, const CFileName& string);
#ifdef _UNICODE
  friend CFileName AFXAPI operator+(const CFileName& string, char ch);
  friend CFileName AFXAPI operator+(char ch, const CFileName& string);
#endif
  friend CFileName AFXAPI operator+(const CFileName& string, LPCTSTR lpsz);
  friend CFileName AFXAPI operator+(LPCTSTR lpsz, const CFileName& string);
  
  // Textual append - no separator functionality
  const CFileName& Append (TCHAR ch);
  const CFileName& Append (LPCTSTR psz);
  
  // Utility functions
  const CFileName FullName() const;   // full path name
  const CFileName NoSpaceName() const;// sans spaces
  const CFileName ShortName() const;	// the type with ~s in it
  const CFileName Tail() const;       // file name sans directory part
  const CFileName Head() const;       // directory part
  const CFileName CygPath() const;    // path mangled for CygWin
  
  static CFileName GetCurrentDirectory();
  const CFileName& ExpandEnvironmentStrings();
  static CFileName ExpandEnvironmentStrings(LPCTSTR psz);
  
  // Form path name relative to given parameter (if NULL, current directory)
  const CFileName& MakeRelative(LPCTSTR pszRelativeTo=0);
  static CFileName Relative(LPCTSTR psz,LPCTSTR pszRelativeTo=0);
  
  bool SameFile (const CFileName &strOther) const;
  bool SetFileAttributes (DWORD dwFileAttributes) const;
  
  DWORD Attributes() const { return ::GetFileAttributes(*this); }
  
  bool Exists     () const {return 0xFFFFFFFF!=Attributes();}
  bool IsDir      () const {	DWORD a=Attributes(); return 0xFFFFFFFF!=a && (0!=(a&FILE_ATTRIBUTE_DIRECTORY));}
  bool IsFile     () const {	DWORD a=Attributes(); return 0xFFFFFFFF!=a && (0==(a&FILE_ATTRIBUTE_DIRECTORY));}
  bool IsReadOnly () const {	DWORD a=Attributes(); return 0xFFFFFFFF!=a && (0!=(a&FILE_ATTRIBUTE_READONLY ));}
  FILETIME LastModificationTime() const;
  
  bool RecursivelyDelete(); 
  
  bool CreateDirectory (bool bParentsToo=true,bool bFailIfAlreadyExists=false) const;
  
  static int FindFiles (LPCTSTR pszDir,CFileNameArray &ar,LPCTSTR pszPattern=_T("*.*"),bool bRecurse=true,DWORD dwExclude=FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN);
    
protected:
  // Helpers for Relative():
  LPCTSTR *Chop ();
  static CFileName Drive(LPCTSTR psz);
  
  // Remove trailing '/' (helper for ctors)
  void Normalize();
  
  // Implementating catenation functionality:
  void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
  void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,int nSrc2Len, LPCTSTR lpszSrc2Data);
};

class CSaveExcursion {
  const CFileName m_strPrevDir;
public:
  CSaveExcursion(LPCTSTR pszDir) : m_strPrevDir(CFileName::SetCurrentDirectory(pszDir)) {}
  ~CSaveExcursion() { ::SetCurrentDirectory(m_strPrevDir); }
  bool Ok() const { return !m_strPrevDir.IsEmpty(); }
};

#endif	
