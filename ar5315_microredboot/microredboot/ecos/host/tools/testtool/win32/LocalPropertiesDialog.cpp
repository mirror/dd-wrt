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
// LocalPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TestToolRes.h"		// main symbols
#include "LocalPropertiesDialog.h"
#include "eCosSocket.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocalPropertiesDialog dialog


CLocalPropertiesDialog::CLocalPropertiesDialog(bool bHideX10Controls)
  : CeCosDialog(IDD_TT_PROPERTIES2, NULL),
  m_bHideX10Controls(bHideX10Controls)
{
  //{{AFX_DATA_INIT(CLocalPropertiesDialog)
  //}}AFX_DATA_INIT
}


void CLocalPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
  CeCosDialog::DoDataExchange(pDX);
  if(pDX->m_bSaveAndValidate){
    m_bSerial=(TRUE==((CButton *)GetDlgItem(IDC_TT_RADIO_SERIAL))->GetCheck());
    
    CComboBox *pCombo=((CComboBox *)GetDlgItem(IDC_TT_LOCAL_PORT));
    int i=pCombo->GetCurSel();
    pCombo->GetLBText(i, m_strPort.GetBuffer(pCombo->GetLBTextLen(i)));
    m_strPort.ReleaseBuffer();
    
    CString strBaud;
    pCombo=((CComboBox *)GetDlgItem(IDC_TT_BAUD));
    i=pCombo->GetCurSel();
    pCombo->GetLBText(i, strBaud.GetBuffer(pCombo->GetLBTextLen(i)));
    strBaud.ReleaseBuffer();
    m_nBaud=_ttoi(strBaud);
  } else {
    ((CButton *)GetDlgItem(IDC_TT_RADIO_SERIAL))->SetCheck(m_bSerial);
    ((CButton *)GetDlgItem(IDC_TT_RADIO_TCPIP))->SetCheck(!m_bSerial);
    SetButtons(true);
    
    CComboBox *pCombo=((CComboBox *)GetDlgItem(IDC_TT_LOCAL_PORT));
    if(pCombo->GetCount()>0){
      int nSel=0;
      for(int i=0;i<pCombo->GetCount();i++){
        CString strPort;
        pCombo->GetLBText(i, strPort.GetBuffer(pCombo->GetLBTextLen(i)));
        strPort.ReleaseBuffer();
        if(0==strPort.Compare(m_strPort)){
          nSel=i;
          break;
        }
      }
      pCombo->SetCurSel(nSel);
    }
    pCombo=((CComboBox *)GetDlgItem(IDC_TT_BAUD));
    int nSel=0;
    for(int i=0;i<pCombo->GetCount();i++){
      CString strBaud;
      pCombo->GetLBText(i, strBaud.GetBuffer(pCombo->GetLBTextLen(i)));
      strBaud.ReleaseBuffer();
      if(m_nBaud==_ttoi(strBaud)){
        nSel=i;
        break;
      }
    }
    pCombo->SetCurSel(nSel);
  }
  //{{AFX_DATA_MAP(CLocalPropertiesDialog)
  DDX_Text(pDX, IDC_TT_LOCALTCPIPHOST, m_strLocalTCPIPHost);
  DDX_Text(pDX, IDC_TT_LOCALTCPIPPORT, m_nLocalTCPIPPort);
  DDV_MinMaxUInt(pDX, m_nLocalTCPIPPort, 1, 65535);
  DDX_CBIndex(pDX, IDC_TT_RESET, m_nReset);
  DDX_Text(pDX, IDC_TT_RESETSTRING, m_strReset);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocalPropertiesDialog, CeCosDialog)
//{{AFX_MSG_MAP(CLocalPropertiesDialog)
ON_BN_CLICKED(IDC_TT_RADIO_SERIAL, OnRadioSerial)
ON_BN_CLICKED(IDC_TT_RADIO_TCPIP, OnRadioTcpip)
ON_CBN_SELCHANGE(IDC_TT_RESET, OnSelchangeReset)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocalPropertiesDialog message handlers

