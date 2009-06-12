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
// ViewOptions.cpp : implementation file
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
#include "ViewOptions.h"
#include "ConfigToolDoc.h"
#include "CellView.h"
#include "CTUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewOptions property page

CViewOptions::CViewOptions() 
	: CeCosDialog(IDD, NULL)
{
  //{{AFX_DATA_INIT(CViewOptions)
  //}}AFX_DATA_INIT
  m_FontChosen=CMainFrame::PaneTypeMax;
}

CViewOptions::~CViewOptions()
{
}

void CViewOptions::DoDataExchange(CDataExchange* pDX)
{
  CeCosDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CViewOptions)
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewOptions, CeCosDialog)
//{{AFX_MSG_MAP(CViewOptions)
ON_BN_CLICKED(IDC_RADIO_ASSOCIATED, OnRadioAssociated)
ON_BN_CLICKED(IDC_RADIO_CUSTOM, OnRadioCustom)
ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
ON_BN_CLICKED(IDC_RADIO_INTERNAL_BROWSER, OnRadioInternalBrowser)
ON_BN_CLICKED(IDC_RADIO_ASSOCIATED_BROWSER, OnRadioAssociatedBrowser)
ON_BN_CLICKED(IDC_RADIO_CUSTOM_BROWSER, OnRadioCustomBrowser)
ON_BN_CLICKED(IDC_BROWSE_BROWSER, OnBrowseBrowser)
ON_BN_CLICKED(IDC_FONT, OnFont)
ON_CBN_SELCHANGE(IDC_PANECOMBO, OnSelchangePanecombo)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewOptions message handlers

const CFileName CViewOptions::strHeaderfileAssociation=CViewOptions::HeaderFileAssociation();

