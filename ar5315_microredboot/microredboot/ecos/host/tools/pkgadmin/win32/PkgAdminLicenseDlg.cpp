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
// PkgAdminLicenseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PkgAdmin.h"
#include "PkgAdminLicenseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminLicenseDlg dialog


CPkgAdminLicenseDlg::CPkgAdminLicenseDlg(CWnd* pParent /*=NULL*/)
	: CeCosDialog(CPkgAdminLicenseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPkgAdminLicenseDlg)
	m_strLicense = _T("");
	m_strCaption = _T("");
	//}}AFX_DATA_INIT
}


void CPkgAdminLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPkgAdminLicenseDlg)
	DDX_Control(pDX, IDC_PKGADMIN_LICENSE, m_edtLicense);
	DDX_Text(pDX, IDC_PKGADMIN_LICENSE, m_strLicense);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPkgAdminLicenseDlg, CeCosDialog)
	//{{AFX_MSG_MAP(CPkgAdminLicenseDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminLicenseDlg message handlers


BOOL CPkgAdminLicenseDlg::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();

	m_fontLicense.CreatePointFont (90, _T("Courier New")); // use monospaced font
	m_edtLicense.SetFont (&m_fontLicense, FALSE);

	if (! m_strCaption.IsEmpty ())
		SetWindowText (m_strCaption);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
