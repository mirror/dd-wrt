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
// RemotePropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "RemotePropertiesDialog.h"
#include "eCosSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemotePropertiesDialog dialog


CRemotePropertiesDialog::CRemotePropertiesDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(IDD_TT_PROPERTIES3, pParent)
{
	//{{AFX_DATA_INIT(CRemotePropertiesDialog)
	//}}AFX_DATA_INIT
}


void CRemotePropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
    if(pDX->m_bSaveAndValidate){
        m_bFarmed=(TRUE==((CButton *)GetDlgItem(IDC_TT_FARMED))->GetCheck());
    } else {
        ((CButton *)GetDlgItem(IDC_TT_FARMED))->SetCheck(m_bFarmed);
        ((CButton *)GetDlgItem(IDC_TT_EXPLICIT))->SetCheck(!m_bFarmed);
    }
	//{{AFX_DATA_MAP(CRemotePropertiesDialog)
	DDX_Text(pDX, IDC_TT_RESOURCEHOST, m_strResourceHost);
	DDX_Text(pDX, IDC_TT_RESOURCEPORT, m_nResourcePort);
	DDX_Text(pDX, IDC_TT_REMOTEHOST, m_strRemoteHost);
	DDX_Text(pDX, IDC_TT_REMOTEPORT, m_nRemotePort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRemotePropertiesDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CRemotePropertiesDialog)
	ON_BN_CLICKED(IDC_TT_FARMED, OnFarmed)
	ON_BN_CLICKED(IDC_TT_EXPLICIT, OnExplicit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemotePropertiesDialog message handlers

void CRemotePropertiesDialog::OnFarmed() 
{
    ((CButton *)GetDlgItem(IDC_TT_EXPLICIT))->SetCheck(FALSE);
    SetButtons();	
}

void CRemotePropertiesDialog::OnExplicit() 
{
    ((CButton *)GetDlgItem(IDC_TT_FARMED))->SetCheck(FALSE);
    SetButtons();	
}

void CRemotePropertiesDialog::SetButtons(bool bFromDataExchange)
{
    if(!bFromDataExchange){
        UpdateData(TRUE);
    }
    GetDlgItem(IDC_TT_RESOURCEHOST)->EnableWindow(m_bFarmed);
    GetDlgItem(IDC_TT_RESOURCEPORT)->EnableWindow(m_bFarmed);
    GetDlgItem(IDC_TT_REMOTEHOST)->EnableWindow(!m_bFarmed);
    GetDlgItem(IDC_TT_REMOTEPORT)->EnableWindow(!m_bFarmed);
}

BOOL CRemotePropertiesDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();
	
    SetButtons();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRemotePropertiesDialog::OnOK() 
{
    UpdateData();
    if(m_bFarmed && !CeCosSocket::IsLegalHostPort(CeCosSocket::HostPort(m_strResourceHost,m_nResourcePort))){
        MessageBox(_T("Please provide a valid host/port combination for resource server"));
    } else if (!m_bFarmed && !CeCosSocket::IsLegalHostPort(CeCosSocket::HostPort(m_strRemoteHost,m_nRemotePort))){
        MessageBox(_T("Please provide a valid host/port combination for remote execution"));
    } else {
    	CeCosDialog::OnOK();
    }

}
