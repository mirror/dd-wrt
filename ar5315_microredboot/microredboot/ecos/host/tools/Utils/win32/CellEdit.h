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
#if !defined(AFX_CELL_H__B7D5072E_9B59_11D3_A537_00A0C949ADAC__INCLUDED_)
#define AFX_CELL_H__B7D5072E_9B59_11D3_A537_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CellEdit.h : header file
//
#include "Cell.h"
/////////////////////////////////////////////////////////////////////////////
// CCellEdit window

class CCellEdit : public CCell
{
// Construction
public:
	CCellEdit(LPCTSTR pszInitialValue);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCellEdit)
	public:
  virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnEditCopy   ();
	virtual void OnEditDelete ();
	virtual void OnEditPaste  ();
	virtual void OnEditCut    ();
	virtual void OnUpdateEditCopy   (CCmdUI *pCmdUI);
	virtual void OnUpdateEditDelete (CCmdUI *pCmdUI);
	virtual void OnUpdateEditPaste  (CCmdUI *pCmdUI);
	virtual void OnUpdateEditCut    (CCmdUI *pCmdUI);

	virtual ~CCellEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCellEdit)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CELL_H__B7D5072E_9B59_11D3_A537_00A0C949ADAC__INCLUDED_)
