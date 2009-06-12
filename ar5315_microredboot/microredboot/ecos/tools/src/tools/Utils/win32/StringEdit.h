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
// Description:	Interface of the masked edit control used by the control view for
//				in-cell editing
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_STRINGEDIT_H__8F19DC24_06B9_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_STRINGEDIT_H__8F19DC24_06B9_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// IntegerEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringEdit window
#include "CellEdit.h"
class CStringEdit : public CCellEdit
{
// Construction
public:
	CStringEdit(LPCTSTR pszInitialValue);

// Attributes
public:
	static LPCTSTR FORMAT_HEX;
	static LPCTSTR FORMAT_INT;
// Operations
public:
  void EnableDoubleClickEdit(bool bEnable=true,UINT idEdit=42) { m_bDoubleClickEdit=bEnable; m_idEdit=idEdit; }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringEdit)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
  void SetValue(__int64 n);
	virtual ~CStringEdit();

	// Generated message map functions
protected:
	bool m_bDoubleClickEdit;
  UINT m_idEdit;

	//CIdleMessage CI;
	CString m_strData;
	//{{AFX_MSG(CStringEdit)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGEDIT_H__8F19DC24_06B9_11D2_80BE_00A0C949ADAC__INCLUDED_)
