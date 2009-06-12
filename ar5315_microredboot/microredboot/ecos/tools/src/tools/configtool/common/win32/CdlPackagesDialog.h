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
#if !defined(AFX_CDLPACKAGESDIALOG_H__A14A0BAF_2ECF_11D3_BFFE_00A0C9554250__INCLUDED_)
#define AFX_CDLPACKAGESDIALOG_H__A14A0BAF_2ECF_11D3_BFFE_00A0C9554250__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CdlPackagesDialog.h : header file
//
#include "AddRemoveDialog.h"
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CCdlPackagesDialog dialog

class CCdlPackagesDialog : public CAddRemoveDialog
{
// Construction
public:
	CCdlPackagesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCdlPackagesDialog)
	enum { IDD = IDD_CDL_PACKAGES };
	CComboBox	m_cboPackageVersion;
	CString	m_strPackageDescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCdlPackagesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetVersion (LPCTSTR pszItem);
	void Insert (LPCTSTR pszItem, bool bAdded, LPCTSTR pszDesc = NULL, LPCTSTR pszVersion = NULL);
protected:
	void HardwarePackageMessageBox();
	void UpdateHardwareSelectionFlag();
	bool m_bHardwarePackageSelected;
	CStringArray m_arstrVersions;
	void UpdatePackageDescription();
	void UpdateVersionList();

	// Generated message map functions
	//{{AFX_MSG(CCdlPackagesDialog)
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeList2();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePackageVersion();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnDblclkList1();
	afx_msg void OnDblclkList2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDLPACKAGESDIALOG_H__A14A0BAF_2ECF_11D3_BFFE_00A0C9554250__INCLUDED_)
