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
// TestViewView.cpp : implementation of the CPropertiesView class
//

#include "stdafx.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "ConfigItem.h"
#include "ControlView.h"
#include "CTUtils.h"
#include "PropertiesList.h"
#include "PropertiesView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertiesView

IMPLEMENT_DYNCREATE(CPropertiesView, CView)

BEGIN_MESSAGE_MAP(CPropertiesView, CView)
	//{{AFX_MSG_MAP(CPropertiesView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesView construction/destruction

CPropertiesView::CPropertiesView()
{
  CConfigTool::SetPropertiesView(this);
}

CPropertiesView::~CPropertiesView()
{
  CConfigTool::SetPropertiesView(0);
}

BOOL CPropertiesView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPropertiesView drawing

void CPropertiesView::OnDraw(CDC* pDC)
{
  UNUSED_ALWAYS(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CPropertiesView diagnostics

#ifdef _DEBUG
void CPropertiesView::AssertValid() const
{
	CView::AssertValid();
}

void CPropertiesView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertiesView message handlers

int CPropertiesView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRect rect;
  GetClientRect(rect);
  m_List.Create(WS_CHILD|WS_VISIBLE,rect,this,1);
	return 0;
}

void CPropertiesView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	CRect rect;
  GetClientRect(rect);
  m_List.MoveWindow(rect);
}

void CPropertiesView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
  CControlView *pControlView=CConfigTool::GetControlView();
  if(pControlView){
    HTREEITEM hItem = pControlView->GetSelectedItem();
    if(NULL!=hItem){
      m_List.Fill(&(pControlView->TI(hItem)));
    }
  }
}

void CPropertiesView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* pHint) 
{
  CConfigItem *pti=(CConfigItem *)pHint;

	// For all known hints we redraw everything
	// The Selchanged hint tells us what to redraw
	CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
	switch(lHint){
		case CConfigToolDoc::SelChanged:
			if(pDoc->ItemCount()>0){
				m_List.Fill(pti);
			} else {
				// In process of destroying configitems - no nothing
				m_List.DeleteAllItems();
			}
			break;
		case CConfigToolDoc::Clear:
			m_List.Fill(NULL);
			break;
		case CConfigToolDoc::IntFormatChanged:
			if(pti && pti->Type()==CConfigItem::Integer){
				m_List.SetItem(CPropertiesList::Value,pti->StringValue());
				m_List.SetItem(CPropertiesList::DefaultValue,CUtils::IntToStr(pti->DefaultValue(),CConfigTool::GetConfigToolDoc()->m_bHex));
			}
			break;
		case CConfigToolDoc::AllSaved:
			if(pti){
				m_List.Fill(pti); // lazy update of default value
			}
			break;
		case CConfigToolDoc::ValueChanged:
			if(pti){
        m_List.RefreshValue();
			}
			break;
		default:
			return;
	}
	
}

BOOL CPropertiesView::OnHelpInfo(HELPINFO*) 
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_PROPERTIES_VIEW_HELP));	
}
