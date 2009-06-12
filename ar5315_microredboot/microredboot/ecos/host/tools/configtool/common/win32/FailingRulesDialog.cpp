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
// FailingRulesDialog.cpp : implementation file
//

#include "stdafx.h"
#ifndef PLUGIN
#include "BCMenu.h"
#endif
#include "FailingRulesDialog.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "ControlView.h"
#include "CTUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFailingRulesDialog dialog


CFailingRulesDialog::CFailingRulesDialog(std::list<CdlConflict> conflicts,CdlTransaction transaction/*=NULL*/,CPtrArray *parConflictsOfInterest/*=NULL*/)
	: CeCosDialog(CFailingRulesDialog::IDD, NULL),
	m_conflicts(conflicts),
  m_Transaction(transaction),
  m_parConflictsOfInterest(parConflictsOfInterest)
{
  for (std::list<CdlConflict>::const_iterator conf_i= m_conflicts.begin (); conf_i != m_conflicts.end (); conf_i++) { // for each conflict
    int nSolutions=(*conf_i)->get_solution().size();
    SolutionInfo *pInfo=(SolutionInfo *)malloc(sizeof(SolutionInfo)+(nSolutions-1)*sizeof(int));
    pInfo->nCount=nSolutions;
    for(int i=0;i<nSolutions;i++){
      pInfo->arItem[i]=SolutionInfo::CHECKED;
    }
    m_Map.SetAt(*conf_i,pInfo);
  }

	//{{AFX_DATA_INIT(CFailingRulesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CFailingRulesDialog::~CFailingRulesDialog()
{
  for(POSITION pos = m_Map.GetStartPosition(); pos != NULL; ){
    CdlConflict conflict;
    SolutionInfo *pInfo=NULL;
    m_Map.GetNextAssoc(pos, (void *&)conflict, (void *&)pInfo);
    free(pInfo);
  }
}

void CFailingRulesDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFailingRulesDialog)
	DDX_Control(pDX, IDC_LIST2, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFailingRulesDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CFailingRulesDialog)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_CONFLICTS_NONE, OnConflictsNone)
	ON_BN_CLICKED(ID_RESOLVE_CONFLICTS_CONTINUE, OnOK)
  ON_BN_CLICKED(ID_RESOLVE_CONFLICTS_CANCEL,OnCancel)
  ON_COMMAND(ID_LOCATE,OnLocate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFailingRulesDialog message handlers

BOOL CFailingRulesDialog::OnInitDialog() 
{
  CeCosDialog::OnInitDialog();
  m_List.SetExtendedStyle(LVS_EX_CHECKBOXES);
  m_List.InsertColumn(0,_T("Item"));
  m_List.InsertColumn(1,_T("Value"));
  
  CRect rect;
  GetDlgItem(IDC_LIST1)->GetWindowRect(rect);
  ScreenToClient(rect);

  m_RulesList.CreateEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,WS_VISIBLE|WS_CHILD|LVS_SHOWSELALWAYS,rect,this,IDC_LIST1);
  // Select the first item and fill the solution set
  m_RulesList.AddConflicts(m_conflicts);

  if(m_parConflictsOfInterest && m_parConflictsOfInterest->GetSize()>0){  
    CPtrArray &arConflictsOfInterest=*m_parConflictsOfInterest;
    for(int i=m_RulesList.GetItemCount()-1;i>=0;--i){
      for(int j=arConflictsOfInterest.GetSize()-1;j>=0;--j){
        CdlConflict conflict=(CdlConflict)m_RulesList.GetItemData(i);
        if(arConflictsOfInterest[j]==conflict){
          m_RulesList.SetItemState(i,LVIS_SELECTED, LVIS_SELECTED);
          m_RulesList.EnsureVisible(i,false);
          arConflictsOfInterest.RemoveAt(j);
          break;
        }
      }
    }
  } else {
    for(int i=m_RulesList.GetItemCount()-1;i>=0;--i){
      m_RulesList.SetItemState(i,LVIS_SELECTED, LVIS_SELECTED);
    }
  }
  SetButtons();
	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFailingRulesDialog::OnCancel() 
{
  CeCosDialog::OnCancel();
}

void CFailingRulesDialog::OnOK() 
{
  // Ensure we have the current conflict check array
  for(POSITION pos = m_RulesList.GetFirstSelectedItemPosition();pos;){
    int nItem = m_RulesList.GetNextSelectedItem(pos);
    RemoveConflictSolutions((CdlConflict)m_RulesList.GetItemData(nItem));
  }

  // Dismiss the window
  CeCosDialog::OnOK();

  for (std::list<CdlConflict>::const_iterator conf_i= m_conflicts.begin (); conf_i != m_conflicts.end (); conf_i++) { // for each conflict
    CdlConflict conflict=*conf_i;
    //int nSolutions=conflict->get_solution().size();
    SolutionInfo &info=Info(conflict);
    int nIndex=0;
    const std::vector<std::pair<CdlValuable, CdlValue> >&Solution=conflict->get_solution();
    for (std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator soln_i = Solution.begin();soln_i != Solution.end(); soln_i++) {
      if(SolutionInfo::CHECKED==info.arItem[nIndex++]){
        CdlValuable valuable  = soln_i->first;
        CdlValue value=soln_i->second;
        CdlValueFlavor flavor = valuable->get_flavor();
        const CString strName(valuable->get_name().c_str());
        const CString strValue(value.get_value().c_str());
        bool rc=true;
        CString str;
        try {
          switch(flavor) {
            case CdlValueFlavor_None :
              str=_T("set CdlValueFlavor_None");
              rc=false;
              break;
            case CdlValueFlavor_Bool :
              str.Format(_T("%s %s\n"),value.is_enabled()?_T("disable"):_T("enable"),strName);
              valuable->set_enabled (m_Transaction, value.is_enabled(), CdlValueSource_User);
              break;
            case CdlValueFlavor_BoolData :
              {
                bool bEnabled=value.is_enabled();
                str.Format(_T("%s %s and set value to %s\n"),bEnabled?_T("disable"):_T("enable"),strName,strValue);
                // This is wrong: it should set the NEW value. This is the cause of a long-standing bug...
                // CdlSimpleValue simple_value = valuable->get_simple_value ();
                //valuable->set_enabled_and_value (m_Transaction, bEnabled, simple_value, CdlValueSource_User);
                valuable->set_enabled_and_value (m_Transaction, bEnabled, CUtils::UnicodeToStdStr (strValue), CdlValueSource_User);

              }
              break;
            case CdlValueFlavor_Data :
              str.Format(_T("set %s to %s\n"),strName,strValue);
              valuable->set_value (m_Transaction, CUtils::UnicodeToStdStr (strValue), CdlValueSource_User);
              break;
          }
        }
        catch(...){
          rc=false;
        }
        if(rc){
          CConfigTool::GetConfigToolDoc()->SetModifiedFlag();
        } else {
          CUtils::MessageBoxF(_T("Failed to %s\n"),str);
        }
      }
    }
  }
}

void CFailingRulesDialog::SetAll(bool bOnOff)
{
  for(int i=m_List.GetItemCount()-1;i>=0;--i){
    m_List.SetCheck(i,bOnOff);
  }
}

void CFailingRulesDialog::OnReset()
{
  SetAll(true);
}

void CFailingRulesDialog::OnConflictsNone() 
{
  SetAll(false);
}

BOOL CFailingRulesDialog::OnItemChanged(UINT, LPNMLISTVIEW pnmv, LRESULT* pResult) 
{
  bool bWasSelected=(pnmv->uOldState & LVIS_SELECTED);
  bool bIsSelected =(pnmv->uNewState & LVIS_SELECTED);
  
  if(bWasSelected != bIsSelected) {
    CdlConflict conflict=(CdlConflict) m_RulesList.GetItemData(pnmv->iItem);
    if(bIsSelected){
      if(1==m_List.GetSelectedCount()){
        GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
        m_List.ShowWindow(SW_SHOW);
      }
      AddConflictSolutions(conflict);
    } else {
      RemoveConflictSolutions(conflict);
    }
  }
  *pResult = 0;
  return false;
}

// We need to use this because the OnItemChanged handler successive receives "not selected" followed by "selected"
// notifications.  The result is that the "Select one or more conflicts to display available solutions" message
// would be seen briefly when clicking from one selection to another.
BOOL CFailingRulesDialog::OnClick(UINT,LPNMLISTVIEW pnmv, LRESULT* pResult) 
{
  if(-1==pnmv->iItem && 0==m_List.GetSelectedCount()){
    SetDlgItemText(IDC_STATIC1,_T("Select one or more conflicts to display available solutions"));
    m_List.ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_RESET)->EnableWindow(false);
    GetDlgItem(IDC_CONFLICTS_NONE)->EnableWindow(false);
  }
  *pResult = 0;
  return false; // not handled
}

