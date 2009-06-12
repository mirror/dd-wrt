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
// OutputPage.cpp : implementation file
//
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the Configuration -> Options View tab
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================


#include "stdafx.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "CTOptionsDialog.h"
#include "ConfigItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolsOptionsDialog property page

CToolsOptionsDialog::CToolsOptionsDialog() 
	: CeCosDialog(IDD, NULL)
{
	//{{AFX_DATA_INIT(CToolsOptionsDialog)
	//}}AFX_DATA_INIT
}

CToolsOptionsDialog::~CToolsOptionsDialog()
{
}

void CToolsOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolsOptionsDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolsOptionsDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CToolsOptionsDialog)
	ON_BN_CLICKED(IDC_DEFERRED, OnDeferred)
	ON_BN_CLICKED(IDC_IMMEDIATE, OnImmediate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolsOptionsDialog message handlers


BOOL CToolsOptionsDialog::OnInitDialog() 
{
  CeCosDialog::OnInitDialog();
  CConfigToolDoc*pDoc=CConfigTool::GetConfigToolDoc();
  ((CButton *)GetDlgItem(IDC_IMMEDIATE))->SetCheck(pDoc->m_nRuleChecking&CConfigToolDoc::Immediate);
  ((CButton *)GetDlgItem(IDC_DEFERRED ))->SetCheck(pDoc->m_nRuleChecking&CConfigToolDoc::Deferred);
  ((CButton *)GetDlgItem(IDC_SUGGEST_FIXES ))->SetCheck(pDoc->m_nRuleChecking&CConfigToolDoc::SuggestFixes);
  UpdateData(FALSE);
  SetButtons();
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CToolsOptionsDialog::OnDeferred() 
{
  SetButtons();
}

void CToolsOptionsDialog::OnImmediate() 
{
  SetButtons();
}

void CToolsOptionsDialog::OnOK() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  UpdateData(TRUE);
  pDoc->m_nRuleChecking=
    (CConfigToolDoc::SuggestFixes * ((CButton *)GetDlgItem(IDC_SUGGEST_FIXES))->GetCheck())|
    (CConfigToolDoc::Immediate    * ((CButton *)GetDlgItem(IDC_IMMEDIATE))->GetCheck())|
    (CConfigToolDoc::Deferred     * ((CButton *)GetDlgItem(IDC_DEFERRED))->GetCheck());
  CeCosDialog::OnOK();
}




void CToolsOptionsDialog::SetButtons()
{
  GetDlgItem(IDC_SUGGEST_FIXES)->EnableWindow(((CButton *)GetDlgItem(IDC_IMMEDIATE))->GetCheck()|((CButton *)GetDlgItem(IDC_DEFERRED))->GetCheck());
}
