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
// eCosPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "eCosPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CeCosPropertyPage dialog
IMPLEMENT_DYNCREATE(CeCosPropertyPage, CCSHPropertyPage)
/*
CeCosPropertyPage::CeCosPropertyPage(UINT id,CWnd* pParent )
	: CCSHPropertyPage(id, pParent)
{
	//{{AFX_DATA_INIT(CeCosPropertyPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
*/

void CeCosPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CCSHPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CeCosPropertyPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CeCosPropertyPage, CCSHPropertyPage)
	//{{AFX_MSG_MAP(CeCosPropertyPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CeCosPropertyPage message handlers

CString CeCosPropertyPage::CSHFile() const
{
  return CeCosDialog::CSHFile((UINT)m_lpszTemplateName);
}

UINT CeCosPropertyPage::HelpID (DWORD dwCtrlID) const
{
  return CeCosDialog::HelpID((int)m_lpszTemplateName,dwCtrlID);
}
