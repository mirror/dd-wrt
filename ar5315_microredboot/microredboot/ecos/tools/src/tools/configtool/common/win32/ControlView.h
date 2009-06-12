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
// Description:	Interface of the tree (control) view class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_CONTROLVIEW_H__A4845250_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_CONTROLVIEW_H__A4845250_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ControlView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CControlView view
class CConfigItem;
#include <afxcview.h>

class CCellView;
class CConfigToolDoc;
class CConfigViewOptionsDialog;
class CFindDialog;

class CControlView : public CTreeView
{
  friend class CConfigItem;
protected:
	CControlView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CControlView)

// Attributes
public:
// Attributes
	// Let us use the View as a control:
	#include "TreeCtrlToView.inl"	
    friend class CCellView;
// Operations
public:
	int GetItemHeight() const { return m_nItemHeight; }
	CConfigItem &TI (HTREEITEM h) const {
		return *(CConfigItem *)GetItemData(h);
	}

	BOOL IsActive(HTREEITEM h) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Refresh (HTREEITEM h);
  CPen m_GrayPen;
	int m_nMaxWidth;
	BOOL m_bHasHScroll;
	void SetHScrollPos();
	void SetHScrollRangePos();
	BOOL m_bHasVScroll;
	void SetScrollRangePos();
	void SetScrollPos();
	void KillScrollBars();
	virtual ~CControlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	void ShowPopupMenu (HTREEITEM h,CPoint point=CPoint(-1,-1));
	void RecalcWorkSpaceSize();
	int m_TreeXOffsetAdjustment;
	int m_nWorkspace; // Height of the virtual workspace
	int TotalDescendantHeight(HTREEITEM hItem)const; // Visible height of descendants (not including hItem itself)
	CSize m_Size;
	int m_nItemHeight; // Height of an item (can use GetItemHeight() in IE4)
	bool BumpItem (HTREEITEM h,int nInc=1); // Increment the item (do what clicking on the icon achieves)
	void AdjustItemImage (HTREEITEM h);
protected:
	bool m_bFindInProgress;
	static bool IsWordChar (TCHAR c);
    CConfigItem *DoFind(LPCTSTR pszWhat,DWORD dwFlags,WhereType where);
	HTREEITEM m_hExpandedForFind;
	CConfigToolDoc * GetDocument();
	void InvalidateItem (HTREEITEM h);
	BOOL IsChanged (HTREEITEM h, BOOL bRecurse);
	void RestoreDefault (HTREEITEM h, BOOL bRecurse = FALSE, BOOL bTopLevel = TRUE);
	HTREEITEM m_hContext; // Where the right-click occurred
	ItemIntegerType Value(HTREEITEM h) const; // Value (taking into account any in-cell activity)
	CImageList m_il; // Image list for the control
	//{{AFX_MSG(CControlView)
	afx_msg void OnPaint();
	afx_msg void OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRestoreDefaults();
	afx_msg void OnPopupProperties();
	afx_msg void OnUnload();
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewUrl();
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewHeader();
  afx_msg LONG OnFind(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditFind();
	afx_msg void OnEditFindAgain();
	afx_msg void OnUpdateEditFindAgain(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditFind(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWhatsThis();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	//}}AFX_MSG
    afx_msg LRESULT OnSetFont(WPARAM, LPARAM lParam);
public:
	void Refresh (LPCTSTR pszMacroName);
	bool SelectItem (const CConfigItem *pItem);
	DECLARE_MESSAGE_MAP()
private:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLVIEW_H__A4845250_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
