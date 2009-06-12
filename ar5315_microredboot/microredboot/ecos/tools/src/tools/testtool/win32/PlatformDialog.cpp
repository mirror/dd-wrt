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
// PlatformDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eCosTest.h"
#include "testtool.h"
#include "PlatformDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlatformDialog dialog


CPlatformDialog::CPlatformDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(CPlatformDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlatformDialog)
	m_strPlatform=_T("");
	m_strPrefix=_T("");
	m_strGDB=_T("");
	m_strInferior = _T("");
	m_strPrompt = _T("");
	m_bServerSideGdb = FALSE;
	//}}AFX_DATA_INIT
}


void CPlatformDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlatformDialog)
  DDX_Text(pDX, IDC_TT_NEW_PLATFORM, m_strPlatform);
  DDX_Text(pDX, IDC_TT_NEW_PLATFORM_PREFIX, m_strPrefix);
  DDX_Text(pDX, IDC_TT_NEW_PLATFORM_GDB, m_strGDB);
	DDX_Text(pDX, IDC_INFERIOR, m_strInferior);
	DDX_Text(pDX, IDC_PROMPT, m_strPrompt);
	DDX_Check(pDX, IDC_SERVER_SIDE_GDB, m_bServerSideGdb);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlatformDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CPlatformDialog)
	ON_CBN_EDITCHANGE(IDC_TT_NEW_PLATFORM_PREFIX, OnChangeNewPlatformPrefix)
	ON_EN_CHANGE(IDC_TT_NEW_PLATFORM, OnChangeNewPlatform)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlatformDialog message handlers

BOOL CPlatformDialog::OnInitDialog()
{
  m_strGDB.Replace(_T(";"),_T("\r\n"));
  CeCosDialog::OnInitDialog();

  SetWindowText(m_strCaption);
  if(!m_strPlatform.IsEmpty()){
    SetDlgItemText(IDC_TT_NEW_PLATFORM,m_strPlatform);
    GetDlgItem(IDC_TT_NEW_PLATFORM)->EnableWindow(false);
  }
  CMapStringToPtr map;
  for(unsigned int i=0;i<CeCosTestPlatform::Count();i++){
    map.SetAt(CeCosTestPlatform::Get(i)->Prefix(),this);
  }
  for(POSITION pos = map.GetStartPosition(); pos != NULL; ){
    void *p;
    CString str;
    map.GetNextAssoc(pos, str, p);
    ((CComboBox *)GetDlgItem(IDC_TT_NEW_PLATFORM_PREFIX))->AddString(str);
  }

  GetDlgItem(IDOK)->EnableWindow(!m_strPlatform.IsEmpty() && !m_strPrefix.IsEmpty());

  return true;
}

void CPlatformDialog::OnChangeNewPlatformPrefix() 
{
  UpdateData(true);
  GetDlgItem(IDOK)->EnableWindow(!m_strPlatform.IsEmpty() && !m_strPrefix.IsEmpty());
}

void CPlatformDialog::OnChangeNewPlatform() 
{
  UpdateData(true);
  GetDlgItem(IDOK)->EnableWindow(!m_strPlatform.IsEmpty() && !m_strPrefix.IsEmpty());
}

void CPlatformDialog::OnOK() 
{
	UpdateData(TRUE);
  m_strGDB.Replace(_T("\r\n"),_T(";"));
	EndDialog(IDOK);
}