void CLocalPropertiesDialog::SetButtons(bool bFromDataExchange)
{
  if(!bFromDataExchange){
    UpdateData(TRUE);
  }
  GetDlgItem(IDC_TT_LOCAL_PORT)->EnableWindow(m_bSerial);
  GetDlgItem(IDC_TT_BAUD)->EnableWindow(m_bSerial);
  GetDlgItem(IDC_TT_LOCALTCPIPHOST)->EnableWindow(!m_bSerial);
  GetDlgItem(IDC_TT_LOCALTCPIPPORT)->EnableWindow(!m_bSerial);
  static const UINT arIDs[]={IDC_TT_RESETSTRING};
  for(int i=0;i<sizeof arIDs/sizeof arIDs[0];i++){
    GetDlgItem(arIDs[i])->EnableWindow(RESET_X10==m_nReset);
  }
}

BOOL CLocalPropertiesDialog::OnInitDialog() 
{
  if(m_bHideX10Controls){
    // Hide these controls:
    static const UINT arIDs1[]={IDC_TT_STATIC_RESET, IDC_TT_STATIC_METHOD, IDC_TT_RESET};
    for(int i=0;i<sizeof arIDs1/sizeof arIDs1[0];i++){
      GetDlgItem(arIDs1[i])->ShowWindow(SW_HIDE);
    }
    // Move the bottom buttons up
    CRect rect;
    GetDlgItem(IDC_TT_STATIC_RESET)->GetWindowRect(rect);
    ScreenToClient(rect);
    int nTop=rect.top;
    static const arIDs2[]={IDOK, IDCANCEL};
    int nDelta=0;
    for(i=0;i<sizeof arIDs2/sizeof arIDs2[0];i++){
      CWnd *pWnd=GetDlgItem(arIDs2[i]);
      pWnd->GetWindowRect(rect);
      ScreenToClient(rect);
      int nHeight=rect.Height();
      nDelta=rect.top-nTop;
      rect.top=nTop;
      rect.bottom=rect.top+nHeight;
      pWnd->MoveWindow(rect);
    }
    GetWindowRect(rect);
    rect.bottom-=nDelta;
    MoveWindow(rect);
  }
  
  CeCosDialog::OnInitDialog();
  CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_TT_LOCAL_PORT);
  TCHAR szPort[]=_T("COMx");
  for(TCHAR c=_TCHAR('1');c<=_TCHAR('8');c++){
    szPort[3]=c;
    pCombo->AddString(szPort);
  }
  UpdateData(false); // because only now has the port combo been populated
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CLocalPropertiesDialog::OnRadioSerial() 
{
  ((CButton *)GetDlgItem(IDC_TT_RADIO_TCPIP))->SetCheck(FALSE);
  SetButtons();
}

void CLocalPropertiesDialog::OnRadioTcpip() 
{
  ((CButton *)GetDlgItem(IDC_TT_RADIO_SERIAL))->SetCheck(FALSE);
  SetButtons();
}

void CLocalPropertiesDialog::OnOK() 
{
  UpdateData();
  
  HANDLE handle=CreateFile(m_strPort, GENERIC_READ ,0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
  if(INVALID_HANDLE_VALUE!=handle){
    CloseHandle(handle);
  } else if (IDNO==MessageBox(_T("The currently selected serial port is not accessible.  Use this setting anyway?"),NULL,MB_YESNO|MB_DEFBUTTON2)){
    return;
  }
  
  if(!m_bSerial && !CeCosSocket::IsLegalHostPort(CeCosSocket::HostPort(m_strLocalTCPIPHost,m_nLocalTCPIPPort))){
    MessageBox(_T("Please provide a valid host/port combination for TCP/IP connection"));
  } else if (RESET_X10==m_nReset && m_strReset.IsEmpty()){
    MessageBox(_T("Please provide a valid reset string for X10 reset"));
  } else {
    CeCosDialog::OnOK();
  }
}

void CLocalPropertiesDialog::OnSelchangeReset() 
{
  SetButtons();
}