void CViewOptions::OnOK() 
{
  
  UpdateData(TRUE);
  CMainFrame *pMain=CConfigTool::GetMain();
  if(CMainFrame::PaneTypeMax!=m_FontChosen){
    pMain->SetPaneFont(m_FontChosen, m_lf);
  }
  
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  
  
  if(((CButton *)GetDlgItem(IDC_RADIO_CUSTOM_BROWSER))->GetCheck()){
    CFileName strBrowser;
    GetDlgItemText(IDC_EDIT_BROWSER,strBrowser);
    if(strBrowser.IsEmpty() || !strBrowser.Exists()){
      CUtils::MessageBoxF(_T("If you wish to use a custom browser you must specify a valid executable"));
      GetDlgItem(IDC_EDIT_BROWSER)->SetFocus();
      
      return;
    }
    pDoc->m_strBrowser=strBrowser;
    pDoc->m_eUseCustomBrowser=CConfigToolDoc::CustomExternal;
  } else if (((CButton *)GetDlgItem(IDC_RADIO_ASSOCIATED_BROWSER))->GetCheck()){
    pDoc->m_eUseCustomBrowser=CConfigToolDoc::AssociatedExternal;
  } else {
    pDoc->m_eUseCustomBrowser=CConfigToolDoc::Internal;
  }

  pDoc->m_bUseCustomViewer=((CButton *)GetDlgItem(IDC_RADIO_CUSTOM))->GetCheck();
  if(pDoc->m_bUseCustomViewer){
    CFileName strViewer;
    GetDlgItemText(IDC_EDIT,strViewer);
    if(strViewer.IsEmpty() || !strViewer.IsFile()){
		    CUtils::MessageBoxF(_T("If you wish to use a custom viewer you must specify a valid executable"));
        GetDlgItem(IDC_EDIT)->SetFocus();
        
        return;
    }
    pDoc->m_strViewer=strViewer;
  }
  
  bool bHex=((CButton *)GetDlgItem(IDC_RADIO_HEXADECIMAL))->GetCheck();
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

void CViewOptions::OnRadioAssociated() 
{
  	
  GetDlgItem(IDC_EDIT)->EnableWindow(FALSE);
  GetDlgItem(IDC_BROWSE)->EnableWindow(FALSE);
}

void CViewOptions::OnRadioCustom() 
{
  	
  GetDlgItem(IDC_EDIT)->EnableWindow(TRUE);
  GetDlgItem(IDC_BROWSE)->EnableWindow(TRUE);
}

void CViewOptions::OnBrowse() 
{
  CFileDialog dlg(TRUE, _T("exe"), _T(""), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    _T("Executables (*.exe;*.bat;*.com)|*.exe;*.bat;*.com"));
  if(IDOK==dlg.DoModal()){
    SetDlgItemText(IDC_EDIT,dlg.GetPathName());
  }
}

BOOL CViewOptions::OnInitDialog() 
{
  CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_PANECOMBO);
  CMainFrame *pMain=CConfigTool::GetMain();
  for(int i=0;i<CMainFrame::PaneTypeMax;i++){
    if(i==CMainFrame::Control || i==CMainFrame::Output || i==CMainFrame::ShortDesc){
      int nIndex=pCombo->AddString(pMain->GetPaneName((CMainFrame::PaneType)i));
      pCombo->SetItemData(nIndex,i);
    }
  }
  pCombo->SetCurSel(0);
  
  CeCosDialog::OnInitDialog();
  
  if(!strHeaderfileAssociation.IsEmpty()){
    CString str1;
    str1.Format(_T("%s (%s)"),_T("Associated viewer"),strHeaderfileAssociation);
    SetDlgItemText(IDC_RADIO_ASSOCIATED,str1);
  } else {
    SetDlgItemText(IDC_RADIO_ASSOCIATED,_T("Associated viewer"));
  }
  
  // Compute the association with doc (.htm) files
  
  if(CConfigToolDoc::DefaultExternalBrowser().IsEmpty()){
    SetDlgItemText(IDC_RADIO_ASSOCIATED_BROWSER,_T("Associated browser"));
  } else {
    CString str1;
    str1.Format(_T("%s (%s)"),_T("Associated browser"),CConfigToolDoc::DefaultExternalBrowser().Tail());
    SetDlgItemText(IDC_RADIO_ASSOCIATED_BROWSER,str1);
  }

  CConfigToolDoc*pDoc=CConfigTool::GetConfigToolDoc();
  if(strHeaderfileAssociation.IsEmpty()){
    GetDlgItem(IDC_RADIO_ASSOCIATED)->EnableWindow(FALSE);
    GetDlgItem(IDC_BROWSE)->EnableWindow(TRUE);
    ((CButton *)GetDlgItem(IDC_RADIO_CUSTOM))->SetCheck(TRUE);
  } else {
    ((CButton *)GetDlgItem(pDoc->m_bUseCustomViewer?IDC_RADIO_CUSTOM:IDC_RADIO_ASSOCIATED))->SetCheck(TRUE);
    GetDlgItem(IDC_EDIT)->EnableWindow(pDoc->m_bUseCustomViewer);
    GetDlgItem(IDC_BROWSE)->EnableWindow(pDoc->m_bUseCustomViewer);
  }
  SetDlgItemText(IDC_EDIT,pDoc->m_strViewer);
  
  if(CConfigToolDoc::DefaultExternalBrowser().IsEmpty()){
    GetDlgItem(IDC_RADIO_ASSOCIATED_BROWSER)->EnableWindow(FALSE);
    GetDlgItem(IDC_BROWSE_BROWSER)->EnableWindow(TRUE);
    if(CConfigToolDoc::AssociatedExternal==pDoc->m_eUseCustomBrowser){
      pDoc->m_eUseCustomBrowser=CConfigToolDoc::CustomExternal;
    }
  }
  
  switch(pDoc->m_eUseCustomBrowser){
		case CConfigToolDoc::Internal:
      ((CButton *)GetDlgItem(IDC_RADIO_INTERNAL_BROWSER))->SetCheck(TRUE);
      break;
    case CConfigToolDoc::AssociatedExternal:
      ((CButton *)GetDlgItem(IDC_RADIO_ASSOCIATED_BROWSER))->SetCheck(TRUE);
      break;
    case CConfigToolDoc::CustomExternal:
      ((CButton *)GetDlgItem(IDC_RADIO_CUSTOM_BROWSER))->SetCheck(TRUE);
      GetDlgItem(IDC_EDIT_BROWSER)->EnableWindow(TRUE);
      GetDlgItem(IDC_BROWSE_BROWSER)->EnableWindow(TRUE);
      break;
    default:
      ASSERT(FALSE);
  }
  
  SetDlgItemText(IDC_EDIT_BROWSER,pDoc->m_strBrowser);
  
  ((CButton *)GetDlgItem(IDC_RADIO_DECIMAL))->SetCheck(!pDoc->m_bHex);
  ((CButton *)GetDlgItem(IDC_RADIO_DESCRIPTIVE_NAMES))->SetCheck(!pDoc->m_bMacroNames);
  ((CButton *)GetDlgItem(IDC_RADIO_HEXADECIMAL))->SetCheck(pDoc->m_bHex);
  ((CButton *)GetDlgItem(IDC_RADIO_MACRO_NAMES))->SetCheck(pDoc->m_bMacroNames);

  UpdateData(FALSE);
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CViewOptions::OnRadioInternalBrowser() 
{
  	
  GetDlgItem(IDC_EDIT_BROWSER)->EnableWindow(FALSE);
  GetDlgItem(IDC_BROWSE_BROWSER)->EnableWindow(FALSE);
}

void CViewOptions::OnRadioAssociatedBrowser() 
{
  	
  GetDlgItem(IDC_EDIT_BROWSER)->EnableWindow(FALSE);
  GetDlgItem(IDC_BROWSE_BROWSER)->EnableWindow(FALSE);
}

void CViewOptions::OnRadioCustomBrowser() 
{
  	
  GetDlgItem(IDC_EDIT_BROWSER)->EnableWindow(TRUE);
  GetDlgItem(IDC_BROWSE_BROWSER)->EnableWindow(TRUE);
}

void CViewOptions::OnBrowseBrowser() 
{
  CFileDialog dlg(TRUE, _T("exe"), _T(""), OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    _T("Executables (*.exe;*.bat;*.com)|*.exe;*.bat;*.com"));
  if(IDOK==dlg.DoModal()){
    SetDlgItemText(IDC_EDIT_BROWSER,dlg.GetPathName());
  }
  
}

void CViewOptions::OnFont() 
{
  LOGFONT lf;
  CMainFrame *pMain=CConfigTool::GetMain();
  CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_PANECOMBO);
  int nSel=pCombo->GetCurSel();
  CMainFrame::PaneType pane=(CMainFrame::PaneType)pCombo->GetItemData(nSel);
  pMain->GetPaneFont(pane).GetLogFont(&lf);
  CFontDialog dlg(&lf);
  if(IDOK==dlg.DoModal()){
    
    m_FontChosen=pane;
    memcpy(&m_lf,dlg.m_cf.lpLogFont, sizeof LOGFONT);
  }
}

void CViewOptions::OnSelchangePanecombo() 
{
  if(CMainFrame::PaneTypeMax!=m_FontChosen){
    CString str;
    ((CComboBox *)GetDlgItem(IDC_PANECOMBO))->GetLBText(m_FontChosen,str);
    if(IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("Apply font changes to %s pane?"),(LPCTSTR)str)){
      CMainFrame *pMain=CConfigTool::GetMain();
      pMain->SetPaneFont(m_FontChosen, m_lf);
    }
    m_FontChosen=CMainFrame::PaneTypeMax;
  }
}

CFileName CViewOptions::HeaderFileAssociation()
{
  // Compute the association with header (.h) files
  const CFileName strFile(CFileName::GetTempPath()+_T("tmp.h"));
  CFileName strExe;
  CFile f;
  if(f.Open(strFile,CFile::modeCreate|CFile::modeWrite)){
    f.Close();
    bool rc=((int)FindExecutable(strFile,_T("."),strExe.GetBuffer(MAX_PATH))>32);
    strExe.ReleaseBuffer();
    if(rc){
      strExe=strExe.Tail();
    } else {
      strExe=_T("");
    }
    ::DeleteFile(strFile);
  }
  return strExe;

}
