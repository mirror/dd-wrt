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
// BuildOptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "BuildOptionsDialog.h"
#include "CTUtils.h"

#define INCLUDEFILE "cdl.hxx"
#include "IncludeSTL.h"
#define INCLUDEFILE "flags.hxx"
#include "IncludeSTL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuildOptionsDialog dialog

CBuildOptionsDialog::CBuildOptionsDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(CBuildOptionsDialog::IDD, pParent),
  m_hCurrent(0),
  arEntries(CConfigTool::GetConfigToolDoc()->BuildInfo().entries)
{
	//{{AFX_DATA_INIT(CBuildOptionsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBuildOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuildOptionsDialog)
	DDX_Control(pDX, IDC_BUILD_OPTIONS_LIST, m_List);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuildOptionsDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CBuildOptionsDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuildOptionsDialog message handlers
void CBuildOptionsDialog::Create(CConfigItem *pti,HTREEITEM hParent)
{
  if(pti->IsPackage()||TVI_ROOT==hParent){
	  HTREEITEM h=m_Tree.InsertItem(pti->ItemNameOrMacro(),hParent);
	  m_Tree.SetItemData(h,(DWORD)pti);
    m_Tree.SetItemImage(h,18,18);
	  for(CConfigItem *pChild=pti->FirstChild();pChild;pChild=pChild->NextSibling()){
		  Create(pChild,h);
	  }
  }
}

BOOL CBuildOptionsDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();
  m_il.Create(IDB_BITMAP2,16,1,RGB(0,128,128));
	m_Tree.SetImageList(&m_il,TVSIL_NORMAL);
  Create(CConfigTool::GetConfigToolDoc()->FirstItem(), TVI_ROOT);	
  ((CComboBox *)GetDlgItem(IDC_COMBO1))->SetCurSel(0);
  HTREEITEM h=m_Tree.GetFirstVisibleItem();
  m_Tree.Expand(h,TVE_EXPAND);
  m_Tree.SetItemState(h,TVIS_SELECTED,TVIS_SELECTED);
  m_Tree.SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBuildOptionsDialog::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  m_hCurrent=pNMTreeView->itemNew.hItem;
	Redisplay(m_hCurrent);
  *pResult = 0;
}

void CBuildOptionsDialog::OnSelchangeCombo() 
{
  if(m_hCurrent){
    Redisplay(m_hCurrent);
  }
}

void CBuildOptionsDialog::Redisplay(HTREEITEM h)
{
  CConfigItem *pti=(CConfigItem *)m_Tree.GetItemData(h);
	const CdlValuable valuable = pti->GetCdlValuable();
  std::string name;
  const CdlBuildInfo_Loadable *pe=NULL;
  if(valuable){
    const char *pszname=valuable->get_name().c_str();
    for(EntriesArray::size_type j=0;j<arEntries.size();j++){
      if(0==strcmp(arEntries[j].name.c_str(),pszname)){
        pe=&arEntries[j];
        break;
      }
    }
  }
  CString strCat;
  GetDlgItemText(IDC_COMBO1,strCat);
  const CString strFlags=get_flags(CConfigTool::GetConfigToolDoc()->GetCdlConfig(),pe,CUtils::UnicodeToStdStr(strCat)).c_str();
  CStringArray ar;
  CUtils::Chop(strFlags,ar,_TCHAR(' '),false,false);
  CString strEdit;
  bool bRedraw=(m_List.GetCount()!=ar.GetSize());
  if(!bRedraw){
    for(int i=0;i<ar.GetSize();i++){
      CString strOld;
      m_List.GetText(i,strOld);
      if(strOld!=ar[i]){
        bRedraw=true;
        break;
      }
    }
  }
  if(bRedraw){
    m_List.ResetContent();
    for(int i=0;i<ar.GetSize();i++){
      m_List.AddString(ar[i]);
    }
  }
}
