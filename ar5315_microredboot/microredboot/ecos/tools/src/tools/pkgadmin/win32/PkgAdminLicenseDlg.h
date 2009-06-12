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
#if !defined(AFX_PKGADMINLICENSEDLG_H__857CC0AB_668D_11D3_8008_00A0C9554250__INCLUDED_)
#define AFX_PKGADMINLICENSEDLG_H__857CC0AB_668D_11D3_8008_00A0C9554250__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PkgAdminLicenseDlg.h : header file
//

#include "PkgAdminres.h"
#include "eCosDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CPkgAdminLicenseDlg dialog

class CPkgAdminLicenseDlg : public CeCosDialog
{
// Construction
public:
	void SetCaption (LPCTSTR pszCaption) { m_strCaption = pszCaption; };
	CPkgAdminLicenseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPkgAdminLicenseDlg)
	enum { IDD = IDD_PKGADMIN_LICENSE };
	CEdit	m_edtLicense;
	CString	m_strLicense;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPkgAdminLicenseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strCaption;
	CFont m_fontLicense;

	// Generated message map functions
	//{{AFX_MSG(CPkgAdminLicenseDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PKGADMINLICENSEDLG_H__857CC0AB_668D_11D3_8008_00A0C9554250__INCLUDED_)
