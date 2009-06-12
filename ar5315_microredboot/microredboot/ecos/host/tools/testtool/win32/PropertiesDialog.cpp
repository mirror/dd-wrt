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
// ConnectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "eCosTestPlatform.h"
#include "eCosSocket.h"
#include "PropertiesDialog.h"
#include "LocalPropertiesDialog.h"
#include "RemotePropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog property page

//IMPLEMENT_DYNCREATE(CPropertiesDialog, CeCosDialog)

CPropertiesDialog::CPropertiesDialog(bool bHideTarget,bool bHideRemoteControls) : 
    CeCosDialog(IDD_TT_PROPERTIES),
    m_bConnectionModified(false),
    m_bHideRemoteControls(bHideRemoteControls),
    m_bHideTarget(bHideTarget)
{
	//{{AFX_DATA_INIT(CPropertiesDialog)
	//}}AFX_DATA_INIT
}

CPropertiesDialog::~CPropertiesDialog()
{
}

void CPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
    CComboBox *pCombo=((CComboBox *)GetDlgItem(IDC_TT_PLATFORM));
    if(pDX->m_bSaveAndValidate){
        int i=pCombo->GetCurSel();
        pCombo->GetLBText(i, m_strTarget.GetBuffer(pCombo->GetLBTextLen(i)));
        m_strTarget.ReleaseBuffer();
        m_bRemote=(TRUE==((CButton *)GetDlgItem(IDC_TT_RADIO_REMOTE))->GetCheck());
    } else {
        if(pCombo->GetCount()>0){
            int nSel=0;
            for(int i=0;i<pCombo->GetCount();i++){
                CString str;
                pCombo->GetLBText(i, str.GetBuffer(pCombo->GetLBTextLen(i)));
                str.ReleaseBuffer();
                if(0==str.Compare(m_strTarget)){
                    nSel=i;
                    break;
                }
            }
            pCombo->SetCurSel(nSel);
        }
        GetDlgItem(IDC_TT_STATIC_PLATFORM)->SetWindowText(m_strTarget);

        SetButtons();
        ((CButton *)GetDlgItem(IDC_TT_RADIO_REMOTE))->SetCheck(m_bRemote);
        ((CButton *)GetDlgItem(IDC_TT_RADIO_LOCAL))->SetCheck(!m_bRemote);
    }
	//{{AFX_DATA_MAP(CPropertiesDialog)
	DDX_Text(pDX, IDC_TT_DOWNLOADTIMEOUT, m_nDownloadTimeout);
	DDV_MinMaxUInt(pDX, m_nDownloadTimeout, 1, 3600);
	DDX_Text(pDX, IDC_TT_TESTTIMEOUT, m_nTimeout);
	DDV_MinMaxUInt(pDX, m_nTimeout, 1, 3600);
	DDX_CBIndex(pDX, IDC_TT_DOWNLOADTIMEOUT_COMBO, m_nDownloadTimeoutType);
	DDX_CBIndex(pDX, IDC_TT_TIMEOUT_COMBO, m_nTimeoutType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertiesDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CPropertiesDialog)
	ON_BN_CLICKED(IDC_TT_RADIO_LOCAL, OnRadioLocal)
	ON_BN_CLICKED(IDC_TT_RADIO_REMOTE, OnRadioRemote)
	ON_CBN_SELCHANGE(IDC_TT_PLATFORM, OnSelchangePlatform)
	ON_BN_CLICKED(IDC_TT_SETTINGS, OnSettings)
	ON_CBN_SELCHANGE(IDC_TT_DOWNLOADTIMEOUT_COMBO, OnSelchangeDownloadtimeoutCombo)
	ON_CBN_SELCHANGE(IDC_TT_TIMEOUT_COMBO, OnSelchangeTimeoutCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog message handlers

void CPropertiesDialog::OnRadioLocal() 
{
    ((CButton *)GetDlgItem(IDC_TT_RADIO_REMOTE))->SetCheck(FALSE);	
    SetButtons();
}

void CPropertiesDialog::OnRadioRemote() 
{
    ((CButton *)GetDlgItem(IDC_TT_RADIO_LOCAL))->SetCheck(FALSE);	
    SetButtons();
}


BOOL CPropertiesDialog::OnInitDialog() 
{
    GetParent()->BringWindowToTop();

    if(m_bHideRemoteControls){
        
        m_bRemote=false;

        // Hide these controls:
        static const arIDs1[]={IDC_STATIC_EXECUTION, IDC_TT_RADIO_LOCAL, IDC_TT_RADIO_REMOTE};
        for(int i=0;i<sizeof arIDs1/sizeof arIDs1[0];i++){
            GetDlgItem(arIDs1[i])->ShowWindow(SW_HIDE);
        }

        // Move the bottom three buttons up
        CRect rect;
        GetDlgItem(IDC_STATIC_EXECUTION)->GetWindowRect(rect);
        ScreenToClient(rect);
        int nTop=rect.top;
        static const arIDs2[]={IDC_TT_SETTINGS, IDOK, IDCANCEL};
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

    CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_TT_PLATFORM);
    for(unsigned int i=0;i<CeCosTestPlatform::Count();i++){
        pCombo->AddString(CeCosTestPlatform::Get(i)->Name());
    }
    
    UpdateData(false);

	((CSpinButtonCtrl *)GetDlgItem(IDC_TT_SPIN3))->SetRange(1,999);
	((CSpinButtonCtrl *)GetDlgItem(IDC_TT_SPIN4))->SetRange(1,999);

    if(m_bHideTarget){
        GetDlgItem(IDC_TT_STATIC_PLATFORM)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_TT_PLATFORM)->ShowWindow(SW_HIDE);
    }

    SetButtons();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropertiesDialog::OnSelchangePlatform() 
{
    SetButtons();
}

void CPropertiesDialog::SetButtons()
{
  CString strTarget;
  GetDlgItemText(IDC_TT_PLATFORM,strTarget);
  static const int arIDs[]={IDC_TT_DOWNLOADTIMEOUT,IDC_TT_SPIN4,IDC_TT_DOWNLOADTIMEOUT_COMBO};
  for(int i=0;i<sizeof arIDs/sizeof arIDs[0];i++){
    GetDlgItem(arIDs[i]) ->EnableWindow(true);
  }
  //GetDlgItem(IDC_TT_SETTINGS)->EnableWindow(!bSim || !((CButton *)GetDlgItem(IDC_TT_RADIO_LOCAL))->GetCheck());
  
  bool b=(TIMEOUT_SPECIFIED==((CComboBox*)GetDlgItem(IDC_TT_TIMEOUT_COMBO))->GetCurSel());
  GetDlgItem(IDC_TT_TESTTIMEOUT)->EnableWindow(b);
  GetDlgItem(IDC_TT_SPIN3)->EnableWindow(b);
  
  b=(TIMEOUT_SPECIFIED==((CComboBox*)GetDlgItem(IDC_TT_DOWNLOADTIMEOUT_COMBO))->GetCurSel());
  GetDlgItem(IDC_TT_DOWNLOADTIMEOUT)->EnableWindow(b);
  GetDlgItem(IDC_TT_SPIN4)->EnableWindow(b);
}

void CPropertiesDialog::OnSettings() 
{
    if(((CButton *)GetDlgItem(IDC_TT_RADIO_REMOTE))->GetCheck()){
        CRemotePropertiesDialog dlg;	
        dlg.m_strResourceHost=m_strResourceHost;
        dlg.m_nResourcePort=m_nResourcePort;
        dlg.m_strRemoteHost=m_strRemoteHost;
        dlg.m_nRemotePort=m_nRemotePort;
        dlg.m_bFarmed=m_bFarmed;
        if(IDOK==dlg.DoModal()){
            m_bConnectionModified=true;

            m_strResourceHost=dlg.m_strResourceHost;
            m_nResourcePort=dlg.m_nResourcePort;
            m_strRemoteHost=dlg.m_strRemoteHost;
            m_nRemotePort=dlg.m_nRemotePort;
            m_bFarmed=dlg.m_bFarmed;
        }
    } else {
        CLocalPropertiesDialog dlg(m_bHideRemoteControls);	
        dlg.m_bSerial=m_bSerial;
        dlg.m_strPort=m_strPort;
        dlg.m_nBaud=m_nBaud;
        dlg.m_strLocalTCPIPHost=m_strLocalTCPIPHost;
        dlg.m_nLocalTCPIPPort=m_nLocalTCPIPPort;
        dlg.m_nReset=m_nReset;
        dlg.m_strPort=m_strPort;
        if(IDOK==dlg.DoModal()){
            m_bConnectionModified=true;

            m_bSerial=dlg.m_bSerial;
            m_strPort=dlg.m_strPort;
            m_nBaud=dlg.m_nBaud;
            m_strPort=dlg.m_strPort;
            m_strLocalTCPIPHost=dlg.m_strLocalTCPIPHost;
            m_nLocalTCPIPPort=dlg.m_nLocalTCPIPPort;
            m_nReset=dlg.m_nReset;
            m_strReset=dlg.m_strReset;
        }
    }
}

void CPropertiesDialog::OnCancel() 
{
    if(!m_bConnectionModified || IDYES==MessageBox(_T("This will discard any changes that may have been made to connection settings.  Do you still wish to cancel?"),NULL,MB_YESNO|MB_DEFBUTTON2)){
    	CeCosDialog::OnCancel();
    }
}

void CPropertiesDialog::OnSelchangeDownloadtimeoutCombo() 
{
    SetButtons();	
}

void CPropertiesDialog::OnSelchangeTimeoutCombo() 
{
    SetButtons();	
}

