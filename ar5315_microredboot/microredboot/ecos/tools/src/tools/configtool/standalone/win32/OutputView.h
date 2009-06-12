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
// Description:	Interface of the output window view
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_OUTPUTVIEW_H__B9DE3972_0F3C_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_OUTPUTVIEW_H__B9DE3972_0F3C_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OutputView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COutputView view

class COutputView : public CEditView
{
protected:
	COutputView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(COutputView)

// Attributes
public: 

// Operations
public:
	bool m_bFindInProgress;
	void GetContents (CStringArray &arstr);
	void Save (const CString &strFile);
	void Clear();
	void AddText (const CString &str);
	afx_msg void OnEditClear();
	afx_msg void OnFileSave();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation

protected:
    void OnFindNext( LPCTSTR lpszFind, BOOL bNext, BOOL bCase );
    void OnTextNotFound( LPCTSTR lpszFind );

	virtual ~COutputView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


	// Generated message map functions
protected:
	//{{AFX_MSG(COutputView)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditFindAgain();
	afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFindAgain(CCmdUI* pCmdUI);
	afx_msg void OnEditFind();
  afx_msg LONG OnEditFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditChange();
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTVIEW_H__B9DE3972_0F3C_11D2_80BE_00A0C949ADAC__INCLUDED_)
