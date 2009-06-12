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
// MLTFrame.h: interface for the CMLTFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MLTFRAME_H__91BD24C7_AF92_11D2_BFDA_00A0C9554250__INCLUDED_)
#define AFX_MLTFRAME_H__91BD24C7_AF92_11D2_BFDA_00A0C9554250__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MLTView.h"

class CMLTFrame : public CFrameWnd  
{
public:
	DECLARE_DYNCREATE(CMLTFrame)
    CMLTView * m_pMLTView;
	CMLTFrame();
	virtual ~CMLTFrame();

protected:
    CToolBar m_wndToolBar;
	//{{AFX_MSG(CMLTFrame)
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MLTFRAME_H__91BD24C7_AF92_11D2_BFDA_00A0C9554250__INCLUDED_)
