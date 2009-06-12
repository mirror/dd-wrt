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
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Interface to various global utility functions.  Everything in this
//				class is static (there are no instances).
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//  
//===========================================================================
#ifndef _UTILS_H
#define _UTILS_H
#include "FileName.h"
#define INCLUDEFILE <string>
#include "IncludeSTL.h"
#include <stdarg.h>     // vsnprintf

class CUtils {
public:
  static bool Launch (const CFileName &strFileName,const CFileName &strViewer);
  static CString GetLastErrorMessageString ();

  static bool AddToPath (const CFileName &strFolder, bool bAtFront=true);
  static const CString LoadString (UINT id);
  
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

  // String functions
  
  // Chop the string into pieces using separator cSep.
  // The Boolean controls whether " and \ make a difference
  static int Chop(LPCTSTR psz,CStringArray &ar,TCHAR cSep=_TCHAR(' '),bool bObserveStrings=false,bool bBackslashQuotes=false);
  static int Chop(LPCTSTR psz,CStringArray &ar,LPCTSTR pszSep,        bool bObserveStrings=false,bool bBackslashQuotes=false);
  // String -> Integer, observing the current hex/decimal setting
  static BOOL StrToItemIntegerType(const CString &str,__int64 &d);
  static BOOL StrToDouble (const CString &strValue, double &dValue);
  // Integer -> String, observing the current hex/decimal setting
  static const CString IntToStr(__int64 d,bool bHex=false);
  static const CString DoubleToStr (double dValue);
  static CString StripExtraWhitespace (const CString & strInput);
  
  // Provide a failure explanation for what just went wrong
  static const CString Explanation (CFileException &exc);
  static void UnicodeToCStr(LPCTSTR str,char *&psz);
  static std::string UnicodeToStdStr(LPCTSTR str);
  static CFileName WPath(const std::string &str);
  static bool CopyFile (LPCTSTR pszSource,LPCTSTR pszDest);

};

#endif
