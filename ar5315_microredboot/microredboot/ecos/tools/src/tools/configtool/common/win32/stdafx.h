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
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#ifndef _ECOS_STDAFX_H
#define _ECOS_STDAFX_H

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <windowsx.h> // for GET_X_LPARAM, GET_Y_LPARAM

#pragma warning (disable: 4290) // C++ Exception Specification ignored
#pragma warning (disable: 4786) // long debug symbols
#pragma warning (disable: 4800) // 'int' : forcing value to bool 'true' or 'false' (performance warning)

#ifdef PLUGIN
typedef CMenu Menu;
#else
class BCMenu;
typedef BCMenu Menu;
#endif

enum BrowserType { Internal, AssociatedExternal, CustomExternal };
enum WhereType {InMacro, InName, InDesc, InCurrentValue, InDefaultValue};
enum RuleViewType {ViewFailing,ViewAll,ViewNone};
typedef __int64 ItemIntegerType;

// "Good practice" delete - sets argument to NULL
#define deleteZ(x)  {delete x;x=0;}
#define deleteZA(x) {delete [] x;x=0;}

#undef TRACE // previous definition in mfc\include\afx.h
#define TRACE ::__Trace
extern void __Trace (LPCTSTR pszFormat,...);


#endif
