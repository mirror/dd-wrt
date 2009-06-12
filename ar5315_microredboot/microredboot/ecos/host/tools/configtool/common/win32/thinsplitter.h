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
// Description:	Interface of a CSplitterWndEx override for control/cell view split
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================#include "stdafx.h"
#ifndef _THINSPLITTER_H
#define _THINSPLITTER_H
#include "SplitterWndEx.h"
class CThinSplitter : public CSplitterWndEx {
public:
	CThinSplitter();
	virtual ~CThinSplitter();
	void DeleteColumn(int colDelete);
	void OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rect );
	int HitTest(CPoint pt) const;
	BOOL SplitColumn(int cxBefore);
	void RecalcLayout();
	virtual BOOL CreateStatic( CWnd* pParentWnd, int nRows, int nCols, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = AFX_IDW_PANE_FIRST );

protected:

  CPen m_GrayPen;
	double m_fColumnRatio;
	//DECLARE_DYNCREATE(CThinSplitter)
	//{{AFX_MSG(CThinSplitter)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif




	
