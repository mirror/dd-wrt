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
#if !defined(AFX_REGKEYEX_H__2E80C136_9E6E_11D3_A538_00A0C949ADAC__INCLUDED_)
#define AFX_REGKEYEX_H__2E80C136_9E6E_11D3_A538_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegKeyEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegKeyEx window
#include <atlbase.h>

class CRegKeyEx : public CRegKey
{
// Construction
public:
  CRegKeyEx():CRegKey(){}
	CRegKeyEx(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired = KEY_ALL_ACCESS);
	virtual ~CRegKeyEx();

  class Value {
  public:
    
    Value ();
    //Value (__int64 i64);
    Value (LPCTSTR psz);
    Value (DWORD dw);
    virtual ~Value();

    DWORD Type() const { return m_dwType; }
  protected:
    DWORD m_dwType;
    union {
      LPTSTR  m_psz;
      DWORD   m_dw;
    };
  };
// Attributes
public:

// Operations
public:

// Implementation
public:
	bool QueryValue (LPCTSTR pszValueName,CString &str);
  bool QueryValue(int nIndex,CString &strName,CString &strValue);
  bool QueryValue (LPCTSTR pszValueName,DWORD &dw);
  bool QueryValue(int nIndex,CString &strName,DWORD &dwValue);

  bool QueryKey(int nIndex,CString &strName);
  
  Value QueryValue (LPCTSTR pszValueName);
	Value QueryValue (int nIndex,CString &strName);

	// Generated message map functions
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_REGKEYEX_H__2E80C136_9E6E_11D3_A538_00A0C949ADAC__INCLUDED_)