void CFailingRulesDialog::RemoveConflictSolutions(CdlConflict conflict)
{
  SolutionInfo &info=Info(conflict);
  for(int i=0;i<info.nCount;i++){
    int nItem=info.arItem[i];
    ASSERT(nItem>=0);
    info.arItem[i]=(1==m_List.GetCheck(nItem)?SolutionInfo::CHECKED:SolutionInfo::UNCHECKED);
    int nRefs=m_List.GetItemData(nItem);
    if(1==nRefs){
      m_List.DeleteItem(nItem);
      for(int i=0;i<m_RulesList.GetItemCount();i++){
        SolutionInfo &info=Info((CdlConflict)m_RulesList.GetItemData(i));
        for(int j=0;j<info.nCount;j++){
          if(info.arItem[j]>nItem){
            info.arItem[j]--;
          }
        }
      }
    } else {
      m_List.SetItemData(nItem,nRefs-1);
    }
  }
}

void CFailingRulesDialog::AddConflictSolutions(CdlConflict conflict)
{
  SolutionInfo &info=Info(conflict);

  const std::vector<std::pair<CdlValuable, CdlValue> >&Solution=conflict->get_solution();

  int i=0;
  for (std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator soln_i = Solution.begin();
       soln_i != Solution.end(); soln_i++) {
    CdlValuable valuable  = soln_i->first;
    CdlValue value=soln_i->second;
    CdlValueFlavor flavor = valuable->get_flavor();

    CString strValue;
    switch(flavor) {
      case CdlValueFlavor_None :
        break;
      case CdlValueFlavor_Bool :
        strValue=value.is_enabled() ? _T("Enabled") : _T("Disabled");
        break;
      case CdlValueFlavor_BoolData :
        strValue.Format(_T("%s, %s"), value.is_enabled() ? _T("Enabled") : _T("Disabled"), CString(value.get_value().c_str()));
        break;
      case CdlValueFlavor_Data :
        strValue=value.get_value().c_str();
        break;
    }
    
    const CString strName(soln_i->first->get_name().c_str());
    LVFINDINFO fi;
    fi.flags=LVFI_STRING;
    fi.psz=strName;
    int nIndex=m_List.FindItem(&fi);
    if(-1==nIndex || strValue!=m_List.GetItemText(nIndex,1)){
      // We don't have an existing solution that matches this one
      nIndex=m_List.GetItemCount();
      m_List.InsertItem(nIndex,strName);
      m_List.SetItemData(nIndex,1);
      m_List.SetItemText(nIndex,1,strValue);
      ASSERT(info.arItem[i]<0);
      m_List.SetCheck(nIndex,SolutionInfo::CHECKED==info.arItem[i]);
    } else {
      // We do - to avoid duplicates, increment the "ref count"
      m_List.SetItemData(nIndex,m_List.GetItemData(nIndex)+1);
    }
    info.arItem[i++]=nIndex; 
  }
  if(0==i){
    SetDlgItemText(IDC_STATIC1,_T("No solution is available for this conflict"));
    m_List.ShowWindow(SW_HIDE);
  } else {
    SetDlgItemText(IDC_STATIC1,_T("Proposed solution:"));
    m_List.ShowWindow(SW_SHOW);
    m_List.SetColumnWidth(0,LVSCW_AUTOSIZE);
    CRect rect;
    m_List.GetClientRect(rect);
    m_List.SetColumnWidth(1,rect.Width()-m_List.GetColumnWidth(0));
  }
}

