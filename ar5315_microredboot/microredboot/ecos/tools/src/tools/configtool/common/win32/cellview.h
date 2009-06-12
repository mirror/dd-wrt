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
// Description:	Interface of a the cell window view class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_CELLVIEW_H__71506369_375A_11D2_80C1_00A0C949ADAC__INCLUDED_)
#define AFX_CELLVIEW_H__71506369_375A_11D2_80C1_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CellView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCellView view
class CConfigItem;
class CCell;
#include "ControlView.h"
#include "ConfigTool.h"

class CCellView : public CView
{
protected:
	// Convert an HTREEITEM to a CConfigItem:
	CConfigItem &TI (HTREEITEM h) const { 
        return *(CConfigItem *)(CConfigTool::GetControlView()->GetItemData(h));
    }

	CCellView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCellView)

	// Create a control for in-cell editing:
	BOOL InCell(HTREEITEM h);

	// Get the value from the cell control
	ItemIntegerType GetCellValue() const;

	// retrieve the bounding rectangle for hItem
	void GetItemRect(HTREEITEM hItem,CRect & rect) const;
	
	// bounding rectangle for cells (this is a little smaller than the item rectangle
	// in the case of editboxes).  bDropped tells us what we want to know about comboboxes
	void GetInCellRect(HTREEITEM h, CRect & rect,BOOL bDropped=TRUE) const;
	
	int m_nFirstVisibleItem;

    CPen m_GrayPen;

// Attributes
public:

	// return the handle of the item with an in-cell edit control, or NULL if none
	HTREEITEM ActiveCell() const { return m_hInCell; }

// Operations
public:

	// Scroll to match our buddy
	void Sync();

	// Destroy any in-cell editing
	void CancelCellEdit(bool bApplyChanges=true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCellView)
	public:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strInitialCell;
	HTREEITEM HitTest();
	bool m_bComboSellPending;
	CCell * m_pwndCell;   // Window for the in-cell editing
	HTREEITEM m_hInCell; // hItem being in-cell edited
	virtual ~CCellView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CCellView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnEditFind(); 
    afx_msg void OnEditFindAgain(); 
	afx_msg void OnUpdateEditFindAgain(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg LRESULT OnCancelEdit(WPARAM wParam, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CELLVIEW_H__71506369_375A_11D2_80C1_00A0C949ADAC__INCLUDED_)
