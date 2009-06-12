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
// eCosPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "eCosPropertySheet.h"
#include "eCosDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CeCosPropertySheet dialog
IMPLEMENT_DYNCREATE(CeCosPropertySheet, CCSHPropertySheet)
/*
CeCosPropertySheet::CeCosPropertySheet(UINT id,CWnd* pParent )
	: CCSHPropertySheet(id, pParent)
{
	//{{AFX_DATA_INIT(CeCosPropertySheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
*/

void CeCosPropertySheet::DoDataExchange(CDataExchange* pDX)
{
	CCSHPropertySheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CeCosPropertySheet)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CeCosPropertySheet, CCSHPropertySheet)
	//{{AFX_MSG_MAP(CeCosPropertySheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CeCosPropertySheet message handlers

CString CeCosPropertySheet::CSHFile() const
{
  return CeCosDialog::CSHFile((UINT)GetDlgCtrlID());
}

UINT CeCosPropertySheet::HelpID (DWORD dwCtrlID) const
{
  return CeCosDialog::HelpID((int)GetDlgCtrlID(),dwCtrlID);
}