BOOL CFailingRulesDialog::OnSolutionItemChanged(UINT,LPNMLISTVIEW, LRESULT*) 
{
  SetButtons();
  return false; // not handled
}

CFailingRulesDialog::SolutionInfo & CFailingRulesDialog::Info(const CdlConflict conflict)
{
  SolutionInfo *pInfo;   
  VERIFY(m_Map.Lookup(conflict,(void *&)pInfo));
  return *pInfo;
}

BOOL CFailingRulesDialog::OnRClick(UINT, LPNMITEMACTIVATE pnmv, LRESULT* pResult) 
{
  DWORD dwPos=GetMessagePos();
  CPoint pt(GET_X_LPARAM(dwPos),GET_Y_LPARAM(dwPos));
  m_nContextItem=pnmv->iItem;
  m_nContextRow=pnmv->iSubItem;
  if(-1!=m_nContextItem){
    //m_RulesList.SetItemState(m_nContextItem,LVIS_SELECTED,LVIS_SELECTED);
  	Menu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(1==m_RulesList.GetSelectedCount() && m_RulesList.AssociatedItem(m_nContextItem,m_nContextRow)?MF_STRING:(MF_STRING|MF_GRAYED),ID_LOCATE,_T("&Locate"));
#ifndef PLUGIN
    SuppressNextContextMenuMessage();
#endif
    menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x,pt.y,this);
  }

  *pResult = 0;
  return TRUE; // handled
}

