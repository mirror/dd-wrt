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
// TestViewView.cpp : implementation of the CRulesView class
//

#include "stdafx.h"
#ifndef PLUGIN
#include "BCMenu.h"
#endif
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "ControlView.h"
#include "CTUtils.h"
#include "resource.h"
#include "RulesList.h"
#include "RulesView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRulesView

IMPLEMENT_DYNCREATE(CRulesView, CView)

BEGIN_MESSAGE_MAP(CRulesView, CView)
	//{{AFX_MSG_MAP(CRulesView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
  ON_COMMAND(ID_LOCATE,OnLocate)
  ON_COMMAND(ID_RESOLVE,OnResolve)
  ON_COMMAND(ID_DISABLE_CONFLICT, OnDisable)
  ON_COMMAND(ID_ENABLE_CONFLICT, OnEnable)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulesView construction/destruction

CRulesView::CRulesView():
  m_nContextItem(-1)
{
  CConfigTool::SetRulesView(this);
}

CRulesView::~CRulesView()
{
  CConfigTool::SetRulesView(0);
}

BOOL CRulesView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CRulesView drawing

void CRulesView::OnDraw(CDC* pDC)
{
  UNUSED_ALWAYS(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CRulesView diagnostics

#ifdef _DEBUG
void CRulesView::AssertValid() const
{
	CView::AssertValid();
}

void CRulesView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRulesView message handlers

int CRulesView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRect rect;
  GetClientRect(rect);
  m_List.Create(WS_CHILD|WS_VISIBLE,rect,this,1);

	return 0;
}

void CRulesView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	CRect rect;
  GetClientRect(rect);
  m_List.MoveWindow(rect);
}

void CRulesView::FillRules() 
{
  CdlConfiguration CdlConfig = CConfigTool::GetConfigToolDoc()->GetCdlConfig ();
  if (CdlConfig) { // if configuration information
    int nCount=0;
    bool bRefill=false;
    CMapPtrToWord arMap;
    for(int i=0;i<m_List.GetItemCount();i++){
      arMap.SetAt((void *)m_List.GetItemData(i),(WORD)i);
    }

    WORD w;
    std::list<CdlConflict>::const_iterator conf_i;
    
    const std::list<CdlConflict>& conflicts=CdlConfig->get_all_conflicts();  
    for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) { // for each conflict
      nCount++;
      if(!arMap.Lookup(*conf_i,w)){
        bRefill=true;
        break;
      }
    }
    //for (conf_i = CdlConfig->get_structural_conflicts().begin (); conf_i != CdlConfig->get_structural_conflicts().end (); conf_i++) { // for each conflict
    //  nCount++;
    //  if(!arMap.Lookup(*conf_i,w)){
    //    bRefill=true;
    //    break;
    //  }
    //}
    if(bRefill || nCount!=m_List.GetItemCount()){
      m_List.DeleteAllItems();
      //m_List.AddConflicts(CdlConfig->get_structural_conflicts());
      m_List.AddConflicts(CdlConfig->get_all_conflicts());
    }
  }
}

void CRulesView::OnContextMenu(CWnd*, CPoint pt) 
{
  ScreenToClient(&pt);
  m_nContextItem=m_List.HitTest(pt,NULL);
  LVHITTESTINFO info;
  info.pt=pt;
  m_List.SubItemHitTest(&info);
  m_nContextRow=info.iSubItem;
  if(-1!=m_nContextItem){
    m_List.SetItemState(m_nContextItem,LVIS_SELECTED,LVIS_SELECTED);
  	Menu menu;
    menu.CreatePopupMenu();
    /*
    FIXME: Not yet implemented in CDL
    CdlConflict conflict=(CdlConflict)m_List.GetItemData (m_nContextItem);
    if(conflict->is_enabled()){
      menu.AppendMenu(MF_STRING,ID_DISABLE_CONFLICT,_T("&Disable..."));
    } else {
      menu.AppendMenu(MF_STRING,ID_ENABLE_CONFLICT,_T("&Enable..."));
    }
    */
    menu.AppendMenu(1==m_List.GetSelectedCount() && m_List.AssociatedItem(m_nContextItem,m_nContextRow)?MF_STRING:(MF_STRING|MF_GRAYED),ID_LOCATE,_T("&Locate"));
    menu.AppendMenu(m_List.AssociatedItem(m_nContextItem,m_nContextRow)?MF_STRING:(MF_STRING|MF_GRAYED),ID_RESOLVE,_T("&Resolve"));

    ClientToScreen(&pt);
    menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x,pt.y,this);
  }
}

void CRulesView::OnDisable()
{
  CdlConflict conflict=(CdlConflict)m_List.GetItemData (m_nContextItem);
  if(IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("Are you sure you wish to disable this conflict?"))){
    conflict->disable("");
  }
}

void CRulesView::OnEnable()
{
  CdlConflict conflict=(CdlConflict)m_List.GetItemData (m_nContextItem);
  if(IDYES==CUtils::MessageBoxF(_T("Are you sure you wish to enable this conflict?"))){
    conflict->enable();
  }
}

void CRulesView::OnLocate()
{
  CConfigItem *pItem=m_List.AssociatedItem(m_nContextItem,m_nContextRow);
  if (pItem) {
    CConfigTool::GetControlView()->SelectItem(pItem);
  }
}

void CRulesView::OnResolve()
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CPtrArray arConflictsOfInterest;
  for(POSITION pos = m_List.GetFirstSelectedItemPosition();pos;){
    int nItem = m_List.GetNextSelectedItem(pos);
    arConflictsOfInterest.Add((void *)m_List.GetItemData(nItem));
  }
  pDoc->ResolveGlobalConflicts(&arConflictsOfInterest);
}

BOOL CRulesView::OnHelpInfo(HELPINFO*) 
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_RULES_VIEW_HELP));	
}
