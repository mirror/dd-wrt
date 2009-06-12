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
// DescView.cpp : implementation file
//

#include "stdafx.h"
#include "DescView.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ControlView.h"
#include "ConfigToolDoc.h"
#include "CTUtils.h"
#include "resource.h"

#ifndef PLUGIN
#include "MainFrm.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDescView

IMPLEMENT_DYNCREATE(CDescView, CScrollView)

CDescView::CDescView():
m_pti(NULL)
{
    CConfigTool::SetDescView(this);
}

CDescView::~CDescView()
{
    CConfigTool::SetDescView(0);
}


BEGIN_MESSAGE_MAP(CDescView, CScrollView)
	//{{AFX_MSG_MAP(CDescView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDescView drawing

void CDescView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
    CControlView *pControlView=CConfigTool::GetControlView();
    if(pControlView){
        HTREEITEM hItem = pControlView->GetSelectedItem();
        if(NULL!=hItem){
            OnUpdate(this,(LPARAM)CConfigToolDoc::SelChanged,(CObject *)&(pControlView->TI(hItem)));
        }
    }
	// TODO: calculate the total size of this view
	SIZE sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CDescView::OnDraw(CDC* pDC)
{
	if(m_pti){
		CRect rcClient;
		GetClientRect(rcClient);
		rcClient.bottom=m_nTotalHeight;
        #ifdef PLUGIN
        CFont *pOldFont=pDC->SelectObject(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
        #else
		CMainFrame *pMain=(CMainFrame *)AfxGetMainWnd();
		CFont *pOldFont=pDC->SelectObject(&pMain->GetPaneFont(CMainFrame::ShortDesc));
        #endif
		pDC->SetBkColor(GetSysColor(COLOR_INFOBK));
        pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(m_pti->Desc(),rcClient,DT_NOPREFIX|DT_WORDBREAK);
		pDC->SelectObject(pOldFont);
    }
}
/////////////////////////////////////////////////////////////////////////////
// CDescView diagnostics

#ifdef _DEBUG
void CDescView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CDescView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDescView message handlers

void CDescView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
	// For all known hints we redraw everything
	// The Selchanged hint tells us what to redraw
	switch(lHint){
		case CConfigToolDoc::SelChanged:
			if(pDoc->ItemCount()>0){
				m_pti=(CConfigItem *)pHint;
			} else {
				// In process of destroying configitems - no nothing
				m_pti=NULL;
			}
			break;
		case CConfigToolDoc::Clear:
			m_pti=NULL;
			break;
		default:
			return;
	}
	SetScrollInfo();
	Invalidate();

	UNUSED_ALWAYS(pSender);
}

BOOL CDescView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcClient;
	pDC->GetClipBox(rcClient);
	pDC->FillSolidRect(rcClient,GetSysColor(COLOR_INFOBK));
		
	return TRUE;//return CScrollView::OnEraseBkgnd(pDC);
}


void CDescView::SetScrollInfo()
{
	CSize sizePage,sizeLine,sizeTotal;
	if(m_pti){
		CDC *pDC=GetDC();
		CRect rcClient;
		GetClientRect(rcClient);
        #ifdef PLUGIN
        CFont *pOldFont=pDC->SelectObject(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
        #else
		CMainFrame *pMain=(CMainFrame *)AfxGetMainWnd();
		CFont *pOldFont=pDC->SelectObject(&pMain->GetPaneFont(CMainFrame::ShortDesc));
        #endif
		//pDC->DrawText(m_pti->Desc(),rcClient,DT_NOPREFIX|DT_WORDBREAK);
		sizeTotal.cx=0;
		m_nTotalHeight=sizeTotal.cy=pDC->DrawText(m_pti->Desc(),rcClient,DT_NOPREFIX|DT_CALCRECT|DT_WORDBREAK);
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		sizeLine.cx=0;
		sizeLine.cy=tm.tmHeight;
		sizePage.cx=0;
		sizePage.cy=sizeTotal.cy-sizeLine.cy;
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
	} else {
		sizeTotal=CSize(0,0);
		sizePage=sizeDefault;
		sizeLine=sizeDefault;
	}
	SetScrollSizes(MM_TEXT,sizeTotal,sizePage,sizeLine);

}

void CDescView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	
	SetScrollInfo();	
}

int CDescView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);	
	return 0;
}


      
BOOL CDescView::OnHelpInfo(HELPINFO*) 
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_DESC_VIEW_HELP));	
}
