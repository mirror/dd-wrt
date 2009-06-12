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
// DoubleEdit.h: interface for the CDoubleEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOUBLEEDIT_H__A38F0509_4EFF_11D3_8003_00A0C9554250__INCLUDED_)
#define AFX_DOUBLEEDIT_H__A38F0509_4EFF_11D3_8003_00A0C9554250__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CellEdit.h"
class CConfigItem;

class CDoubleEdit : public CCellEdit  
{
public:
	CDoubleEdit(double dInitialValue);
	virtual ~CDoubleEdit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDoubleEdit)
	public:
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	CString TrimRightNoCase (const CString & strInput, LPCTSTR lpszTrimChars);
	CString m_strPrevText;

	//{{AFX_MSG(CDoubleEdit)
	afx_msg void OnUpdate();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DOUBLEEDIT_H__A38F0509_4EFF_11D3_8003_00A0C9554250__INCLUDED_)
