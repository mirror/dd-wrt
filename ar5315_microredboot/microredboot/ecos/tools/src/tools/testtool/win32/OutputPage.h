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
#if !defined(AFX_OUTPUTPAGE_H__44CEA28A_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
#define AFX_OUTPUTPAGE_H__44CEA28A_11C4_11D3_A505_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputPage.h : header file
//
#include "TestToolRes.h"		// main symbols
#include "OutputEdit.h"
#include "eCosPropertyPage.h"
/////////////////////////////////////////////////////////////////////////////
// COutputPage dialog

class COutputPage : public CeCosPropertyPage
{
	DECLARE_DYNCREATE(COutputPage)

// Construction
public:
	void AddLogMsg(LPCTSTR psz);
	void AddText (LPCTSTR psz);
	COutputPage();
	~COutputPage();

// Dialog Data
	//{{AFX_DATA(COutputPage)
	COutputEdit	m_Edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COutputPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRect m_rcPrev;
    CFont m_Font;
	// Generated message map functions
	//{{AFX_MSG(COutputPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTPAGE_H__44CEA28A_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
