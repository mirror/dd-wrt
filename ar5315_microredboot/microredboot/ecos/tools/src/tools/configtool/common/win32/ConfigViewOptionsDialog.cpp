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
// ConfigViewOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigViewOptionsDialog.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigViewOptionsDialog dialog


CConfigViewOptionsDialog::CConfigViewOptionsDialog()
	: CeCosDialog(IDD_CONFIGURATION_VIEW_OPTIONS_DIALOG, NULL)
{
	//{{AFX_DATA_INIT(CConfigViewOptionsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigViewOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigViewOptionsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigViewOptionsDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CConfigViewOptionsDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigViewOptionsDialog message handlers

BOOL CConfigViewOptionsDialog::OnInitDialog() 
{
  CeCosDialog::OnInitDialog();
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  ((CButton *)GetDlgItem(IDC_RADIO_DECIMAL))->SetCheck(!pDoc->m_bHex);
  ((CButton *)GetDlgItem(IDC_RADIO_DESCRIPTIVE_NAMES))->SetCheck(!pDoc->m_bMacroNames);
  ((CButton *)GetDlgItem(IDC_RADIO_HEXADECIMAL))->SetCheck(pDoc->m_bHex);
  ((CButton *)GetDlgItem(IDC_RADIO_MACRO_NAMES))->SetCheck(pDoc->m_bMacroNames);
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigViewOptionsDialog::OnOK() 
{
  bool bHex=((CButton *)GetDlgItem(IDC_RADIO_HEXADECIMAL))->GetCheck();
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  if(bHex!=pDoc->m_bHex){
    pDoc->m_bHex=bHex;
    pDoc->UpdateAllViews(0,CConfigToolDoc::IntFormatChanged);
  }
  
  bool bMacros=((CButton *)GetDlgItem(IDC_RADIO_MACRO_NAMES))->GetCheck();
  if(bMacros!=pDoc->m_bMacroNames){
    pDoc->m_bMacroNames=bMacros;
    pDoc->UpdateAllViews(0,CConfigToolDoc::NameFormatChanged);
  }
  CeCosDialog::OnOK();
}