void CFailingRulesDialog::OnLocate()
{
  CConfigItem *pItem=m_RulesList.AssociatedItem(m_nContextItem,m_nContextRow);
  if (pItem) {
    CConfigTool::GetControlView()->SelectItem(pItem);
  }
}


void CFailingRulesDialog::SetButtons()
{
  int nCheckCount=0;
  int nItemCount=m_List.GetItemCount();
  for(int i=nItemCount-1;i>=0;--i){
    nCheckCount+=m_List.GetCheck(i);
  }
  GetDlgItem(IDC_RESET)->EnableWindow(nItemCount>0 && nCheckCount<nItemCount);
  GetDlgItem(IDC_CONFLICTS_NONE)->EnableWindow(nItemCount>0 && nCheckCount>0);
}

// We have to dispatch our own notify messages because the multiple inheritance of CCSHDialog prevents
// the message map from compiling properly for ON_NOTIFY messages.
BOOL CFailingRulesDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
  LPNMHDR pHdr=(LPNMHDR)lParam;
  bool bHandled=false;
  switch(pHdr->idFrom){
    case IDC_LIST1:
      switch (pHdr->code) {
        case LVN_ITEMCHANGED: 
          bHandled=OnItemChanged(wParam, (LPNMLISTVIEW)lParam, pResult);
          break;
        case NM_CLICK: 
          bHandled=OnClick(wParam, (LPNMLISTVIEW)lParam, pResult);
          break;
        case NM_RCLICK: 
          bHandled=OnRClick(wParam, (LPNMITEMACTIVATE)lParam, pResult);
          break;
        default:
          break;
      }
      break;
    case IDC_LIST2:
      switch (pHdr->code) {
        case LVN_ITEMCHANGED: 
          bHandled=OnSolutionItemChanged(wParam,(LPNMLISTVIEW)lParam, pResult);
          break;
        default:
          break;
      }
      break;
  }
  return bHandled || CeCosDialog::OnNotify(wParam,lParam,pResult);
}
