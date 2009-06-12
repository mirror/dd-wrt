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
// Description:	Interface of the find folder dialog
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_FOLDERDIALOG_H__15EC5D2F_1707_11D2_80C0_00A0C949ADAC__INCLUDED_)
#define AFX_FOLDERDIALOG_H__15EC5D2F_1707_11D2_80C0_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FolderDialog.h : header file
//

#include "FileName.h"
#include "resource.h"
#include "eCosDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CFolderDialog dialog

class CFolderDialog : public CeCosDialog
{
// Construction
public:
	CString m_strDesc;
	CFolderDialog(BOOL bAllowCreation=TRUE, UINT id=0);   // standard constructor
	CString m_strTitle;
	CFileName m_strFolder;

// Dialog Data
	//{{AFX_DATA(CFolderDialog)
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFolderDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	
	
	static WNDPROC m_wndProc;
	static LRESULT CALLBACK WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam);
	CButton *m_pButton;

	BOOL m_bAllowCreation;
	static int CALLBACK CBBrowseCallbackProc( HWND hwnd, 
		UINT uMsg, 
		LPARAM lParam, 
		LPARAM lpData 
		);
	void BrowseCallbackProc( HWND hwnd, 
		UINT uMsg, 
		LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CFolderDialog)
	afx_msg void OnBrowse();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnChangeFolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOLDERDIALOG_H__15EC5D2F_1707_11D2_80C0_00A0C949ADAC__INCLUDED_)
