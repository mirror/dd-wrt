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
// RulesView.h : interface of the CRulesView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_RULESVIEW_H__9D3E02FA_8D4E_11D3_A535_00A0C949ADAC__INCLUDED_)
#define AFX_RULESVIEW_H__9D3E02FA_8D4E_11D3_A535_00A0C949ADAC__INCLUDED_

#include "RulesList.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CRulesView : public CView
{
protected: // create from serialization only
	CRulesView();
	DECLARE_DYNCREATE(CRulesView)

// Attributes
public:

// Operations
public:
  void FillRules();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulesView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRulesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int m_nContextItem;

// Generated message map functions
protected:
	int m_nContextRow;
	CRulesList m_List;
	//{{AFX_MSG(CRulesView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnDisable();
  afx_msg void OnEnable();
  afx_msg void OnLocate();
  afx_msg void OnResolve();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RULESVIEW_H__9D3E02FA_8D4E_11D3_A535_00A0C949ADAC__INCLUDED_)
