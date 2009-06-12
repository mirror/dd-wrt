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
// Description:	Interface of the customer splitter window class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#ifndef _SPLITTERWNDEX
#define _SPLITTERWNDEX
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
////////////////////////////////////////////////////////////////////////////
//
// splitex.h
// (c) 1997, Oleg G. Galkin

class CSplitterWndEx : public CSplitterWnd
{
public:
	void SetSplitPoint (double fSplit);
	double GetSplitPoint();
	void RecalcLayout();
	void ShowPane (CWnd *pWnd,BOOL bShow);
    CSplitterWndEx();
	//virtual BOOL CreateStatic( CWnd* pParentWnd, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST );

// ClassWizard generated virtual function overrides
     //{{AFX_VIRTUAL(CSplitterWndEx)
     //}}AFX_VIRTUAL

// Generated message map functions
protected:
  bool IsHorizontal() const { return m_nCols>m_nRows; }
	void SwapRowColInfo();
	double m_fRowColumnRatio;
	BOOL m_bParentHide;
	CWnd * m_pwnd1;
	CWnd * m_pwnd2;
     //{{AFX_MSG(CSplitterWndEx)
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};
#endif
