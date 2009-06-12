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
#if !defined(AFX_SUMMARYPAGE_H__AC331EDE_1201_11D3_A507_00A0C949ADAC__INCLUDED_)
#define AFX_SUMMARYPAGE_H__AC331EDE_1201_11D3_A507_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SummaryPage.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CSummaryPage dialog
class CeCosTest;
#include "eCosPropertyPage.h"

class CSummaryPage : public CeCosPropertyPage
{
	DECLARE_DYNCREATE(CSummaryPage)

// Construction
public:
	void AddResult (CeCosTest *pTest);
	CSummaryPage();
	~CSummaryPage();

// Dialog Data
	//{{AFX_DATA(CSummaryPage)
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSummaryPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static const int nCols;
	static LPCTSTR arpszCols[];
	CRect m_rcPrev,m_rcOffset;
	int m_nLastCol;
	static int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2, LPARAM lParamSort);
	// Generated message map functions
	//{{AFX_MSG(CSummaryPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClear();
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUMMARYPAGE_H__AC331EDE_1201_11D3_A507_00A0C949ADAC__INCLUDED_)
