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
// MLTView.h: interface for the CMLTView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLTVIEW_H__C2A5FFD9_AEE9_11D2_BFDA_00A0C9554250__INCLUDED_)
#define AFX_MLTVIEW_H__C2A5FFD9_AEE9_11D2_BFDA_00A0C9554250__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxtempl.h>

#include "memmap.h"

typedef struct tagREGIONRECT
{
    std::list <mem_region>::iterator Region;
    CRect Rect;
}
REGIONRECT;

typedef struct tagSECTIONRECT
{
    std::list <mem_section_view>::iterator SectionView;
    CRect Rect;
}
SECTIONRECT;

class CMLTView : public CScrollView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CMLTView)

// Attributes
public:
	CMLTView();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMLTView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMLTView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
  
  CMapStringToPtr m_arstrTooltipRects;

	void DrawRegion (CDC* pDC, UINT uRegion, UINT uUnitCount, UINT uPixelsPerUnit, std::list <mem_region>::iterator region);
    CList <SECTIONRECT, SECTIONRECT &> listSectionRect;
    CList <REGIONRECT, REGIONRECT &> listRegionRect;
    SECTIONRECT * SectionHitTest (CPoint pntTest);
    REGIONRECT * RegionHitTest (CPoint pntTest);
    std::list <mem_section_view>::iterator m_sviSelectedSectionView;
    std::list <mem_region>::iterator m_riSelectedRegion;
    UINT m_uViewWidth, m_uClientWidth;
	void CalcUnitCountMax ();
    UINT m_uUnitCountMax;
	CRect m_rectSelectedItem;

protected:
  virtual int OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;

// Generated message map functions
protected:
	CString m_strTipText;
	//{{AFX_MSG(CMLTView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnMLTNewRegion ();
  afx_msg void OnMLTNewSection ();
  afx_msg void OnMLTDelete ();
  afx_msg void OnMLTProperties ();
  afx_msg void OnUpdateMLTNewSection (CCmdUI* pCmdUI);
  afx_msg void OnUpdateMLTDelete (CCmdUI* pCmdUI);
  afx_msg void OnUpdateMLTProperties (CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPopupProperties();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	//}}AFX_MSG
  BOOL OnToolTipText(UINT nID, NMHDR* pNMHDR, LRESULT*pResult);
  afx_msg void OnUpdateMLTNewRegion (CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MLTVIEW_H__C2A5FFD9_AEE9_11D2_BFDA_00A0C9554250__INCLUDED_)
