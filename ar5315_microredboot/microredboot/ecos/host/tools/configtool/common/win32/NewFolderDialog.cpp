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
// NewFolderDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NewFolderDialog.h"
#include "CTUtils.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewFolderDialog dialog


CNewFolderDialog::CNewFolderDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(IDD_NEW_FOLDER_DIALOG, pParent)
{
	//{{AFX_DATA_INIT(CNewFolderDialog)
	m_strFolder = _T("");
	//}}AFX_DATA_INIT
}


void CNewFolderDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewFolderDialog)
	DDX_Text(pDX, IDC_NEW_FOLDER_EDIT, m_strFolder);
	DDV_MaxChars(pDX, m_strFolder, 256);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewFolderDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CNewFolderDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewFolderDialog message handlers

void CNewFolderDialog::OnOK() 
{
	UpdateData();
	if(!m_strFolder.CreateDirectory()){
		CUtils::MessageBoxF(_T("Could not create folder %s"),m_strFolder);
	} else {
		CeCosDialog::OnOK();
	}
}

BOOL CNewFolderDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();
	CEdit *pEdit=(CEdit *)GetDlgItem(IDC_NEW_FOLDER_EDIT);
	pEdit->SetFocus();
	pEdit->SetSel(0,-1);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
