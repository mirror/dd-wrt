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
// PlatformsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "testtool.h"
#include "PLatformsDialog.h"
#include "PlatformDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlatformsDialog dialog


CPlatformsDialog::CPlatformsDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(CPlatformsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlatformsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPlatformsDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlatformsDialog)
	DDX_Control(pDX, IDC_TT_PLATFORM_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlatformsDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CPlatformsDialog)
	ON_BN_CLICKED(IDC_TT_ADD_PLATFORM, OnAddPlatform)
	ON_BN_CLICKED(IDC_TT_DELETE_PLATFORM, OnDeletePlatform)
	ON_NOTIFY(NM_DBLCLK, IDC_TT_PLATFORM_LIST, OnDblclkPlatformList)
	ON_BN_CLICKED(IDC_TT_MODIFY_PLATFORM, OnModifyPlatform)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TT_PLATFORM_LIST, OnKeydownPlatformList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TT_PLATFORM_LIST, OnItemchangedPlatformList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlatformsDialog message handlers
const LPCTSTR CPlatformsDialog::arpszTypes[]={
  _T("Hardware with breakpoint support"),
  _T("Simulator"),
  _T("Synthetic target"),
  _T("Hardware without breakpoint support"),
  _T("Remote simulator")
};

BOOL CPlatformsDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();

	ListView_SetExtendedListViewStyle(m_List.GetSafeHwnd(),LVS_EX_FULLROWSELECT);

	m_List.InsertColumn(0,_T("Target"));
  m_List.InsertColumn(1,_T("Prefix"));
  m_List.InsertColumn(2,_T("Commands"));
  m_List.InsertColumn(3,_T("Inferior"));
  m_List.InsertColumn(4,_T("Prompt"));
  m_List.InsertColumn(5,_T("ServerSideGdb"));
  for(unsigned int i=0;i<CeCosTestPlatform::Count();i++){
    Add(*CeCosTestPlatform::Get(i));
  }
  CRect rect;
  m_List.GetClientRect(rect);
  m_List.SetColumnWidth(0,rect.Width()/6);
  m_List.SetColumnWidth(1,rect.Width()/6);
  m_List.SetColumnWidth(2,rect.Width()/6);
  m_List.SetColumnWidth(3,rect.Width()/6);
  m_List.SetColumnWidth(4,rect.Width()/6);
  m_List.SetColumnWidth(5,rect.Width()/6);
	
	bool bSel=(NULL!=m_List.GetFirstSelectedItemPosition());
	GetDlgItem(IDC_TT_MODIFY_PLATFORM)->EnableWindow(bSel);
	GetDlgItem(IDC_TT_DELETE_PLATFORM)->EnableWindow(bSel);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlatformsDialog::OnAddPlatform() 
{
  CPlatformDialog dlg;
  if(IDOK==dlg.DoModal()){
    LVFINDINFO fi;
    fi.flags=LVFI_STRING;
    fi.psz=dlg.m_strPlatform;
    if(-1==m_List.FindItem(&fi)){
      Add(CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior));  
    } else {
      MessageBox(_T("That platform name is already in use."));
    }
  }
}

void CPlatformsDialog::OnDeletePlatform() 
{
  POSITION pos = m_List.GetFirstSelectedItemPosition();
  if(pos){
    int nIndex= m_List.GetNextSelectedItem(pos);
    
    if(IDYES==MessageBox(_T("Are you sure you wish to delete this platform?"),_T("Delete"),MB_YESNO)){
      delete Platform(nIndex);
      m_List.DeleteItem(nIndex);
      m_arTargetInfo.RemoveAt(nIndex);
    }
  }
}

void CPlatformsDialog::Add(const CeCosTestPlatform &ti)
{
  int i=m_List.GetItemCount();
  m_List.InsertItem(i,ti.Name());
  m_List.SetItemText(i,1,ti.Prefix());
  m_List.SetItemText(i,2,ti.GdbCmds());
  m_List.SetItemText(i,3,ti.Inferior());
  m_List.SetItemText(i,4,ti.Prompt());
  m_List.SetItemText(i,5,ti.ServerSideGdb()?_T("y"):_T("n"));
  m_arTargetInfo.Add(new CeCosTestPlatform(ti));
}

void CPlatformsDialog::OnDblclkPlatformList(NMHDR*, LRESULT* pResult) 
{
  OnModifyPlatform();
	*pResult = 0;
}

void CPlatformsDialog::OnModifyPlatform() 
{
  POSITION pos = m_List.GetFirstSelectedItemPosition();
  if(pos){
    int nIndex= m_List.GetNextSelectedItem(pos);
    CeCosTestPlatform *pti=Platform(nIndex);
    CPlatformDialog dlg;
    dlg.m_strPlatform=pti->Name();
    dlg.m_strPrefix=pti->Prefix();
    dlg.m_strGDB=pti->GdbCmds();
    dlg.m_strCaption=_T("Modify");
    dlg.m_strPrompt=pti->Prompt();
    dlg.m_bServerSideGdb=pti->ServerSideGdb();
    dlg.m_strInferior=pti->Inferior();
    if(IDCANCEL!=dlg.DoModal()){
      *pti=CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior);
      m_List.SetItemText(nIndex,1,pti->Prefix());
      m_List.SetItemText(nIndex,2,pti->GdbCmds());
      m_List.SetItemText(nIndex,3,pti->Inferior());
      m_List.SetItemText(nIndex,4,pti->Prompt());
      m_List.SetItemText(nIndex,5,pti->ServerSideGdb()?_T("Y"):_T("N"));
    }
  }
}

void CPlatformsDialog::OnKeydownPlatformList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
  if(VK_DELETE==pLVKeyDow->wVKey){
    OnDeletePlatform();
  }
	*pResult = 0;
}

void CPlatformsDialog::OnItemchangedPlatformList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	bool bSel=(NULL!=m_List.GetFirstSelectedItemPosition());
	GetDlgItem(IDC_TT_MODIFY_PLATFORM)->EnableWindow(bSel);
	GetDlgItem(IDC_TT_DELETE_PLATFORM)->EnableWindow(bSel);
	*pResult = 0;
}
