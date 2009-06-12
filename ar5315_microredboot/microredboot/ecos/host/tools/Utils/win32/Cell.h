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
#if !defined(AFX_CELLEDIT_H__B7D5072A_9B59_11D3_A537_00A0C949ADAC__INCLUDED_)
#define AFX_CELLEDIT_H__B7D5072A_9B59_11D3_A537_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Cell.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCell window
enum {WM_CANCEL_EDIT=WM_USER+99}; // wParam=1 for end, 0 for cancel
class CCell : public CWnd
{
// Construction
public:
	CCell(LPCTSTR pszInitialValue);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCell)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCell();
	virtual void OnUpdateEditCopy   (CCmdUI*){}
	virtual void OnUpdateEditDelete (CCmdUI*){}
	virtual void OnUpdateEditPaste  (CCmdUI*){}
	virtual void OnUpdateEditCut    (CCmdUI*){}
	virtual void OnEditCopy   (){}
	virtual void OnEditDelete (){}
	virtual void OnEditPaste  (){}
	virtual void OnEditCut    (){}
	// Generated message map functions
protected:
  CString m_strInitialValue;
	//{{AFX_MSG(CCell)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CELLEDIT_H__B7D5072A_9B59_11D3_A537_00A0C949ADAC__INCLUDED_)
