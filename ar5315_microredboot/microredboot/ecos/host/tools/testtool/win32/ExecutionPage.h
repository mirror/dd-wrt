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
#if !defined(AFX_EXECUTIONPAGE_H__44CEA288_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
#define AFX_EXECUTIONPAGE_H__44CEA288_11C4_11D3_A505_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExecutionPage.h : header file
//
#include "eCosPropertyPage.h"
#include "FileListBox.h"
#include "Collections.h"
/////////////////////////////////////////////////////////////////////////////
// CExecutionPage dialog

class CExecutionPage : public CeCosPropertyPage
{
	DECLARE_DYNCREATE(CExecutionPage)

// Construction
public:
	bool SomeTestsSelected();
	String m_strExtension;
	CMapStringToPtr m_arstrPreLoad; // If set on startup, load files from here
	CString SelectedTest(int nIndex);
	int SelectedTestCount();
	bool IsSelected(int i);
	CString m_strFolder;
	CExecutionPage();
	~CExecutionPage();
	bool m_bRecurse;
	void FillListBox(LPCTSTR pszFolder);

// Dialog Data
	//{{AFX_DATA(CExecutionPage)
	CFileListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExecutionPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStatic m_Static;
	CRect m_rcPrev;
	CEdit   m_Combo;
	WNDPROC m_wndProc;
    static CExecutionPage *pDlg;
	static LRESULT CALLBACK WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam);
	CButton m_Button;

	static int CALLBACK CBBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);

	// Generated message map functions
	//{{AFX_MSG(CExecutionPage)
	afx_msg void OnFolder();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectAll();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXECUTIONPAGE_H__44CEA288_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
